#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "metrics.pb.h"

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in  address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "📥 C++ Sink listening on 8080..." << std::endl;

    while(true){
        int new_socket = accept(server_fd, nullptr, nullptr);
        char buffer[1024] = {0};
        
        int valread = read(new_socket, buffer, 1024);

        metrics::MetricReport report;
        if (report.ParseFromArray(buffer, valread)) {
            std::cout << "📊 Received: " << report.host_name() 
                      << " | CPU: " << report.cpu_usage() << "%" << std::endl;
        }
        close(new_socket);
    }
    return 0;   
}