# Azure_IOT


![alt text](https://github.com/ivanrlg/TelemetryInBusisnessCentral/blob/master/Images/Diagram.png)

This project contains 2 parts

1) Azure_IOT.ino, is the code that controls the ESP8266 device to send temperature and humidity messages to Azure IoT through the mqtt protocol.

2) EventHubTrigger is an Azure function project that is subscribed to the notifications sent to Azure IoT and that are later sent to Business Central through a Webservice published for this purpose.

For more information in the following link: https://ivansingleton.dev/?p=1541
