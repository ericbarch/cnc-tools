/*
 * CNC Optic Drive Emulator v1
 * by Eric Barch (ericbarch.com)
 */
 

/* SD CARD STUFF */
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
File cncProgram;

/* PIN DEFINES */
#define LSB_BIT_PIN 53
#define IN_POSITION_PIN 37
#define TAPE_FEED_PIN 14
#define CHAR_DELAY_MS 2

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip(192, 168, 1, 143);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
boolean inComment = false;
// flag that tells us if a new program has been loaded in while running
boolean newProgram = false;

// interrupt commands
#define PENDING_CMD_BUF_SIZE 1000
byte pendingCmdData[PENDING_CMD_BUF_SIZE];
boolean pendingCmd = false;
int pendingBytes = 0;

// telnet defaults to port 23
EthernetServer server(23);


void setup() {
  // initial settle delay
  delay(500);

  // init serial
  Serial1.begin(115200);
  
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);
  
  // start listening for clients
  server.begin();
  
  // state setup (char pins)
  for (char i=0; i<=7; i++) {
    pinMode(LSB_BIT_PIN-(i*2), OUTPUT);
    digitalWrite(LSB_BIT_PIN-(i*2), LOW);
  }
  
  // state setup (in position pin)
  pinMode(IN_POSITION_PIN, OUTPUT);
  digitalWrite(IN_POSITION_PIN, LOW);
  
  // input from CNC machine
  pinMode(TAPE_FEED_PIN, INPUT);
  
  // SD SS
  pinMode(4, OUTPUT);
  
  if (!SD.begin(4)) {
    // sd initialization failed
    while(1){}
  }
}

void sendBits(uint8_t b7, uint8_t b6, uint8_t b5, uint8_t b4, uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0) {
  digitalWrite(LSB_BIT_PIN, b0);
  digitalWrite(LSB_BIT_PIN-2, b1);
  digitalWrite(LSB_BIT_PIN-4, b2);
  digitalWrite(LSB_BIT_PIN-6, b3);
  digitalWrite(LSB_BIT_PIN-8, b4);
  digitalWrite(LSB_BIT_PIN-10, b5);
  digitalWrite(LSB_BIT_PIN-12, b6);
  digitalWrite(LSB_BIT_PIN-14, b7);
  digitalWrite(IN_POSITION_PIN, HIGH);
  delay(CHAR_DELAY_MS);
  digitalWrite(IN_POSITION_PIN, LOW);
  delay(CHAR_DELAY_MS);
}

void sendChar(char incomingChar) {
  switch (incomingChar) {
    case '0':
      sendBits(LOW,LOW,HIGH,LOW,LOW,LOW,LOW,LOW);
      break;
    case '1':
      sendBits(LOW,LOW,LOW,LOW,LOW,LOW,LOW,HIGH);
      break;
    case '2':
      sendBits(LOW,LOW,LOW,LOW,LOW,LOW,HIGH,LOW);
      break;
    case '3':
      sendBits(LOW,LOW,LOW,HIGH,LOW,LOW,HIGH,HIGH);
      break;
    case '4':
      sendBits(LOW,LOW,LOW,LOW,LOW,HIGH,LOW,LOW);
      break;
    case '5':
      sendBits(LOW,LOW,LOW,HIGH,LOW,HIGH,LOW,HIGH);
      break;
    case '6':
      sendBits(LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH,LOW);
      break;
    case '7':
      sendBits(LOW,LOW,LOW,LOW,LOW,HIGH,HIGH,HIGH);
      break;
    case '8':
      sendBits(LOW,LOW,LOW,LOW,HIGH,LOW,LOW,LOW);
      break;
    case '9':
      sendBits(LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,HIGH);
      break;
    case 'a':
    case 'A':
      sendBits(LOW,HIGH,HIGH,LOW,LOW,LOW,LOW,HIGH);
      break;
    case 'b':
    case 'B':
      sendBits(LOW,HIGH,HIGH,LOW,LOW,LOW,HIGH,LOW);
      break;
    case 'c':
    case 'C':
      sendBits(LOW,HIGH,HIGH,HIGH,LOW,LOW,HIGH,HIGH);
      break;
    case 'd':
    case 'D':
      sendBits(LOW,HIGH,HIGH,LOW,LOW,HIGH,LOW,LOW);
      break;
    case 'e':
    case 'E':
      sendBits(LOW,HIGH,HIGH,HIGH,LOW,HIGH,LOW,HIGH);
      break;
    case 'f':
    case 'F':
      sendBits(LOW,HIGH,HIGH,HIGH,LOW,HIGH,HIGH,LOW);
      break;
    case 'g':
    case 'G':
      sendBits(LOW,HIGH,HIGH,LOW,LOW,HIGH,HIGH,HIGH);
      break;
    case 'h':
    case 'H':
      sendBits(LOW,HIGH,HIGH,LOW,HIGH,LOW,LOW,LOW);
      break;
    case 'i':
    case 'I':
      sendBits(LOW,HIGH,HIGH,HIGH,HIGH,LOW,LOW,HIGH);
      break;
    case 'j':
    case 'J':
      sendBits(LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,HIGH);
      break;
    case 'k':
    case 'K':
      sendBits(LOW,HIGH,LOW,HIGH,LOW,LOW,HIGH,LOW);
      break;
    case 'l':
    case 'L':
      sendBits(LOW,HIGH,LOW,LOW,LOW,LOW,HIGH,HIGH);
      break;
    case 'm':
    case 'M':
      sendBits(LOW,HIGH,LOW,HIGH,LOW,HIGH,LOW,LOW);
      break;
    case 'n':
    case 'N':
      sendBits(LOW,HIGH,LOW,LOW,LOW,HIGH,LOW,HIGH);
      break;
    case 'o':
    case 'O':
      sendBits(LOW,HIGH,LOW,LOW,LOW,HIGH,HIGH,LOW);
      break;
    case 'p':
    case 'P':
      sendBits(LOW,HIGH,LOW,HIGH,LOW,HIGH,HIGH,HIGH);
      break;
    case 'q':
    case 'Q':
      sendBits(LOW,HIGH,LOW,HIGH,HIGH,LOW,LOW,LOW);
      break;
    case 'r':
    case 'R':
      sendBits(LOW,HIGH,LOW,LOW,HIGH,LOW,LOW,HIGH);
      break;
    case 's':
    case 'S':
      sendBits(LOW,LOW,HIGH,HIGH,LOW,LOW,HIGH,LOW);
      break;
    case 't':
    case 'T':
      sendBits(LOW,LOW,HIGH,LOW,LOW,LOW,HIGH,HIGH);
      break;
    case 'u':
    case 'U':
      sendBits(LOW,LOW,HIGH,HIGH,LOW,HIGH,LOW,LOW);
      break;
    case 'v':
    case 'V':
      sendBits(LOW,LOW,HIGH,LOW,LOW,HIGH,LOW,HIGH);
      break;
    case 'w':
    case 'W':
      sendBits(LOW,LOW,HIGH,LOW,LOW,HIGH,HIGH,LOW);
      break;
    case 'x':
    case 'X':
      sendBits(LOW,LOW,HIGH,HIGH,LOW,HIGH,HIGH,HIGH);
      break;
    case 'y':
    case 'Y':
      sendBits(LOW,LOW,HIGH,HIGH,HIGH,LOW,LOW,LOW);
      break;
    case 'z':
    case 'Z':
      sendBits(LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,HIGH);
      break;
    // new line
    case 0xa:
      sendBits(HIGH,LOW,LOW,LOW,LOW,LOW,LOW,LOW);
      break;
    case ' ':
      sendBits(LOW,LOW,LOW,HIGH,LOW,LOW,LOW,LOW);
      break;
    case '%':
      sendBits(LOW,LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH);
      break;
    case '(':
      sendBits(LOW,LOW,LOW,HIGH,HIGH,LOW,HIGH,LOW);
      break;
    case ')':
      sendBits(LOW,HIGH,LOW,LOW,HIGH,LOW,HIGH,LOW);
      break;
    case '+':
      sendBits(LOW,HIGH,HIGH,HIGH,LOW,LOW,LOW,LOW);
      break;
    case '-':
      sendBits(LOW,HIGH,LOW,LOW,LOW,LOW,LOW,LOW);
      break;
    case '/':
      sendBits(LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,HIGH);
      break;
    case '.':
      sendBits(LOW,HIGH,HIGH,LOW,HIGH,LOW,HIGH,HIGH);
      break;
    case '&':
      sendBits(LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH,LOW);
      break;
    case ',':
      sendBits(LOW,LOW,HIGH,HIGH,HIGH,LOW,HIGH,HIGH);
      break;
    default:
      break;
  }
}

boolean checkForNewFile() {
  // check for clients
  EthernetClient client = server.available();

  if (client) {
    if (client.available()) {
      if (client.read() == 'n') {
        // close the already open file
        cncProgram.close();

        SD.remove("cnc.txt");
        
        cncProgram = SD.open("cnc.txt", FILE_WRITE);
        
        while (client.connected()) {
          if (client.available()) {
            while (client.available()) {
              char c = client.read();
              cncProgram.write(c);
            }
            client.write('k');
          }
        }

        // main loop needs to reset (it will handle closing the file)
        newProgram = true;

        // yes, we did load a new file
        return true;
      }
    }
  }

  // a new file was not loaded
  return false;
}

void checkForSerialData() {
  while (Serial1.available() > 0) {
    // read the incoming byte:
    byte incomingByte = Serial1.read();

    // if we see a new line, consider the command as fully received
    if (incomingByte == 0xA) {
      pendingCmd = true;
    } else if (pendingCmd || pendingBytes >= PENDING_CMD_BUF_SIZE) {
      // if new data is showing up and we already have a pendingCmd, clear it and replace it
      pendingCmd = false;
      pendingBytes = 0;
    }

    pendingCmdData[pendingBytes] = incomingByte;
    pendingBytes++;
  }
}

void sendCustomCommandIfPending() {
  if (pendingCmd) {
    for (int i = 0; i < pendingBytes; i++) {
      byte newChar = pendingCmdData[i];

      // wait for tape feed to go high
      while (digitalRead(TAPE_FEED_PIN) == LOW) {}

      // send char to machine
      sendChar(newChar);
      // send via telnet to anyone connected
      server.write(newChar);
      // send via serial console
      Serial1.write(newChar);
    }

    // the command has been transmitted
    pendingCmd = false;
    pendingBytes = 0;
  }
}

void loop() {
  cncProgram = SD.open("cnc.txt", FILE_READ);
  
  if (!cncProgram) {
    // could not open cnc.txt
    while(1){}
  }

  // read from the file until there's nothing else in it:
  while (cncProgram.available() && !newProgram) {
    char newChar = cncProgram.read();

    if (newChar == '(')
      inComment = true;
    else if (newChar == ')')
      inComment = false;

    // send via telnet to anyone connected
    server.write(newChar);

    // send via serial console
    Serial1.write(newChar);

    // don't bother seeing if the machine is busy if this is just a comment
    // (keep reading the file)
    if (inComment) {
      continue;
    }

    // wait for tape feed to go high
    while (digitalRead(TAPE_FEED_PIN) == LOW) {
      // while we're at it, check for a new file load
      if (checkForNewFile()) {
        // new program was loaded, break from the loop
        break;
      }
    }

    if (!newProgram) {
      sendChar(newChar);

      if (newChar == 0xA) {
        // read in a command if sent via the pi
        checkForSerialData();
        
        // new line, let's send a command if pending
        sendCustomCommandIfPending();
      }
    }
  }

  // ALWAYS reset state
  inComment = false;
  newProgram = false;
  
  // close the file:
  cncProgram.close();
}
