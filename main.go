package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"

	pb "gopher-sre/proto"

	"google.golang.org/protobuf/proto"
)

// Registry stores the last known state of each reporter
type NodeRegistry struct {
	mu    sync.RWMutex
	nodes map[string]*pb.MetricReport
}

func (r *NodeRegistry) Update(report *pb.MetricReport) {
	r.mu.Lock()
	defer r.mu.Unlock()
	r.nodes[report.HostId] = report
}

func (r *NodeRegistry) PrintSummary() {
	r.mu.RLock()
	defer r.mu.RUnlock()
	fmt.Printf("\n--- 🌐 Active Nodes Summary (%d) ---\n", len(r.nodes))
	for id, report := range r.nodes {
		fmt.Printf("[%s] %s | CPU: %.1f%% | Mem: %d/%d MB\n",
			id, report.HostName, report.CpuUsagePercent,
			report.MemoryUsedBytes/1024/1024, report.MemoryTotalBytes/1024/1024)
	}
}

func main() {
	registry := &NodeRegistry{nodes: make(map[string]*pb.MetricReport)}

	listener, err := net.Listen("tcp", ":8080")
	if err != nil {
		log.Fatalf("❌ Failed to start listener: %v", err)
	}
	defer listener.Close()

	fmt.Println("🛰️  GopherWatch Collector listening on :8080")

	// Print summary every 10 seconds
	go func() {
		for {
			time.Sleep(10 * time.Second)
			registry.PrintSummary()
		}
	}()

	// Signal handling for graceful shutdown
	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)

	go func() {
		for {
			conn, err := listener.Accept()
			if err != nil {
				select {
				case <-stop:
					return
				default:
					log.Printf("⚠️ Accept error: %v", err)
					continue
				}
			}
			go handleConnection(conn, registry)
		}
	}()

	<-stop
	fmt.Println("\n🛑 Shutting down collector...")
}

func handleConnection(conn net.Conn, registry *NodeRegistry) {
	defer conn.Close()
	remoteAddr := conn.RemoteAddr().String()
	fmt.Printf("🔌 New agent connected: %s\n", remoteAddr)

	for {
		// 1. Read the length prefix (uint32, 4 bytes)
		// This is CRITICAL for network telemetry: TCP is a stream, not packet-based.
		var length uint32
		err := binary.Read(conn, binary.BigEndian, &length)
		if err != nil {
			if err != io.EOF {
				log.Printf("❌ Read error from %s: %v", remoteAddr, err)
			}
			break
		}

		// 2. Read the actual Protobuf payload based on the length
		data := make([]byte, length)
		_, err = io.ReadFull(conn, data)
		if err != nil {
			log.Printf("❌ Failed to read payload from %s: %v", remoteAddr, err)
			break
		}

		// 3. Unmarshal the report
		report := &pb.MetricReport{}
		if err := proto.Unmarshal(data, report); err != nil {
			log.Printf("❌ Unmarshal error: %v", err)
			continue
		}

		// 4. Update the registry
		registry.Update(report)
		fmt.Printf("📊 [RCVD] %s - CPU: %.1f%%\n", report.HostName, report.CpuUsagePercent)
	}

	fmt.Printf("🔌 Agent disconnected: %s\n", remoteAddr)
}