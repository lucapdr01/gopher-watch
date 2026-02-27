package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"
	"time"

	// We alias our local package as 'pb' (Protocol Buffer) 
	// This is a very common pattern at Google.
	pb "gopher-sre/proto" 
	
	// This is the actual library that does the encoding/decoding
	"google.golang.org/protobuf/proto"
)

func main() {
	stop :=make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)

	fmt.Println("🚀 GopherWatch starting...")

	go func() {
		for {
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

	<-stop
	fmt.Println("🛑 Shutting down...")
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