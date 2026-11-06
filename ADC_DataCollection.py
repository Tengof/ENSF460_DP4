

import os                        # file paths
import time                      # timing of each data capture
import serial                    # pyserial: reads data from the COM port
import pandas as pd              # data frame and CSV file
import matplotlib.pyplot as plt  # plotting


# Settings

PORT = "COM7"           # COM port of the board (check Device Manager)
BAUD = 9600             
VDD = 3.0               # ADC reference voltage (Vmax) in volts
ADC_MAX = 1023          # largest 10-bit ADC value
DURATION = 10.0         # recording interval in seconds
WAIT_LIMIT = 15.0       # stop if no data arrives within this many seconds
# CSV is saved in the same folder as this script
CSV_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "adc_data.csv")


def open_port():
  
    ser = serial.Serial()
    ser.port = PORT
    ser.baudrate = BAUD
    ser.timeout = 1         # readline() gives up after 1 s with no data
    ser.dtr = False
    ser.rts = False
    ser.open()
    return ser


def collect_data():
 

    times = []
    adc_values = []
    start = None            # time of the first value received

    with open_port() as ser:
        ser.reset_input_buffer()    # discard data sent before we started
        print(f"Connected to {PORT}. Recording for {DURATION:.0f} seconds...\n")
        opened = time.perf_counter()

        while True:
            # Read one line, e.g. b"512\r\n", and convert it to "512"
            # (readline gives up after 1 s if nothing arrives)
            raw = ser.readline()
            # Remove null bytes (\x00) the UART send function adds, then whitespace
            line = raw.decode("ascii", errors="ignore").replace("\x00", "").strip()
            now = time.perf_counter()

            # Stop after the 10-second interval, even if data stopped arriving
            if start is not None and now - start > DURATION:
                break

            # Skip anything that is not a number (blank lines, mode messages)
            if not line.isdigit():
                if raw:
                    print("Board sent (not a number):", repr(raw))
                if start is None and now - opened > WAIT_LIMIT:
                    print("No data received. Check that the board is in Mode 1 "
                          "and that PORT and BAUD are correct.")
                    break
                continue

            # Time of this capture, measured from the first value
            if start is None:
                start = now
            t = now - start

            value = int(line)
            times.append(round(t, 3))
            adc_values.append(value)
            print(f"t = {t:6.3f} s   ADC = {value:4d}")

    return times, adc_values


def build_data_frame(times, adc_values):

    df = pd.DataFrame({
        "Time (sec)": times,
        "ADC Buffer Value": adc_values,
    })
    df["ADC Input Voltage (V)"] = (df["ADC Buffer Value"] / ADC_MAX * VDD).round(4)
    return df


def plot_data(df):
    """Show the two required plots, each in its own window."""

    # Plot 1: ADC buffer value vs time
    plt.figure("ADC Buffer Value vs Time")
    plt.plot(df["Time (sec)"], df["ADC Buffer Value"], marker=".")
    plt.xlabel("Time (sec)")
    plt.ylabel("ADC Buffer Value")
    plt.title("ADC Buffer Value vs Time")
    plt.ylim(-20, ADC_MAX + 20)
    plt.grid(True)

    # Plot 2: ADC input voltage vs time
    plt.figure("ADC Input Voltage vs Time")
    plt.plot(df["Time (sec)"], df["ADC Input Voltage (V)"], marker=".", color="tab:orange")
    plt.xlabel("Time (sec)")
    plt.ylabel("ADC Input Voltage (V)")
    plt.title("ADC Input Voltage vs Time")
    plt.ylim(-0.1, VDD + 0.1)
    plt.grid(True)

    plt.show()


def main():
    # 1. Receive data from the microcontroller
    times, adc_values = collect_data()
    if not times:
        print("No data recorded, so no CSV file or plots were made.")
        return

    # 2. Build the data frame and display it in the terminal
    df = build_data_frame(times, adc_values)
    print("\nRecorded data:")
    print(df.to_string(index=False))

    # 3. Save the data to a CSV file with labeled columns
    df.to_csv(CSV_FILE, index=False)
    print(f"\nSaved {len(df)} samples to:\n{CSV_FILE}")

    # 4. Plot ADC value and voltage vs time in two separate windows
    plot_data(df)


if __name__ == "__main__":
    main()
