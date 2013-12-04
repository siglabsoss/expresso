#include <SPI.h>





const int slaveSelectPin = 53;

void setup() {
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
//  SPI.begin();  
//  SPI.setClockDivider(SPI_CLOCK_DIV64);  
//  SPI.setDataMode(SPI_MODE0);
//  //DUH also default
//  SPI.setBitOrder(MSBFIRST);
  
  Serial.begin( 9600 );
  


//  delay(3000);
  
//  sendTestPattern();
}


void sendTestPattern()
{
  // 0x02 is all 1's
  // 0x03 is all 0's
  
//  Serial.println("writing..\n");
//  spiWrite(0x0D, 0x03);
//  Serial.println("done\n");

   spiWrite(0x0D, 0x07);
}


void sendPowerDown()
{
   spiWrite(0x08, 0x01);
}


void loop() {
  // do nothing

   Serial.println("resending");
  
//    sendTestPattern();
//    sendPowerDown();


//  unsigned char testMode[] =
//                        { 0, 0, 0, 0,    0, 0, 0, 0,
//                          0, 0, 0, 0,    1, 1, 0, 1,
//                          0, 0, 0, 0,    0, 0, 1, 0};
//
//  spiWriteArray(testMode, 24);

  
  unsigned char powerDown[] =
                        { 0, 0, 0, 0,    0, 0, 0, 0,
                          0, 0, 0, 0,    1, 0, 0, 0,
                          0, 0, 0, 0,    0, 0, 0, 1};

  spiWriteArray(powerDown, 24);
  
  delay(3);

  unsigned char commitChanges[] =
                        { 0, 0, 0, 0,    0, 0, 0, 0,
                          1, 1, 1, 1,    1, 1, 1, 1,
                          0, 0, 0, 0,    0, 0, 0, 1};

  spiWriteArray(commitChanges, 24);


//spiRead(0,0);
   
   delay(100);
}

void spiRead(int address, int value)
{
  
  
   // master is output
  pinMode(MOSI, OUTPUT);

  
  // CSB low
  digitalWrite(slaveSelectPin,LOW); 
  
  delayMicroseconds(1);
  
   // set high bit for read
   SPI.transfer(0x80);
   
   delayMicroseconds(1);
   
   // address we want to read
   SPI.transfer(0x01);
   
      delayMicroseconds(1);
   
   // send zero because FUCK IT
   SPI.transfer(0x00);

    // float so we can read back (with scope)
    pinMode(MOSI, INPUT);

   delayMicroseconds(1);
    
    int result =  SPI.transfer(0x00);
    int result2 =  SPI.transfer(0x00);
       result2 =  SPI.transfer(0x00);
    
          
    // CSB high
    digitalWrite(slaveSelectPin,HIGH); 
//    
//    Serial.println("read:");
//    Serial.println(result);
//    
//    Serial.println("read2:");
//    Serial.println(result2);
    

    
}

void spiWriteArray(unsigned char* dat, unsigned bits)
{
    
  // master is output
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  

  delay(1);
  digitalWrite(MOSI,LOW);
  digitalWrite(SCK,LOW);
  delay(1);
        // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);

  
  int n;
  delay(1);
  for(n = 0; n < 8; n++)
{
    digitalWrite(MOSI,dat[n]);
  delay(1);
  digitalWrite(SCK,HIGH);
  delay(1);
  digitalWrite(SCK,LOW);
}
  digitalWrite(slaveSelectPin,HIGH);
  delay(1);
  digitalWrite(slaveSelectPin,LOW);
  for(n = 8; n < 16; n++)
{
    digitalWrite(MOSI,dat[n]);
  delay(1);
  digitalWrite(SCK,HIGH);
  delay(1);
  digitalWrite(SCK,LOW);
}
  digitalWrite(slaveSelectPin,HIGH);
  delay(1);
  digitalWrite(slaveSelectPin,LOW);
  for(n = 16; n < 24; n++)
{
    digitalWrite(MOSI,dat[n]);
  delay(1);
  digitalWrite(SCK,HIGH);
  delay(1);
  digitalWrite(SCK,LOW);
}
  
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}

// address is the register address, value is the value
void spiWrite(int address, int value) {
  
  // master is output
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  
//  pinMode(MISO, INPUT);
  // take the SS pin low to select the chip:
      digitalWrite(MOSI,LOW);
      digitalWrite(SCK,LOW);
  digitalWrite(slaveSelectPin,LOW);

  //  send in the address and value via SPI:
  
  
  unsigned char dat[] = { 0, 0, 0, 0,    0, 0, 0, 0,
                          0, 0, 0, 0,    1, 0, 0, 0,
                          0, 0, 0, 0,    0, 0, 0, 1};
  int n;
  delay(1);
  for(n = 0; n < 24; n++)
{
    digitalWrite(MOSI,dat[n]);
  delay(1);
  digitalWrite(SCK,HIGH);
  delay(1);
  digitalWrite(SCK,LOW);
}
  delay(1);
  
  /*SPI.transfer(0x00);
  
  
  SPI.transfer(address);
  SPI.transfer(value);*/
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}



/*
void ad9467_write(uint16_t regAddr, uint8_t regVal)
{
        int32_t ret;
        uint8_t        write_buffer[3];

    regAddr += AD9467_WRITE;

        write_buffer[0] = (uint8_t)((regAddr & 0xFF00) >> 8);
        write_buffer[1] = (uint8_t)(regAddr & 0x00FF);
        write_buffer[2] = regVal;

        ret = SPI_TransferData(spiBaseAddress, 3, (char*)write_buffer, 0, NULL, 
                        spiSlaveSelect);

    return ret;
}
*/

