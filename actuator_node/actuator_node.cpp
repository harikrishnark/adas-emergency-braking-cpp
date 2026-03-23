#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

#define TCP_PORT 5006
#define BUFFER_SIZE 256

using namespace std;

int main() {
    cout << "🛑 Actuator Node (Brakes) Booting..." << endl;
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket fail");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port 5006
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_PORT);
    
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind fail");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Listen fail");
        exit(EXIT_FAILURE);
    }
    
    cout << "🎧 Listening for ADAS Brake Commands on TCP Port " << TCP_PORT << endl;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept fail");
        exit(EXIT_FAILURE);
    }
    
    cout << "🔗 ADAS Controller Connected." << endl;

    while(true) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            string cmd(buffer);
            if (cmd.find("BRAKE_100_PERCENT") != string::npos) {
                auto now = chrono::system_clock::now();
                auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
                
                cout << "\n==============================================" << endl;
                cout << "     🔥 EMERGENCY BRAKE DEPLOYED 🔥        " << endl;
                cout << "==============================================" << endl;
                cout << "Command received: " << cmd << endl;
                cout << "Actuator applying 100% clamping force." << endl;
                cout << "ABS engaging... Vehicle is coming to a stop." << endl;
                cout << "Timestamp: " << timestamp << " ms" << endl;
                cout << "==============================================\n" << endl;
                break; // End simulation on crash/stop
            }
        }
    }

    close(new_socket);
    close(server_fd);
    
    return 0;
}
