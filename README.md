# GopherWatch: Distributed Telemetry System

A cross-language monitoring system designed to simulate industrial metrics collection. This project demonstrates how high-performance C++ agents can stream structured telemetry to a Go-based central collector.

## 🏗 Architecture (The "Collector" Model)

*   **Collector (Go):** A concurrent TCP server. It handles multiple incoming agent connections, manages an in-memory node registry, and solves the "TCP Streaming Problem" using a length-prefixed protocol.
*   **Agent (C++):** A high-perf client that simulates hardware sensors (CPU, Memory, Disk) with realistic random-walk drift. It handles networking retries and cross-platform socket logic.
*   **Protocol (Protobuf + Framing):**
    *   **Schema:** `metrics.proto` defines the data model.
    *   **Framing:** Each message is prefixed with a 4-byte Big-Endian length header. This is a critical networking pattern for reliable streaming.

## 🛠 Tech Stack

*   **Languages:** Go 1.22+, C++17
*   **Communication:** Raw TCP Sockets, Protocol Buffers (proto3)
*   **Key Patterns:** Goroutines, Length-Prefixed Framing, RAII, Mutexes, Service Discovery (localhost).

## 🚀 Getting Started

### 1. Protobuf Generation (If you change .proto)

```bash
# Generate Go code
protoc --go_out=. --go_opt=paths=source_relative proto/metrics.proto

# C++ code is generated automatically by CMake during the build process
```

### 2. The Go Collector (Start this FIRST)

```bash
# From root
go mod tidy
go run main.go
```

### 3. The C++ Agent

```bash
cd cpp-backend
mkdir build && cd build
cmake ..
# On Windows: cmake --build . --config Release
# On Linux: make
./cstore (or cstore.exe)
```

