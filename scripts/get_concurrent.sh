#!/bin/bash

echo "Starting test concurrent sending GET requests...";
echo ;

ab -n 50 -c 5 http://localhost:8080/health

echo ;
echo "Finished";