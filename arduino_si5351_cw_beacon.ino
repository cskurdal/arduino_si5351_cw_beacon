// Fork of code from Z33T: https://www.youtube.com/watch?v=3EChVmHqaGU
//  https://drive.google.com/drive/folders/1y-XUIgxe68LVmUknwdRTXC5v-bblNKVL
// CW Beacon using Si5351

//---------------------------------------------------------------------------------------------------------
#include "si5351.h"


#define PIN_SP 0 // Speaker pin. 0 to disable
#define PIN_TX 0 // LED pin. 0 to disable
#define CHARS_PER_LINE 19 //for SSD1306 scrolling display of sent characters

#include <U8g2lib.h>

//If using Heltec WiFi Kit
//https://robotzero.one/heltec-wifi-kit-32/
// #define SCK 5
// #define SDA 4
// the OLED used
//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCK, /* data=*/ SDA, /* reset=*/ 16);

/*
 * Wemos D1 Mini with SSD1306 0.96 OLED 128x64 pixel screen
 * On the D1 Mini pin D1 is SCL and D2 is SDA
 * U8G2_R0 => normal U8G2_R2 => rotated 180 degrees
*/
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCK, /* data=*/ SDA); // page buffer mode
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2); // Use "full buffer mode" since there's enough memory

//Setup the SI5351
Si5351 si5351(0x60);

uint16_t duration = 75;    // Typing speed of Morse code in milliseconds - higher number means slower typing
uint16_t hz = 750;         // Висина на тонот доколку се приклучи високоомски звучник за контрола на работата на морзе-генераторот - Volume up if a high ohm speaker is connected to control the operation of the wall generator
String cw_message = "VVV de CALLSIGN/B = LOCATOR IS XY00 = PWR IS 10mW = ANT IS VERT ";   // Текст на пораката - Message text

/*
 * YOU MUST HAVE A LICENSE TO TRANSMIT
 */
uint32_t tx = 144480000; // Radio Frequency Output Frequency in Hertz

//Counter for serial/display
uint32_t count = 0;

//Variables for serial and SSD1306 display
char cstr[16];
char str_buf[CHARS_PER_LINE];
String display_line = "";


//---------------------------------------------------------------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  u8g2.begin();
  
  u8g2.setFont(u8g2_font_7x14_tf);  
  u8g2.setCursor(15, 15);
  u8g2.print("Hello world!");
  u8g2.sendBuffer();
  
  delay(1000);
  
  pinMode(PIN_TX, OUTPUT);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25002152, 0);
  si5351.set_freq(tx * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 0);
}
//---------------------------------------------------------------------------------------------------------
void loop() {
  count++;
  display_line = ""; //reset

  //Set some different fonts and print the count
  if (true)
  {
    itoa(count, cstr, 10); //convert int into cstr for serial command
    Serial.print("count: ");
    Serial.println(count);

    //print
    u8g2.clearBuffer();          // clear the internal memory
    
    u8g2.setFont(u8g2_font_7x14_tf); // choose a suitable font
    u8g2.drawStr(0,10,"Hello World!");  // write something to the internal memory
    
    u8g2.setFont(u8g2_font_ncenB08_tr);  
    u8g2.drawStr(0, 20, "Freq: ");
    itoa(tx, cstr, 10);
    u8g2.drawStr(30, 20, cstr);
    
    u8g2.setFont(u8g2_font_ncenB14_tr);  
    u8g2.drawStr(0,40, "Count:");
    
    //u8g2.setFont(u8g2_font_chikita_tf);
    u8g2.setFont(u8g2_font_7x14_tf);
    itoa(count, cstr, 10);
    u8g2.drawStr(75,40, cstr);
    
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(0,60, "Sending:");
    
    u8g2.sendBuffer(); // transfer internal memory to the display
    delay(5000); //pause for 5 seconds
  }

  //Send the 
  if (true) {
    cw_string_proc(cw_message);
    delay(500);                           // Времетраење на паузата на крајот пред долгиот сигнал - во милисекунди
  
    cw(true);
    delay(10000);                         // Времетраење на долгиот сигнал на крајот - во милисекунди
  
    cw(false);
    delay(1000);                          // Времетраење на пауза на крајот по долгиот сигнал - во милисекунди
  }
}
//---------------------------------------------------------------------------------------------------------
void cw_string_proc(String str) {                      // processing string to characters
  for (uint8_t j = 0; j < str.length(); j++)
    cw_char_proc(str[j]);
}
//---------------------------------------------------------------------------------------------------------
void cw_char_proc(char m) {                            // processing characters to Morse code symbols
  String s;

  //CES 20210417 - Scroll the characters
  display_line += m;

  if (display_line.length() > CHARS_PER_LINE)
    display_line = display_line.substring(display_line.length() - CHARS_PER_LINE);
 
  if (true) {
    display_line.toCharArray(str_buf, CHARS_PER_LINE); // copy 16 chars into array
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_7x14_tf);
    //u8g2.drawStr(50,60, (char*)&m);
    u8g2.drawStr(0,60, str_buf);
    
    u8g2.sendBuffer();          // transfer internal memory to the display
  }
  
  if (m == ' ') {                                      // Пауза помеѓу зборовите
    word_space();
    return;
  }

  if (m > 96)                                          // ACSII, case change a-z to A-Z
    if (m < 123)
      m -= 32;

  switch (m) {                                         // Morse code
    case 'A': s = ".-#";     break;
    case 'B': s = "-...#";   break;
    case 'C': s = "-.-.#";   break;
    case 'D': s = "-..#";    break;
    case 'E': s = ".#";      break;
    case 'F': s = "..-.#";   break;
    case 'G': s = "--.#";    break;
    case 'H': s = "....#";   break;
    case 'I': s = "..#";     break;
    case 'J': s = ".---#";   break;
    case 'K': s = "-.-#";    break;
    case 'L': s = ".-..#";   break;
    case 'M': s = "--#";     break;
    case 'N': s = "-.#";     break;
    case 'O': s = "---#";    break;
    case 'P': s = ".--.#";   break;
    case 'Q': s = "--.-#";   break;
    case 'R': s = ".-.#";    break;
    case 'S': s = "...#";    break;
    case 'T': s = "-#";      break;
    case 'U': s = "..-#";    break;
    case 'V': s = "...-#";   break;
    case 'W': s = ".--#";    break;
    case 'X': s = "-..-#";   break;
    case 'Y': s = "-.--#";   break;
    case 'Z': s = "--..#";   break;

    case '1': s = ".----#";  break;
    case '2': s = "..---#";  break;
    case '3': s = "...--#";  break;
    case '4': s = "....-#";  break;
    case '5': s = ".....#";  break;
    case '6': s = "-....#";  break;
    case '7': s = "--...#";  break;
    case '8': s = "---..#";  break;
    case '9': s = "----.#";  break;
    case '0': s = "-----#";  break;

    case '?': s = "..--..#"; break;
    case '=': s = "-...-#"; break;
    case ',': s = "--..--#"; break;
    case '/': s = "-..-.#";  break;
  }

  for (uint8_t i = 0; i < 7; i++) {
    switch (s[i]) {
      case '.': ti();  break;                          // TI
      case '-': ta();  break;                          // TA
      case '#': char_space(); return;                  // end of Morse code symbol
    }
  }
}
//---------------------------------------------------------------------------------------------------------
void ti() {
  cw(true);                                            // TX TI
  delay(duration);

  cw(false);                                           // stop TX TI
  delay(duration);
}
//---------------------------------------------------------------------------------------------------------
void ta() {
  cw(true);                                            // TX TA
  delay(3 * duration);

  cw(false);                                           // stop TX TA
  delay(duration);
}
//---------------------------------------------------------------------------------------------------------
void char_space() {                                    // 3x, пауза помеѓу буквите
  delay(2 * duration);                                 // 1 from element-end + 2 new
}
//---------------------------------------------------------------------------------------------------------
void word_space() {                                    // 7x, пауза помеѓу зборовите
  delay(6 * duration);                                 // 1 from element-end + 6 new
}
//---------------------------------------------------------------------------------------------------------
void cw(bool state) {                                  // TX-CW, TX-LED, 750 Hz sound
  if (state) {
    si5351.output_enable(SI5351_CLK0, 1);

    if (PIN_TX > 0)
      digitalWrite(PIN_TX, HIGH);

    if (PIN_SP > 0)
      tone(PIN_SP, hz);
  }
  else {
    si5351.output_enable(SI5351_CLK0, 0);
    
    if (PIN_TX > 0)
      digitalWrite(PIN_TX, LOW);

    if (PIN_SP > 0)
      noTone(PIN_SP);
  }
}
