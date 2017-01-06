#include <ChariotEPLib.h>
#include <coap-constants.h>
#include <Servo.h>

 /*
 Web-of-Bare-Metal-Things  --  Model-View-Controller 
 This sketch provides the Model. 
 
 This example for the Arduino Mega 2560 shows how servos can be controlled from a
 a remote CoAP client. The purpose is to show how bare metal device sketches can  
 interoperate over the Chariot 6LoWPAN/CoAP network to control other sketches in a 6LoWPAN.

 This sketch makes an azimuthal camera mount (I used Adafruit Min Pan-Tilt Kit (#1967))
 servo visible on a 6LoWPAN net. It can then be controlled (it's azimuth set) and viewed. 

 See the Controller and View sketches in this folder for how the pattern is implemented!
 
 by George Wayne, Copyright 2016 Qualia Networks Inc
 */

/*
 * Servo control is via standard Arduino library
 */
Servo azimuthServo;  // create servo object to control a servo

/*
 * Simple resource state vars
 */
static int azimuthPos = 75; // variable to store the azimuth servo position

// Resource creation yields positive handle
static int azimuthHandle = -1;

// Time value to retransmit azimuth update events.
static unsigned long upSeconds = 0L;
static unsigned long lastServoSearch = 1;

// Dynamic resource variables
static String azimuthResource = "event/servos/azimuth"; // resource name must begin with "event/"
static String azimuthAttributes = "title=\"Azimuth angle\?get|obs|put\"";
static String azimuthValStr = String(azimuthPos) + "(deg)";

String * azimuthPutCallback(String& param); // RESTful PUTs on this URI come here.
bool azimuthCreate();
bool storeAzimuthAngle(bool generateEvent);

// If using the Serial port--type an integer within 5 secs to activate.
static bool debug = false;

void setup() {  
 // arduino IDE/Tools/Port for console and debug:
  Serial.begin(9600);
  Serial.println(F("Serial port is active"));
  debug = true;
  ChariotEP.enableDebugMsgs();
  delay(500);

  //---Put Arduino resources on the air---
  String location = "Mega-RF-Slave-" + String(millis());
  ChariotEP.begin(location);

  //---Attach the azimuth servo on pin 46 to the servo object
  azimuthServo.attach(46);
  azimuthServo.write(azimuthPos);

  //---Create event tilt and azimuth resources---
  if (azimuthCreate()) 
  {
    Serial.println(F("Setup complete; azimuth resource created."));
  }
  else
  {
    Serial.println(F("Setup failed: could not create azimuth resource."));
  }
}

void loop() 
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
   * Handle any serial command input.
   */
   ChariotEP.processSerialInput(60);
   
#ifdef TIMED_TRIGGER
 /*
   * We are the servo mote; transmit our position every 10 secs.
   */
  if (((upSeconds % 10) == 0) && (upSeconds > lastServoSearch)) 
  {
    ChariotEP.triggerResourceEvent(azimuthHandle, azimuthValStr, true);
  }
#endif

  /*
   * Add a short delay.
   */
  delay(50);
}

/*
 * Create an azimuth servo resource that Chariot connects to your web of things.
 *   --also set up the PUT callback that will receive RESTful API calls.
 */
bool azimuthCreate()
{
  if ((azimuthHandle = ChariotEP.createResource(azimuthResource, 63, azimuthAttributes)) >= 0)   // create resource on Chariot
  {
    if (ChariotEP.setPutHandler(azimuthHandle, azimuthPutCallback) > 0)     // set RESTful PUT handler
    {
      Serial.print(F("Azimuth resource: "));
      Serial.println(azimuthHandle);
      return ChariotEP.triggerResourceEvent(azimuthHandle, azimuthValStr, true); // set its initial condition
    }
    else 
    {
      Serial.println(F("Could not create azimuth resource!"));
      return false;
    }
  } 
  return false;
}

/*
 * This is the handler for all PUT API calls. By convention,
 * we will receive a string that looks like this:
 *   param=name val=value
 *   "name and val" are strings that consist of just about any 
 *   characters other than '='.
 */
String * azimuthPutCallback(String& param)
{
  String Name, Value, result;
  int newPos;

  Name = param.substring(0, param.indexOf('='));
  Value = param.substring(param.indexOf('=')+1, '\r');

  if (Name == "pos") 
  {
    newPos = Value.toInt();
    if ((newPos < 0) || (newPos > 180))
    {
      badInput(param);
      return NULL;
    }

    if (azimuthPos != newPos)
    {
      azimuthPos = newPos;
      azimuthServo.write(azimuthPos);
    }
  }
  else
  {
     badInput(param);
     return NULL;
  }

complete_put:
  result = "azimuth set to ";
  result += newPos;
  result += "(deg)";
  result += "\n\0";
  /*
   * PUT callbacks MUST provide responses in 
   * order to complete the operation.
   */
  sendResult(result); 

  /*
   * Send the event by returning this string.
   * --Note that when this PUT handler returns,
   *   it automatically delays before publishing the
   *   event. This protects the outgoing PUT response.
   */
  azimuthValStr = azimuthPos; 
  azimuthValStr += "(deg)";
  return &azimuthValStr;
}

/*
 * Param not understood
 */
void badInput(String param) 
{
  String result = "4.02 UNKNOWN, MISSING, OR BAD CMD/PARAMETER";
  result += "\n\0";
  // PUT callbacks MUST provide this to complete the operation.
  sendResult(result);
  return;
}

/*
 * Complete PUT by returning null terminated result payload to requestor.
 */
void sendResult(String& result)
{
  ChariotClient.print(result);
  return;
}
