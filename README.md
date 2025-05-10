# Concurrent Web Server

A high-performance HTTP/1.0 web server implementation in C using thread-per-request concurrency.

## Features
- Thread-per-request architecture with POSIX threads
- Static file serving with MIME type detection
- HTTP/1.0 protocol support
- GET request handling with proper status codes
- Path traversal protection
- Thread-safe operations

## Architecture
```
Client Request to Main Thread to Worker Thread to File System to Response
```

### Components
1. **Main Thread**: Server initialization, connection acceptance
2. **Worker Threads**: Request processing, file handling, response generation

## Implementation
- **Request Flow**: Socket creation → Request parsing → File operations → Response
- **Error Handling**: HTTP status codes (200, 404, 501) and system errors
- **Security**: Path validation, request sanitization
- **Performance**: Buffer optimization (4096 bytes), thread management

## Usage
```bash
# Build
make

# Run
./server <port> <directory>

# Test
curl http://localhost:8080/index.html
```
