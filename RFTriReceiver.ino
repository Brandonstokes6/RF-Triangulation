#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>

//Pycom UART, pins on GPIO 17 = RX1, GPIO 16 = TX1
#define RX1       GPIO_NUM_17 
#define TX1       GPIO_NUM_16 

//Magnetometer SPI, pins on GPIO12, GPIO13, GPIO14, GPIO15
#define VSPI_MISO   12
#define VSPI_MOSI   13
#define VSPI_SCLK   14
#define VSPI_SS     15

//XBee SPI, pins on GPIO19, GPIO23, GPIO18, GPIO5
#define HSPI_MISO   19
#define HSPI_MOSI   23
#define HSPI_SCLK   18
#define HSPI_SS     5
// 1 MHz clock
static const int spiClk = 1000000;

//U-BLOX GPS I2C, pins on GPIO21 and GPIO22
#define I2C_DEV_ADDR 0x55
int sdaPin = 21;
int sclPin = 22;
uint32_t i = 0;

//sets up UART 1
HardwareSerial MySerial1(1); 

// SPI
SPIClass * vspi = NULL;
SPIClass * hspi = NULL;

void setup() {
  // put your setup code here, to run once:
  // Serial monitor baud rate
  Serial.begin(115200); 
  //i2c master setup - ublox
  bool begin(int sdaPin, int sclPin, uint32_t frequency)
  Serial.setDebugOutput(true);
  Wire.begin();

  //spi set up - magnetometer/xbee
  // SPI classes
  vspi = new SPIClass(VSPI);
  hspi = new SPIClass(HSPI);
  //SCLK, MISO, MOSI, SS
  vspi.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  hspi.begin(HSPI_CLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  pinMode(vspi->pinSS(), OUTPUT); //VSPI SS
  pinMode(hspi->pinSS(), OUTPUT); //HSPI SS
  // starts UART peripheral
  // SERIAL_8N1 = data packet = 1 start bit, 8 data bits, 0 parity bits, 1 stop bit
  MySerial1.begin(115200,SERIAL_8N1, RX1, TX1);
}

void loop() {
  delay(5000);

  //i2c master loop - ublox
  //Write message to the slave
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.printf("UBLOX communication %u", i++);
  uint8_t error = Wire.endTransmission(true);
  Serial.printf("endTransmission: %u\n", error);
  
  //Read 16 bytes from the slave
  uint8_t bytesReceived = Wire.requestFrom(I2C_DEV_ADDR, 16);
  Serial.printf("requestFrom: %u\n", bytesReceived);
   //If received more than zero bytes
  if((bool)bytesReceived){
    uint8_t temp[bytesReceived];
    Wire.readBytes(temp, bytesReceived);
    log_print_buf(temp, bytesReceived);

  //spi loop - magnetometer/xbee
  spiCommand(vspi, ""); 
  spiCommand(hspi, "");
  delay(1000);  
}

void spiCommand(SPIClass *spi, byte data) {
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //pull SS slow to prep other end for transfer
  digitalWrite(spi->pinSS(), LOW); 
  spi->transfer(data);
  //pull ss high to signify end of data transfer
  digitalWrite(spi->pinSS(), HIGH); 
  spi->endTransaction();
}
