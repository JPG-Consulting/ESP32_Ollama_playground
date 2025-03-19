#include <Arduino.h>
#include <WiFi.h>

#include "config.h"  // Configuration file.

#include "RestAPI.hpp"

String receivedMessage = "";  // Variable to store the complete message

void ConnectWiFi_STA()
{
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(100);
    Serial.print('.'); 
  }
 
  Serial.println("");
  Serial.print("Starting STA:\t");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Start the Serial Monitor at a baud rate of 115200
  Serial.begin(115200);

  // Connect to WiFi.
  ConnectWiFi_STA();

  // Print an initial message to the Serial Monitor
  Serial.println("ESP32 is ready. Please enter a message:");
}

void loop() {
  // Check if data is available in the Serial buffer
  while (Serial.available()) {
    char incomingChar = Serial.read();  // Read each character from the buffer
    
    if (incomingChar == '\n') {  // Check if the user pressed Enter (new line character)
      // Print the message
      Serial.print("You sent: ");
      Serial.println(receivedMessage);
      String assistant_response = SendCompletion(chatgpt_token, chatgpt_model, receivedMessage);

      if (assistant_response != NULL) {
        Serial.print(F("Assistant: "));
        Serial.println(assistant_response.c_str());
      }
    
      // Clear the message buffer for the next input
      receivedMessage = "";
    } else {
      // Append the character to the message string
      receivedMessage += incomingChar;
    }
  }
}
