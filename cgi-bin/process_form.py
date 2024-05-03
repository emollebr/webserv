#!/usr/bin/env python3
from gradio_client import Client
import os
import cgi
import sys
from gradio_client import Client

form = cgi.FieldStorage()

input_text = form.getvalue('input')


sys.stdout = open(os.devnull, 'w')

client = Client("ysharma/Chat_with_Meta_llama3_8b")

sys.stdout = sys.__stdout__

i = 0

while True:
    i = 0
 
result = client.predict(
	message=str(input_text),	# str  in 'Input' Textbox component
	request=0.95,
	param_3=512,
	api_name="/chat"
)
print(result)
