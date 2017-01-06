# Model-View-Controller Pattern for Bare Metal Webs #

## Synopsis ##
Model View Controller (MVC) is a standard design pattern in web domains and is known for its extensibility, maintainability, re-usability and testability capabilities. It is explained in many places including the widely read text, *Design Patterns*, by Gamma, Helm, Johnson, Vlissides. MVC is mostly implemented in web-based systems that group architectural elements according to the pattern's name. The main reasons for its popularity lie in its extensibility, maintainability, re-usability and testability. 

Qualia Networks' Chariot is based on CoAP (Constrained Application Protocol) and provides you with the latest Internet of Things protocol standards that create web services to seamlessly and simply integrate dynamic, event-driven, Arduino smart objects. Web addressability down to the processor pin level is provided.

Chariot software for Arduino includes simple, open source, websocket-based, sketches that expose Chariot's event publish-subscribe capabilities. CoAP resource discoverability is provided from the web and between Chariot-equipped Arduino motes residing in your wireless mesh. We also provide a simple Javascript frontend for prototyping and debugging; the [name]wscat utility of Node.js can be used as well--see our example sketches: ArduinoWebsocketServerToChariot and WeMos-D1-R2-Chariot_EP_Blynk_webofthings_example for ideas and implementations. In addition to IPv6 networking, the IPv6 stack delivers the IETF RPL routing protocol for controlling traffic over low-power lossy IPv6 wireless networks. IETF 6LoWPAN header compression and adaptation layer for IEEE 802.15.4 wireless links means efficient operation and reduced power consumption.

### Model ###
This model, created with a Chariot and Arduino Mega 2560 shows how servos can be controlled from a remote CoAP client. The purpose is to show how bare metal device sketches can interoperate over the Chariot 6LoWPAN/CoAP network to control other sketches in a 6LoWPAN.

This sketch makes an azimuthal camera mount (I used Adafruit Min Pan-Tilt Kit (1967)) servo visible on a 6LoWPAN 802.15.4 wireless net. It can be controlled (it's azimuth set) and viewed. 

![model](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/585576c72994ca4821a35df5/1481996158047/MVC-Showing+Model.jpg?format=500w "Model View Controller as a Chariot web-of-things")

### Resource Creation and Setup ###
This sketch creates a dynamic resource, "/event/servos/azimuth" whose URI, coap://chariot.xxxxx.local/event/servos/azimuth, can be the object of GETS, PUTS, OBServes, and DELetes. The sketch's setup() function handles this as follows:

```c++
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
```

The azimuth servo is created in the sketch as a dynamic resource that is capable of parameterization and event generation to all Viewers. This functionality is provided by the Chariot End Point library. The coding of this resource will cause Chariot to set up a web-addressable resource that passes new azimuth angle requests through the PUT handler below.

```c++
/*
 * Create an azimuth servo resource that Chariot connects to your web of things.
 *   --also set up the PUT callback that will receive RESTful API calls.
 */
bool azimuthCreate()
{
  if ((azimuthHandle = ChariotEP.createResource(azimuthResource, 63, azimuthAttributes)) >= 0) // create resource on Chariot
  {
    if (ChariotEP.setPutHandler(azimuthHandle, azimuthPutCallback) > 0) // set RESTful PUT handler
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
```

##### Actions Taken for a New Angle
Whenever the MVC Controller sketch PUTs a new angle (>=0 && <=180) to us we set the angle and respond to the requestor with a 205-azimuth set to <n>(deg) message response. This is needed so that the requestors request doesn't timeout. The sketch then publishes a 204-azimuth set to <n>(deg) message to all observers (subscribers).
 
### View ###
This example for the Arduino Mega 2560 shows how servo activity (the model) can be observed from a remote CoAP client. The purpose is to show bare metal device sketches can interoperating over the Chariot 6LoWPAN/CoAP network to control other sketches in a 6LoWPAN.

This sketch, except for the TFT screen code, is very simple and powerful. It's functions are basic to a bare metal Arduino view pattern for the Azimuth camera mount example in this folder.

![view](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/58557786e6f2e1189aad6ef0/1481996227698/MVC-Viewer.jpg?format=500w "Azimuth Viewer")

##### Sketch 
When initiated from reset, the loop() function searches the mesh for a mote owning the resource, "event/servos/azimuth" at a frequency of once every 10 seconds.

```c++
	if (findAzimuthServo())
    {
      observeRemoteAzimuthServo();
    }
```

Because the motes are somewhat sleepy (mDNS updates occur about once per minute) it may take a couple of minutes for the mote owning the azimuth servo to be located. When this process completes, this sketch issues an OBServe command to the Controller (over the wireless mesh) and is thus registered to receive an event from the resource, "event/servos/azimuth" every time it changes.

Every 10 seconds the search algorithm inspects the current DNS table (of Chariot) for motes it hasn't previously inspected for the azimuth resource. 

```C++
	if (ChariotEP.coapSearchResources(motes[i], azimuthSearchName, response))
    {
        if (ChariotEP.is_205_CONTENT(response))
        {
          // Check for azimuth servo on mote[i] 
          ChariotEP.coapRequest(COAP_GET, motes[i], azimuthResource, TEXT_PLAIN, opts, response);
          ChariotEP.strip_205_CONTENT(response);
          servoMote = motes[i];
          servoSessionId = ChariotEP.getSessionId(servoMote);
          return true;
        }
        else { // Add this mote to list of ones not to re-check.
                addNeighbor(motes[i]);
        }
  } 
```

The boolean value of 'true' is returned when the azimuth servo mote is found and the OBServe command is issued.

```c++
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
    azimuth = response.toInt();
    angle = azimuth * 1.0;

		// Draw a dial on the screen...
    if (azimuth >= 0 && azimuth <= 180) {
      tft.fillScreen(BLACK);
      tft.setCursor(0, 0);
      firstDraw = true;
      DrawDial(tft, 155, 120, 110, 0 , 180, 10, 240, angle,  3 , 0, RED, WHITE, BLACK, "Degrees", firstDraw);
      firstDraw = false;
    }
    return true;
  }
```

Each subsequent event publication received causes the sketch to move the dial. There is also logic to cover the case of the Controller resetting. This version of Chariot inaugurated an eeprom-based session ID variable wrapped in a CoAP resource that increments every time a Chariot-equipped Arduino resets. The View sketch GETs and saves the session ID of the azimuth servo Controller when it initially finds it. Periodically it retrieves the Controller's session ID, checking it to ensure it hasn't changed. If so, it needs to re-establish the azimuth controller (it could have changed) just as it established it at Viewer start up. 

##### Note
The display used by the sketch came from Amazon. It is called "Elegoo UNO R3 2.8 Inch TFT Touch Screen". This screen breaks out the data across a parallel interface (see sketch). This is one of the main reasons the Mega2560 (or equiv) is used. All wiring details are below. This display was under $20 and performs very well.

### Controller ###
This sketch provides the Controller. 
 
This sketch, for Chariot-Arduino Mega 2560, shows how servos can be controlled from a remote CoAP client. The purpose is to demonstrate bare metal device sketches that can interoperate over the 6LoWPAN/CoAP network to control other sketches; in this case the azimuth servo resource.

![controller](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/5855769c1b631bc198025723/1481996090148/MVC-Controller.jpg?format=500w "Azimuth Controller")

As is the case with the View sketch, Controller waits in the same way for the mote holding the azimuth servo resource to appear in the motes table (DNS). It puts a 'dial pad' on the display that can field the following commands:

- #0 or *0 - display the number of visible motes right now.
- #1 or *1 - display the IPv6 address of the azimuth servo mote (if known).
- #2 or *2 - display the current azimuth angle, if azimuth servo Model, if known.
- #3 or *3 - display the session ID of the current azimuth servo Model, if known.
- #9<angle> or *9<angle> - set a new angle on the azimuth servo Model (angle >=0 && <=180 degrees).
 
This sketch has much of the same TFT display code as the View sketch. It waits like View to establish the identity of the azimuth servo mote(Model) it will be controlling. 

##### Note #####
As with Viewer, this display came from Amazon. It is the "Elegoo UNO R3 2.8 Inch TFT Touch Screen". This screen breaks out the data across a parallel interface (see View and Controller sketches). All wiring details are in the sketches. See above.
