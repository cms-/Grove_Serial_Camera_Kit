
//  Based on original work: SerialCamera_DemoCode_CJ-OV528.ino
//  8/8/2013 by Jack Shao www.seeedstudio.com
//
//  Adapted for use with Aidevision AD-7701 camera module
//  2016-01-10 by Chris Slothouber chris@dreaming.org

#include <SD.h>
#include <SPI.h>

#define PIC_PKT_LEN    512        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     Serial3

#define PIC_FMT        3

File myFile;
//SoftwareSerial softSerial(4, 5);  //rx,tx (11-13 is used by sd shield)

const int buttonPin = 33;                 // the number of the pushbutton pin
const int sdCS = 10;
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;

/*********************************************************************/
void setup()
{
  
  Serial1.begin(115200);
  delay(1000);
  CAM_SERIAL.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);    // initialize the pushbutton pin as an input
  
  Serial.println("Initializing SD card....");
  //pinMode(sdCS,OUTPUT);          // CS pin of SD Card Shield
  
  if (!SD.begin(sdCS)) {
    Serial.println("sd init fail");
  }
  else
  Serial.println("sd init done.");
  
  initialize();
}
/*********************************************************************/
void loop()
{
  int n = 0;
  while(1){
    Serial.println("\r\nPress the button to take a snapshot...");
    while (digitalRead(buttonPin) == HIGH);      //wait for buttonPin status to HIGH
    Serial.println("Processing...");
    if(digitalRead(buttonPin) == LOW){
      delay(20);                               //Debounce
      if (digitalRead(buttonPin) == LOW)
      {
        delay(200);
        if (n == 0) preCapture();
        Capture();
        Serial.print("Saving picture...");
        GetData();
      }
      Serial.print("\r\nDone, number: ");
      Serial.println(n);
      n++ ;
      }
    }
}
/*********************************************************************/
void printResp(uint8_t r[], int r_len)
{
  Serial.print("in: ");
  for (char i = 0; i < r_len; i++)
  {
    Serial.print(r[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}
/*********************************************************************/
void clearRxBuf()
{
  while (CAM_SERIAL.available()) 
  {
    CAM_SERIAL.read(); 
  }
}
/*********************************************************************/
void sendCmd(char cmd[], int cmd_len)
{
  Serial.print("out: ");
  for (char i = 0; i < cmd_len; i++)
  {
    Serial.print(cmd[i], HEX);
    CAM_SERIAL.write(cmd[i]);
    Serial.print(" "); 
  }
  Serial.println(" ");
}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (CAM_SERIAL.available()<1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
    }
    *(dest+read_len) = CAM_SERIAL.read();
    //Serial.write(*(dest+read_len));
    read_len++;
  }
  return read_len;
}
/*********************************************************************/
void initialize()
{   
  char cmd[] = {0xaa,0x0d,0x00,0x00,0x00,0x00} ;  
  unsigned char resp[6];

  Serial.print("initializing camera...");
  
  while (1) 
  {
    sendCmd(cmd,6);
    if (readBytes((char *)resp, 6,1000) != 6)
    {
      Serial.print(".");
      continue;
    }
    printResp(resp, 6);
    if (resp[0] == 0xaa && resp[1] == 0x0e && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0) 
    {
      //cameraAddr = resp[3];
      if (readBytes((char *)resp, 6, 500) != 6) continue; 
      printResp(resp, 6);
      if (resp[0] == 0xaa && resp[1] == 0x0d && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
    }
  }  
  cmd[1] = 0x0e;
  cmd[2] = 0x0d;
  cmd[3] = 0x00;
  sendCmd(cmd,6); 
  
  Serial.println("\nCamera initialization done.");
}
/*********************************************************************/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01, 0x00, 0x07, 0x03, PIC_FMT };  
  unsigned char resp[6]; 
  
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue; 
    printResp(resp, 6);
    if (resp[0] == 0xaa && resp[1] == 0x0e && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break; 
  }
}
void Capture()
{
  char cmd[] = { 0xaa, 0x06, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    printResp(resp, 6);
    if (resp[0] == 0xaa && resp[1] == 0x0e && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break; 
  }
  cmd[1] = 0x05;
  cmd[2] = 0;
  cmd[3] = 0x10;
  cmd[4] = 0;
  cmd[5] = 0; 
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    printResp(resp, 6);
    if (resp[0] == 0xaa && resp[1] == 0x0e && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04;
  cmd[2] = 0x01;
  while (1) 
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    printResp(resp, 6);
    if (resp[0] == 0xaa && resp[1] == 0x0e && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      if (readBytes((char *)resp, 6, 1000) != 6)
      {
        continue;
      }
      printResp(resp, 6);
      if (resp[0] == 0xaa && resp[1] == 0x0a && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
        Serial.print("picTotalLen: ");
        Serial.println(picTotalLen);
        break;
      }
    }
  }
  
}
/*********************************************************************/
void GetData()
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6); 
  if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;
  
  char cmd[] = { 0xaa, 0x0e, 0x00, 0x00, 0x00, 0x00 };  
  unsigned char pkt[PIC_PKT_LEN];
  
  char picName[] = "pic00.jpg";
  picName[3] = picNameNum/10 + '0';
  picName[4] = picNameNum%10 + '0';
  
  if (SD.exists(picName))
  {
    SD.remove(picName);
  }
  
  myFile = SD.open(picName, FILE_WRITE); 
  if(!myFile){
    Serial.println("myFile open fail...");
  }
  else{
    for (unsigned int i = 0; i < pktCnt; i++)
    {
      cmd[4] = i & 0xff;
      cmd[5] = (i >> 8) & 0xff;
      
      int retry_cnt = 0;
    retry:
      delay(10);
      clearRxBuf(); 
      sendCmd(cmd, 6);
      uint16_t cnt = readBytes((char *)pkt, PIC_PKT_LEN, 200);
      
      unsigned char sum = 0; 
      
      for (int y = 0; y < cnt - 2; y++)
      {
        sum += pkt[y];
      }
      if (sum != pkt[cnt-2])
      {
        if (++retry_cnt < 100) goto retry;
        else break;
      }
      else
      {
        Serial.print("--> chksum pass: ");
        Serial.print(pkt[cnt-2], HEX);
        Serial.print(" == ");
        Serial.println(sum, HEX);
      }
      
      //Serial.println(pkt[2], DEC);
      Serial.println(cnt);
      for (int frame = 4; frame < cnt - 2; frame++)
      {
        Serial.print(" ");
        
        Serial.print(pkt[frame], HEX);
        Serial.print(' ');
        
        myFile.write((uint8_t)pkt[frame]);
      }
      Serial.println("written.");
      //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0; 
    sendCmd(cmd, 6); 
  }
  Serial.print("Total packages: ");
  Serial.println(pktCnt, DEC);
  Serial.print("Package size: ");
  Serial.println(PIC_PKT_LEN, DEC);
  myFile.close();
  picNameNum ++;
}
