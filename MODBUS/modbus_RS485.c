#include "header.h"

// ---------- CRC Calculation Function ----------
uint16_t modbus_crc(uint8_t *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

int main()
{
    int fd, n;
    struct termios rs485;
    uint8_t msg[8];
    uint8_t buf[256];
    uint16_t crc;
    uint16_t start_addr = 172, qty = 2;

    // ----------- Build Modbus RTU Request -----------
    msg[0] = 0x01;                    // Slave ID
    msg[1] = 0x03;                    // Function Code (Read Holding Registers)
    msg[2] = (start_addr >> 8) & 0xFF;
    msg[3] = start_addr & 0xFF;
    msg[4] = (qty >> 8) & 0xFF;
    msg[5] = qty & 0xFF;

    // Compute CRC dynamically
    crc = modbus_crc(msg, 6);
    msg[6] = crc & 0xFF;              // CRC Low byte
    msg[7] = (crc >> 8) & 0xFF;       // CRC High byte

    // ----------- Open Serial Port -----------
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Port open failed");
        return -1;
    }

    // ----------- Configure UART 9600 8N1 -----------
    if (tcgetattr(fd, &rs485) != 0) {
        perror("tcgetattr failed");
        close(fd);
        return -1;
    }

    cfsetispeed(&rs485, B9600);
    cfsetospeed(&rs485, B9600);

    rs485.c_cflag = (rs485.c_cflag & ~CSIZE) | CS8; // 8 data bits
    rs485.c_iflag &= ~IGNBRK;
    rs485.c_lflag = 0;
    rs485.c_oflag = 0;
    rs485.c_cc[VMIN] = 0;
    rs485.c_cc[VTIME] = 20;  // 2 seconds timeout (0.1s * 20)

    rs485.c_iflag &= ~(IXON | IXOFF | IXANY);
    rs485.c_cflag |= (CLOCAL | CREAD);
    rs485.c_cflag &= ~(PARENB | PARODD); // No parity
    rs485.c_cflag &= ~CSTOPB;            // 1 stop bit
    rs485.c_cflag &= ~CRTSCTS;           // Disable HW flow control

    tcsetattr(fd, TCSANOW, &rs485);

    // ----------- Send Modbus Request -----------
    int rc = write(fd, msg, 8);
    if (rc < 0) {
        perror("write failed");
        close(fd);
        return -1;
    }

    tcdrain(fd); // Wait for transmission complete

    // ----------- Read Response -----------
    memset(buf, 0, sizeof(buf));
    n = read(fd, buf, sizeof(buf));

    if (n > 0) {
        printf("Received %d bytes:\n", n);
        for (int i = 0; i < n; i++)
            printf("%02X ", buf[i]);
        printf("\n");
    } else if (n == 0) {
        printf("No data received (timeout)\n");
    } else {
        perror("read failed");
    }

    close(fd);
    return 0;
}

