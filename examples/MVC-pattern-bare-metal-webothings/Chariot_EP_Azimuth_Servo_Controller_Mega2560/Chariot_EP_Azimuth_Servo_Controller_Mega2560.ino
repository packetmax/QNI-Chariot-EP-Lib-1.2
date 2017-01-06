#include <ChariotEPLib.h>
#include <coap-constants.h>
#define SerialMon Serial

/*
 Web-of-Bare-Metal-Things  --  Model-View-Controller 
 This sketch provides the Controller. 
 
 This example for the Arduino Mega 2560 shows how servos can be controlled from a
 a remote CoAP client. The purpose is to show how bare metal device sketches can  
 interoperate over the Chariot 6LoWPAN/CoAP network to control other sketches in a 6LoWPAN.

 The display used by the sketch came from Amazon. It is called "Elegoo UNO R3 2.8 Inch TFT
 Touch Screen". This screen breaks out the data across a parallel interface (see below). This 
 is one of the main reasons the Mega2560 (or equiv) is used. All wiring details are below. 
 This display was under $20 and performs very well.

 Most of this code is touch screen handling. A rewrite that hides it in a library 
 is definitely in order.
 
 by George Wayne, Copyright 2016 Qualia Networks Inc
 */
 
// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.
//Technical support:goodtft@163.com

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Color definitions
#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

/******************* UI details */
#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

// text box where numbers go
#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 220
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR ILI9341_MAGENTA
// the data (phone #) we store in the textfield
#define TEXT_LEN 12
#define MAX_STATUS_LEN  38
char statusBlankField[MAX_STATUS_LEN] = {"                                               "};
char textfield[TEXT_LEN+1] = "";
uint8_t textfield_i=0;

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 23  // can be a digital pin
#define XP 22  // can be a digital pin

//2.8inch_280-5
#define TS_MINX 180
#define TS_MINY 170
#define TS_MAXX 920
#define TS_MAXY 930
// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

Adafruit_GFX_Button buttons[15];
/* create 15 buttons, in classic candybar phone style */
char buttonlabels[15][5] = {"Send", "Clr", "End", "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#" };
uint16_t buttoncolors[15] = {ILI9341_DARKGREEN, ILI9341_DARKGREY, ILI9341_RED, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_ORANGE, ILI9341_BLUE, ILI9341_ORANGE};

static String azimuthResource = "event/servos/azimuth";
static String opts = "";
static String servoMote = "";
static String response;
static int servoSessionId = -1;
static String sessionId = "session-id";
static int sessionTimeouts = 0;
static unsigned long lastServoSearch = 1L, upSeconds = 0L;

bool findAzimuthServo();
uint8_t countMotes();
void setRemoteAzimuthServo(String &cmd);
void getRemoteAzimuthServo();

/*
 * display text as status
 */
void displayStatusMsg(String &msg)
{
  char buf[MAX_STATUS_LEN] = {'\0'};
  unsigned int len = msg.length();

  len = min(msg.length(), MAX_STATUS_LEN);
  msg.toCharArray(buf, len);
  buf[len] = '\0';
  statusMsg(buf);
}

/*
 * Clear out the numerical text window
 */
void clearTextWindow()
{
  textfield_i = 0;
  int ii;
  for (ii=0; ii<TEXT_LEN; ii++)
  {
    textfield[ii] = ' ';
  }
  textfield[TEXT_LEN] = 0;
}

/*
 * Print something in the mini status bar with either flashstring
 */
void statusMsg(const __FlashStringHelper *msg) {
  tft.fillRect(STATUS_X, STATUS_Y, 240, 8, ILI9341_BLACK);
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(msg);
}

// or charstring
void statusMsg(char *msg) {
  tft.fillRect(STATUS_X, STATUS_Y, 240, 8, ILI9341_BLACK);
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1.5);
  tft.print(msg);
}

bool sendCommand()
{
  String cmd, displayMsg;
  
  cmd = String(textfield);
  clearTextWindow();
  statusMsg(statusBlankField);
  
  if ((cmd == String("*0")) || (cmd == String("#0")))
  {
    countMotes();
    return true;
  }
  else if ((cmd == String("*1"))  || (cmd == String("#1")))
  {
     if (servoMote.length() > 18) {
      displayMsg = "Servo mote is: " + servoMote + ".";
      displayStatusMsg(displayMsg);
     } 
     else 
     {
      findAzimuthServo();
     }
     return true;
  }
  else if ((cmd == String("*2"))  || (cmd == String("#2")))
  {
     if (servoMote.length() > 18) {
      getRemoteAzimuthServo();
     } 
     else 
     {
      String statusErrMsg;
      statusErrMsg = "No visible azimuth servos. ";
      displayStatusMsg(statusErrMsg);
     }
     return true;
  }
  else if ((cmd == String("*3"))  || (cmd == String("#3")))
  {
    String statusErrMsg;
    if (servoMote.length() > 0) {
      servoSessionId = ChariotEP.getSessionId(servoMote);
      statusErrMsg = "Model's session ID: " + String(servoSessionId) + ". ";
      displayStatusMsg(statusErrMsg);
    }
    else {
      statusErrMsg = "No visible azimuth servos. ";
      displayStatusMsg(statusErrMsg);
    }
  }
  else if (cmd.startsWith(F("#9"), 0) || cmd.startsWith(F("*9"), 0))
  {
    if (servoMote.length() > 18) { // "chariot.c35xx.local" 
      setRemoteAzimuthServo(cmd);
      return true;
    } else {
      String statusErrMsg;
      statusErrMsg = "No visible azimuth servos. ";
      displayStatusMsg(statusErrMsg);
    }
  }
  else {
    String statusMsg;
    statusMsg = String("\"") + cmd + String("\"") + "  not understood. ";
    displayStatusMsg(statusMsg);
    return false;
  }
}

void setup(void) {
  String setupString;

  Serial.begin(9600);
  Serial.println(F("Servo remote control and sense aquisition."));

  ChariotEP.enableDebugMsgs();
  delay(500);

  ChariotEP.begin();
  
#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  Serial.println(F("Using Adafruit 2.4\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Adafruit 2.4\" TFT Breakout Board Pinout"));
#endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9341;
   
  }

  tft.begin(identifier);
  tft.setRotation(0);
  tft.fillScreen(BLACK);
  
  // create buttons
  for (uint8_t row=0; row<5; row++) {
    for (uint8_t col=0; col<3; col++) {
      buttons[col + row*3].initButton(&tft, BUTTON_X+col*(BUTTON_W+BUTTON_SPACING_X), 
                 BUTTON_Y+row*(BUTTON_H+BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text
                  BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col+row*3], ILI9341_WHITE,
                  buttonlabels[col + row*3], BUTTON_TEXTSIZE); 
      buttons[col + row*3].drawButton();
    }
  }
  
  // create 'text field'
  tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);
  
  Serial.println(F("Setup complete."));
  setupString = "Setup complete. ";
  displayStatusMsg(setupString);
  statusBlankField[MAX_STATUS_LEN-1] = 0;
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

void loop(void)
{
  upSeconds = millis()/1000;
  
  /*
   * Answer remote RESTful GET, PUT, DELETE, OBSERVE API calls
   * transparently.
   */
  if (ChariotEP.available()) {
   ChariotEP.process();
  }

  /* 
   *  Filter your own inputs first--pass everthing else here.
   *   --try typing 'sys/help' into the Serial window
   */
  if (Serial.available()) {
    delay(75); // wait for all chars to arrive
    ChariotEP.serialChariotCmd();
  }

/*
   * Look for the servo mote and display some messages
   */
  if (servoMote.length() == 0) 
  {
    if (((upSeconds % 10) == 0) && (upSeconds > lastServoSearch)) 
    {
      String msg = "Searching for servo...";
  
      lastServoSearch = upSeconds;
      displayStatusMsg(msg);
      if (findAzimuthServo())
      {
        msg = "Servo mote found: " + servoMote;
        displayStatusMsg(msg);
      }
    }
  }
  else 
  {
    /*
     * Horrendous button press code follows.
     * --cribbed from Adafruit, it needs to be hidden away.
     */
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);
  
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    pinMode(XP, OUTPUT);  // uncommented
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    pinMode(YM, OUTPUT); // uncommented
  
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!
    // Scale from ~0->4000 to tft.width using the calibration #'s
     if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      // scale from 0->1023 to tft.width
      p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
      p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
     }
     
    // go thru all the buttons, checking if they were pressed
    for (uint8_t b=0; b<15; b++) {
      if (buttons[b].contains(p.x, p.y)) {
        //Serial.print("Pressing: "); Serial.println(b);
        buttons[b].press(true);  // tell the button it is pressed
      } else {
        buttons[b].press(false);  // tell the button it is NOT pressed
      }
    }
  
    // now we can ask the buttons if their state has changed
    for (uint8_t b=0; b<15; b++) {
      if (buttons[b].justReleased()) {
        //Serial.print("Released: "); Serial.println(b);
        buttons[b].drawButton();  // draw normal
      }
      
      if (buttons[b].justPressed()) {
          buttons[b].drawButton(true);  // draw invert!
          
          // if a numberpad button, append the relevant # to the textfield
          if (b >= 3) {
            if (textfield_i < TEXT_LEN) {
              textfield[textfield_i] = buttonlabels[b][0];
              textfield_i++;
  	          textfield[textfield_i] = 0; // zero terminate
            }
          }
  
          // clr button! delete one char
          if (b == 1) {
            textfield[textfield_i] = 0;
            if (textfield_i > 0) {  //GHW made it the index textfield_i not textfield.
              textfield_i--;
              textfield[textfield_i] = ' ';
            }
          }
  
           // its always OK to just hang up or cancel
          if (b == 2) {
            statusMsg(F("Cancelled"));
            clearTextWindow();
          }
          
          // we dont really check that the text field makes sense
          if (b == 0) {
            sendCommand();
            Serial.print("Sending "); Serial.print(textfield);
          }
           // update the current text field
          Serial.println(textfield);
          tft.setCursor(TEXT_X + 2, TEXT_Y+10);
          tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
          tft.setTextSize(TEXT_TSIZE);
          tft.print(textfield);
          
        delay(100); // UI debouncing
      }
    }
  }
}

/*
 * Find azimuth servo
 * -- We're only going to use the first for the keypad.
 * -- This is a dial-an-azimuth Chariot demo only
 * -- Modify to suit if your ambitions exceed this modest goal!
 */
 
/*
 * Find azimuth servo
 */
/*
 * The following two functions keep track of motes that have been
 * previously searched and found not to contain the azimuth controller
 * resource. We avoid thrashing on the 6LoWPAN by not revisiting motes
 * previously searched.
 */
static String nonAzimuthNeighbors[MAX_MOTES] = {};
static int nextNeighbor = 0;
bool addNeighbor(String& neighbor)
{
  if (nextNeighbor < MAX_MOTES) {
    nonAzimuthNeighbors[nextNeighbor++] = neighbor;
  }
  return false;
}

bool findNeighbor(String& neighbor)
{
  if (nextNeighbor == 0) {
    return false;
  }
  int i;
  for (i=0; i < nextNeighbor; i++)
  {
    if (neighbor == nonAzimuthNeighbors[i]) {
      return true;
    }
  }
  return false;
}

bool findAzimuthServo()
{
  String motes[MAX_MOTES];
  uint8_t nMotes;
  String azimuthSearchName = "azimuth";
  String response;
  String displayMsg;

  nMotes = ChariotEP.getMotes(motes);
  delay(100);
  if (nMotes > 0)
  {
    int i;
    for (i=0; i<nMotes; i++)
    {
      /*
       * If we already know this node doesn't possess an azimuch servo, skip it.
       */
      if (!findNeighbor(motes[i])) 
      {
        // Search for resource to see if azimuth servo exists 
        displayMsg = "Searching mote: " + motes[i] + " for azimuth servo";
        SerialMon.println(displayMsg);
        
        if (ChariotEP.coapSearchResources(motes[i], azimuthSearchName, response))
        {
          if (ChariotEP.is_205_CONTENT(response))
          {
            // Check for azimuth servo on mote[i] 
            delay(100);
            ChariotEP.coapRequest(COAP_GET, motes[i], azimuthResource, TEXT_PLAIN, opts, response);
            ChariotEP.strip_205_CONTENT(response);
            //SerialMon.println(response);
            servoMote = motes[i];
            displayMsg = "Servo mote is: " + servoMote + ".";
            Serial.println(displayMsg);
            displayStatusMsg(displayMsg);
            delay(250);
            servoSessionId = ChariotEP.getSessionId(servoMote);
            SerialMon.print(F("COAP GET of session-id for "));
            SerialMon.print(servoMote);
            SerialMon.print(F(" returned "));
            SerialMon.print(servoSessionId, DEC);
            SerialMon.println(".");
            delay(250);
            return true;
          }
          else { // Add this mote to list of ones not to re-check.
            addNeighbor(motes[i]);
          }
        }
        delay(250); // Go on to check of next mote after a delay
      }
    }
    displayMsg = "Servo mote not found.";
  } 
  else 
  {
    displayMsg = "No motes visible.";
  }
  SerialMon.println(displayMsg);
  displayStatusMsg(displayMsg);
  return false;
}

uint8_t countMotes()
{
  String motes[MAX_MOTES];
  uint8_t nMotes;
  String displayMsg;

  nMotes = ChariotEP.getMotes(motes);
  displayMsg = String(nMotes) + "-6LoWPAN Chariot nodes visible. ";
  displayStatusMsg(displayMsg);
  delay(100);
  return nMotes;
}

void setRemoteAzimuthServo(String &cmd)
{
  String angleStr, statusStr;
  int angle;

  angleStr = cmd;
  angleStr.remove(0, 2);
  Serial.print("Requested azimuth angle(deg): ");
  Serial.println(angleStr);
  angle = angleStr.toInt();
  
  if (angle >=0 && angle <= 180)
  {
    opts = "param=pos&val=" + String(angleStr);
    ChariotEP.coapRequest (COAP_PUT, servoMote, azimuthResource, TEXT_PLAIN, opts, response);
    SerialMon.println(response);
    if (ChariotEP.is_205_CONTENT(response))
    {
      ChariotEP.strip_205_CONTENT(response);
      response += " ";
      displayStatusMsg(response);
    }
    else {
      response += " ";
      displayStatusMsg(response);
    }
    delay(1000); // azimuth servo settle time
  }
  else
  {
    statusStr = angleStr + "(deg) is invalid. ";
    displayStatusMsg(statusStr);
  }
}

/*
 * Perform GET to the azimuth servo (model) and report angle returned.
 */
void getRemoteAzimuthServo()
{
  opts = "";
  ChariotEP.coapRequest (COAP_GET, servoMote, azimuthResource, TEXT_PLAIN, opts, response);
  SerialMon.println(response);

  if (ChariotEP.is_205_CONTENT(response))
  {
    ChariotEP.strip_205_CONTENT(response);
    response += " ";
    displayStatusMsg(response);
    delay(1000); // azimuth servo settle time
  }
  else if (ChariotEP.is_204_CHANGED(response))
  {
    ChariotEP.strip_204_CHANGED(response);
    response.remove(0, 14);
    response += " ";
    displayStatusMsg(response);
    delay(1000); // azimuth servo settle time
  }
  else {
    response += " ";
    displayStatusMsg(response);
  }
}
