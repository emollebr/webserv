#!/usr/bin/env python3

import sys
import cgi
from gradio_client import Client

# Parse CGI parameters
form = cgi.FieldStorage()

# Get uploaded image file path from CGI parameters
image_file = form['image'].value

# Get other form data from CGI parameters
prompt = form.getvalue('prompt', "")
prompt_style = form.getvalue('prompt_style', "")
style = form.getvalue('style', "No Style")
seed = form.getvalue('seed', "")
sketch_guidance = float(form.getvalue('sketch_guidance', 0))

# Create Gradio Client
client = Client("https://gparmar-img2img-turbo-sketch.hf.space/")

# Make prediction using Gradio Client
result = client.predict(
    image_file,
    prompt,
    prompt_style,
    style,
    seed,
    sketch_guidance,
    fn_index=11
)

# Generate HTML response with Gradio result
print("Content-type: text/html\n")
print("<html><head><title>Sketch to Image Result</title></head><body>")
print("<h1>Sketch to Image Result</h1>")
print(f"<img src='{result}' alt='Converted Image'>")
print("</body></html>")
