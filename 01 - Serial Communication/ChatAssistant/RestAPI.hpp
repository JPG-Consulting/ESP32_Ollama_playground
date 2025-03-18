#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

void processResponse(int httpCode, HTTPClient& http)
{
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {      
      // Get the raw and the decoded stream
      Stream& rawStream = http.getStream();
      ChunkDecodingStream decodedStream(http.getStream());

      // Choose the right stream depending on the Transfer-Encoding header
      Stream& response =
        http.header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;
    
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, response);
      
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return; 
      }
      
      const char* response_content = doc["choices"][0]["message"]["content"];

      Serial.print(F("Assistant replied: "));
      Serial.println(response_content);
    }
    else
    {
      Serial.printf("Response code: %d\t", httpCode);
    }
  }
  else {
    Serial.printf("Request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void SendCompletion(String token, String model, String newData)
{
  HTTPClient http;
  http.begin(ApiHost);
  http.addHeader("Content-Type", "application/json");
  String token_key = String("Bearer ") + token;
  http.addHeader("Authorization", token_key);
  
  String message = "";
  
  StaticJsonDocument<300> jsonDoc;
  
  jsonDoc["model"] = model;
  jsonDoc["messages"][0]["role"] = "user";
  jsonDoc["messages"][0]["content"] = newData;
  
  serializeJson(jsonDoc, message);

  http.setTimeout(65500);
  int httpCode = http.POST(message);
  processResponse(httpCode, http);
}
