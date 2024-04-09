#!/usr/bin/env python3
import os

# Get the query string from environment variables
query_string = os.environ.get('QUERY_STRING', '')

# Parse the query string to get the form data
form_data = {}
for item in query_string.split('&'):
    key_value = item.split('=')
    if len(key_value) == 2:
        key, value = key_value
        form_data[key] = value
    elif len(key_value) == 1:
        key = key_value[0]
        form_data[key] = ''  # Assign an empty string if no value is provided

# Retrieve form data
prompt = form_data.get('prompt')
prompt_style = form_data.get('prompt_style')
style = form_data.get('style')
seed = form_data.get('seed')
sketch_guidance = form_data.get('sketch_guidance')

# Print the form data
print("<h2>Form Data:</h2>")
print("<p><strong>Prompt:</strong> </p>", prompt)
print("<p><strong>Prompt Style:</strong> </p>", prompt_style)
print("<p><strong>Style:</strong> </p>", style)
print("<p><strong>Seed:</strong> </p>", seed)
print("<p><strong>Sketch Guidance:</strong> </p>", sketch_guidance)