#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "metrics.pb.h"

// MockSensor simulates real-world hardware metrics with some noise
class MockSensor {
public:
    MockSensor(std::string id, std::string name) : id_(id), name_(name) {
        std::random_device rd;
        gen_ = std::mt19937(rd());
        cpu_dist_ = std::uniform_real_distribution<>(0.0, 10.0);
    }

    metrics::MetricReport capture() {
        metrics::MetricReport report;
        report.set_host_id(id_);
        report.set_host_name(name_);
        
        // Random walk for CPU simulation
        current_cpu_ += cpu_dist_(gen_) - 5.0;
        if (current_cpu_ < 0) current_cpu_ = 0;
        if (current_cpu_ > 100) current_cpu_ = 100;

        report.set_cpu_usage_percent(current_cpu_);
        report.set_memory_used_bytes(4ULL * 1024 * 1024 * 1024 + (uint64_t)(current_cpu_ * 10000000));
        report.set_memory_total_bytes(16ULL * 1024 * 1024 * 1024);
        report.set_disk_utilization(45.2);
        report.set_timestamp_unix(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        
        auto& labels = *report.mutable_labels();
        labels["os"] = "cpp-linux";
        labels["version"] = "1.0.0";

        return report;
    }

private:
    std::string id_, name_;
    double current_cpu_ = 20.0;
    std::mt19937 gen_;
    std::uniform_real_distribution<> cpu_dist_;
};

int main() {


    MockSensor sensor("agent-01-cpp", "Dublin-Edge-Node");

    while (true) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(8080);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        std::cout << "📡 Attempting to connect to GopherWatch Collector..." << std::endl;
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "⚠️  Collector unreachable. Retrying in 5s..." << std::endl;
            close(sock);
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        std::cout << "✅ Connected. Sending telemetry pulses..." << std::endl;

        while (true) {
            metrics::MetricReport report = sensor.capture();
            std::string serialized;
            report.SerializeToString(&serialized);

            // 1. Send Length Prefix (4 bytes, Big Endian)
            // This is key for the Go collector to know how many bytes to read
            uint32_t size = htonl(static_cast<uint32_t>(serialized.size()));
            if (send(sock, reinterpret_cast<const char*>(&size), 4, 0) < 0) break;

            // 2. Send Actual Data
            if (send(sock, serialized.c_str(), serialized.size(), 0) < 0) break;

            std::cout << "📊 [SENT] CPU: " << report.cpu_usage_percent() << "%" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        std::cout << "❌ Connection lost. Reconnecting..." << std::endl;
        close(sock);
    }


    return 0;   
}