import serial
import csv
import time
import numpy as np
import pandas as pd
from scipy.signal import butter, filtfilt
import matplotlib.pyplot as plt

# Part 1: Collect data from the serial port and write to CSV

# Configure the serial port
ser = serial.Serial('COM8', 19200)  # Replace 'COM8' with your Arduino's serial port
time.sleep(2)  # Wait for the serial connection to initialize

# Open the CSV file for writing
csv_filename = 'danash.csv'
with open(csv_filename, mode='w', newline='', encoding='utf-8') as file:
    writer = csv.writer(file)
    writer.writerow(['Channel 1'])  # Write the header

    sample_count = 0
    while sample_count < 4000:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            writer.writerow([line])
            sample_count += 1
            print(f'Sample {sample_count}: {line}')

print('Data collection complete.')
ser.close()

# Part 2: Read the data from CSV, apply filter, and plot the signal

# Load the data from the CSV file using pandas
data = pd.read_csv(csv_filename)

# Get the signal values from the desired column
column_name = 'Channel 1'  # Replace with the actual column name
signal = data[column_name].values.astype(float)  # Convert to float
time = np.arange(len(signal))
signal = signal / ((2 ** 24) - 1) * 2400  # Convert the signal to microvolts

# Define the sampling frequency (replace with your actual sampling frequency)
fs = 500  # Example: 500 Hz

# Design a band-pass filter from 0.5 Hz to 50 Hz
def bandpass_filter(data, lowcut, highcut, fs, order=4):
    nyquist = 0.5 * fs
    low = lowcut / nyquist
    high = highcut / nyquist
    b, a = butter(order, [low, high], btype='band')
    y = filtfilt(b, a, data)
    return y

# Apply the band-pass filter
filtered_signal = bandpass_filter(signal, 0.5, 50, fs)

# Plot the filtered signal in the time domain
plt.figure(figsize=(12, 8))
plt.plot(time, filtered_signal)
plt.xlabel('Time')
plt.ylabel('Amplitude (µV)')
plt.title(f'{column_name} Signal in Time Domain (Filtered)')
plt.grid(True)

plt.tight_layout()
plt.show()