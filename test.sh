#!/bin/bash

# URL to send requests to
url1="http://localhost:9999/uploads/bigfile.txt"

# Send 3 requests concurrently
curl -Z -X GET $url1 -o output1.txt &
curl -Z -X GET $url1 -o output2.txt &
curl -Z -X GET $url1 -o output3.txt &
wait

# Send 5 requests to the URL
for i in {1..5}
do
  curl $url1 -o output4.txt
done
