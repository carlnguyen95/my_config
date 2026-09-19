#!/usr/bin/python

import json
import os

import requests

MODEL = "gemma4"
URL = "http://localhost:11434/api/chat"
USE_GEMINI = os.getenv("USE_GEMINI") is not None

SYSTEM_PROMPT = """You are a helpful Python tutor. Your job is done when the user
finishes 3 questions: return status 'done'. Otherwise return status 'continue'."""
RESPONSE_SCHEMA = {
    "type": "object",
    "properties": {
        "example": {"type": "string"},
        "explanation": {"type": "string"},
        "status": {"type": "string"},
    },
    "required": ["example", "explanation", "status"],
}


if USE_GEMINI:
    # Import only for the Gemini path so local Ollama does not need google-genai.
    from google import genai
    from google.genai import types

    with open("/home/nconghuan/.env", "r") as fd:
        my_api_key = json.load(fd)["API_KEY"]

    client = genai.Client(api_key=my_api_key)
    chat = client.chats.create(
        model="gemini-3.6-flash",
        config=types.GenerateContentConfig(
            system_instruction=SYSTEM_PROMPT,
            response_mime_type="application/json",
            response_schema=RESPONSE_SCHEMA,
        ),
    )
    print("Using Gemini")
else:
    messages = [{"role": "system", "content": SYSTEM_PROMPT}]
    print(f"Using local Ollama model: {MODEL}")


print("Input 'exit' to quit the program")
while True:
    user_input = input(">> ")
    if user_input.lower() == "exit":
        break

    if USE_GEMINI:
        response = chat.send_message(user_input)
        ass_msg = json.loads(response.text)
    else:
        messages.append({"role": "user", "content": user_input})
        response = requests.post(
            URL,
            json={
                "model": MODEL,
                "messages": messages,
                "format": "json",
                "stream": False,
            },
        )
        response.raise_for_status()
        ass_msg = json.loads(response.json()["message"]["content"])
        messages.append({"role": "assistant", "content": json.dumps(ass_msg)})

    print("AI:", ass_msg)
    if ass_msg["status"].lower() == "done":
        print("DONE")
        break

if USE_GEMINI:
    client.close()
