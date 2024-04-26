#!/bin/bash

# URL to send requests to
url="http://localhost:9999/uploads/bigfile.txt"

# Send 3 requests concurrently
for i in {1..3}; do
  curl -sS "$url" -o "output$i.txt" &
done
wait

# Send 5 sequential requests
for i in {1..5}; do
  curl -sS "$url" -o "output$((i+3)).txt"
done

echo "Downloads completed."