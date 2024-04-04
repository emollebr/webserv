#!/usr/bin/env python3
import cgi

# Print the Content-Type header to indicate that this script outputs HTML
print("Content-Type: text/html\n")

# Create a FieldStorage instance to parse the form data
form = cgi.FieldStorage()

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
    <form action="process_form.py" method="post">
        <label for="image">Image File:</label>
        <input type="file" id="image" name="image"><br><br>

        <label for="prompt">Prompt:</label>
        <input type="text" id="prompt" name="prompt"><br><br>

        <label for="prompt_style">Prompt Style Template:</label>
        <input type="text" id="prompt_style" name="prompt_style"><br><br>

        <label for="style">Style:</label>
        <select id="style" name="style">
            <option value="No Style">No Style</option>
            <option value="Cinematic">Cinematic</option>
            <!-- Add other style options here -->
        </select><br><br>

        <label for="seed">Seed:</label>
        <input type="text" id="seed" name="seed"><br><br>

        <label for="sketch_guidance">Sketch Guidance:</label>
        <input type="range" id="sketch_guidance" name="sketch_guidance" min="0" max="1" step="0.1" value="0"><br><br>

        <input type="submit" value="Convert">
    </form>
</body>
</html>""")

# Check if the form has been submitted
if form.getvalue('image'):  # Check if the 'image' field is present in the form data
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


    # Execute the Python script and capture its output
    # gradio_output=$(python3 cgi-bin/script.py "$image_file" "$prompt" "$prompt_style" "$style" "$seed" "$sketch_guidance")

    # Output the Gradio output
    # echo "<h2>Gradio Output:</h2>"
    # echo "$gradio_output"

