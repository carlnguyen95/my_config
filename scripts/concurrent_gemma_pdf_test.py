"""
Run 5 concurrent requests to a Gemma model served by Ollama.

Each request:
- retrieves a small relevant context from the supplied Vietnamese history PDF,
- sends the question + retrieved context to Gemma,
- records Ollama token counts,
- records wall-clock processing time,
- saves the result to ./log/log_1 ... ./log/log_5.

Usage:
    pip install pypdf requests

    python3 concurrent_gemma_pdf_test.py "/path/to/history.pdf"

Optional environment variables:
    OLLAMA_MODEL=gemma3:4b
    OLLAMA_URL=http://localhost:11434/api/generate
    REQUEST_TIMEOUT=300
    TOP_PAGES=3
    MAX_CONTEXT_CHARS=9000
"""

from __future__ import annotations

import json
import os
import re
import sys
import time
import unicodedata
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

import requests
from pypdf import PdfReader


MODEL = os.getenv("OLLAMA_MODEL", "gemma3:4b")
OLLAMA_URL = os.getenv("OLLAMA_URL", "http://localhost:11434/api/generate")
REQUEST_TIMEOUT = float(os.getenv("REQUEST_TIMEOUT", "300"))
TOP_PAGES = int(os.getenv("TOP_PAGES", "3"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "9000"))

DEFAULT_PDF = "Vietnam_History.pdf"
PDF_PATH = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(DEFAULT_PDF)

LOG_DIR = Path("./log")
LOG_DIR.mkdir(parents=True, exist_ok=True)

# 5 questions all target information that appears in this history volume.
REQUESTS = [
    {
        "id": 1,
        "question": "Bộ sách Lịch sử Việt Nam gồm bao nhiêu tập?",
        "keywords": ["bộ sách", "15 tập", "lịch sử việt nam"],
    },
    {
        "id": 2,
        "question": "Tập 1 trình bày lịch sử Việt Nam trong giai đoạn nào?",
        "keywords": ["tập 1", "khởi thủy", "thế kỷ x"],
    },
    {
        "id": 3,
        "question": "Ai là chủ biên của Tập 1?",
        "keywords": ["vũ duy mền", "chủ biên", "tập 1"],
    },
    {
        "id": 4,
        "question": (
            "Cuốn Lịch sử Việt Nam từ khởi thủy đến thế kỷ X được biên soạn "
            "trong giai đoạn nào và có bao nhiêu chương?"
        ),
        "keywords": ["2007", "2011", "9 chương", "435"],
    },
    {
        "id": 5,
        "question": (
            "Ba trung tâm văn hóa dẫn đến sự hình thành các nhà nước sơ khai "
            "được nêu trong sách là những trung tâm nào?"
        ),
        "keywords": ["đông sơn", "sa huỳnh", "óc eo", "văn lang", "phù nam"],
    },
]

# Synchronize the 5 HTTP requests so they start as close together as possible.
START_BARRIER = threading.Barrier(len(REQUESTS))


def normalize(text: str) -> str:
    """Normalize text for rough keyword matching, including Vietnamese accents."""
    text = text.lower()
    text = unicodedata.normalize("NFD", text)
    text = "".join(ch for ch in text if unicodedata.category(ch) != "Mn")
    text = re.sub(r"\s+", " ", text)
    return text


def extract_pdf_pages(pdf_path: Path) -> list[str]:
    if not pdf_path.exists():
        raise FileNotFoundError(f"PDF not found: {pdf_path}")

    print(f"[INFO] Reading PDF: {pdf_path}")
    reader = PdfReader(str(pdf_path))
    pages: list[str] = []

    for i, page in enumerate(reader.pages, start=1):
        try:
            text = page.extract_text() or ""
        except Exception as exc:
            print(f"[WARN] Cannot extract page {i}: {exc}", file=sys.stderr)
            text = ""
        pages.append(text)

    print(f"[INFO] Extracted {len(pages)} pages")
    return pages


def retrieve_context(
    pages: list[str],
    question: str,
    keywords: list[str],
    top_pages: int = TOP_PAGES,
) -> tuple[str, list[int]]:
    """
    Lightweight retrieval for benchmarking.
    It ranks pages by keyword/word overlap and returns only a few pages.
    """
    norm_keywords = [normalize(k) for k in keywords]
    question_words = {
        w for w in re.findall(r"[a-zA-Z0-9_]+", normalize(question))
        if len(w) >= 4
    }

    scored: list[tuple[float, int, str]] = []

    for idx, page_text in enumerate(pages):
        if not page_text.strip():
            continue

        ntext = normalize(page_text)
        score = 0.0

        # Strong weight for explicit keywords/phrases.
        for kw in norm_keywords:
            count = ntext.count(kw)
            if count:
                score += count * (4.0 + min(len(kw) / 10.0, 3.0))

        # Fallback lexical overlap with question words.
        for word in question_words:
            if word in ntext:
                score += 0.25

        if score > 0:
            scored.append((score, idx, page_text))

    scored.sort(key=lambda x: x[0], reverse=True)
    selected = scored[:top_pages]

    if not selected:
        # Fallback: first few text-bearing pages.
        selected = [
            (0.0, idx, text)
            for idx, text in enumerate(pages)
            if text.strip()
        ][:top_pages]

    context_parts: list[str] = []
    page_numbers: list[int] = []

    for _, idx, text in selected:
        page_no = idx + 1
        page_numbers.append(page_no)
        context_parts.append(f"\n--- PDF PAGE {page_no} ---\n{text.strip()}\n")

    context = "".join(context_parts)

    if len(context) > MAX_CONTEXT_CHARS:
        context = context[:MAX_CONTEXT_CHARS] + "\n[CONTEXT TRUNCATED]\n"

    return context, page_numbers


def make_prompt(question: str, context: str) -> str:
    return f"""Bạn đang trả lời câu hỏi dựa CHỈ trên nội dung trích từ tài liệu Lịch sử Việt Nam bên dưới.

Yêu cầu:
- Trả lời ngắn gọn và chính xác.
- Không dùng kiến thức ngoài phần CONTEXT.
- Nếu CONTEXT không đủ thông tin, trả lời: "Không đủ thông tin trong phần trích được cung cấp."
- Nêu số trang PDF nếu thông tin trong CONTEXT có đánh dấu trang.

CONTEXT:
{context}

QUESTION:
{question}

ANSWER:
"""


def ns_to_seconds(value) -> float | None:
    if not isinstance(value, (int, float)):
        return None
    return value / 1_000_000_000.0


def call_gemma(req: dict, pages: list[str]) -> dict:
    request_id = req["id"]
    question = req["question"]

    context, source_pages = retrieve_context(
        pages=pages,
        question=question,
        keywords=req["keywords"],
    )
    prompt = make_prompt(question, context)

    payload = {
        "model": MODEL,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.1,
        },
    }

    # Wait until all five worker threads are ready.
    START_BARRIER.wait()

    start = time.perf_counter()
    try:
        http_response = requests.post(
            OLLAMA_URL,
            json=payload,
            timeout=REQUEST_TIMEOUT,
        )
        http_response.raise_for_status()
        data = http_response.json()
        error = None
    except Exception as exc:
        elapsed = time.perf_counter() - start
        data = {}
        error = f"{type(exc).__name__}: {exc}"
        result = {
            "request_id": request_id,
            "question": question,
            "source_pages": source_pages,
            "model": MODEL,
            "answer": "",
            "error": error,
            "prompt_tokens": None,
            "completion_tokens": None,
            "total_tokens": None,
            "client_elapsed_seconds": elapsed,
            "ollama_total_seconds": None,
            "ollama_prompt_eval_seconds": None,
            "ollama_eval_seconds": None,
            "tokens_per_second": None,
        }
        write_log(result)
        return result

    elapsed = time.perf_counter() - start

    prompt_tokens = data.get("prompt_eval_count", 0)
    completion_tokens = data.get("eval_count", 0)
    total_tokens = prompt_tokens + completion_tokens

    eval_seconds = ns_to_seconds(data.get("eval_duration"))
    total_seconds = ns_to_seconds(data.get("total_duration"))
    prompt_eval_seconds = ns_to_seconds(data.get("prompt_eval_duration"))

    tokens_per_second = None
    if eval_seconds and eval_seconds > 0:
        tokens_per_second = completion_tokens / eval_seconds

    result = {
        "request_id": request_id,
        "question": question,
        "source_pages": source_pages,
        "model": data.get("model", MODEL),
        "answer": data.get("response", "").strip(),
        "error": error,
        "prompt_tokens": prompt_tokens,
        "completion_tokens": completion_tokens,
        "total_tokens": total_tokens,
        "client_elapsed_seconds": elapsed,
        "ollama_total_seconds": total_seconds,
        "ollama_prompt_eval_seconds": prompt_eval_seconds,
        "ollama_eval_seconds": eval_seconds,
        "tokens_per_second": tokens_per_second,
    }

    write_log(result)
    return result


def fmt_number(value, digits: int = 3) -> str:
    if value is None:
        return "N/A"
    if isinstance(value, float):
        return f"{value:.{digits}f}"
    return str(value)


def write_log(result: dict) -> None:
    log_path = LOG_DIR / f"log_{result['request_id']}"

    content = f"""REQUEST_ID: {result['request_id']}
MODEL: {result['model']}
QUESTION: {result['question']}
SOURCE_PDF_PAGES: {result['source_pages']}

=== RESPONSE ===
{result['answer']}

=== METRICS ===
prompt_tokens: {fmt_number(result['prompt_tokens'])}
completion_tokens: {fmt_number(result['completion_tokens'])}
total_tokens: {fmt_number(result['total_tokens'])}

client_elapsed_seconds: {fmt_number(result['client_elapsed_seconds'])}
ollama_total_seconds: {fmt_number(result['ollama_total_seconds'])}
ollama_prompt_eval_seconds: {fmt_number(result['ollama_prompt_eval_seconds'])}
ollama_eval_seconds: {fmt_number(result['ollama_eval_seconds'])}
generation_tokens_per_second: {fmt_number(result['tokens_per_second'])}

ERROR: {result['error'] or 'None'}
"""

    log_path.write_text(content, encoding="utf-8")


def main() -> int:
    print(f"[INFO] Model      : {MODEL}")
    print(f"[INFO] Ollama URL : {OLLAMA_URL}")
    print(f"[INFO] PDF        : {PDF_PATH}")
    print(f"[INFO] Logs       : {LOG_DIR.resolve()}")

    try:
        pages = extract_pdf_pages(PDF_PATH)
    except Exception as exc:
        print(f"[ERROR] PDF extraction failed: {exc}", file=sys.stderr)
        return 1

    batch_start = time.perf_counter()
    results: list[dict] = []

    # Exactly 5 concurrent model requests.
    with ThreadPoolExecutor(max_workers=5) as executor:
        futures = [executor.submit(call_gemma, req, pages) for req in REQUESTS]

        for future in as_completed(futures):
            result = future.result()
            results.append(result)
            status = "ERROR" if result["error"] else "OK"
            print(
                f"[{status}] Request {result['request_id']} | "
                f"tokens={fmt_number(result['total_tokens'])} | "
                f"wall={fmt_number(result['client_elapsed_seconds'])}s | "
                f"log=./log/log_{result['request_id']}"
            )

    batch_elapsed = time.perf_counter() - batch_start

    print("\n=== BATCH SUMMARY ===")
    print(f"5 concurrent requests completed in {batch_elapsed:.3f} seconds")

    successful = [r for r in results if not r["error"]]
    if successful:
        total_tokens = sum(r["total_tokens"] or 0 for r in successful)
        max_latency = max(r["client_elapsed_seconds"] for r in successful)
        avg_latency = sum(r["client_elapsed_seconds"] for r in successful) / len(successful)

        print(f"successful_requests : {len(successful)}/5")
        print(f"aggregate_tokens    : {total_tokens}")
        print(f"average_latency     : {avg_latency:.3f} seconds")
        print(f"max_latency         : {max_latency:.3f} seconds")
    else:
        print("successful_requests : 0/5")

    return 0 if len(successful) == 5 else 2


if __name__ == "__main__":
    raise SystemExit(main())
