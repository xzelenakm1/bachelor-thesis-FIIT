#include <SPI.h>
#include <WiFiUdp.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <CRC32.h>
#include <ArduinoSTL.h>

using namespace std;
int chipSelect = 7;
int waitingResponse = 0;
int requestSent;
int once = 1;
int data = 0;
vector<int> stakes;
vector<IPAddress> stakesIps;

char packetBuffer[255];
char d[] = "D;c0a8c807;807d3a86899c;1553595857";

unsigned int listeningPort = 2390;      // local port to listen on
WiFiUDP Udp;

IPAddress Ip1(192, 168, 200, 4);
IPAddress Ip2(192, 168, 200, 6);
IPAddress Ip3(192, 168, 200, 7);
IPAddress Ip4(192, 168, 200, 8);
IPAddress pc(192, 168, 200, 3);
IPAddress myIp;
IPAddress validator;

void setup() {
  Serial.begin(9600);

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

  randomSeed(analogRead(A0));

  if(once && myIp == IPAddress(192, 168, 200, 8)) {
    sendStakeRequest();
    once = 0;
  }

}

void loop() {

  if(waitingResponse && (millis() - requestSent) > 2000) { // caka sa 2 sekundy na odpoved na stavky
    waitingResponse = 0;
    int maxStake = -1;
    IPAddress maxStakeIp;
    for (int i = 0; i != stakes.size(); i++) {
      if (stakes[i] > maxStake) {
        maxStake = stakes[i];
        maxStakeIp = stakesIps[i];
      }
    }
    Serial.print("Pocet prijatych stavok je: ");
    Serial.println(stakes.size());
    stakes.clear();
    sendValidatorInfo(maxStakeIp);
    Serial.println("Najvyssia stavka: ");
    Serial.println(maxStake);
    Serial.println(maxStakeIp);
  }

  if(data) {
    sendDataForValidation(d);
    data = 0;
  }

  int packetSize = Udp.parsePacket();
  if (packetSize)
    packetReceived(packetSize);

}

void sendStakeRequest() {
  waitingResponse = 1;
  requestSent = millis();
  
  if(myIp != Ip1) {
    Udp.beginPacket(Ip1, listeningPort);
    Udp.write("1");
    Udp.endPacket();
    Serial.println("Sent stake request to Ip1");
  }
  if(myIp != Ip2) {
    Udp.beginPacket(Ip2, listeningPort);
    Udp.write("1");
    Udp.endPacket();
    Serial.println("Sent stake request to Ip2");
  }
  if(myIp != Ip3) {
    Udp.beginPacket(Ip3, listeningPort);
    Udp.write("1");
    Udp.endPacket();
    Serial.println("Sent stake request to Ip3");
  }
  if(myIp != Ip4) {
    Udp.beginPacket(Ip4, listeningPort);
    Udp.write("1");
    Udp.endPacket();
    Serial.println("Sent stake request to Ip4");
  }
}

void sendStakeResponse(IPAddress remoteIp) {
  Udp.beginPacket(remoteIp, listeningPort);
  String resp = String("2;") + String(random(1, 101));
  char respChar[255];
  resp.toCharArray(respChar, 255);
  Udp.write(respChar);
  Udp.endPacket();
}

void sendValidatorInfo(IPAddress val) {
  //Serial.println("aj posielam");
  //Serial.println(remoteIp);
  //Serial.println(listeningPort);
  String resp = String("3;") + String(val);
  char respChar[255];
  resp.toCharArray(respChar, 255);
  validator = val;

  if(myIp != Ip1) {
    Udp.beginPacket(Ip1, listeningPort);
    Udp.write(respChar);
    Udp.endPacket();
    Serial.println("Sent validator info to Ip1");
  }
  if(myIp != Ip2) {
    Udp.beginPacket(Ip2, listeningPort);
    Udp.write(respChar);
    Udp.endPacket();
    Serial.println("Sent validator info to Ip2");
  }
  if(myIp != Ip3) {
    Udp.beginPacket(Ip3, listeningPort);
    Udp.write(respChar);
    Udp.endPacket();
    Serial.println("Sent validator info to Ip3");
  }
  if(myIp != Ip4) {
    Udp.beginPacket(Ip4, listeningPort);
    Udp.write(respChar);
    Udp.endPacket();
    Serial.println("Sent validator info to Ip4");
  }
}

void sendDataForValidation(String data) {
  int len = data.length();

  char dataChar[255];
  data.toCharArray(dataChar, 255);
  uint32_t checksum = CRC32::calculate(dataChar, (size_t)len);
  
  String("4;" + data + ";" + checksum).toCharArray(dataChar, 255);

  Udp.beginPacket(validator, listeningPort);
  Udp.write(dataChar);
  Udp.endPacket();
  
}

void sendValidatedData(char *data) {
  if(myIp != Ip1) {
    Udp.beginPacket(Ip1, listeningPort);
    Udp.write(data);
    Udp.endPacket();
    Serial.println("Sent data to Ip1");
  }
  if(myIp != Ip2) {
    Udp.beginPacket(Ip2, listeningPort);
    Udp.write(data);
    Udp.endPacket();
    Serial.println("Sent data to Ip2");
  }
  if(myIp != Ip3) {
    Udp.beginPacket(Ip3, listeningPort);
    Udp.write(data);
    Udp.endPacket();
    Serial.println("Sent data to Ip3");
  }
  if(myIp != Ip4) {
    Udp.beginPacket(Ip4, listeningPort);
    Udp.write(data);
    Udp.endPacket();
    Serial.println("Sent data to Ip4");
  }
}

void packetReceived(int packetSize) {
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
    Serial.println(len);
    packetBuffer[len] = 0;
  }
  Serial.println("Contents:");
  Serial.println(packetBuffer);

  if(packetBuffer[0] == 'i') {
    Serial.println("Send stake request");
    sendStakeRequest();
  }
  if(packetBuffer[0] == '1') {
    sendStakeResponse(remoteIp);
    return;
  }
  if(packetBuffer[0] == '2') {
    if(waitingResponse) {
      char stake[5];
      strcpy(stake, &packetBuffer[2]);
      stakes.push_back(atoi(stake));
      stakesIps.push_back(remoteIp);
    }
  }
  if(packetBuffer[0] == '3') {
    char ipValidator[15];
    strcpy(ipValidator, &packetBuffer[2]);
    IPAddress val(atoi(ipValidator));
    validator = val;
  }
  if(packetBuffer[0] == '4') {
    String checksum4 = String(packetBuffer);
    checksum4.remove(0, String(packetBuffer).lastIndexOf(';') + 1);
    
    String ipSender = String(packetBuffer);
    ipSender.remove(ipSender.lastIndexOf(';'));
    ipSender.remove(ipSender.lastIndexOf(';'));
    ipSender.remove(ipSender.lastIndexOf(';'));
    ipSender.remove(0, 4);
    ipSender.trim();
    char ipSenderChar[20];
    ipSender.toCharArray(ipSenderChar, 20);
    char *ipDec = ipToDec(ipSenderChar);
    String ipDecStr = String(ipDec);
    String ipSender1, ipSender2, ipSender3, ipSender4;
    ipSender4 = ipDecStr.substring(ipDecStr.lastIndexOf('.') + 1);
    ipDecStr.remove(ipDecStr.lastIndexOf('.'));
    ipSender3 = ipDecStr.substring(ipDecStr.lastIndexOf('.') + 1);
    ipDecStr.remove(ipDecStr.lastIndexOf('.'));
    ipSender2 = ipDecStr.substring(ipDecStr.lastIndexOf('.') + 1);
    ipDecStr.remove(ipDecStr.lastIndexOf('.'));
    ipSender1 = ipDecStr.substring(ipDecStr.lastIndexOf('.') + 1);
    ipDecStr.remove(ipDecStr.lastIndexOf('.'));

    IPAddress sender(ipSender1.toInt(), ipSender2.toInt(), ipSender3.toInt(), ipSender4.toInt());
    /*Serial.println(ipSender1);
    Serial.println(ipSender2);
    Serial.println(ipSender3);
    Serial.println(ipSender4);*/


    String msgData = String(packetBuffer);
    msgData.remove(String(packetBuffer).lastIndexOf(';'));
    msgData.remove(0, 2);
    msgData.trim();
    //Serial.println("Data: " + msgData);
    
    int len = msgData.length();
    char msgBuffer[255];
    msgData.toCharArray(msgBuffer, 255);

    uint32_t checksumCalc4 = CRC32::calculate(msgBuffer, (size_t)len);

    if(sender == remoteIp && checksum4 == String(checksumCalc4)) {
      char newMessage[255];
      strcpy(newMessage, packetBuffer);
      newMessage[0] = '5';
      writeToLedger(&packetBuffer[2]);
      sendValidatedData(newMessage);
    }
    else {
      Serial.print("Ipcky sa nezhoduju: ");
    }
   
  }
  if(packetBuffer[0] == '5') {
    String checksum5 = String(packetBuffer);
    checksum5.remove(0, String(packetBuffer).lastIndexOf(';') + 1);
    //Serial.println("Checksum: " + checksum5);
    
    String msgData = String(packetBuffer);
    msgData.remove(String(packetBuffer).lastIndexOf(';'));
    msgData.remove(0, 2);
    msgData.trim();
    Serial.println("Data: " + msgData);
    
    int len = msgData.length();
    char msgBuffer[255];
    msgData.toCharArray(msgBuffer, 255);

    uint32_t checksumCalc5 = CRC32::calculate(msgBuffer, (size_t)len);

    if (checksum5 == String(checksumCalc5)) {
      writeToLedger(msgBuffer);
    }
    else {
      char wrong[255];
      String("Checksum wrong " + checksum5).toCharArray(wrong, 255);
      writeToLedger(wrong);
    }
    
  }
  
}

void writeToLedger(char *data){
  File ledger = SD.open("ledger.txt", FILE_WRITE);

  if (ledger) {
  ledger.println(data);
  Serial.print("Written to ledger: ");
  Serial.println(data);
  ledger.close();
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
    // l for swap with index 2 
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

    reverse(str); 
  
    return str;
}

static char *ipToDec(const char *in)
{
    char *out = (char*)malloc(sizeof(char) * 16);
    unsigned int p, q, r, s;

    if (sscanf(in, "%2x%2x%2x%2x", &p, &q, &r, &s) != 4)
        return out;
    sprintf(out, "%u.%u.%u.%u", p, q, r, s);
    return out;
}
