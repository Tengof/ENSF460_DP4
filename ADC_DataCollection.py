import serial
import time
import csv
import matplotlib.pyplot as plt

# Serial communication settings
PORT = "/dev/cu.usbserial-0001"
BAUD_RATE = 4800

# Collect data for 10 seconds
DURATION = 10

# Open serial connection
with serial.Serial(PORT, BAUD_RATE, timeout=1) as ser:
    print("Connected to PIC24")
    print("Collecting data for {} seconds...".format(DURATION))
    
    # Lists to store the time and ADC readings
    time_values = []
    adc_values = []
    
    start_time = time.monotonic()
    
    # Continuously read ADC values from UART for 10 seconds
    while (time.monotonic() - start_time) < DURATION:
        # Read one line from UART
        line = ser.readline().decode("ascii", errors="ignore").replace("\x00", "").strip()
        
        # Ignore empty lines and non-numeric data
        if not line.isdigit():
            continue
        
        # Convert received string to integer
        adc_value = int(line)
        
        # Make sure the received value is within the expected range (0-1023 for 10-bit ADC)
        if 0 <= adc_value <= 1023:
            elapsed_time = time.monotonic() - start_time
            
            # Only store readings within 10 seconds
            if elapsed_time <= DURATION:
                
                # Store the time and ADC value in the lists
                time_values.append(elapsed_time)
                adc_values.append(adc_value)
            
                print(f"Time: {elapsed_time:.2f}s, | ADC: {adc_value}")
            
    # Display the total number of valid ADC readings collected
    print("Total ADC readings collected:", len(adc_values))
    
    print("Data collection complete.")
    
    # Save the collected data to a CSV file
    with open("ADC_Data.csv", mode="w", newline="") as file:
        writer = csv.writer(file)
        
        # Write column headings
        writer.writerow(["Time (s)", "ADC Value"])
        
        # Write each ADC reading with its corresponding time
        for i in range(len(time_values)):
            writer.writerow([time_values[i], adc_values[i]])
    
    print("Data saved to ADC_Data.csv")
    
    # Plot the ADC readings over time
    plt.figure()
    plt.plot(time_values, adc_values, marker='o')
    plt.xlabel("Time (s)")
    plt.ylabel("ADC Value")
    plt.title("ADC Value vs. Time")
    
    # Set x-axis limits to the duration of data collection
    plt.xlim(0, DURATION)
    # Set y-axis limits to the range of ADC values (0-1023 for 10-bit ADC)
    plt.ylim(0, 1100)
    plt.grid(True)
    
    plt.savefig("ADC_Graph.png")
    
    # Convert ADC readings to voltages
    VREF = 3.3
    voltage_values = []
    
    for adc_value in adc_values:
        voltage = (adc_value / 1023) * VREF
        voltage_values.append(voltage)
    
    # Plot the voltage readings over time
    plt.figure()
    plt.plot(time_values, voltage_values, marker='o')
    
    plt.xlabel("Time (s)")
    plt.ylabel("Voltage (V)")
    plt.title("Voltage vs. Time")
    
    # Set x-axis limits to the duration of data collection
    plt.xlim(0, DURATION)
    # Set y-axis limits to the range of voltage values (0 to VREF)
    plt.ylim(0, 3.5)
    plt.grid(True)
    
    plt.savefig("Voltage_Graph.png")
    
    # Display both graphs
    plt.show()
