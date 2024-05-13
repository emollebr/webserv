#!/usr/bin/env python3
from gradio_client import Client
import os
import cgi
import sys
from gradio_client import Client

form = cgi.FieldStorage()

input_text = form.getvalue('input')


sys.stdout = open(os.devnull, 'w')

client = Client("Qwen/Qwen1.5-110B-Chat-demo")

sys.stdout = sys.__stdout__

while True:
	i = 0


result = client.predict(
	query=str(input_text),	# str  in 'Input' Textbox component
	history=[],
	system="You are a helpful assistant.",
	api_name="/model_chat"
)
print(result[1][0][1])