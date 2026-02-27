# GopherWatch: Distributed SRE Metrics Pipeline

A cross-language monitoring proof-of-concept demonstrating high-performance telemetry collection. This project mimics a large company internal infrastructure by using **Protocol Buffers** to bridge a **Go** telemetry agent with a **C++** metrics aggregator.

## 🏗 Architecture

* **Collector (Go):** A lightweight background agent using Goroutines and Channels to sample system metrics. It features self-healing logic to reconnect to the sink if the network drops.
* **Sink (C++):** A high-performance TCP server that deserializes binary streams into structured data using RAII for memory safety.
* **Contract (Protobuf):** A language-agnostic schema (`metrics.proto`) that ensures type-safe communication and minimal network footprint.

## 🛠 Tech Stack

* **Languages:** Go 1.22+, C++17
* **Communication:** Raw TCP Sockets, Protocol Buffers (proto3)
* **Build Systems:** Go Modules, CMake 3.10+

## 🚀 Getting Started

### 1. Prerequisites

Ensure you have the Protobuf compiler (`protoc`) installed:

* **macOS:** `brew install protobuf`
* **Linux:** `sudo apt install -y protobuf-compiler libprotobuf-dev`

### 2. The Go Agent

```bash
# Navigate to the root
go mod tidy
go run main.go

```

### 3. The C++ Backend

```bash
cd cpp-backend
mkdir build && cd build
cmake ..
make
./cstore

```

