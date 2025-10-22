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
8. Dockerfile to create a lightweight container to run the server using Alpine image

**There are 3 script files in the scripts/ folder**
* **runWithValgrind.sh**: run the program with Valgrind to check for memory leaks (Valgrind is not included in the container)
* **get_concurrent.sh**: run concurrent GET requests sent to the /health endpoint with Apache Benchmark (Apache Benchmark is not included in the container)
* **post_data.sh**: makes a POST request with text/plain content type with curl

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

--- 

**References**
This project used different sources for reference and includes snippets of code from different github projects, some parts were modified slightly, others were left as they were in the original source. Those sources are:
* https://medium.com/@nipunweerasiri/a-simple-web-server-written-in-c-cf7445002e6
* https://www.geeksforgeeks.org/c/socket-programming-cc/
* https://github.com/nir9/welcome/blob/master/lnx/minimalist-https-web-server/server.c
* https://github.com/JeffreytheCoder/Simple-HTTP-Server
* https://bruinsslot.jp/post/simple-http-webserver-in-c/
* https://github.com/oduortoni/c-http-server
* https://dev.to/jeffreythecoder/how-i-built-a-simple-http-server-from-scratch-using-c-739
* https://github.com/nipunchamikara/c-web-server
* https://github.com/bloominstituteoftechnology/C-Web-Server
* https://github.com/mavstuff/threadtest