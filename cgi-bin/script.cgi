#!/bin/bash

# Print the Content-Type header to indicate that this script outputs HTML
echo "Content-Type: text/html"
echo ""

# Output the HTML content
echo "<!DOCTYPE html>"
echo "<html>"
echo "<head>"
echo "    <title>CGI Script</title>"
echo "</head>"
echo "<body>"
echo "    <h1>Welcome to the CGI Script</h1>"
echo "    <p>This is a basic CGI script.</p>"
echo "    <p>Here, you can perform server-side processing.</p>"
echo "</body>"
echo "</html>"
