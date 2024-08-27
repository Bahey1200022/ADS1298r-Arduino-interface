#include <SPI.h>

#define READ       0x20
#define WRITE      0x40

#define ID         0x00
#define CONFIG1    0x01
#define CONFIG2    0x02
#define CONFIG3    0x03
#define LOFF       0x04
#define RDATA      0x12

uint32_t channelValues[3]; // Array to store channel readings

const int csPin = 10;  // Chip Select pin
const int drdyPin = 6; // Data Ready pin
const int resetPin = 4; // Reset pin

const int DIN = 11; // DIN
const int DOUT = 12; // DOUT
const int CLK1 = 13; // SCLK



void ADS1298rInit() {
    send_command(0x02); // WAKEUP command
    delay(1000);
    send_command(0x06); // RESET command
    delay(1000);

    send_command(0x11); // SDATAC command (Stop Data Continuous Mode)
    delay(1000);
}

void beginSPI() {
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
}

uint8_t readRegister(byte reg) {
    uint8_t result, result2;

    digitalWrite(csPin, LOW); // Select ADS1298
    delayMicroseconds(5); // Small delay after selecting the chip

    SPI.transfer(READ | reg); // Send read command combined with the register address
    SPI.transfer(0x01); // Number of registers to read minus one (0x00 means read 1 byte)

    result = SPI.transfer(0x00); // Read the register value
    result2 = SPI.transfer(0x00); 

    digitalWrite(csPin, HIGH); // Deselect ADS1298

    Serial.print("BYTE 1: ");
    Serial.println(result, HEX);
    // Serial.print("BYTE 2: ");
    // Serial.println(result2, HEX);

    return result;
}

void send_command(uint8_t cmd) {
    digitalWrite(csPin, LOW);
    delayMicroseconds(5); // Small delay after selecting the chip
    SPI.transfer(cmd);
    delayMicroseconds(10);
    digitalWrite(csPin, HIGH);
    delay(10);
}

void writeRegister(byte reg, byte value) {
    digitalWrite(csPin, LOW); // Select ADS1298
    delayMicroseconds(5); // Small delay after selecting the chip
    SPI.transfer(WRITE | reg); // Write command combined with register address
    SPI.transfer(0x01);

    SPI.transfer(value); // Send the value to write
    digitalWrite(csPin, HIGH); // Deselect ADS1298
    delay(10);
}

void ADS1298rSettings() {
    writeRegister(CONFIG1, 0b10000010);  //0b10100110
    delay(10);
    writeRegister(CONFIG2, 0x10); // 0b00001000
    delay(10);
    writeRegister(CONFIG3, 0xEC);
    delay(10);
    writeRegister(LOFF, 0x03);
    delay(10);
    //CH1-CH8
    writeRegister(0x05, 0x00);  //CH1SET//0x00
    delay(10);
    writeRegister(0x06, 0x00); // 2 0b01100000
    delay(10);
    writeRegister(0x07, 0x00);  // 3
    delay(10);
    writeRegister(0x08, 0x00);  // 4
    delay(10);
    writeRegister(0x09, 0x00); //5
    delay(10);
    writeRegister(0x0A, 0x00);  // 6
    delay(10);
    writeRegister(0x0B, 0x00);   // 7  
    delay(10);
    writeRegister(0x0C, 0x00);  //CH8SET   //0x00
    delay(10);

    writeRegister(0x0D, 0x2F);  //RLD_SENSP  //0x00
    delay(10);
    writeRegister(0x0E, 0x2F);  //RLD_SENSN  //0x00  
    delay(10);
    writeRegister(0x0F, 0xFF);  //LOFF_SENSP //0xFF
    delay(10);
    writeRegister(0x10, 0x02);  //LOFF_SENSN //0x02
    delay(10);
    writeRegister(0x11, 0x00);  //LOFF_FLIP  //0x00
    delay(10);
    writeRegister(0x12, 0x00);  //LOFF_STATP //0x00
    delay(10);

    
    writeRegister(0x13, 0x00);  //LOFF_STATN //0x00
    delay(10);
    writeRegister(0x14, 0x00);  //GPIO       //0x00
    delay(10);
    writeRegister(0x15, 0x00);  //PACE       //0x00
    delay(10);
    writeRegister(0x16, 0x00);  //RESP       //0x00
    delay(10);
    writeRegister(0x17, 0x00);  //CONFIG4    //0x00
    delay(10);
    writeRegister(0x18, 0x00);  //WCT1       //0x00
    delay(10);
    writeRegister(0x19, 0x00);  //WCT2       //0x00
    delay(10);
}

void setup() {
    pinMode(csPin, OUTPUT);
    pinMode(drdyPin, INPUT);
    pinMode(resetPin, OUTPUT);
    pinMode(DIN, INPUT);
    pinMode(DOUT, OUTPUT);
    pinMode(CLK1, OUTPUT);

    digitalWrite(csPin, HIGH); // Set CS high to start
    delay(10);

    digitalWrite(resetPin, LOW); // Reset the ADS1298R
    delay(10);
    digitalWrite(resetPin, HIGH); // Bring out of reset
    delay(10);

    delay(10);

    SPI.begin();
    beginSPI();

    Serial.begin(19200); // Start serial communication at 2400 baud rate

    ADS1298rInit();
    delay(10);

    ADS1298rSettings();
    delay(10);
}

void readChannels() {
    if (digitalRead(drdyPin) == LOW) { // Check if DRDY is low (data ready)
        digitalWrite(csPin, LOW); // Select ADS1298
        delayMicroseconds(5); // Small delay after selecting the chip

        SPI.transfer(RDATA); // Send RDATA command

        // Read the data for channels 1 to 3 (24 bits each) +status register 
        for (int channel = 0; channel < 4; channel++) {
            uint32_t value = 0;
            for (int i = 0; i < 3; i++) { // Read 3 bytes (24 bits)
                value = (value << 8) | SPI.transfer(0x00); // 8 bits << 8 bits << 8 bits
            }

            if (channel > 1) {
                channelValues[channel - 2] = value; // Store the raw value in the array
            }
        }


        //  Serial.print("\tI:"); Serial.println(channelValues[0]);
        // Serial.print("\tII:"); Serial.print(channelValues[1]);
        // Serial.print("\tIII:"); Serial.println(channelValues[1] - channelValues[0]);

                Serial.println(channelValues[0]); // Send channelValues[0] over serial


        digitalWrite(csPin, HIGH); // Deselect ADS1298
    }
}

// CH1 (Lead I): LA - RA
// CH2 (Lead II): LL - RA
// CH3 (Lead III): LL - LA

void loop() {
    readChannels();
}