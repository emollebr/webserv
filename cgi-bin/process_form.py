#!/usr/bin/env python3
import os
import cgi
import sys

UPLOAD_DIR = './uploads'
form = cgi.FieldStorage()
# Get the query string from environment variables

# Retrieve form data
image = form.getvalue('image')
prompt = form.getvalue('prompt')
prompt_style = form.getvalue('prompt_style')
style = form.getvalue('style')
seed = form.getvalue('seed')
sketch_guidance = form.getvalue('sketch_guidance')


# Check if file was uploaded
if image:
    # Get file name
    filename = os.path.basename(image)
    # Check if file data exists
    if form['image'].file:
        # Save the uploaded file to the uploads directory
        filepath = os.path.join(UPLOAD_DIR, filename)
        with open(filepath, 'wb') as f:
            f.write(form['image'].file.read())
        print("Content-type: text/html\r\n\r\n")
        print("<html><body>")
        print("<p>Image saved successfully: {}</p>".format(filepath))
        print("</body></html>")
    else:
        print("Content-type: text/html\r\n\r\n")
        print("<html><body>")
        print("<p>Error: No file data received for 'image' field.</p>")
        print("</body></html>")
else:
    print("Content-type: text/html\r\n\r\n")
    print("<html><body>")
    print("<p>Error: No 'image' field found in form data.</p>")
    print("</body></html>")

# print("HTTP/1.1 200 OK")
print("Content-type: text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("<html>")
print("<head>")
print("<h2>Form Data:</h2>")
print("<p><strong>Image:</strong> </p>", image)
print("<p><strong>Prompt:</strong> </p>", prompt)
print("<p><strong>Prompt Style:</strong> </p>", prompt_style)
print("<p><strong>Style:</strong> </p>", style)
print("<p><strong>Seed:</strong> </p>", seed)
print("<p><strong>Sketch Guidance:</strong> </p>", sketch_guidance)
print("</body>")
print("</html>")