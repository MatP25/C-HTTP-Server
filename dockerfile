# use alpine as base image
FROM alpine AS build-env
# install build-base meta package inside build-env container
RUN apk add --no-cache build-base
# change directory to /app
WORKDIR /app
# copy all files from current directory inside the build-env container
COPY . .
# Compile the source code and generate hello binary executable file
RUN make
# use another container to run the program
FROM alpine

ENV PORT=8080

# copy binary executable to new container
COPY --from=build-env /app/server /app/server
WORKDIR /app
# at last run the program
EXPOSE 8080

CMD ["/app/server", "8080"] 