#!/usr/bin/python

import  requests

url = "http://localhost:11434/api/chat"

messages = [
    {
        "role": "system",
        "content": "You are a helpful Python tutor"
    },
    {
        "role": "user",
        "content": "Give a Python list example"
    }
]

response = requests.post(
    url,
    json={
        "model": "gemma4:latest",
        "messages": messages,
        "stream": False
    }
)

data = response.json()

print(data)
