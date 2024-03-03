/**********************************************************************************************/
/*	                                      Orbion Firmware                                     */
/**********************************************************************************************/
/*                                        version 0.26.5                                      */
/*                                       Since: June 2021                                     */
/*                                Creator: Mattia Russo (FaqTotum)                            */
/**********************************************************************************************/
/*                                           LICENSE                                          */
/*                                                                                            */
/*                                      (CC BY-NC-ND 4.0)                                     */
/*               CC - Attribution-NonCommercial-NoDerivatives 4.0 International               */
/**********************************************************************************************/

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
@KamoshikaCamper
Qiita上の説明サイト
https://qiita.com/kamoshika9999/items/3424c062310145d481c8

このコードはオリジナルのファームを改変しシリアル通信によってAutoDesk Fusion上のAPIと連携させ
マウス/キーボードエミュレーションでは無く、完全な左手デバイス独立デバイスとして機能させる。
従ってFusion以外では使用出来ない

Fusion側にもアドインを設定する必要がある
Fusion APIで使用しているPython側にもpyserialをインストールする必要がある
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

#include <string.h>
#include "Oled.h"
#include "Bitmap.h"
#include "Settings.h"
#include "Arduino.h"
#include <EEPROM.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <Wire.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

Adafruit_NeoPixel pixels(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);
byte posLed = 0;
bool cycle = LOW;

bool timeoff = LOW;
bool timeoffent = LOW;

/////////////// Encoder ////////////////////

bool rot = HIGH;
bool exo = HIGH;  
bool encBefClick = 0;     

/////////////// JoyStick ///////////////////

uint8_t smooth = 0;
bool moved = LOW;        
int YZero, XZero = 0;     
int YValue, XValue = 0;   
uint8_t sens = 0;         
uint8_t arSens[20] = {20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};   

uint16_t tim, h, tim1, h1, tim2, h2, tim3, h3;

////////// Rear/Knob Button  //////////////// 

uint8_t butFunBef = LOW;   
const char arButt [36] = {'q','w','e','r','t','y','u','i','o','p','a','s','d','f','g','h','j','k','l','z','x','c','v','b','n','m','1','2','3','4','5','6','7','8','9','0'};
bool bt1_flg = false;
bool bt2_flg = false;
bool bt3_flg = false;
bool jb_flg = false;
uint16_t bt_tim,bt1_t,bt2_t,bt3_t,jb_t;

/////////////// MENU ////////////////////////

uint8_t epr = 0;
uint8_t i = 0;


void ringKnob (uint8_t *p)
  {
    if(posLed == NUMPIXELS)
      {
        posLed=0;
      }
    if(posLed > 250)
      {
        posLed=6;
      }
    else
      {
        posLed=posLed-*p;
      }
    pixels.clear();
    pixels.setPixelColor(posLed, pixels.Color(255, 255, 255));
    pixels.show();
  }

//////////////////////////////////////////////////////////////////////////// SCROLL ///////////////////////////////////            

uint8_t scroll (void)
  {
    char buf[16];
    uint8_t ret = 0;
    bool scr = digitalRead(DT);
    bool scr2 = digitalRead(CLK);
    uint16_t timS;
    uint16_t hS = 0;
    timS = millis() - hS;
    if(scr != scr2)
      {
        timeoff = LOW;
      }
    if (timS > 10)
      {
      hS = millis();
      if(rot == HIGH)
        {
          ret = scr-scr2;
          if(exo==HIGH)
            {
              //ret = scr2-scr; // invert scroll (only main)
               if( EEPROM.read(10) != 10){
                Mouse.move(0, 0, ret );
                Mouse.move(0, 0, ret );
               }else{
                if( ret != 0 ){
                  sprintf(buf,"%d",ret);
                  Serial.print("S=");
                  Serial.println(buf);
                }
               }
            }
          rot=!rot; 
        }
      if(scr==HIGH && scr2==HIGH)
        {
          rot=!rot; 
        }
      if((cycle==HIGH) && (timeoffent == LOW))
        {
          ringKnob(&ret);
        }
    }
    return ret;
  }

///////////////////////////////////////////////////////////////////////////// NEOPIXEL /////////////////////////////////

void setLED(uint8_t *e)
  {
    const String color[11] = {"Off", "Red", "Green", "Blue", "Yellow", "White", "Purple", "Sky", "Pink", "Orange", "Cycle"};
    uint8_t item = 10;
    bool ex = LOW;
    uint8_t t = EEPROM.read(*e);
    delay(300);
    do 
      {
      display.clearDisplay();
      if(SSD1306==LOW)
        {
          display.setTextColor(SH110X_WHITE);
        }
      else if (SSD1306==HIGH)
        {
          display.setTextColor(WHITE);
        }
      display.setTextSize(2);
      display.setCursor(36,10);
      display.println("Color");
      switch (t)
        {
          case 0:
            display.setCursor(45,35);
          break;
          case 1:
            display.setCursor(45,35);
          break;
          case 2:
            display.setCursor(37,35);
          break;
          case 3:
            display.setCursor(39,35);
          break;
          case 4:
            display.setCursor(32,35);
          break;
          case 5:
            display.setCursor(36,35);
          break;
          case 6:
            display.setCursor(32,35);
          break;
          case 7:
            display.setCursor(45,35);
          break;
          case 8:
            display.setCursor(40,35);
          break;
          case 9:
            display.setCursor(32,35);
          break;
          case 10:
            display.setCursor(37,35);
          break;
        }
      display.print(color[t]);
      display.display();
      if(t == 1+item)
          {
            t=0;
          }
        if(t > 250)
          {
            t=item;
          }
        else
          {
            t=t+scroll();
          }
      if(digitalRead(joyButt)==LOW)
        {
        ex=HIGH;
        EEPROM.update(*e, t);
        }
      }
    while (ex==LOW);
    delay(300);
    ex=LOW;
  }

void neopixel (void)
  {
    pixels.clear();
    /*
      for(byte a=0; a<NUMPIXELS; a++)
      {
        pixels.setPixelColor(a, pixels.Color(0, 0, 0));//Off
      }  
    */
    switch(EEPROM.read(45))
      {
        case 0:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(0, 0, 0));//Off
            }          
        break;
        
        case 1:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(255, 0, 0));//Red
            }          
        break;

        case 2:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(0, 255, 0));//Green
            }          
        break;

        case 3:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(0, 0, 255));//Blue
            }          
        break;

        case 4:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(200, 130, 0));//Yellow
            }          
        break;

        case 5:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(255, 255, 255));//White
            }          
        break;

        case 6:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(100, 0, 255));//Purple
            }          
        break;

        case 7:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(100, 130, 255)); //Cyan
            }          
        break;

        case 8:
          for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(255, 0, 255));//Pink
            }  
        break;

        case 9:
           for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(255, 70, 0));//Orange
            }          
        break;

        case 10:
           cycle=HIGH;          
        break;
      }
    if(EEPROM.read(45) != 10)
      {
        cycle=LOW;
      }
      
    
    pixels.show();
  }

void setup() 
  {
    pinMode (horzPin, INPUT_PULLUP);        
    pinMode (vertPin, INPUT_PULLUP);       
    pinMode (button1, INPUT_PULLUP);
    pinMode (button2, INPUT_PULLUP);
    pinMode (button3, INPUT_PULLUP);      
    pinMode (joyButt, INPUT_PULLUP);  
    pinMode (DT, INPUT_PULLUP);
    pinMode (CLK, INPUT_PULLUP);
    
    YZero = analogRead(vertPin);  
    XZero = analogRead(horzPin);  
    
    Mouse.begin();      
    Keyboard.begin();   

     //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);          // Uncomment if your oled is SSD1306
    display.begin(0x3C, true); // Address 0x3C default     // comment if your oled is SSD1306
    
    display.setRotation(2);
    display.clearDisplay();
    do
      {
        display.clearDisplay();
        display.drawBitmap(0, 0, faq, 12*i, 6*i, 1);
        display.display();
        i++;
      }
    while(i != 20);  
    display.clearDisplay();
    display.drawBitmap(0, 0, faq, 128, 64, 1);
    display.display();
    Serial.begin(115200);
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.show();
    neopixel();
  }
  
//////////////////////////////////////////// BUTTON MODE ////////////////////////////////

void buttMode(uint8_t *e)
  {
    bool ex = LOW;
    uint8_t t = EEPROM.read(*e);
    uint8_t item = 45;
    delay(500);
    do 
      {
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(2);
        display.setCursor(10,10);
        display.println("SELECT FN");
        if(t == 1+item)
          {
            t=0;
          }
        if(t > 250)
          {
            t=item;
          }
        else
          {
            t=t+scroll();
          }
        if(t <= 35)
          {
            display.setCursor(58,35);
            display.print(arButt[t]);
          }
        switch (t)
          {
            case 36:
                display.setCursor(36,35);
                display.print("Space");
            break;
            
            case 37:
                display.setCursor(45,35);
                display.print("CTRL");
            break;
            
            case 38:
                display.setCursor(37,35);
                display.print("Shift");
            break;
            
            case 39:
                display.setCursor(48,35);
                display.print("Alt");
            break;
            
            case 40:
                display.setCursor(48,35);
                display.print("Tab");
            break;
            
            case 41:
                display.setCursor(49,35);
                display.print("Esc");
            break;
            
            case 42:
                display.setCursor(49,35);
                display.print("Del");
            break;
            
            case 43:
                display.setCursor(31,35);
                display.print("Scroll");
            break;
            
            case 44:
                display.setCursor(10,35);
                display.print("Solid W/E");
            break;
            
            case 45:
                display.setCursor(41,35);
                display.print("ANSA");
            break;
          }
        display.display();
        if(digitalRead(joyButt)==LOW)
          {
            ex = HIGH;
            EEPROM.update(*e, t);
          }
      }
    while (ex == LOW);
    delay(300);
  }

void selButt(uint8_t *e)
  {
  switch (EEPROM.read(*e))
      {
        case 36:
          Keyboard.press(32);          
        break;
  
        case 37:
          Keyboard.press(KEY_LEFT_CTRL);
        break;
  
        case 38:
          Keyboard.press(KEY_LEFT_SHIFT);
        break;
  
        case 39:
          Keyboard.press(KEY_LEFT_ALT);
        break;
  
        case 40:
          Keyboard.press(KEY_TAB);
        break;
  
        case 41:
          Keyboard.press(KEY_ESC);
        break;
  
        case 42:
          Keyboard.press(KEY_DELETE);
        break;

        case 43:
          Mouse.press(MOUSE_MIDDLE);
        break;

        case 44:
          Keyboard.press(KEY_LEFT_GUI);
          Mouse.press(MOUSE_MIDDLE);
        break;
        
        case 45:
          Keyboard.press(KEY_LEFT_CTRL);
          Mouse.press(MOUSE_MIDDLE);
        break;
      } 
  }

///////////////////////////////////////////// JOY SENSE /////////////////////////////////

void joySens(void)
  {
  bool ex = LOW;
  uint8_t t = EEPROM.read(0);
  uint8_t item = 19;
  delay(300);
  do 
    {
      if(t == 1+item)
        {
          t=0;
        }
      if(t > 250)
        {
          t=item;
        }
      else
        {
          t=t+scroll();
        }
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(2);
      display.setCursor(10,10);
      display.println("Joy Sense");
      display.setCursor(56,35);
      display.print(t+1);
      display.display();
      if(digitalRead(joyButt)==LOW)
        {
        ex=HIGH;
        EEPROM.update(0, t);
        }
    }
  while (ex==LOW);
  delay(300);
  ex=LOW;
  }

///////////////////////////////////////////// brightness /////////////////////////////////

uint8_t brightness(void)
  {
  bool ex = LOW;
  uint8_t t = EEPROM.read(50);
  uint8_t item = 10;
  uint8_t brt = 0;
  delay(300);
  do 
    {
      if(t == 1+item)
        {
          t=0;
        }
      if(t > 250)
        {
          t=item;
        }
      else
        {
          t=t+scroll();
        }
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(2);
      display.setCursor(29,10);
      display.println("Bright");
      display.setCursor(56,35);
      display.print(t);
      display.display();
      brt = map(t, 0, 10, 0, 255);
      Serial.println(brt);
      if(digitalRead(joyButt)==LOW)
        {
        ex=HIGH;
        EEPROM.update(50, t);
        }
    }
  while (ex==LOW);
  delay(300);
  ex=LOW;
  return brt;
  }

///////////////////////////////////////////// JOY MODES /////////////////////////////////

void joyMode(void)
  {
    uint8_t t = EEPROM.read(10);
    bool ex = LOW;
    uint8_t item = 10;
    delay(300);
    do 
      {
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(2);
        display.setCursor(16,10);
        display.println("Joy Mode");
        if(t == 1+item)
          {
            t=0;
          }
        if(t > 250)
          {
            t=item;
          }
        else
          {
            t=t+scroll();
          }
        switch (t)
          {
            case 0:
              display.setCursor(33,35);
              display.print("Mouse");
            break;
  
            case 1:
              display.setCursor(17,35);
              display.print("Autodesk");
            break;
            
            case 2:
              display.setCursor(10,35);
              display.print("Solid W/E");
            break;
  
            case 3:
              display.setCursor(23,35);
              display.print("Blender");
            break;
  
            case 4:
              display.setCursor(33,35);
              display.print("Adobe");
            break;
  
            case 5:
               display.setCursor(23,35);
               display.print("FreeCad");
            break;
  
            case 6:
              display.setCursor(33,35);
              display.print("Rhino");
            break;
  
            case 7:
              display.setCursor(28,35);
              display.print("ZBrush");
            break;
  
            case 8:
              display.setCursor(41,35);
              display.print("ANSA");
            break;
  
            case 9:
              display.setCursor(23,35);
              display.print("Onshape");
            break;
            case 10:
              display.setCursor(23,35);
              display.print("Serial");
            break;
          }
        display.display();                                 
        if(digitalRead(joyButt)==LOW)
          {
            ex = HIGH;
            EEPROM.update(10, t);
          }
      }
    while (ex == LOW);
    delay(300);
  }

void selModes(void)
  {
  switch (EEPROM.read(10))
    {
      case 0:
        //mouse mode
      break;

      case 1:
        //Autodesk
        Keyboard.press(KEY_LEFT_SHIFT);
        delay(3);
        Mouse.press(MOUSE_MIDDLE);
      break;

      case 2:
        //Solid Work/Edge
        Mouse.press(MOUSE_MIDDLE);
      break;

      case 3:
        //Blender
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_LEFT_ALT);
        Mouse.press(MOUSE_MIDDLE);
      break;

      case 4:
        //Adobe
        Keyboard.press(32);
        Mouse.press(MOUSE_LEFT);
      break;

      case 5:
        //Freecad
        Mouse.press(MOUSE_MIDDLE);
        Mouse.press(MOUSE_RIGHT);
      break;

      case 6:
        //Rhino
        Mouse.press(MOUSE_RIGHT);
      break;

      case 7:
        //ZBrush
        Mouse.press(MOUSE_LEFT);
      break;

      case 8:
        //ANSA
        Keyboard.press(KEY_LEFT_CTRL);
        Mouse.press(MOUSE_LEFT);
      break;

      case 9:
        //Onshape
        Mouse.press(MOUSE_RIGHT);
      break;
      case 10:
        //Serial
      break;

    }  
  }

//////////////////////////////////////////////  MENU /////////////////////////////////////

void menu(void)
  {
   byte sel = 0;
   exo = LOW;
   uint8_t item = 8;
   display.clearDisplay();
   display.setTextColor(SH110X_WHITE);
    do
      {
        if(sel == 1+item)
          {
            sel=0;
          }
        if(sel > 250)
          {
            sel=item;
          }
        else
          {
            sel=sel+scroll();
          }
        switch (sel)
          {
            case 0:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(11,22);
              display.println("Joy Sense");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  joySens();
                }
            break;
      
            case 1:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(16,22);
              display.println("Joy Mode");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  joyMode();
                }
            break;
      
            case 2:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(10,22);
              display.println("Knob Push");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  epr = 35;
                  buttMode(&epr);
                }
            break;
      
            case 3:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(17,22);
              display.println("Button 1");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  epr = 20;
                  buttMode(&epr);
                }
            break;

            case 4:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(17,22);
              display.println("Button 2");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  epr = 23;
                  buttMode(&epr);
                }
            break;

            case 5:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(17,22);
              display.println("Button 3");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  epr = 27;
                  buttMode(&epr);
                }
            break;
    
            case 6:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(16,22);
              display.println("Lighting");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  epr = 45;
                  setLED(&epr);
                  neopixel();
                }
            break;
      
            case 7:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(29,22);
              display.println("Bright");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  pixels.setBrightness(brightness());
                  neopixel();
                }
            break;

            case 8:
              display.clearDisplay();
              display.setTextSize(2);
              display.setCursor(40,22);
              display.println("EXIT");
              display.setTextSize(1);
              display.setCursor(45,45);
              display.println("0.26.5");
              display.display();
              if(digitalRead(joyButt)==LOW)
                {
                  exo = HIGH;
                }
            break;
          }
      } while (exo==LOW); 
  }

///////////////////////////////////////////  Rear Button /////////////////////////////////

void button (uint8_t *a)
  {
    timeoff = LOW;
    if (EEPROM.read(*a) <= 35) 
      {
        Keyboard.press(arButt [EEPROM.read(*a)]);
      }
    else if (EEPROM.read(*a) >= 36) 
      {
        selButt(&*a);
      }
    butFunBef = HIGH;
  }

//////////////////////////////////////////////  LOOP /////////////////////////////////////


void loop() 
  {
    char buf1[30] = "Y=0";
    char buf2[15] = "X=0";
    const char sep[] = ",";


//////////////////////////////////////////////////////////////////////////////// BUTTON ////////////////////////////////////

    bt_tim = millis();
    if(digitalRead(button1) == LOW && !bt1_flg && bt_tim-bt1_t>300)
      {
         bt1_flg = true;
         bt1_t = bt_tim;
         if(EEPROM.read(10) == 10){
          Serial.println("BT1");
         }else{
          epr=20;
          button (&epr);
         }
      }
      if(digitalRead(button1) == HIGH)
      {
        bt1_flg = false;
      }

    if(digitalRead(button2) == LOW && !bt2_flg && bt_tim-bt2_t>300)
      {
         bt2_flg = true;
         bt2_t = bt_tim;
        if(EEPROM.read(10) == 10){
          Serial.println("BT2");
        }else{
          epr=23;
          button (&epr);
        }
      }
      if(digitalRead(button2) == HIGH)
      {
        bt2_flg = false;
      }

      if(digitalRead(button3) == LOW && !bt3_flg && bt_tim-bt3_t>300)
      {
        bt3_flg = true;
        bt3_t = bt_tim;
        if(EEPROM.read(10) == 10){
          Serial.println("BT3");
        }else{
          epr=27;
          button (&epr);
        }
      }
      if(digitalRead(button3) == HIGH)
      {
        bt3_flg = false;
      }

      if(digitalRead(joyButt) == LOW && !jb_flg && bt_tim-jb_t>300)
      {
        jb_flg = true;
        jb_t = bt_tim;
        if(EEPROM.read(10) == 10){
          Serial.println("JB");
        }else{
          epr=35;
          button (&epr);
        }
      }
      if(digitalRead(joyButt) == HIGH)
      {
        jb_flg = false;
      }
    

  //////////////////////////////////////////////////////////////////////////// ORBIT & PAN //////////////////////////////

    if(EEPROM.read(10) == 0)
      {
        smooth = 5;
      }
    else
      {
        smooth = 25;
      }
    tim = millis() - h;
    if (tim > smooth)
      {
        sens = arSens[EEPROM.read(0)];
        YValue = analogRead(vertPin) - YZero;  
        XValue = analogRead(horzPin) - XZero;
        h = millis();
        const int deadZone = 5;
        if ((YValue > deadZone)||(YValue < (-deadZone)))
          { 
            timeoff = LOW;
            if( EEPROM.read(10) != 10){
              selModes();
              Mouse.move(0, (YValue / sens), 0);
              moved=1;
            }else{
              sprintf(buf1,"Y=%d",(YValue/sens));
            }
          }
        if ((XValue > deadZone)||(XValue < (-deadZone)))
          { 
            timeoff = LOW;
            if( EEPROM.read(10) != 10){
              selModes();
              Mouse.move((-1 * (XValue / sens)), 0, 0);
              moved=1;
            }else{
              sprintf(buf2,"X=%d",(-1*(XValue/sens)));
            }
          }

        if ( (YValue <= deadZone)&&(YValue >= (-deadZone))  &&  
                    (XValue <= deadZone)&&(XValue >= (-deadZone)))
          {  
            moved=0;
            if (((digitalRead(joyButt) == 1) && (digitalRead(button1) == 1) && (digitalRead(button2) == 1) && (digitalRead(button3) == 1)) || (EEPROM.read(10) == 4) )
              {
                if( EEPROM.read(10) != 10){
                  Keyboard.releaseAll();
                  tim1 = millis() - h1;
                  if (tim1 > 120)
                    {
                      h1 = millis();
                      Mouse.release(MOUSE_MIDDLE);
                      Mouse.release(MOUSE_LEFT);
                      Mouse.release(MOUSE_RIGHT);
                    }
                }
              }
          }
          else if( EEPROM.read(10) == 10){
            strcat(buf1,sep);
            strcat(buf1,buf2);
            Serial.println(buf1);
          }
      }

    if(moved==0)
      {
        scroll();
      }

  //////////////////////////////////////////////////////////////////////// Menu Entry /////////////////////////////////////

    if((digitalRead(joyButt) == LOW) && (digitalRead(button1)== LOW))
      {
        delay(500);
        Keyboard.releaseAll();
        Mouse.release(MOUSE_MIDDLE);
        Mouse.release(MOUSE_LEFT);
        Mouse.release(MOUSE_RIGHT);
        menu();
        display.clearDisplay();
        display.drawBitmap(0, 0, faq, 128, 64, 1 );
        display.display();
      }
    
    //////////////////////////////////////////////////////////////////// Time out Display //////////////////////////////////

    if(timeoff == HIGH)
      {
        tim3 = millis() - h3;
        if (tim3 > timeout)
          {
            h3 = millis();
            display.clearDisplay();
            display.display();
            for(byte a=0; a<NUMPIXELS; a++)
            {
              pixels.setPixelColor(a, pixels.Color(0, 0, 0));//Off
            }
            pixels.show();
            timeoffent = HIGH;
          }
      }
    else  
      {
        h3 = millis();
        if(timeoffent == HIGH)
          {
            display.clearDisplay();
            display.drawBitmap(0, -3, faq, 128, 64, 1 );
            display.display();
            neopixel();
            timeoffent = LOW;
          }
        
      }
    timeoff = HIGH;
    
  }



