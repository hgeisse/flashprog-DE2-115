/*
 * flashprog.c -- Terasic DE2-115 Flash ROM programmer
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


/**************************************************************/


static int sfd = 0;
static struct termios origOptions;
static struct termios currOptions;


/**************************************************************/


void standby(void);
void serialClose(void);


void error(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("Error: ");
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
  if (sfd != 0) {
    standby();
    tcdrain(sfd);
    sleep(1);
    serialClose();
    sfd = 0;
  }
  exit(1);
}


/**************************************************************/


void serialOpen(char *serialPort) {
  sfd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY);
  if (sfd == -1) {
    error("cannot open serial port '%s'", serialPort);
  }
  tcgetattr(sfd, &origOptions);
  currOptions = origOptions;
  cfsetispeed(&currOptions, B38400);
  cfsetospeed(&currOptions, B38400);
  currOptions.c_cflag |= (CLOCAL | CREAD);
  currOptions.c_cflag &= ~PARENB;
  currOptions.c_cflag &= ~CSTOPB;
  currOptions.c_cflag &= ~CSIZE;
  currOptions.c_cflag |= CS8;
  currOptions.c_cflag &= ~CRTSCTS;
  currOptions.c_lflag &= ~(ICANON | ECHO | ECHONL | ISIG | IEXTEN);
  currOptions.c_iflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK);
  currOptions.c_iflag &= ~(INPCK | ISTRIP | INLCR | IGNCR | ICRNL);
  currOptions.c_iflag &= ~(IXON | IXOFF | IXANY);
  currOptions.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONOCR | ONLRET);
  tcsetattr(sfd, TCSANOW, &currOptions);
}


void serialClose(void) {
  tcsetattr(sfd, TCSANOW, &origOptions);
  close(sfd);
}


int serialSnd(unsigned char b) {
  int n;

  n = write(sfd, &b, 1);
  return n == 1;
}


int serialRcv(unsigned char *bp) {
  int n;

  n = read(sfd, bp, 1);
  return n == 1;
}


/**************************************************************/


#define SET_ADDR_0	0x00
#define SET_ADDR_1	0x10
#define SET_ADDR_2	0x20
#define SET_ADDR_3	0x30
#define SET_ADDR_4	0x40
#define SET_ADDR_5	0x50
#define SET_DATA_0	0x60
#define SET_DATA_1	0x70
#define SET_CTRL	0x80
#define GET_DATA	0x90
#define GET_READY	0xA0
#define IGNORE_B	0xB0
#define IGNORE_C	0xC0
#define IGNORE_D	0xD0
#define IGNORE_E	0xE0
#define IGNORE_F	0xF0


unsigned char lastAddr_0 = 0x00;
unsigned char lastAddr_1 = 0x00;
unsigned char lastAddr_2 = 0x00;
unsigned char lastAddr_3 = 0x00;
unsigned char lastAddr_4 = 0x00;
unsigned char lastAddr_5 = 0x00;
unsigned char lastData_0 = 0x00;
unsigned char lastData_1 = 0x00;
unsigned char lastCtrl   = 0x0F;


void initBoard(void) {
  lastAddr_0 = 0x00;
  while (!serialSnd(SET_ADDR_0 | lastAddr_0)) ;
  lastAddr_1 = 0x00;
  while (!serialSnd(SET_ADDR_1 | lastAddr_1)) ;
  lastAddr_2 = 0x00;
  while (!serialSnd(SET_ADDR_2 | lastAddr_2)) ;
  lastAddr_3 = 0x00;
  while (!serialSnd(SET_ADDR_3 | lastAddr_3)) ;
  lastAddr_4 = 0x00;
  while (!serialSnd(SET_ADDR_4 | lastAddr_4)) ;
  lastAddr_5 = 0x00;
  while (!serialSnd(SET_ADDR_5 | lastAddr_5)) ;
  lastData_0 = 0x00;
  while (!serialSnd(SET_DATA_0 | lastData_0)) ;
  lastData_1 = 0x00;
  while (!serialSnd(SET_DATA_1 | lastData_1)) ;
  lastCtrl = 0x0F;
  while (!serialSnd(SET_CTRL | lastCtrl)) ;
}


void setAddr(unsigned int addr) {
  unsigned char b;

  b = (addr >> 0) & 0x0F;
  if (lastAddr_0 != b) {
    while (!serialSnd(SET_ADDR_0 | b)) ;
    lastAddr_0 = b;
  }
  b = (addr >> 4) & 0x0F;
  if (lastAddr_1 != b) {
    while (!serialSnd(SET_ADDR_1 | b)) ;
    lastAddr_1 = b;
  }
  b = (addr >> 8) & 0x0F;
  if (lastAddr_2 != b) {
    while (!serialSnd(SET_ADDR_2 | b)) ;
    lastAddr_2 = b;
  }
  b = (addr >> 12) & 0x0F;
  if (lastAddr_3 != b) {
    while (!serialSnd(SET_ADDR_3 | b)) ;
    lastAddr_3 = b;
  }
  b = (addr >> 16) & 0x0F;
  if (lastAddr_4 != b) {
    while (!serialSnd(SET_ADDR_4 | b)) ;
    lastAddr_4 = b;
  }
  b = (addr >> 20) & 0x0F;
  if (lastAddr_5 != b) {
    while (!serialSnd(SET_ADDR_5 | b)) ;
    lastAddr_5 = b;
  }
}


void setData(unsigned char data) {
  unsigned char b;

  b = (data >> 0) & 0x0F;
  if (lastData_0 != b) {
    while (!serialSnd(SET_DATA_0 | b)) ;
    lastData_0 = b;
  }
  b = (data >> 4) & 0x0F;
  if (lastData_1 != b) {
    while (!serialSnd(SET_DATA_1 | b)) ;
    lastData_1 = b;
  }
}


void setCtrl(unsigned char ctrl) {
  unsigned char b;

  b = ctrl & 0x0F;
  if (lastCtrl != b) {
    while (!serialSnd(SET_CTRL | b)) ;
    lastCtrl = b;
  }
}


unsigned char getData(void) {
  unsigned char b;

  while (!serialSnd(GET_DATA)) ;
  while (!serialRcv(&b)) ;
  return b;
}


unsigned char getReady(void) {
  unsigned char b;

  while (!serialSnd(GET_READY)) ;
  while (!serialRcv(&b)) ;
  return b;
}


/**************************************************************/


void reset(void) {
  setCtrl(0x0E);
  setCtrl(0x03);
}


void standby(void) {
  setCtrl(0x0F);
}


unsigned char readData(unsigned int addr) {
  unsigned char b;

  setAddr(addr);
  setCtrl(0x03);
  b = getData();
  return b;
}


void writeData(unsigned int addr, unsigned char data) {
  setAddr(addr);
  setData(data);
  setCtrl(0x05);
  setCtrl(0x03);
}


void waitReady(void) {
  while ((getReady() & 0x01) == 0) ;
}


/**************************************************************/


void showIdentifiers(void) {
  unsigned char b;

  printf("result should be    : 0x01 0x7E 0x10 0x00\n");
  printf("result actually is  :");
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x90);
  b = readData(0x00);
  printf(" 0x%02X", b);
  b = readData(0x02);
  printf(" 0x%02X", b);
  b = readData(0x1C);
  printf(" 0x%02X", b);
  b = readData(0x1E);
  printf(" 0x%02X", b);
  printf("\n");
  writeData(0xAAA, 0xF0);
}


void eraseChip(void) {
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x80);
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x10);
  waitReady();
}


void eraseSector(unsigned int sector) {
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x80);
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(sector << 16, 0x30);
  waitReady();
}


void eraseBootSector(unsigned int sector) {
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x80);
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(sector << 13, 0x30);
  waitReady();
}


void checkChip(void) {
  error("this command would take 2:30h to complete, "
        "and thus is not implemented");
}


void checkSector(unsigned int sector) {
  unsigned int addr;
  unsigned char b;
  int i;

  for (i = 0; i < 64 * 1024; i++) {
    addr = (sector << 16) + i;
    b = readData(addr);
    if (b != 0xFF) {
      error("addr 0x%06X not empty, data is 0x%02X", addr, b);
    }
  }
}


void checkBootSector(unsigned int sector) {
  unsigned int addr;
  unsigned char b;
  int i;

  for (i = 0; i < 8 * 1024; i++) {
    addr = (sector << 13) + i;
    b = readData(addr);
    if (b != 0xFF) {
      error("addr 0x%06X not empty, data is 0x%02X", addr, b);
    }
  }
}


void readChip(char *fileName) {
  error("this command would take 2:30h to complete, "
        "and thus is not implemented");
}


void readSector(unsigned int sector, char *fileName) {
  FILE *outFile;
  unsigned int addr;
  unsigned char b;
  int i;

  outFile = fopen(fileName, "wb");
  if (outFile == NULL) {
    error("cannot open output file '%s'", fileName);
  }
  for (i = 0; i < 64 * 1024; i++) {
    addr = (sector << 16) + i;
    b = readData(addr);
    if (fwrite(&b, 1, 1, outFile) != 1) {
      error("cannot write to output file '%s'", fileName);
    }
  }
  fclose(outFile);
}


void readBootSector(unsigned int sector, char *fileName, int append) {
  FILE *outFile;
  unsigned int addr;
  unsigned char b;
  int i;

  outFile = fopen(fileName, append ? "ab" : "wb");
  if (outFile == NULL) {
    error("cannot open output file '%s'", fileName);
  }
  for (i = 0; i < 8 * 1024; i++) {
    addr = (sector << 13) + i;
    b = readData(addr);
    if (fwrite(&b, 1, 1, outFile) != 1) {
      error("cannot write to output file '%s'", fileName);
    }
  }
  fclose(outFile);
}


void programByte(unsigned int addr, unsigned char data) {
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0xA0);
  writeData(addr, data);
  waitReady();
}


void programFile(unsigned int start, char *fileName) {
  FILE *inFile;
  unsigned int size, i;
  unsigned int addr;
  unsigned char b;

  inFile = fopen(fileName, "rb");
  if (inFile == NULL) {
    error("cannot open input file '%s'", fileName);
  }
  fseek(inFile, 0, SEEK_END);
  size = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);
  if (size > 8 * 1024 * 1024) {
    fclose(inFile);
    error("size of file is bigger than the capacity of the Flash ROM");
  }
  writeData(0xAAA, 0xAA);
  writeData(0x555, 0x55);
  writeData(0xAAA, 0x20);
  addr = 0xAAA;
  for (i = 0; i < size; i++) {
    if (fread(&b, 1, 1, inFile) != 1) {
      error("cannot read from input file '%s'", fileName);
    }
    writeData(addr, 0xA0);  /* addr is don't care, use previous one */
    addr = start + i;
    writeData(addr, b);
  }
  waitReady();
  writeData(addr, 0x90);  /* addr is don't care, use previous one */
  writeData(addr, 0x00);  /* addr is don't care, use previous one */
  fclose(inFile);
}


void verifyFile(unsigned int start, char *fileName) {
  FILE *inFile;
  unsigned int size, i;
  unsigned int addr;
  unsigned char b, c;

  inFile = fopen(fileName, "rb");
  if (inFile == NULL) {
    error("cannot open input file '%s'", fileName);
  }
  fseek(inFile, 0, SEEK_END);
  size = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);
  if (size > 8 * 1024 * 1024) {
    fclose(inFile);
    error("size of file is bigger than the capacity of the Flash ROM");
  }
  for (i = 0; i < size; i++) {
    if (fread(&b, 1, 1, inFile) != 1) {
      error("cannot read from input file '%s'", fileName);
    }
    addr = start + i;
    c = readData(addr);
    if (b != c) {
      error("addr 0x%06X, file = 0x%02X, ROM = 0x%02X", addr, b, c);
    }
  }
  fclose(inFile);
}


/**************************************************************/


#define CMD_ID		0
#define CMD_ET		1
#define CMD_ES		2
#define CMD_EB		3
#define CMD_CT		4
#define CMD_CS		5
#define CMD_CB		6
#define CMD_RT		7
#define CMD_RS		8
#define CMD_RB		9
#define CMD_PB		10
#define CMD_PF		11
#define CMD_VF		12


void usage(char *myself) {
  printf("Usage: %s <serial port> <command> ...\n", myself);
  printf("valid commands are:\n");
  printf("    --id           identify chip\n");
  printf("    --et           erase total chip\n");
  printf("    --es <n>       erase 64 KB sector <n> (0..127)\n");
  printf("    --eb <n>       erase 8 KB boot sector <n> (0..7)\n");
  printf("    --ct           check empty total chip\n");
  printf("    --cs <n>       check empty 64 KB sector <n> (0..127)\n");
  printf("    --cb <n>       check empty 8 KB boot sector <n> (0..7)\n");
  printf("    --rt <f>       read total chip to file <f>\n");
  printf("    --rs <n> <f>   read 64 KB sector <n> (0..127) to file <f>\n");
  printf("    --rb <n> <f>   read 8 KB boot sector <n> (0..7) to file <f>\n");
  printf("    --pb <a> <d>   program addr <a> with data byte <d>\n");
  printf("    --pf <a> <f>   program start addr <a>, data from file <f>\n");
  printf("    --vf <a> <f>   verify start addr <a>, data from file <f>\n");
  printf("Note: sector 0 comprises the eight boot sectors 0..7\n");
  exit(1);
}


int main(int argc, char *argv[]) {
  char *serialPort;
  char *cmdName;
  int cmd;
  unsigned int n;
  char *endptr;
  int i;
  char *fileName;
  unsigned int addr;
  unsigned int data;

  if (argc < 3) {
    usage(argv[0]);
  }
  serialPort = argv[1];
  cmdName = argv[2];
  if (strcmp(cmdName, "--id") == 0) {
    if (argc != 3) {
      usage(argv[0]);
    }
    cmd = CMD_ID;
  } else
  if (strcmp(cmdName, "--et") == 0) {
    if (argc != 3) {
      usage(argv[0]);
    }
    cmd = CMD_ET;
  } else
  if (strcmp(cmdName, "--es") == 0) {
    if (argc != 4) {
      usage(argv[0]);
    }
    cmd = CMD_ES;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read sector number");
    }
    if (n > 127) {
      error("illegal sector number %d", n);
    }
  } else
  if (strcmp(cmdName, "--eb") == 0) {
    if (argc != 4) {
      usage(argv[0]);
    }
    cmd = CMD_EB;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read boot sector number");
    }
    if (n > 7) {
      error("illegal boot sector number %d", n);
    }
  } else
  if (strcmp(cmdName, "--ct") == 0) {
    if (argc != 3) {
      usage(argv[0]);
    }
    cmd = CMD_CT;
  } else
  if (strcmp(cmdName, "--cs") == 0) {
    if (argc != 4) {
      usage(argv[0]);
    }
    cmd = CMD_CS;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read sector number");
    }
    if (n > 127) {
      error("illegal sector number %d", n);
    }
  } else
  if (strcmp(cmdName, "--cb") == 0) {
    if (argc != 4) {
      usage(argv[0]);
    }
    cmd = CMD_CB;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read boot sector number");
    }
    if (n > 7) {
      error("illegal boot sector number %d", n);
    }
  } else
  if (strcmp(cmdName, "--rt") == 0) {
    if (argc != 4) {
      usage(argv[0]);
    }
    cmd = CMD_RT;
    fileName = argv[3];
  } else
  if (strcmp(cmdName, "--rs") == 0) {
    if (argc != 5) {
      usage(argv[0]);
    }
    cmd = CMD_RS;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read sector number");
    }
    if (n > 127) {
      error("illegal sector number %d", n);
    }
    fileName = argv[4];
  } else
  if (strcmp(cmdName, "--rb") == 0) {
    if (argc != 5) {
      usage(argv[0]);
    }
    cmd = CMD_RB;
    n = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read boot sector number");
    }
    if (n > 7) {
      error("illegal boot sector number %d", n);
    }
    fileName = argv[4];
  } else
  if (strcmp(cmdName, "--pb") == 0) {
    if (argc != 5) {
      usage(argv[0]);
    }
    cmd = CMD_PB;
    addr = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read address value");
    }
    addr &= 0x007FFFFF;
    data = strtoul(argv[4], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read data value");
    }
    data &= 0x000000FF;
  } else
  if (strcmp(cmdName, "--pf") == 0) {
    if (argc != 5) {
      usage(argv[0]);
    }
    cmd = CMD_PF;
    addr = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read address value");
    }
    addr &= 0x007FFFFF;
    fileName = argv[4];
  } else
  if (strcmp(cmdName, "--vf") == 0) {
    if (argc != 5) {
      usage(argv[0]);
    }
    cmd = CMD_VF;
    addr = strtoul(argv[3], &endptr, 0);
    if (*endptr != '\0') {
      error("cannot read address value");
    }
    addr &= 0x007FFFFF;
    fileName = argv[4];
  } else {
    usage(argv[0]);
  }
  serialOpen(serialPort);
  initBoard();
  reset();
  switch (cmd) {
    case CMD_ID:
      showIdentifiers();
      break;
    case CMD_ET:
      eraseChip();
      break;
    case CMD_ES:
      if (n == 0) {
        for (i = 0; i < 8; i++) {
          eraseBootSector(i);
        }
      } else {
        eraseSector(n);
      }
      break;
    case CMD_EB:
      eraseBootSector(n);
      break;
    case CMD_CT:
      checkChip();
      break;
    case CMD_CS:
      if (n == 0) {
        for (i = 0; i < 8; i++) {
          checkBootSector(i);
        }
      } else {
        checkSector(n);
      }
      break;
    case CMD_CB:
      checkBootSector(n);
      break;
    case CMD_RT:
      readChip(fileName);
      break;
    case CMD_RS:
      if (n == 0) {
        for (i = 0; i < 8; i++) {
          readBootSector(i, fileName, i != 0);
        }
      } else {
        readSector(n, fileName);
      }
      break;
    case CMD_RB:
      readBootSector(n, fileName, 0);
      break;
    case CMD_PB:
      programByte(addr, data);
      break;
    case CMD_PF:
      programFile(addr, fileName);
      break;
    case CMD_VF:
      verifyFile(addr, fileName);
      break;
    default:
      error("unknown command number %d", cmd);
      break;
  }
  if (sfd != 0) {
    standby();
    tcdrain(sfd);
    sleep(1);
    serialClose();
    sfd = 0;
  }
  return 0;
}
