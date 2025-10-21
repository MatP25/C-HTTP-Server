#!/bin/bash

echo "Posting text data...";
echo ;

curl -v http://localhost:8080/post -H 'Expect: ' -H 'Content-Type: text/plain' -d 'Example text data sent via curl POST request.'

echo ;
echo "Finished";