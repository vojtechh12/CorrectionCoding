#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

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

    // Send a byte of data (for example, 'A')
    char data_byte = 0b00110110;                // ascii num 6
    write(fd, &data_byte, 1);
    write(fd, &data_byte, 1);


    // Close the serial port
    close(fd);

    return 0;
}
