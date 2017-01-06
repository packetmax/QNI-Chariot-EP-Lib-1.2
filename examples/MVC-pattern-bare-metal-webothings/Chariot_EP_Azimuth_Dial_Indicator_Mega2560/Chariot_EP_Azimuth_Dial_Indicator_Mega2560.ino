#include <ChariotEPLib.h>
#include <coap-constants.h>
#define SerialMon Serial

/*
 Web-of-Bare-Metal-Things  --  Model-View-Controller 
 This sketch provides the View(er). 
 
 This example for the Arduino Mega 2560 shows how servo activity can be monitored from a
 a remote CoAP client. The purpose is to show how bare metal device sketches can  
 interoperate over the Chariot 6LoWPAN/CoAP network to control other sketches in a 6LoWPAN.

 The display used by the sketch came from Amazon. It is called "Elegoo UNO R3 2.8 Inch TFT
 Touch Screen". This screen breaks out the data across a parallel interface (see below). This 
 is one of the main reasons the Mega2560 (or equiv) is used. All wiring details are below. 
 This display was under $20 and performs very well.
 
 Much of this code is screen handling. A rewrite that hides it in a library 
 is definitely in order.
 
 by George Wayne, Copyright 2016 Qualia Networks Inc
 */
 
static String azimuthResource = "event/servos/azimuth";
static String opts = "";
static String servoMote = "";
static String response;
static unsigned long upSeconds = 0L;
static unsigned long lastScreenUpdate = 1, 
                     lastServoSearch = 1,
                     lastSessionGet = 1;
static int servoSessionId = -1;
static boolean firstDraw = true;
static String sessionId = "session-id";
static int sessionTimeouts = 0;

bool findAzimuthServo();
void setRemoteAzimuthServo(String &cmd);
void obsRemoteAzimuthServo();
void displayInputTextMsg(String& msg);
void displayTextMsg(String& msg);

// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.
//Technical support:goodtft@163.com

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

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

String Format(double val, int dec, int dig );
void DrawDial(Adafruit_TFTLCD &d, int cx, int cy, int r, double loval, double hival, 
              double inc, double sa, double curval, int dig, int dec, unsigned int needlecolor, 
              unsigned int dialcolor, unsigned int  textcolor, String label, boolean & redraw);

/*
 * This is the all important object for controlling the LCD.
 * --almost all code and complexity has to do with it.
 */
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

bool observeRemoteAzimuthServo();

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Servo remote sense aquisition example."));
  
#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  Serial.println(F("Using Adafruit 2.8\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Adafruit 2.8\" TFT Breakout Board Pinout"));
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
  } else {
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
  ChariotEP.enableDebugMsgs();
  delay(500);
  
  ChariotEP.begin();
  delay(1000);
  Serial.println(F("Setup complete."));

  tft.setRotation(3);
}

/*
 * Loop: 1) find the azimuth controller; 2) subscribe to angle changes; 
 */
void loop(void) {
  char ch;
  String eventResp;
  
  upSeconds = millis()/1000;
  
    /*
   * Receive published events from Azimuth servo (and Chariot inputs).
   * --we don't process anything else here because we're just
   * --providing the function of gauge!
   */
  if (ChariotEP.available()) {
    ChariotEP.coapResponseGet(eventResp); // Get 6LoWPAN input.
    
   /*
    * Case 1: No servo mote known.
    */
   if (servoMote.length() == 0) 
   {
     Serial.print(F("Non-event input: "));
     Serial.println(eventResp);
     displayInputTextMsg(eventResp);
   }
   /*
    * Case 2: We know the location of the mote controlling the servo.
    * Now, try to move the needle.
    */
   else {
      int azimuth;
      int degStart, degEnd;
      double angle;

      Serial.print(F("Event/input arrived: "));
      Serial.println(eventResp);
      
      if (ChariotEP.is_204_CHANGED(eventResp)) { // If angle change event... 
        degStart = eventResp.lastIndexOf("=") + 1;
        degEnd = eventResp.indexOf("(deg)");

        eventResp.remove(degEnd); 
        eventResp.remove(0, degStart);
        eventResp += String('\n');

        Serial.print(F("(loop)Azimuth = ")); 
        azimuth = eventResp.toInt();
        angle = azimuth * 1.0;
        Serial.println(eventResp.toInt());
      

        /*
         * Case 2a: Good angle.
         */
        if (azimuth >= 0 && azimuth <= 180) {
          DrawDial(tft, 155, 120, 110, 0 , 180, 10, 240, angle,  3 , 0, RED, WHITE, BLACK, "Degrees", firstDraw);
          firstDraw = false; // Always disable after first draw of gauge.
        }
        
        /*
         * Case 2b: Bad angle or arriving message garbled.
         */
        else { 
          Serial.println(eventResp);
          displayInputTextMsg(eventResp);
        }
      }     
   }
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
  if ((servoMote.length() == 0) && ((upSeconds % 10) == 0) && (upSeconds > lastServoSearch)) 
  {
    String msg = "Searching for servo...";

    lastServoSearch = upSeconds;
    displayTextMsg(msg);
    if (findAzimuthServo())
    {
      observeRemoteAzimuthServo();
    }
  }

  if ((servoMote.length() > 0) && ((upSeconds % 10) == 0) && (upSeconds > lastSessionGet))
  {
    int sessionId = -1;
    lastSessionGet = upSeconds;

    /*
     * If we established a connection to the servo session.
     */
    if (servoSessionId != -1) {
      sessionId = ChariotEP.getSessionId(servoMote);
      /*
       * Session ID no longer GETable.
       */
      if (sessionId < 0) {
        sessionTimeouts++;
        if (sessionTimeouts > 2) {
          SerialMon.print(F("Servo session "));
          SerialMon.print(servoSessionId, DEC);
          SerialMon.println(F(" no longer reachable--possibly rebooting."));
          sessionTimeouts = 0;
        }
      } 
      /*
       * New session ID has been established.
       */
      else {
        sessionTimeouts = 0;
        if (sessionId != servoSessionId) {
          SerialMon.print(F("COAP GET of session-id for "));
          SerialMon.print(servoMote);
          SerialMon.print(F(" returned "));
          SerialMon.print(sessionId, DEC);
          SerialMon.println(".");
          SerialMon.print(F("Original sessionId was "));
          SerialMon.print(servoSessionId, DEC);
          SerialMon.println("."); 
          
          /*
           * Reeeissue the OBSERVE.
           */
          servoSessionId = sessionId;
          observeRemoteAzimuthServo();
          
        }
      }
    } else {
      SerialMon.println(F("Session never established."));
    }
  }
}

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
            SerialMon.println(response);
            servoMote = motes[i];
            displayMsg = "Servo mote is: " + servoMote + ".";
            Serial.println(displayMsg);
            displayInputTextMsg(displayMsg);
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
  displayTextMsg(displayMsg);
  return false;
}

bool observeRemoteAzimuthServo()
{
  opts = "";
  ChariotEP.coapRequest (COAP_OBSERVE, servoMote, azimuthResource, TEXT_PLAIN, opts, response);

  if ((ChariotEP.is_205_CONTENT(response)) || (ChariotEP.is_204_CHANGED(response)))
  {
    int azimuth, degStart, degEnd;
    double angle;
    
    Serial.println(response);
    degStart = response.lastIndexOf("=") + 1;
    degEnd = response.indexOf("(deg)");

    response.remove(degEnd); 
    response.remove(0, degStart);
    response += String('\n');
    Serial.print(F("Azimuth = ")); 
    azimuth = response.toInt();
    angle = azimuth * 1.0;
    Serial.println(response.toInt());

    if (azimuth >= 0 && azimuth <= 180) {
      tft.fillScreen(BLACK);
      tft.setCursor(0, 0);
      firstDraw = true;
      DrawDial(tft, 155, 120, 110, 0 , 180, 10, 240, angle,  3 , 0, RED, WHITE, BLACK, "Degrees", firstDraw);
      firstDraw = false;
    }
    delay(1000); // azimuth servo settle time
    return true;
  }
  else {
    Serial.println(response);
    return false;
  }
}

/*
  This method will draw a dial-type graph for single input
  it has a rather large arguement list and is as follows

  &d = display object name
  cx = center position of dial
  cy = center position of dial
  r = radius of the dial
  loval = lower value of the scale (can be negative)
  hival = upper value of the scale
  inc = scale division between loval and hival
  sa = sweep angle for the dials scale
  curval = date to graph (must be between loval and hival)
  dig = format control to set number of digits to display (not includeing the decimal)
  dec = format control to set number of decimals to display (not includeing the decimal)
  needlecolor = color of the needle
  dialcolor = color of the dial
  textcolor = color of all text (background is dialcolor)
  label = bottom lable text for the graph
  redraw = flag to redraw display only on first pass (to reduce flickering)
*/

void DrawDial(Adafruit_TFTLCD &d, int cx, int cy, int r, double loval, double hival, 
              double inc, double sa, double curval, int dig, int dec, unsigned int needlecolor, 
              unsigned int dialcolor, unsigned int  textcolor, String label, boolean & redraw) 
 {

  double ix, iy, ox, oy, tx, ty, lx, rx, ly, ry, i, offset, stepval, data, angle;
  double degtorad = .0174532778;
  static double px = cx, py = cy, pix = cx, piy = cy, plx = cx, ply = cy, prx = cx, pry = cy;

  // draw the dial only one time--this will minimize flicker
  if ( redraw == true) {
    redraw = false;
    d.fillCircle(cx, cy, r - 2, dialcolor);
    d.drawCircle(cx, cy, r, needlecolor);
    d.drawCircle(cx, cy, r - 1, needlecolor);
    d.setTextColor(textcolor, dialcolor);
    d.setTextSize(2);
    d.setCursor(cx - 40, cy + 60); 
    d.println(label);

  }
  // draw the current value
  d.setTextSize(2);
  d.setTextColor(textcolor, dialcolor);
  d.setCursor(cx - 25, cy + 40 );
  d.println(Format(curval, dig, dec));
  // center the scale about the vertical axis--and use this to offset the needle, and scale text
  offset = (270 +  sa / 2) * degtorad;
  // find hte scale step value based on the hival low val and the scale sweep angle
  // deducting a small value to eliminate round off errors
  // this val may need to be adjusted
  stepval = ( inc) * (double (sa) / (double (hival - loval))) + .00;
  // draw the scale and numbers
  // note draw this each time to repaint where the needle was
  for (i = 0; i <= sa; i += stepval) {
    angle = ( i  * degtorad);
    angle = offset - angle ;
    ox =  (r - 2) * cos(angle) + cx;
    oy =  (r - 2) * sin(angle) + cy;
    ix =  (r - 10) * cos(angle) + cx;
    iy =  (r - 10) * sin(angle) + cy;
    tx =  (r - 30) * cos(angle) + cx;
    ty =  (r - 30) * sin(angle) + cy;
    d.drawLine(ox, oy, ix, iy, textcolor);
    d.setTextSize(1.5);
    d.setTextColor(textcolor, dialcolor);
    d.setCursor(tx - 10, ty );
    data = hival - ( i * (inc / stepval)) ;
    d.println(Format(data, dig, dec));
  }
  // compute and draw the needle
  angle = (sa * (1 - (((curval - loval) / (hival - loval)))));
  angle = angle * degtorad;
  angle = offset - angle  ;
  ix =  (r - 10) * cos(angle) + cx;
  iy =  (r - 10) * sin(angle) + cy;
  // draw a triangle for the needle (compute and store 3 vertiticies)
  lx =  5 * cos(angle - 90 * degtorad) + cx;
  ly =  5 * sin(angle - 90 * degtorad) + cy;
  rx =  5 * cos(angle + 90 * degtorad) + cx;
  ry =  5 * sin(angle + 90 * degtorad) + cy;
  // first draw the previous needle in dial color to hide it
  // note draw performance for triangle is pretty bad...

  d.fillTriangle (pix, piy, plx, ply, prx, pry, dialcolor);
  d.drawTriangle (pix, piy, plx, ply, prx, pry, dialcolor);

  // then draw the old needle in need color to display it
  d.fillTriangle (ix, iy, lx, ly, rx, ry, needlecolor);
  d.drawTriangle (ix, iy, lx, ly, rx, ry, textcolor);

  // draw a cute little dial center
  d.fillCircle(cx, cy, 8, textcolor);
  //save all current to old so the previous dial can be hidden
  pix = ix;
  piy = iy;
  plx = lx;
  ply = ly;
  prx = rx;
  pry = ry;
}

String Format(double val, int dec, int dig ) {
  int addpad = 0;
  char sbuf[20];
  String condata = (dtostrf(val, dec, dig, sbuf));
  int slen = condata.length();
  for ( addpad = 1; addpad <= dec + dig - slen; addpad++) {
    condata = " " + condata;
  }
  return (condata);
}

/*
 * Common codes for displaying a message on the LCD.
 */
void initTextDisplay(bool inputMsg)
{
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(RED);
  tft.setTextSize(3);
  tft.println("Azimuth Gauge");
  tft.setTextSize(2);
  if (inputMsg) {
    tft.println("Input arrived:");
  }
  tft.println();
  tft.setTextColor(GREEN);
}

void displayInputTextMsg(String& msg)
{
  initTextDisplay(true);
  tft.println(msg);
}

void displayTextMsg(String& msg)
{
  initTextDisplay(false);
  tft.println(msg);
}
