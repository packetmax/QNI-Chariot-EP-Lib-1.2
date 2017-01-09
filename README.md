# Chariot Web of Things Shield for Arduino--Endpoint Library version 1.2 
###  *by Qualia Networks Incorporated*

## ChariotEPClass
### Synopsis
This library enables you to use the Chariot 6LoWPAN/CoAP shield to drive Arduino resources and sketches in new and exciting ways. It does so by creating a cloud- or internet-connectable mesh of wireless Arduino motes. Using Chariot, sketches can process and transmit sense and actuation information within and without the wireless mesh. This library provides simple, powerful web-of-things capabilities to any collection of Arduinos
equipped with the Qualia Networks Chariot wireless shield. 

![packet_traffic](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/585571e5d2b857abafe4a54c/1481996569236/Wireshark+Capture+pic.GIF?format=500w "Wireshark capture of Chariot W-o-T traffic")

The public functions of
this library and Chariot firmware turn your Arduino into a mote (i.e., 'particle'), extending atomic
access to the resources on your Arduino via API or URI proxy to other Arduinos, webapps or browsers
anywhere. 

![Arduino_Chariot_mesh_component](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/585571cb1b631bc19802286a/1481994701536/IO-mote.jpg?format=500w "Chariot-equipped Arduino mesh node")

### Motivation
The purpose of this library for Chariot is to provide Arduino with the ability to create and 
control dynamic event driven resources within a sketch, across sketches, and across the net to clouds or webapps. These resources become objects of Chariot's RESTful web services, identified by unique URIs, which provide
global addressing space enabling discovery and access.

### Code examples accompanying this release
Arduino sketches in the examples folder in this repository highlight a number of powerful features and use cases for Chariot:
 - Basic capabilities demonstration
 - Resource discovery in the Chariot mesh
 - Dynamic resource creation and event generation
 - Distributed Arduino temp monitoring example
 - Blynk smartphone/cloud tracking example
 - Websocket interfacing of the Chariot mesh to the internet and CoAP browser example for Chrome
 - Gang of Four Model-View-Controller design pattern for bare metal example 

## API and URI usage
### API (*partial list*)

|   Function:                                                                  |   Signature:         |
|:-----------------------------------------------------------------------------|--------------------------------|
| Constructs an instance of the *ChariotEPClass* class.|`ChariotEPClass()`|
| Initialize Chariot comm chan and event pins. Set location string if desired.|`bool begin() or bool begin(String& loc)`|
| Get the number of bytes (characters) available for reading from Chariot's serial port.|`int available()`|
| Handle asynchronous messages from arduino and event resources int the background loop.|`void process()`|
| Generate a RESTful resource request (GET, POST, PUT, DELETE, OBSERVE) to DNS-named mote.|`bool coapRequest(coap_method_t method, String& mote,  String& resource, coap_content_format_t content, String& opts, String& response)`|
| Create a list of all current motes in the neighborhood. The number found is returned. |`uint8_t getMotes(String (&motes)[MAX_MOTES])`|
| Search resources at *mote* for full or partial matches of *resource*.    |`bool coapSearchResources(String& mote, String& resource, String& response)`|
| Create a resource known by *uri*, specifying resource value len (up to 64 bytes) and an attribute string (which will appear in */.well-known/core requests*).  |`int createResource(const String& uri, uint8_t maxBufLen, const String& attrib);`|
| Store *eventVal* in the resource designated by *handle*. If *signalChariot* is true cause Chariot to send the new resource value to all observers.    |`bool triggerResourceEvent(int handle, String& eventVal, bool signalChariot)`|
| Set up a handler for all PUT commands arriving for resource designated by *handle*. PUTs can set parameter values for resources created by *createResource()*. See URI example below for setting "state* to *on* for the dynamic resource */event/tmp275-c*. An arbitrary number of parameters can be supported--see temp trigger example. |`int setPutHandler(int handle, String * (*putCallback)(String& putCmd))`|
| Issue commands to Chariot from Arduino's Serial window input. Type 'help' to see available commands.   |`void serialChariotCmd()`|
| Issue a local command from the sketch. See *serialChariotCmd()*.   |`bool localChariotCmd(String& command, String& response)`|


#### URI examples for Arduino resources (see [ref])
Chariot acts as a CoAP proxy that communicates over its serial channel with applications using URIs. It converts these into the CoAP protocol and transmits resultant packets to the targetted mote wirelessly over a private CoAP/UDP/6LoWPAN/IEEE 802.15.4 network. The URIs may be generated by an Arduino, a remote webapp, etc., depending upon application. We provide a number sketches that illustrate the possibilites in our examples.

![CoAP](https://static1.squarespace.com/static/5665efb6c647add60e3ed416/585571412994ca4821a326ac/585571d89de4bb0a69e41bab/1481996546061/Chariot-web-of-things-chat.GIF?format=500w "CoAP URLs in action")

#### This table shows some URI constructions and their conseqent actions

| Arduino pin access from Chariot mesh, cloud, net               | Resulting local action         |
|:---------------------------------------------------------------|--------------------------------|
| coap://chariot.c350e.local/arduino/mode?put&pin=13&val=input   |`return pinMode(13, INPUT)`    |
| coap://chariot.c350e.local/arduino/digital?put&pin=13&val=0    |`perform digitalWrite(13, LOW)`|
| coap://chariot.c350e.local/arduino/digital?get&pin=13          |`return digitalRead(13)`     |
| coap://chariot.c350e.local/arduino/analog?get&pin=5            |`return analogRead(5)`      |
| coap://chariot.c350e.local/arduino/analog?put&pin=13&val=128   |`perform analogWrite(2, 123)`  |
|**Chariot builtin sensors access:**                            |                               |
| coap://chariot.c350e.local/sensors/tmp275-c?get;ct=50  |`return temp(C) in JSON`    |
| coap://chariot.c350e.local/sensors/tmp275-c?get  |`return temp(C) in plain text`    |
| coap://chariot.c350e.local/sensors/fxos8700cq-c/accel?get;ct=50  |`return accelerometer in JSON`    |
| coap://chariot.c350e.local/sensors/fxos8700cq-c/mag?get;ct=50  |`return magnetometer in JSON`    |
| coap://chariot.c350f.local/sensors/radio?get    |`return radio LQI and Rssi values for mote`|
| coap://chariot.c350e.local/sensors/battery?get;ct=50          |`return VCC reading for radio in JSON`     |
|**Resource Query and Search:**                                     |                               |
| coap://chariot.c350e.local/.well-known/core?get  |`return list of resources for mote (cf. RFC7252)`    |
| coap://chariot.c350e.local/search?get&name=sensors  |`return hits matching resource substring "sensors"`|
| coap://chariot.c350e.local/search?get&name=sensors  |`return 2.05 CONTENT {"Name": "sensors", "Matched":"sensors/battery", "Which": 1, "Hits": 5}`|
| coap://chariot.c350e.local/search?get&name=sensors&which=2  |`return 2.05 CONTENT {"Name": "sensors", "Matched":"sensors/tmp275-c", "Which": 2, "Hits": 5}`    |
|**Event Resources:**                                     |                               |
| coap://chariot.c350f.local/event/tmp275-c?put&param=triggerval&val=35  |`set temp event trigger to 35C for observers (see example sketch)`|
| coap://chariot.c350f.local/event/tmp275-c/?put&param=func&val=gt  |`set temp trigger function to "greater than"`|
| coap://chariot.c350f.local/event/tmp275-c/?put&param=state&val=on  |`enable temp trigger`|
| coap://chariot.c350f.local/event/tmp275-c?obs |`become an observer, receiving trigger events`|
|coap://chariot.c350f.local/event/tmp275-c?put&param=state&val=on|`set a parameter of the resource named *state* to *on*.`
## Note
In the CoAP (RFC7252)  URLscheme, "coap:" is the internet-of-things analogue to "http:". In
addition to this, Chariot can also perform a great number of other RESTful
operations, and even enable your sketch to do new and exciting applications. For
more on this, check the top level documentation of this library.

## Note on Installation and Usage
This library provides the main Chariot class. It is instantiated for you by the library as
"ChariotEP" (Chariot End Point--see sketches in examples folder). Also, upon detecting the Serial configuration
of your Arduino, the Arduino-Chariot communication channel named ChariotClient is derived and instantiated from
either Serial or SoftwareSerial class. For UNO class boards (including the WeMos D1 UNO knockoff that uses the ESP8266 WiFi processor), digital pins used for RX and TX functions must be jumpered to the Chariot. For MEGA and DUE class boards, Serial3 is jumpered. In both cases jumpering is to the Chariot's port labelled "UART1.". Jumpering and operating details are included with instructions that came with your Chariot. Jumpers are provided with every Chariot and are pre-installed on Chariot-Arduino motes in our web-of-things kits.
[insert images and add refs to docs]


This software release requires the Chariot firmware version included in this repository. We load Chariot firmware over the JTAG port using Atmel's JTAGICE and Atmel Studio(available for free). See schematics for connector pinout. 

> Qualia Networks Incorporated -- Chariot Web-of-Things Shield and software for Arduino              
> Copyright, Qualia Networks, Inc., 2016-17.	
