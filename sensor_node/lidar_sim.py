import socket
import time
import json
import random

# UDP Settings for Broadcasting Lidar Data
UDP_IP = "127.0.0.1"
UDP_PORT = 5005

print(f"📡 Lidar Sensor Node initialized. Broadcasting to {UDP_IP}:{UDP_PORT}")

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Simulated scenario variables
# Car in front starts at 100 meters, traveling at same speed (relative velocity = 0)
distance_to_target = 100.0
relative_velocity = 0.0

scenario = "CRUISING" # States: CRUISING, TARGET_BRAKING, COLLISION_IMMINENT

print("🚗 Scenario: Cruising on highway...")
try:
    while True:
        # Simulate an ADAS scenario timeline
        if distance_to_target > 40 and scenario == "CRUISING":
            # Cruising, gradually closing distance
            distance_to_target -= 2.0 + random.uniform(-0.5, 0.5)
            
        elif distance_to_target <= 40 and distance_to_target > 10:
            if scenario != "TARGET_BRAKING":
                print("\n⚠️  Scenario Shift: Target vehicle slams on brakes!")
                scenario = "TARGET_BRAKING"
            # Target car is braking hard, relative velocity becomes highly negative
            relative_velocity = -12.0 # Target is approaching us at 12 m/s relative
            distance_to_target += relative_velocity * 0.1 # Distance closes fast
            
        elif distance_to_target <= 10:
            if scenario != "COLLISION_IMMINENT":
                print("\n🚨 Scenario Shift: Collision Imminent!")
                scenario = "COLLISION_IMMINENT"
            # Getting dangerously close, relative velocity still high
            distance_to_target += relative_velocity * 0.1
            
        if distance_to_target < 0:
            print("💥 COLLISION OCCURRED! ADAS FAILED!")
            break

        # Package data as a simple CSV string instead of JSON
        # Format: timestamp_ms,front_distance_m,relative_velocity_mps
        csv_data = f"{int(time.time() * 1000)},{distance_to_target:.2f},{relative_velocity:.2f}"

        # Send over UDP
        message = csv_data.encode('utf-8')
        sock.sendto(message, (UDP_IP, UDP_PORT))
        
        # Output visually for testing
        print(f"📡 [LIDAR UPDATE] Distance: {distance_to_target:.2f}m | Rel. Vel: {relative_velocity:.2f}m/s")
        
        # Sensor operates at 10Hz (100ms per frame)
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nLidar Sensor Shutdown.")
finally:
    sock.close()
