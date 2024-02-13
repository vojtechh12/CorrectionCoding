#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// global variables
char mxG[4] = {0x45, 0x27, 0x16, 0x0B};

// function definitions
char encode(char b) {
    // hamming code encoding
    char tmp;
    char state = 0x00;
    for (int i=0; i<4; i++) {
        if (((b>>(3-i)) & 0x01) == 0) {         // masking columns of generator matrix
            continue;
        }
        tmp = mxG[i];
        if (state != 0x00){
            tmp ^= state;
        }
        state = tmp;
    }
    // add some logic for null b word
    printf("The resulting codeword: %x\n", tmp);

    return tmp;
}

char sdch(char c, int index) {
    // corruption of encoded data via AWGN channel
    // index implies the bit of encoded data to toggle
    c ^= (1 << index);
    return c;
}


int main() {
    int fd;
    struct termios serial_port_settings;

    // Open the serial port (replace "/dev/ttyACM0" with the appropriate port on your system)
    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    // Configure serial port settings
    tcgetattr(fd, &serial_port_settings);
    cfsetispeed(&serial_port_settings, B9600); // Set baud rate to 9600
    cfsetospeed(&serial_port_settings, B9600);
    serial_port_settings.c_cflag |= (CLOCAL | CREAD);
    serial_port_settings.c_cflag &= ~PARENB;   // No parity
    serial_port_settings.c_cflag &= ~CSTOPB;   // One stop bit
    serial_port_settings.c_cflag &= ~CSIZE;
    serial_port_settings.c_cflag |= CS8;       // 8 bits per byte
    tcsetattr(fd, TCSANOW, &serial_port_settings);

    // Send a byte of data
    char data_byte = 0b00110101;                // ascii num X
    char c = 0x00;

    // encode and send low nibble
    c = encode(data_byte & 0x0F);
    c = sdch(c, 1);
    write(fd, &c, 1);

    // encode and send high nibble
    c = encode((data_byte & 0xF0)>>4);
    c = sdch(c, 5);
    write(fd, &c, 1);


    // Close the serial port
    close(fd);

    return 0;
}
