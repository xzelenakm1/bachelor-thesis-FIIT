#include <SPI.h>
#include <WiFiUdp.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <CRC32.h>

unsigned int listeningPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char d[] = "D;c0a8c807;807d3a86899c;1553595857";
unsigned long start;
unsigned long ULONG_MAX = 4294967294;

WiFiUDP Udp;

int chipSelect = 7;
int numberOfDevices = 3;
int numberOfResponses = 0;
int waitingResponse = 0;
int prepAttempt = 0;
int nOfResponses = 0;

IPAddress Ip1(192, 168, 200, 4);
IPAddress Ip2(192, 168, 200, 6);
IPAddress Ip3(192, 168, 200, 7);
IPAddress Ip4(192, 168, 200, 8);
IPAddress myIp;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  
  setupWiFi();

  Udp.begin(listeningPort);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  SD.remove("ledger.txt");
  Serial.println("card initialized.");

  myIp = WiFi.localIP();
  
}

void loop() {

  if(millis() - start >= 5000 && waitingResponse){ // caka sa 5 sekund
    if(prepAttempt < 3){ // prvy alebo druhy pokus
      waitingResponse = 0;
      sendPrepRequest();
    }
    else{
      prepAttempt = 0;
      waitingResponse = 0;
      start = ULONG_MAX;
    }
  }

 
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
    packetReceived(packetSize);
    
}

void sendPrepRequest() {
  waitingResponse = 1;
  prepAttempt++;
  start = millis();
  
  if(myIp != Ip1) {
    Udp.beginPacket(Ip1, listeningPort);
    Udp.write("1");
    Udp.endPacket();
  }
  if(myIp != Ip2) {
    Udp.beginPacket(Ip2, listeningPort);
    Udp.write("1");
    Udp.endPacket();
  }
  if(myIp != Ip3) {
    Udp.beginPacket(Ip3, listeningPort);
    Udp.write("1");
    Udp.endPacket();
  }
  if(myIp != Ip4) {
    Udp.beginPacket(Ip4, listeningPort);
    Udp.write("1");
    Udp.endPacket();
  }
}

void sendPrepResponse(IPAddress remoteIp) {
  Udp.beginPacket(remoteIp, listeningPort);
  Udp.write("2");
  Udp.endPacket();
}

void sendData(String data) {
  int len = data.length();
  //Serial.println(len);

  char dataChar[255];
  data.toCharArray(dataChar, 255);
  uint32_t checksum = CRC32::calculate(dataChar, (size_t)len);
  
  String("3;" + data + ";" + checksum).toCharArray(dataChar, 255);

  if(myIp != Ip1) {
    Udp.beginPacket(Ip1, listeningPort);
    Udp.write(dataChar);
    Udp.endPacket();
  }
  if(myIp != Ip2) {
    Udp.beginPacket(Ip2, listeningPort);
    Udp.write(dataChar);
    Udp.endPacket();
  }
  if(myIp != Ip3) {
    Udp.beginPacket(Ip3, listeningPort);
    Udp.write(dataChar);
    Udp.endPacket();
  }
  if(myIp != Ip4) {
    Udp.beginPacket(Ip4, listeningPort);
    Udp.write(dataChar);
    Udp.endPacket();
  }
  
}

void sendDataResponse(IPAddress remoteIp) {
  Udp.beginPacket(remoteIp, listeningPort);
  Udp.write("4");
  Udp.endPacket();
}

int packetReceived(int packetSize) {

  IPAddress remoteIp = Udp.remoteIP();
  Serial.print("Received packet of size ");
  Serial.println(packetSize);
  Serial.print("From ");
  Serial.print(remoteIp);
  Serial.print(", port ");
  Serial.println(Udp.remotePort());

  // read the packet into packetBufffer
  int len = Udp.read(packetBuffer, 255);
  if (len > 0) {
    packetBuffer[len] = 0;
  }
  //Serial.println("Contents:");
  //Serial.println(packetBuffer);

  if(packetBuffer[0] == 'i') {
    sendPrepRequest();
    return 1;
  }
  if(packetBuffer[0] == '1') {
    sendPrepResponse(remoteIp);
    return 1;
  }
  if(packetBuffer[0] == '2') {
    if(waitingResponse == 1) {
      numberOfResponses++;
      if(numberOfResponses == ((numberOfDevices * 2) + 2) / 3) {
        waitingResponse = 0;
        numberOfResponses = 0;
        sendData(d);
        return 2;
      }
    }
    return 0;
  }
  if(packetBuffer[0] == '3') {
    String checksum = String(packetBuffer);
    checksum.remove(0, String(packetBuffer).lastIndexOf(';') + 1);
    //Serial.println("Checksum: " + checksum);
    String msgData = String(packetBuffer);
    msgData.remove(String(packetBuffer).lastIndexOf(';'));
    msgData.remove(0, 2);
    msgData.trim();
    //Serial.println("Data: " + msgData);
    
    int len = msgData.length();
    char msgBuffer[255];
    msgData.toCharArray(msgBuffer, 255);

    uint32_t checksumCalc = CRC32::calculate(msgBuffer, (size_t)len);

    if (checksum == String(checksumCalc)) {
      //Serial.println("ROVNA SA");
      
      writeToLedger(msgBuffer);
      Serial.println("Po zapisani dat pred odoslanim");
      sendDataResponse(remoteIp);
      Serial.println("Odoslany potvrdzovaci response");
    }
    else {
      char wrong[255];
      String("Checksum wrong " + checksum).toCharArray(wrong, 255);
      Serial.println(wrong);
    }
  }
  if(packetBuffer[0] == '4') {
    Serial.print("Record successfully written to ledger on IP: ");
    Serial.print(remoteIp);
    nOfResponses++;
    if(nOfResponses == ((numberOfDevices * 2) + 2) / 3){
      nOfResponses = 0;
      char dataChar[255];
      int len = String(d).length();
      uint32_t checksum = CRC32::calculate(d, (size_t)len);
      String(String(d) + ";" + checksum).toCharArray(dataChar, 255);

      writeToLedger(dataChar);
    }
    
  }

  return 0;
}


void writeToLedger(char *data){
  File ledger = SD.open("ledger.txt", FILE_WRITE);

  if (ledger) {
  ledger.println(data);
  ledger.close();
  Serial.println(data);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening ledger.txt");
  }
}

void swap(char *a, char *b) {
   char c = *a;
   *a = *b;
   *b = c;
}


void reverse(char* str) 
{ 
    int l = 2; 
    int r = strlen(str) - 2; 
  
    // swap with in two-2 pair 
    while (l < r) { 
        swap(&str[l++], &str[r++]); 
        swap(&str[l++], &str[r]); 
        r = r - 3; 
    } 
} 
  
char *ipToHexa(int addr) 
{ 
    char str[15]; 
  
    // convert integer to string for reverse 
    sprintf(str, "0x%08x", addr); 
  
    // reverse for get actual hexadecimal 
    // number without reverse it will 
    // print 0x0100007f for 127.0.0.1 
    reverse(str); 
  
    return str;
} 
