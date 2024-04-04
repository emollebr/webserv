#!/usr/bin/env python3
import cgi

# Print the Content-Type header to indicate that this script outputs HTML
print("Content-Type: text/html\n")

# Create a FieldStorage instance to parse the form data
form = cgi.FieldStorage()

# Check if the 'image' field exists in the form data
if 'image' in form.keys():
    # Retrieve form data
    image_file = form['image'].filename
    prompt = form.getvalue('prompt')
    prompt_style = form.getvalue('prompt_style')
    style = form.getvalue('style')
    seed = form.getvalue('seed')
    sketch_guidance = form.getvalue('sketch_guidance')

    # Print the form data
    print("<h2>Form Data:</h2>")
    print("<p><strong>Image File:</strong> {}</p>".format(image_file))
    print("<p><strong>Prompt:</strong> {}</p>".format(prompt))
    print("<p><strong>Prompt Style:</strong> {}</p>".format(prompt_style))
    print("<p><strong>Style:</strong> {}</p>".format(style))
    print("<p><strong>Seed:</strong> {}</p>".format(seed))
    print("<p><strong>Sketch Guidance:</strong> {}</p>".format(sketch_guidance))
else:
    print("<p>Error: 'image' field not found in form data.</p>")
