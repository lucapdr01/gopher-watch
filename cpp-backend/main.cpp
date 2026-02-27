package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"
	"time"

	pb "gopher-sre/proto"
	"google.golang.org/protobuf/proto"
)

func main() {
	// 1. Setup Signal Handling (The SRE way to handle restarts)
	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)

	fmt.Println("🚀 GopherWatch starting...")

	// 2. Start the background collection loop
	go func() {
		for {
			// We try to connect inside the loop (Self-healing logic)
			conn, err := net.DialTimeout("tcp", "localhost:8080", 2*time.Second)
			if err != nil {
				log.Println("⚠️ Backend unreachable, retrying in 5s...")
				time.Sleep(5 * time.Second)
				continue
			}

			fmt.Println("✅ Connected to C++ Backend.")
			sendMetrics(conn)
			conn.Close()
		}
	}()

	// Wait here until Ctrl+C
	<-stop
	fmt.Println("\n🛑 Shutdown signal received. Exiting.")
}

func sendMetrics(conn net.Conn) {
	for {
		report := &pb.MetricReport{
			HostName:   "dublin-srv-01",
			CpuUsage:   42.0, // In a real app, you'd pull this from the OS
			MemoryFree: 1024,
			Timestamp:  time.Now().Unix(),
		}

		data, err := proto.Marshal(report)
		if err != nil {
			log.Println("Marshaling error:", err)
			return
		}

		// Write data to the TCP socket
		_, err = conn.Write(data)
		if err != nil {
			log.Println("Connection lost:", err)
			return // Exit and let the main loop reconnect
		}

		fmt.Printf(" [SENT] %d bytes\n", len(data))
		time.Sleep(2 * time.Second)
	}
}