#!/usr/bin/python

from google import genai
from google.genai import types
import json
import requests

MODEL = "gemma4"
URL = "http://localhost:11434/api/chat"

with open("/home/nconghuan/.env", "r") as fd:
    data = fd.read()
    data = json.loads(data)
    my_api_key = data["API_KEY"]

client = genai.Client(api_key=my_api_key)

messages = [
    {
        "role": "system",
        "parts": [{"You are a helpful Python tutor, \
                   your job is done when user finish 3 question and you will return status 'done', \
                   otherwise return status 'continue'"}]
    }
]

print("Input 'exit' to quit the program")
while True:
    user_input = input(">> ")

    if user_input.lower() == "exit":
        break

    messages.append({
        "role": "user",
        "parts": [{user_input}]
    })

    resp = client.models.generate_content(
        model="gemini-3.6-flash",
        contents=str(messages),
        config=types.GenerateContentConfig(
            response_mime_type="application/json",
            response_schema={
                "type": "object",
                "properties": {
                    "example": {"type": "string"},
                    "explanation": {"type": "string"},
                    "status": {"type": "string"}
                },
                "required": ["example", "explanation", "status"],
            },
        ),
    )

    ass_msg = json.loads(resp.text)
    print("AI: ", ass_msg)

    if ass_msg["status"].lower() == "done":
        print("DONE")
        break

    messages.append({
        "role": "assistant",
        "parts": [{str(ass_msg)}]
    })

client.close()
