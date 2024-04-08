#!/usr/bin/env python3
from gradio_client import Client
import os
import cgi
import sys
from gradio_client import Client

form = cgi.FieldStorage()

input_text = form.getvalue('input')


sys.stdout = open(os.devnull, 'w')

client = Client("https://cohereforai-c4ai-command-r-plus.hf.space/--replicas/00ew2/")

sys.stdout = sys.__stdout__

result = client.predict(
	str(input_text),	# str  in 'Input' Textbox component
	api_name="/generate_response"
)
print(result[0][1])
