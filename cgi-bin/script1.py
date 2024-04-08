#!/usr/bin/env python3
import cgi
import os

# Directory to store uploaded images
UPLOAD_DIR = '/uploads'  # Replace '/path/to/uploads' with the actual directory path

# Print the Content-Type header to indicate that this script outputs HTML
print("Content-Type: text/html\n")

# Create a FieldStorage instance to parse the form data


# Output the HTML content (including the form)
print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sketch to Image Conversion</title>
</head>
<body>
    <h1>Sketch to Image Conversion</h1>
    <form action="process_form.py" method="get">
        <label for="image">Image File:</label>
        <input type="file" name="image"><br><br>

        <label for="prompt">Prompt:</label>
        <input type="text" name="prompt"><br><br>

        <label for="prompt_style">Prompt Style Template:</label>
        <input type="text" name="prompt_style"><br><br>

        <label for="style">Style:</label>
        <select name="style">
            <option value="No Style">No Style</option>
            <option value="Cinematic">Cinematic</option>
            <!-- Add other style options here -->
        </select><br><br>

        <label for="seed">Seed:</label>
        <input type="text" name="seed"><br><br>

        <label for="sketch_guidance">Sketch Guidance:</label>
        <input type="range" name="sketch_guidance" min="0" max="1" step="0.1" value="0"><br><br>

        <input type="submit" value="Convert">
    </form>
</body>
</html>""")

# Check if the form has been submitted
 if 'image' in form:
    # Retrieve the uploaded file
    uploaded_file = form['image']
    
    # Save the uploaded file to the uploads directory
    if uploaded_file.filename:
        filepath = os.path.join(UPLOAD_DIR, uploaded_file.filename)
        with open(filepath, 'wb') as f:
            f.write(uploaded_file.file.read())
        print("<p>Image saved successfully: {}</p>".format(filepath))
    else:
        print("<p>Error: No file selected.</p>") 
