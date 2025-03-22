#include <Arduino.h>
#include <WiFi.h>

#include "config.h"  // Configuration file.

#include "RestAPI.hpp"
#include <Audio.h>

#define MAX98357_I2S_DOUT      25
#define MAX98357_I2S_BCLK      27
#define MAX98357_I2S_LRC       26

Audio max98357;

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

int getSentencePauseOffset(const char* text, int start_offset, int end_offset) 
{
  if ((text == NULL) || (text[0] == '\0') || (end_offset <= start_offset)) {
    return -1;
  }

  int char_count = end_offset - start_offset;

  // For really small sentences we do not need to further split the sentence.
  if (char_count < max_sentence_length) {
    return end_offset;
  }

  for (int x = start_offset; x < end_offset; x++) {
    char c = text[x];

    // Check for punctuation to identify a sentence ending.
    if ((c == ';') || (c == ':') || (c == ',')) {
      // Check next char.
      if (x == end_offset - 1) {
        // No next char, so we can return.
        return x + 1;  
      }

      char next_char = text[x + 1];

      if ((next_char == ' ') || (next_char == '\n') || (next_char == '\r') || (next_char == '\0'))
        return x + 1;
    }
  }

  // Cross your fingers!
  // Nothing found to split the sentence even further so it will return the end offset.
  return end_offset;
}

int getSentenceEndOffset(const char* text, int offset) {
  if ((text == NULL) || (text[0] == '\0')) {
    return -1;
  }
  
  int textLength = strlen(text);

  for (int x = offset; x < textLength; x++) {
    char c = text[x];

    // Check for punctuation to identify a sentence ending.
    if ((c == '.') || (c == '?') || (c == '!') || (c == '\n') || (c == '\r') || (c == '\0')) {
      // Check next char.
      if (x == textLength - 1) {
        if ((max_sentence_length > 0) && (textLength > max_sentence_length)) {
          // Try to split the sentence even further.
          return getSentencePauseOffset(text, offset, x + 1);
        } else { 
          // No next char, so we can return.
          return x + 1;
        }
      }

      char next_char = text[x + 1];

      if ((next_char == ' ') || (next_char == '\n') || (next_char == '\r') || (next_char == '\0'))
        if ((max_sentence_length > 0) && (textLength > max_sentence_length)) {
          // Try to split the sentence even further.
          return getSentencePauseOffset(text, offset, x + 1);
        } else { 
          return x + 1;
        }
    }
  }

  // Nothing found and we have reached the end of the text.
  if (textLength > (offset + 1)) {
    return textLength + 1;
  } else {
    // The default is -1.
    return -1;
  }
}

void setup() {
  // Start the Serial Monitor at a baud rate of 115200
  Serial.begin(115200);

  // Connect to WiFi.
  ConnectWiFi_STA();

  // Audio out
  max98357.setPinout(MAX98357_I2S_BCLK, MAX98357_I2S_LRC, MAX98357_I2S_DOUT);
  max98357.setVolume(100);
  
  // Print an initial message to the Serial Monitor
  Serial.println("ESP32 is ready.");
  Serial.println("Please enter a message:");
}

void loop() {
  max98357.loop();
  
  // Check if data is available in the Serial buffer
  while (Serial.available()) {
    char incomingChar = Serial.read();  // Read each character from the buffer
    
    if (incomingChar == '\n') {  // Check if the user pressed Enter (new line character)
      // Print the message
      Serial.print("You sent: ");
      Serial.println(receivedMessage);
      const char* received_message = receivedMessage.c_str();
      String assistant_response = SendCompletion(chatgpt_token, chatgpt_model, received_message);

      if (assistant_response != NULL) {
        Serial.print(F("Assistant: "));
        Serial.println(assistant_response.c_str());

        int sentence_start_pos = 0;

        int sentence_end_offset = getSentenceEndOffset((const char*)assistant_response.c_str(), sentence_start_pos);

        while((sentence_end_offset = getSentenceEndOffset((const char*)assistant_response.c_str(), sentence_start_pos)) > 0) {
          String sentence = assistant_response.substring(sentence_start_pos, sentence_end_offset);

          if (sentence != NULL) {
            sentence.trim();

            if ((sentence != NULL) && (sentence.length() > 0)) {
#ifdef DEBUG
              Serial.print(F("[Sentence] >>> "));
              Serial.println(sentence.c_str());
#endif

              max98357.connecttospeech(sentence.c_str(), tts_lang);
            
              while(max98357.isRunning()) {
                max98357.loop();
              }
            }
          }
          
          sentence_start_pos = sentence_end_offset;
        }
      }
    
      // Clear the message buffer for the next input
      receivedMessage = "";

      Serial.println("Please enter a message:");
    } else {
      // Append the character to the message string
      receivedMessage += incomingChar;
    }
  }
}
