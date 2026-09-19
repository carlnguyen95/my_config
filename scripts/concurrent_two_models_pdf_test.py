#!/usr/bin/env python3
"""
Run 5 concurrent requests against TWO Ollama models using a Vietnamese history PDF.

Model assignment:
    Request 1 -> gemma4
    Request 2 -> gemma4
    Request 3 -> deepseek-coder-v2
    Request 4 -> deepseek-coder-v2
    Request 5 -> deepseek-coder-v2

Each request:
- retrieves a small relevant context from the PDF,
- sends the question + context to its assigned Ollama model,
- records token counts,
- records processing time,
- saves the result to ./log/log_1 ... ./log/log_5.

Usage:
    pip install -r requirements.txt

    python3 concurrent_two_models_pdf_test.py "/path/to/history.pdf"

Optional environment variables:
    GEMMA_MODEL=gemma4
    DEEPSEEK_MODEL=deepseek-coder-v2
    OLLAMA_URL=http://localhost:11434/api/generate
    REQUEST_TIMEOUT=300
    TOP_PAGES=2
    MAX_CONTEXT_CHARS=4000
    NUM_PREDICT=100
    KEEP_ALIVE=10m
"""

from __future__ import annotations

import os
import re
import sys
import time
import unicodedata
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from threading import Barrier

import requests
from pypdf import PdfReader


GEMMA_MODEL = os.getenv("GEMMA_MODEL", "gemma4")
DEEPSEEK_MODEL = os.getenv("DEEPSEEK_MODEL", "deepseek-coder-v2")

OLLAMA_URL = os.getenv(
    "OLLAMA_URL",
    "http://localhost:11434/api/generate",
)

REQUEST_TIMEOUT = float(os.getenv("REQUEST_TIMEOUT", "300"))
TOP_PAGES = int(os.getenv("TOP_PAGES", "2"))
MAX_CONTEXT_CHARS = int(os.getenv("MAX_CONTEXT_CHARS", "4000"))
NUM_PREDICT = int(os.getenv("NUM_PREDICT", "100"))
KEEP_ALIVE = os.getenv("KEEP_ALIVE", "10m")

DEFAULT_PDF = (
    "Lịch sử Việt Nam tập 01 Từ khởi thủy đến thế kỷ X-"
    "Cao Duy Mến-2013.pdf"
)

PDF_PATH = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(DEFAULT_PDF)

LOG_DIR = Path("./log")
LOG_DIR.mkdir(parents=True, exist_ok=True)

# Request -> model mapping is explicit here.
REQUESTS = [
    {
        "id": 1,
        "model": GEMMA_MODEL,
        "question": "Bộ sách Lịch sử Việt Nam gồm bao nhiêu tập?",
        "keywords": ["bộ sách", "15 tập", "lịch sử việt nam"],
    },
    {
        "id": 2,
        "model": GEMMA_MODEL,
        "question": "Tập 1 trình bày lịch sử Việt Nam trong giai đoạn nào?",
        "keywords": ["tập 1", "khởi thủy", "thế kỷ x"],
    },
    {
        "id": 3,
        "model": DEEPSEEK_MODEL,
        "question": "Ai là chủ biên của Tập 1?",
        "keywords": ["vũ duy mền", "chủ biên", "tập 1"],
    },
    {
        "id": 4,
        "model": DEEPSEEK_MODEL,
        "question": (
            "Cuốn Lịch sử Việt Nam từ khởi thủy đến thế kỷ X "
            "được biên soạn trong giai đoạn nào và có bao nhiêu chương?"
        ),
        "keywords": ["2007", "2011", "9 chương", "435"],
    },
    {
        "id": 5,
        "model": DEEPSEEK_MODEL,
        "question": (
            "Ba trung tâm văn hóa dẫn đến sự hình thành các nhà nước "
            "sơ khai được nêu trong sách là những trung tâm nào?"
        ),
        "keywords": [
            "đông sơn",
            "sa huỳnh",
            "óc eo",
            "văn lang",
            "phù nam",
        ],
    },
]

# All five HTTP calls will be released at approximately the same time.
START_BARRIER = Barrier(len(REQUESTS))


def normalize(text: str) -> str:
    text = text.lower()
    text = unicodedata.normalize("NFD", text)
    text = "".join(
        ch for ch in text
        if unicodedata.category(ch) != "Mn"
    )
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
            print(
                f"[WARN] Cannot extract page {i}: {exc}",
                file=sys.stderr,
            )
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
    Lightweight retrieval:
    rank PDF pages by keyword/lexical overlap and return only a few pages.
    """
    norm_keywords = [normalize(k) for k in keywords]

    question_words = {
        word
        for word in re.findall(
            r"[a-zA-Z0-9_]+",
            normalize(question),
        )
        if len(word) >= 4
    }

    scored: list[tuple[float, int, str]] = []

    for idx, page_text in enumerate(pages):
        if not page_text.strip():
            continue

        ntext = normalize(page_text)
        score = 0.0

        for keyword in norm_keywords:
            count = ntext.count(keyword)

            if count:
                score += count * (
                    4.0 + min(len(keyword) / 10.0, 3.0)
                )

        for word in question_words:
            if word in ntext:
                score += 0.25

        if score > 0:
            scored.append((score, idx, page_text))

    scored.sort(key=lambda item: item[0], reverse=True)
    selected = scored[:top_pages]

    if not selected:
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

        context_parts.append(
            f"\n--- PDF PAGE {page_no} ---\n"
            f"{text.strip()}\n"
        )

    context = "".join(context_parts)

    if len(context) > MAX_CONTEXT_CHARS:
        context = (
            context[:MAX_CONTEXT_CHARS]
            + "\n[CONTEXT TRUNCATED]\n"
        )

    return context, page_numbers


def make_prompt(question: str, context: str) -> str:
    return f"""Bạn đang trả lời câu hỏi dựa CHỈ trên nội dung trích từ tài liệu Lịch sử Việt Nam bên dưới.

Yêu cầu:
- Trả lời ngắn gọn và chính xác.
- Không dùng kiến thức ngoài phần CONTEXT.
- Nếu CONTEXT không đủ thông tin, trả lời:
  "Không đủ thông tin trong phần trích được cung cấp."
- Nếu có thể, nêu số trang PDF từ marker PDF PAGE.

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


def fmt(value, digits: int = 3) -> str:
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
prompt_tokens: {fmt(result['prompt_tokens'])}
completion_tokens: {fmt(result['completion_tokens'])}
total_tokens: {fmt(result['total_tokens'])}

client_elapsed_seconds: {fmt(result['client_elapsed_seconds'])}
ollama_total_seconds: {fmt(result['ollama_total_seconds'])}
ollama_load_seconds: {fmt(result['ollama_load_seconds'])}
ollama_prompt_eval_seconds: {fmt(result['ollama_prompt_eval_seconds'])}
ollama_eval_seconds: {fmt(result['ollama_eval_seconds'])}
generation_tokens_per_second: {fmt(result['tokens_per_second'])}

ERROR: {result['error'] or 'None'}
"""

    log_path.write_text(content, encoding="utf-8")


def call_model(prepared: dict) -> dict:
    request_id = prepared["id"]
    model = prepared["model"]
    question = prepared["question"]
    context = prepared["context"]
    source_pages = prepared["source_pages"]

    prompt = make_prompt(question, context)

    payload = {
        "model": model,
        "prompt": prompt,
        "stream": False,
        "keep_alive": KEEP_ALIVE,
        "options": {
            "temperature": 0.1,
            "num_predict": NUM_PREDICT,
        },
    }

    # All 5 worker threads wait here before issuing their POST request.
    START_BARRIER.wait()

    start = time.perf_counter()

    try:
        response = requests.post(
            OLLAMA_URL,
            json=payload,
            timeout=REQUEST_TIMEOUT,
        )

        response.raise_for_status()
        data = response.json()
        error = None

    except Exception as exc:
        elapsed = time.perf_counter() - start

        result = {
            "request_id": request_id,
            "question": question,
            "source_pages": source_pages,
            "model": model,
            "answer": "",
            "error": f"{type(exc).__name__}: {exc}",
            "prompt_tokens": None,
            "completion_tokens": None,
            "total_tokens": None,
            "client_elapsed_seconds": elapsed,
            "ollama_total_seconds": None,
            "ollama_load_seconds": None,
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

    total_seconds = ns_to_seconds(
        data.get("total_duration")
    )

    load_seconds = ns_to_seconds(
        data.get("load_duration")
    )

    prompt_eval_seconds = ns_to_seconds(
        data.get("prompt_eval_duration")
    )

    eval_seconds = ns_to_seconds(
        data.get("eval_duration")
    )

    tokens_per_second = None

    if eval_seconds and eval_seconds > 0:
        tokens_per_second = (
            completion_tokens / eval_seconds
        )

    result = {
        "request_id": request_id,
        "question": question,
        "source_pages": source_pages,
        "model": data.get("model", model),
        "answer": data.get("response", "").strip(),
        "error": error,
        "prompt_tokens": prompt_tokens,
        "completion_tokens": completion_tokens,
        "total_tokens": total_tokens,
        "client_elapsed_seconds": elapsed,
        "ollama_total_seconds": total_seconds,
        "ollama_load_seconds": load_seconds,
        "ollama_prompt_eval_seconds": prompt_eval_seconds,
        "ollama_eval_seconds": eval_seconds,
        "tokens_per_second": tokens_per_second,
    }

    write_log(result)
    return result


def print_model_summary(
    results: list[dict],
    model_name: str,
) -> None:
    model_results = [
        result
        for result in results
        if result["model"] == model_name
        and not result["error"]
    ]

    if not model_results:
        print(f"\nMODEL: {model_name}")
        print("  successful_requests : 0")
        return

    total_tokens = sum(
        result["total_tokens"] or 0
        for result in model_results
    )

    latencies = [
        result["client_elapsed_seconds"]
        for result in model_results
    ]

    avg_latency = sum(latencies) / len(latencies)
    max_latency = max(latencies)

    print(f"\nMODEL: {model_name}")
    print(
        f"  successful_requests : {len(model_results)}"
    )
    print(f"  aggregate_tokens    : {total_tokens}")
    print(
        f"  average_latency     : "
        f"{avg_latency:.3f} seconds"
    )
    print(
        f"  max_latency         : "
        f"{max_latency:.3f} seconds"
    )


def main() -> int:
    print(f"[INFO] Gemma model    : {GEMMA_MODEL}")
    print(f"[INFO] DeepSeek model : {DEEPSEEK_MODEL}")
    print(f"[INFO] Ollama URL     : {OLLAMA_URL}")
    print(f"[INFO] PDF            : {PDF_PATH}")
    print(f"[INFO] Logs           : {LOG_DIR.resolve()}")

    print("\n[INFO] Request assignment:")
    for req in REQUESTS:
        print(
            f"  request {req['id']} -> "
            f"{req['model']}"
        )

    try:
        pages = extract_pdf_pages(PDF_PATH)

    except Exception as exc:
        print(
            f"[ERROR] PDF extraction failed: {exc}",
            file=sys.stderr,
        )
        return 1

    # Precompute retrieval BEFORE starting the concurrency timer/barrier.
    # Therefore measured request latency mainly reflects Ollama/model serving,
    # not PDF extraction/retrieval.
    prepared_requests = []

    for req in REQUESTS:
        context, source_pages = retrieve_context(
            pages=pages,
            question=req["question"],
            keywords=req["keywords"],
        )

        prepared_requests.append({
            **req,
            "context": context,
            "source_pages": source_pages,
        })

    print("\n[INFO] Retrieval finished.")
    print("[INFO] Starting 5 concurrent model requests...\n")

    batch_start = time.perf_counter()
    results: list[dict] = []

    with ThreadPoolExecutor(max_workers=5) as executor:
        futures = [
            executor.submit(
                call_model,
                prepared,
            )
            for prepared in prepared_requests
        ]

        for future in as_completed(futures):
            result = future.result()
            results.append(result)

            status = (
                "ERROR"
                if result["error"]
                else "OK"
            )

            print(
                f"[{status}] "
                f"request={result['request_id']} | "
                f"model={result['model']} | "
                f"tokens={fmt(result['total_tokens'])} | "
                f"wall={fmt(result['client_elapsed_seconds'])}s | "
                f"log=./log/log_{result['request_id']}"
            )

    batch_elapsed = time.perf_counter() - batch_start

    print("\n=== BATCH SUMMARY ===")
    print(
        f"5 concurrent requests completed in "
        f"{batch_elapsed:.3f} seconds"
    )

    successful = [
        result
        for result in results
        if not result["error"]
    ]

    print(
        f"successful_requests : "
        f"{len(successful)}/5"
    )

    if successful:
        total_tokens = sum(
            result["total_tokens"] or 0
            for result in successful
        )

        latencies = [
            result["client_elapsed_seconds"]
            for result in successful
        ]

        print(
            f"aggregate_tokens    : "
            f"{total_tokens}"
        )
        print(
            f"average_latency     : "
            f"{sum(latencies) / len(latencies):.3f} seconds"
        )
        print(
            f"max_latency         : "
            f"{max(latencies):.3f} seconds"
        )

    # Summaries by configured model.
    print_model_summary(
        results,
        GEMMA_MODEL,
    )

    print_model_summary(
        results,
        DEEPSEEK_MODEL,
    )

    return 0 if len(successful) == 5 else 2


if __name__ == "__main__":
    raise SystemExit(main())
