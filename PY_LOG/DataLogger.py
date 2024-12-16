import serial
import csv
import time

# Set up the serial connection parameters
com_port = 'COM3'
baud_rate = 9600
timeout_duration = 30 * 60  # 20 minutes in seconds

# Open the serial connection and CSV file
with serial.Serial(com_port, baud_rate, timeout=1) as ser, open("TEC1_Data.csv", mode='w', newline='') as file:
    csv_writer = csv.writer(file)
    csv_writer.writerow(["Elapsed Time (s)", "Hot(C)", "Cold(C)", "Voltage(V)"])  # CSV headers

    start_time = time.time()
    elapsed_time = 0.0

    isStart = 0
    isStop = 0

    while time.time() - start_time < timeout_duration:
        # Sending a command example (at 10 seconds)
        if elapsed_time == 10.0:
            if isStart == 0 :
                command1 = "SET:START\n"
                ser.write(command1.encode())
                print(f"Sent command: {command1.strip()} at {elapsed_time:.1f}s")
                
                command2 = "SET:VSET,12\n"
                ser.write(command2.encode())
                print(f"Sent command: {command2.strip()} at {elapsed_time:.1f}s")

                isStart = 1
            
        if elapsed_time == 1200:        # after 20 mins
            if isStop == 0:
                command1 = "SET:STOP\n"
                ser.write(command1.encode())
                print(f"Sent command: {command1.strip()} at {elapsed_time:.1f}s")

                isStop = 1
        
        # Receiving data
        if ser.in_waiting > 0:  # Check if data is available
            line = ser.readline().decode().strip()
            
            try:
                # Split the line by commas and parse each value
                if line.startswith("DATA:"):
                    # Remove "DATA: " prefix and split the values
                    data = line[6:].split(",")
                    if len(data) == 3:
                        hotTemp = float(data[0])
                        coldTemp = float(data[1])
                        Voltage = float(data[2])
                    
                        # Print and save the data with the current elapsed time
                        print(f"{elapsed_time:.1f}s, Hot: {hotTemp} C, Cold: {coldTemp} C, Voltage: {Voltage} V")
                        csv_writer.writerow([elapsed_time, hotTemp, coldTemp, Voltage])
                    
                        # Update the timestamp by 0.5 seconds
                        elapsed_time += 0.5
            
            except ValueError:
                print("Received malformed data:", line)

print("Data collection complete.")
