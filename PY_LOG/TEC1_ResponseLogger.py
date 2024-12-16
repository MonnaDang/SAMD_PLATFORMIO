import serial
import csv
import time

# Set up the serial connection parameters
com_port = 'COM3'
baud_rate = 9600
timeout_duration = 35 * 60  # 35 minutes in seconds
time_step = 5 * 60  # Step change every 2 minutes
temp_profile = [10, 20, 15, 0, -4, 15]
temp_index = 0

# Open the serial connection and CSV file
with serial.Serial(com_port, baud_rate, timeout=1) as ser, open("TEC1_Response.csv", mode='w', newline='') as file:
    csv_writer = csv.writer(file)
    csv_writer.writerow(["Elapsed Time (s)", "Hot(C)", "Cold(C)", "Tset(C)", "Voltage(V)"])  # CSV headers

    start_time = time.time()
    elapsed_time = 0.0

    last_command_time = 0  # Track the last time a command was sent
    last_temp_change_time = 0  # Track the last time Tset was updated

    is_started = False
    is_stopped = False

    while elapsed_time < timeout_duration:
        current_time = time.time()
        elapsed_time = current_time - start_time

        # Start sequence at 10 seconds
        if not is_started and elapsed_time >= 10.0:
            command1 = "SET:START\n"
            ser.write(command1.encode())
            print(f"Sent command: {command1.strip()} at {elapsed_time:.1f}s")

            command2 = f"SET:TSET,{temp_profile[temp_index]}\n"
            ser.write(command2.encode())
            print(f"Sent command: {command2.strip()} at {elapsed_time:.1f}s")

            is_started = True
            last_temp_change_time = elapsed_time

        # Update temperature setpoint at intervals
        if is_started and (elapsed_time - last_temp_change_time >= time_step):
            temp_index = (temp_index + 1) % len(temp_profile)
            command2 = f"SET:TSET,{temp_profile[temp_index]}\n"
            ser.write(command2.encode())
            print(f"Sent command: {command2.strip()} at {elapsed_time:.1f}s")

            last_temp_change_time = elapsed_time

        # Stop sequence after 30 minutes
        if not is_stopped and elapsed_time >= 1800.0:
            command1 = "SET:STOP\n"
            ser.write(command1.encode())
            print(f"Sent command: {command1.strip()} at {elapsed_time:.1f}s")

            is_stopped = True

        # Receiving data
        if ser.in_waiting > 0:  # Check if data is available
            line = ser.readline().decode().strip()

            try:
                # Split the line by commas and parse each value
                if line.startswith("DATA:"):
                    # Remove "DATA: " prefix and split the values
                    data = line[6:].split(",")
                    if len(data) == 4:
                        hotTemp = float(data[0])
                        coldTemp = float(data[1])
                        Tset = float(data[2])
                        Voltage = float(data[3])

                        # Print and save the data with the current elapsed time
                        print(f"{elapsed_time:.1f}s, Hot: {hotTemp} C, Cold: {coldTemp} C, Tset: {Tset} C, Voltage: {Voltage} V")
                        csv_writer.writerow([elapsed_time, hotTemp, coldTemp, Tset, Voltage])
            
            except ValueError:
                print("Received malformed data:", line)

        # Small delay to avoid busy-looping
        time.sleep(0.1)

print("Data collection complete.")
