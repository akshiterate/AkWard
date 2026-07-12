# GLaSS - General Linux and Server System

GLaSS is a lightweight HTTP web server written from scratch in C++ using POSIX sockets on Linux.

No frameworks. No networking libraries. Just sockets, the standard library and a lot of trial and error.

Live Demo: https://akshitsachdeva.me

## Features

- HTTP/1.0 web server
- Static file serving
- MIME type detection
- Live metrics dashboard
- JSON API (`/api/metrics`)
- RAM usage monitoring
- CPU usage monitoring
- Request and bandwidth statistics
- HTTP status code tracking
- Persistent metrics across restarts
- Rolling server logs
- Clean HTML/CSS frontend

## Metrics Dashboard

The dashboard polls the server every 2 seconds and displays live information including:

- Total requests
- Bytes sent
- RAM usage
- CPU usage
- HTTP response code counts
- Recent server logs

The statistics survive server restarts using a small binary data file.

## Built With

- C++17
- POSIX Sockets
- Linux `/proc` filesystem
- HTML
- CSS
- Vanilla JavaScript

## Building

```bash
g++ *.cpp -o akward
./akward
```

## Project Structure

```
.
├── webpages/
├── logs/
├── metrics.dat
├── *.cpp
├── *.hpp
└── README.md
```

## Current Limitations

- HTTP/1.0 only
- Single-threaded
- One request per connection
- No HTTPS (Cloudflare handles TLS)
- No directory listing
- Basic routing

## Planned

- epoll-based event loop
- HTTP/1.1
- Keep-Alive connections
- Better logging
- Binary file improvements
- Configuration file
- Virtual hosts

## License

MIT
