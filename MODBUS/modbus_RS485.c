#include "header.h"         // Your header file (add std headers if needed)
#include <termios.h>        // For serial port configuration
#include <fcntl.h>          // For open(), O_RDWR flags
#include <unistd.h>         // For read(), write(), close()
#include <stdio.h>          // For printf(), perror()
#include <string.h>         // For memset()
#include <sys/ioctl.h>      // For RTS control (TIOCMGET / TIOCMSET)

// ---------- Main Program ----------
int main() {
    int fd, n;
    short int sa = 172, q = 2;      // Start address = 172, number of registers = 2
    struct termios rs485;           // Serial port structure
    unsigned char buf[256], msg[8]; // Buffers for TX and RX

    // -------- Build Modbus RTU Frame --------
    // Function: Read Holding Registers (0x03)
    msg[0] = 0x01;                       // Slave address = 1
    msg[1] = 0x03;                       // Function code = 3 (Read Holding Registers)
    msg[2] = (sa >> 8) & 0xFF;           // Start address high byte
    msg[3] = sa & 0xFF;                  // Start address low byte
    msg[4] = (q >> 8) & 0xFF;            // Quantity high byte
    msg[5] = q & 0xFF;                   // Quantity low byte
    msg[6] = 0xC9;                       // CRC low byte (for this frame)
    msg[7] = 0xC5;                       // CRC high byte

    // -------- Open Serial Port --------
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("Error opening port");
        return -1;
    }

    // Clear non-blocking flag for read()
    fcntl(fd, F_SETFL, 0);

    // -------- Get Current Port Settings --------
    if (tcgetattr(fd, &rs485) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // -------- Configure Serial Port --------
    cfsetispeed(&rs485, B9600);          // Set input baud rate
    cfsetospeed(&rs485, B9600);          // Set output baud rate
    rs485.c_cflag = (rs485.c_cflag & ~CSIZE) | CS8;   // 8-bit data
    rs485.c_iflag &= ~IGNBRK;            // Disable break processing
    rs485.c_lflag = 0;                   // No signaling chars, no echo
    rs485.c_oflag = 0;                   // No remapping, no delays
    rs485.c_cc[VMIN] = 0;                // Non-blocking read
    rs485.c_cc[VTIME] = 20;              // Timeout = 2.0 seconds (20 * 0.1s)
    rs485.c_iflag &= ~(IXON | IXOFF | IXANY);  // Disable XON/XOFF
    rs485.c_cflag |= (CLOCAL | CREAD);         // Enable receiver, ignore modem ctrl
    rs485.c_cflag &= ~(PARENB | PARODD);       // No parity
    rs485.c_cflag &= ~CSTOPB;                  // 1 stop bit
    rs485.c_cflag &= ~CRTSCTS;                 // No hardware flow control

    // -------- Apply Serial Settings --------
    if (tcsetattr(fd, TCSANOW, &rs485) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    // -------- Optional: Control RTS for Half-Duplex --------
    int flags;
    ioctl(fd, TIOCMGET, &flags);    // Get current modem status
    flags |= TIOCM_RTS;             // Enable RTS = transmit mode
    ioctl(fd, TIOCMSET, &flags);    // Apply RTS

    // -------- Send Modbus Request Frame --------
    int rc = write(fd, msg, sizeof(msg));   // Send full 8-byte frame
    if (rc < 0) {
        perror("write");
        close(fd);
        return -1;
    }

    tcdrain(fd);  // Wait until all data is transmitted

    // -------- Switch to Receive Mode --------
    flags &= ~TIOCM_RTS;            // Disable RTS = receive mode
    ioctl(fd, TIOCMSET, &flags);    // Apply

    // -------- Wait for Slave Response --------
    usleep(500000); // 500ms delay to give slave time to respond

    // -------- Read Response --------
    memset(buf, 0, sizeof(buf));
    n = read(fd, buf, sizeof(buf)); // Read response bytes

    if (n > 0) {
        printf("Received (%d bytes): ", n);
        for (int i = 0; i < n; i++)
            printf("%02X ", buf[i]);   // Print each byte in HEX
        printf("\n");
    } else if (n < 0) {
        perror("read");
    } else {
        printf("No data received\n");
    }

    // -------- Close Port --------
    close(fd);
    return 0;
}

