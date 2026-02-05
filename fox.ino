/*   
 *   Much of this code was copied from / inspired by the project
 *   "rot13labs fox" https://github.com/c0ldbru/fox
 *   which was in turn copied from / inspired by Yet Another Foxbox
 *   (YAFB) by Gregory Stoike (KN4CK) which can be found here:
 *       https://github.com/N8HR/YAFB.
 *
 *   It has been adapted for use with HackerBox 0096 - Two Meter:
 *       https://hackerboxes.com/products/hackerbox-0096-two-meter
 *   Which uses the TTGO ESP32 T-Display and an SA818 VHF transciever.
 *
 *   This project is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This project is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this. If not, see <https://www.gnu.org/licenses/>.
 */

#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();

// hardware I/O pins
#define SA818_AudioOn 17
#define SA818_AF_OUT  38  //Analog input to MCU
#define SA818_PTT     13
#define SA818_PD      27
#define SA818_HL      33
#define SA818_RXD     21
#define SA818_TXD     22
#define SA818_AF_IN 	25  //Analog output from MCU
#define Button_A      15
#define Button_B      26
#define Button_C      32

String callmessage = "KD0MLN fox. If found call for info."; // your callsign goes here
String morse = ""; // leave this blank for now; it will be filled in during setup
float frequency = 146.550; // 146.565 is the normal TX frequency for foxes

void setup(void) {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, SA818_TXD, SA818_RXD);

  morse = createMorse(callmessage);

  pinMode(Button_A, INPUT_PULLDOWN);
  pinMode(Button_B, INPUT_PULLDOWN);
  pinMode(Button_C, INPUT_PULLDOWN);

  pinMode(SA818_AudioOn, INPUT);  //SA818 drives pin LOW to tell MCU that a signal is being received

  // SA818_AF_IN is an output from the MCU, but we set it as an INPUT when not in use
  pinMode(SA818_AF_IN, INPUT);

  //Output Power High/Low
  //  High Power: Float (MCU=input)
  //  Low Power: Drive pin LOW
  //  INPORTANT NOTE: NEVER Drive the pin HIGH (undefined state)
  pinMode(SA818_HL, OUTPUT);   
  digitalWrite(SA818_HL, LOW);   
  
  pinMode(SA818_PTT, OUTPUT);      //Push to Talk (Transmit)
  digitalWrite(SA818_PTT, HIGH);   //(0=TX, 1=RX) DO NOT Transmit Without License

  pinMode(SA818_PD, OUTPUT); 
  digitalWrite(SA818_PD, HIGH);    //Release Powerdown (1=Radio ON)

  tft.init();
  tft.setRotation(3);
  
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
  tft.setTextColor(TFT_RED); 
  tft.setCursor(0, 60, 4);
  tft.println("     2m FOX HUNT");
  delay(3000);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  tft.setCursor(0, 0, 4);
  tft.println("Radio Connect:");
  Serial2.println("AT+DMOCONNECT\r");  // connect communications with SA818
  tft.print(Serial2.readString());     // returns :0 if good
  tft.println("Radio Version:");
  Serial2.print("AT+VERSION\r\n");     // get version
  tft.print(Serial2.readString());     // returns: version number
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 4);
  tft.println("Set Group:");           // group set
  setfreq();                           // set frequency
  tft.print(Serial2.readString());     // returns :0 if good
  
  tft.println("Set Volume: ");
  Serial2.print("AT+DMOSETVOLUME=8\r\n"); // set volume
  tft.print(Serial2.readString());        // returns :0 if good
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  disp_channel();
}

void loop() {
  digitalWrite(SA818_PTT,LOW);      // assert push to talk
  delay(750);
  playMelody();
  delay(750);
  playMorse();
  digitalWrite(SA818_PTT,HIGH);     // release push to talk
  delay(30000); 
}

void setfreq() {
  Serial2.print("AT+DMOSETGROUP=0,");
  Serial2.print(String(frequency,4));
  Serial2.print(",");
  Serial2.print(String(frequency,4));
  Serial2.print(",0000,1,0000\r\n");
}

void disp_channel() {
  tft.setCursor(0, 0, 4);
  tft.setTextColor(TFT_RED,TFT_BLACK);
  tft.println("Ham Radio Fox Hunt");
  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  tft.println("Leave in place, or ");
  tft.println("Ask KD0MLN (Tron)");
  tft.println("for more info");
  tft.setTextColor(TFT_BLUE,TFT_BLACK);
  tft.print("Freq: ");
  tft.println(String(frequency,4));  
}
