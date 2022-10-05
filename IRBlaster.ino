#include <AltSoftSerial.h>
#include <IRremote.hpp>

#define NO_LED_FEEDBACK_CODE

uint16_t common[36] = { 417, 278, 167, 278, 167, 611, 167, 444, 167, 611, 167, 278, 167, 278, 167, 278, 167, 278, 167, 278, 167, 611, 167, 444, 167, 611, 167, 278, 167, 278, 167, 0, 0, 0, 0, 0 };
AltSoftSerial softSerial; //uno pin 8 Rx, uno pin 9 Tx
const String resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: application/json;charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\n\r\n";

void setup() {
  pinMode(3, OUTPUT);
  //pinMode(8, INPUT);
  //pinMode(9, OUTPUT);
  IrSender.begin(3, ENABLE_LED_FEEDBACK);
  Serial.begin(9600);
  while (!Serial);
  setupESPSerial(9600);
  setupESPServer();
}

void loop() {
  Serial.println("WiFi Connected. Waiting for a request!");
  
  while(softSerial.available() == 0);
  String espResp = "";
  espResp = readSerial();
  Serial.println("some req"+espResp);
  String clientReq = parseRequest(espResp);
  espResp = "";
  
  if(clientReq != "blank") {
    blastIR(clientReq);
    sendResponse(clientReq);
  }
}

String readSerial() {
  int nbytes = 0;
  int maxbytes = 600;
  String espResp;
  bool reading = false;
  //delay(100);
  while(softSerial.available() > 0){
    while(true) {
      char c = softSerial.read();
      if(nbytes < 50 && espResp.indexOf("HTTP") == -1) {
        espResp += String(c);
      } else {
        if(nbytes < maxbytes)
          softSerial.read();
        else
          break;
      }
      nbytes++;
    }
  }
  return espResp;
}

void setupESPSerial(int baud) {
  softSerial.begin(baud);
}

void setupESPServer() {
  Serial.println("Server Setup Started----->");
  runCommand( "AT" );
  while(!softSerial.find("OK")) {
    boolean breakw = waitAndRetry("AT", 5);
    if(breakw) break;
  }
  
  runCommand( "AT+CIPSTATUS" );
  while(!softSerial.find("OK")) {
    boolean breakw = waitAndRetry("AT+CIPSTATUS", 5);
    if(breakw) break;
  }
  
  runCommand( "AT+CIPMUX=1" ); //allow multiple connections
  while(!softSerial.find("OK")) {
    boolean breakw = waitAndRetry("AT+CIPMUX=1", 5);
    if(breakw) break;
  }
  
  runCommand( "AT+CIPSERVER=1,80" ); //start server at Channel: 1, Port: 8080
  while(!softSerial.find("OK")) {
    boolean breakw = waitAndRetry("AT+CIPSERVER=1,80", 5);
    if(breakw) break;
  }
  
  Serial.println("Server Setup Completed. Ready for Action!");
}

void runCommand(String cmd) {
  softSerial.flush();
  softSerial.println(cmd);
  Serial.println("==="+cmd);
  delay(200);
  //if(!softSerial.available()){
  //  Serial.println("Something is wrong :( ESP comm down");
  //}
}

boolean waitAndRetry(String cmd, int retry) {
  int i;
  for(i = 0; i < retry; i++) {
    delay(100);
    if(softSerial.find("OK")) break;
  }
  if(i == retry) {
    runCommand(cmd);
    return false;
  }
  return true;
}

String parseRequest(String espResp) {
  String info = "";
  if(espResp.indexOf("+IPD") != -1) {
    info += espResp.substring( espResp.indexOf("+IPD"), espResp.indexOf("+IPD")+6 );
    if(espResp.indexOf("fan=") != -1) {
      return info + espResp.substring(espResp.indexOf("fan="), espResp.indexOf("fan=")+5);
    } else if(espResp.indexOf("channel=") != -1) {
      return info + espResp.substring(espResp.indexOf("channel="), espResp.indexOf("channel=")+11);
    }
  }
  return "blank";
}

void blastIR(String clientReq) {
  if(clientReq.indexOf("fan=") != -1) {
    String fanop = clientReq.substring(clientReq.indexOf("fan=")+4, clientReq.indexOf("fan=")+5);
    uint8_t cmd = 0x1;
    if( fanop=="0" ) {
      cmd = 0x0;
    } else if( fanop=="1" ) {
      cmd = 0x4;
    } else if( fanop=="2" ) {
      cmd = 0x5;
    } else if( fanop=="3" ) {
      cmd = 0x6;
    } else if( fanop=="4" ) {
      cmd = 0x7;
    } else if( fanop=="5" ) {
      cmd = 0x8;
    }
    IrSender.sendNEC(0x8C, cmd, 3);
    fanop = "";
  } else if(clientReq.indexOf("channel=") != -1) {
    String chno = clientReq.substring(clientReq.indexOf("channel=")+8, clientReq.indexOf("channel=")+11);
    for(int i = 0; i < 3; i++) {
      String chnoi = (String)chno[i];
      if( chnoi=="0" ) {
        const uint16_t rawData[] = { 278, 167, 278, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="1" ) {
        const uint16_t rawData[] = { 278, 167, 444, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="2" ) {
        const uint16_t rawData[] = { 278, 167, 611, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="3" ) {
        const uint16_t rawData[] = { 278, 167, 778, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="4" ) {
        const uint16_t rawData[] = { 444, 167, 278, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="5" ) {
        const uint16_t rawData[] = { 444, 167, 444, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="6" ) {
        const uint16_t rawData[] = { 444, 167, 611, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="7" ) {
        const uint16_t rawData[] = { 444, 167, 778, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="8" ) {
        const uint16_t rawData[] = { 611, 167, 278, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      } else if( chnoi=="9" ) {
        const uint16_t rawData[] = { 611, 167, 444, 167, 65535 };
        buildArr(rawData);
        IrSender.sendRaw(common, 36, NEC_KHZ);
      }
      delay(300);
    }
    chno = "";
  }
}

void buildArr(uint16_t* specific) {
  for(int i = 0; i < 5; i++) {
    common[i+31] = specific[i];
  }
}

void sendResponse(String clientReq) {
  String espCh = clientReq.substring( clientReq.indexOf(",")+1,  clientReq.indexOf(",")+2);
  String jsonr = "{\"sucess\": true}";
  
  String tempresp = "";
  runCommand( "AT+CIPSEND="+ espCh +"," + (String)resp.length() );
  runCommand( resp );

  runCommand( "AT+CIPSEND="+ espCh +"," + (String)jsonr.length() );
  runCommand( jsonr );
  
  delay(200);
  runCommand ( "AT+CIPCLOSE="+espCh ); //close all connections
  
}
