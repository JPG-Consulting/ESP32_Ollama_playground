#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

String processResponse(int httpCode, HTTPClient& http)
{
  const char* response_content = NULL;
  
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
        return response_content; 
      }
      
      response_content = doc["choices"][0]["message"]["content"];
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

  return response_content;
}

String SendCompletion(const char * token, const char * model, const char * newData)
{
  HTTPClient http;
  http.begin(ApiHost);
  http.addHeader("Content-Type", "application/json");

  if ((token != NULL) && (token[0] != '\0')) {
    char authorization_bearer[7 + strlen(token)];
    strcpy(authorization_bearer, "Bearer ");
    strcat(authorization_bearer, token);
    http.addHeader("Authorization", authorization_bearer);
  }
  
  String message = "";
  
  StaticJsonDocument<300> jsonDoc;
  
  jsonDoc["model"] = model;
  jsonDoc["messages"][0]["role"] = "user";
  jsonDoc["messages"][0]["content"] = newData;
  
  serializeJson(jsonDoc, message);

  http.setTimeout(65500);
  int httpCode = http.POST(message);
  return processResponse(httpCode, http);
}
