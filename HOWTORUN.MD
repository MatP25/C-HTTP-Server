# RUN IN LINUX:

### 1. COMPILE:
```
make
```

### 2. RUN:
```
./server {port number}
```
***
# RUN WITH DOCKER:

### 1. BUILD:
```
docker build -t chttpserver .
```

### 2. RUN:
```
docker run -d -p 8080:8080 --name chttpserver chttpserver
```
***
# CHECK FOR MEMORY LEAKS:

```
./runWithValgrind.sh
```