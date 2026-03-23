#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define UDP_PORT 5005           // Port to receive Lidar Data
#define TCP_PORT 5006           // Port to send Brake Commands to Actuator
#define BUFFER_SIZE 1024
#define TTC_THRESHOLD 2.0       // Time-To-Collision threshold in seconds

using namespace std;

// Quick helper to split a string by comma
vector<string> split(const string &s, char delim) {
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

int main() {
    cout << "🧠 ADAS Controller Node Booting..." << endl;

    // --- 1. SETUP UDP LISTENER FOR LIDAR DATA ---
    int udp_sock;
    struct sockaddr_in udp_addr, client_addr;
    char buffer[BUFFER_SIZE];

    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket creation failed");
        return 1;
    }

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (::bind(udp_sock, (const struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("UDP Bind failed");
        return 1;
    }
    cout << "📡 Listening for Lidar Data on UDP Port " << UDP_PORT << endl;

    // --- 2. SETUP TCP CLIENT FOR ACTUATOR NODE ---
    int tcp_sock;
    struct sockaddr_in serv_addr;

    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "TCP Socket creation error" << endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    
    // Connect to Actuator Node (We expect it to be running already or we keep trying)
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        cout << "Invalid address/ Address not supported" << endl;
        return 1;
    }
    
    // Non-blocking connection approach omitted for brevity. 
    // We assume the actuator node is booted FIRST for simplicity.
    if (connect(tcp_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "⚠️ WARNING: Actuator Node offline. Brake commands will not be sent." << endl;
    } else {
        cout << "🔗 Connected to Actuator Node on TCP Port " << TCP_PORT << endl;
    }

    // --- 3. MAIN CONTROL LOOP ---
    bool emergency_triggered = false;

    while (true) {
        socklen_t len = sizeof(client_addr);
        int n = recvfrom(udp_sock, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);
        buffer[n] = '\0';

        // Parse CSV Data from Python (timestamp, distance, velocity)
        string data(buffer);
        vector<string> tokens = split(data, ',');
        
        if (tokens.size() == 3) {
            double distance = stod(tokens[1]);
            double rel_velocity = stod(tokens[2]);

            // Calculate Time-To-Collision (TTC)
            // TTC is only valid if relative velocity is negative (target is closing distance)
            double ttc = 999.0; 
            if (rel_velocity < 0) {
                ttc = distance / abs(rel_velocity);
            }

            if (ttc <= TTC_THRESHOLD && !emergency_triggered && distance > 0) {
                cout << "\n🚨 CRITICAL DANGER: TTC = " << ttc << "s. COLLISION IMMINENT!" << endl;
                cout << "⚡ INITIATING AUTOMATIC EMERGENCY BRAKING (AEB)..." << endl;
                
                // Send brake command to the actuator
                string brake_cmd = "BRAKE_100_PERCENT";
                send(tcp_sock, brake_cmd.c_str(), brake_cmd.length(), 0);
                emergency_triggered = true;
            } 
            else if (distance <= 0) {
                cout << "💥 Collision Detected." << endl;
                break;
            }
            else if (!emergency_triggered) {
                // Normal output
                // cout << "Cruising. Distance: " << distance << "m | TTC: " << ttc << "s" << endl;
            }
        }
    }

    close(udp_sock);
    close(tcp_sock);
    return 0;
}
