import serial
import time

# Define the serial port and baud rate
ser = serial.Serial('/dev/ttyACM0', 9600)  # Update '/dev/ttyACM0' based on ur system

# create a binary int
data = 0b00110111                           # binary of ASCII 7

# mask lower and higher nibbles (segmentation)
high_nibble = (data & 0xF0)
low_nibble = (data & 0x0F)

# add 4 zeros to each nibble (redundancy)
# high_nibble = high_nibble << 4
# low_nibble = low_nibble << 4
print('Low nibble', bin(low_nibble))
print('high nibble', bin(high_nibble))

# convert int object to byte object to send via UART
data_byte_hn = bytes([high_nibble])
data_byte_ln = bytes([low_nibble])

# Send the bytes of data representing both nibbles
hn = 0b00110000
ln = 0b00000111
hn = bytes([hn])
ln = bytes([ln])
ser.write(ln)
#time.sleep(0.01)
ser.write(hn)

# Close the serial port
ser.close()
