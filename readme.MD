# ABOUT

HTTP Server written in C

### Features:
1. Processing GET requests to serve HTML, CSS, JS and Txt files
2. Serving JPG and PNG images
3. Processing POST requests with Content-Type multipart/form-data (text based parts only, no support for file uploading)
4. Processing POST requests with Content-Type text/plain (can be tested via curl)
5. Handles multiple concurrent connections
6. Uploaded data gets written (appended) to a txt file. The file is created if it doesn't exist
7. Rudimentary routing which allows adding new static files without modifying the source code
8. Dockerfile to create a lightweight container to run the server using alpine image
9. Contains script to run the program with Valgrind to check for memory leaks (Valgrind is not included in the container)
10. Contains scripts to run concurrent requests with Apache Benchmark (Apache Benchmark not included in the container)
11. Contains script to make a post request with text/plain content type with curl

## RUN IN LINUX:

### 1. COMPILE:
```
make
```

### 2. RUN:
```
./server {port number}
```
***
## RUN WITH DOCKER:

### 1. BUILD:
```
docker build -t chttpserver .
```

### 2. RUN:
```
docker run -d -p 8080:8080 --name chttpserver chttpserver
```
***
## CHECK FOR MEMORY LEAKS:

```
./scripts/runWithValgrind.sh
```