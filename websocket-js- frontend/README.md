## Synopsis

We have included a simple mechanism for connecting communicating via URL to the motes in a Chariot mesh. This note applies to the example found in the examples folder named *ArduinoWebSocketServerToChariot*. 

Please note that if you have Node.js installed on a system that is connected to this mote, the utility *wscat* can be run in a Windows command prompt (batch shell) or linux bash shell. This does exactly the same thing as our simple Javascript frontend. The *wscat* utility would be invoked as:

    wscat -c ws://92.168.0.77
    
for the default setup of this sketch.

![WS interface](https://www.qualianetworks.com/images/32m6n72s6x271ta8fm9ver7qytzyx0)

To connect right click on "arduino-chat-frontend-logo" and open with Google Chrome or Firefox. A websocket connection will
be attempted, by way of a popup screen. If your Arduino/Ethernet/Chariot stack is operating with a different IP address
(i.e., not 192.168.0.77--the default for the Websocket library), you can change it in the popup or simply edit the
javascript file.

This code is mostly of interest to those users who need some guidance in structuring their own Javascript/HTML webapps. Please also note that alternatives to this ethernet setup are supported by Chariot. In the examples folder, please see the *WeMos-D1-R2-Chariot_EP_Blynk_webofthings_example* sketch.