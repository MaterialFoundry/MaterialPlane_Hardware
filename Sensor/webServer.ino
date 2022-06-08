void initializeWebserver() {

  webServer.addHandler(new SPIFFSEditor(SPIFFS, "admin","admin"));

  webServer.on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
        
        // the request handler is triggered after the upload has finished... 
        // create the response, add header, and send response
        AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
        response->addHeader("Connection", "close");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        ESP.restart();
    }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        //Upload handler chunks in data
      
        if (!index) {
            //if(!request->hasParam("MD5", true)) {
            //    return request->send(400, "text/plain", "MD5 parameter missing");
            //}

            //if(!Update.setMD5(request->getParam("MD5", true)->value().c_str())) {
            //    return request->send(400, "text/plain", "MD5 parameter invalid");
            //}

            int cmd = (filename.indexOf("webserver") > -1) ? U_SPIFFS : U_FLASH;
            Serial.println("Cmd: " + (String)cmd + "\tFN: " + (String)filename);
            
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
              Update.printError(Serial);
              return request->send(400, "text/plain", "OTA could not begin");
            }
        }

        // Write chunked data to the free sketch space
        if(len){
            if (Update.write(data, len) != len) {
                return request->send(400, "text/plain", "OTA could not begin");
            }
        }
            
        if (final) { // if the final flag is set then this is the last frame of data
            if (!Update.end(true)) { //true to set the size to the current progress
                Update.printError(Serial);
                return request->send(400, "text/plain", "Could not end OTA");
            }
        }else{
            return;
        }
        
    });
  
  webServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  webServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  webServer.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  /*
  webServer.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  */
  webServer.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  webServer.begin();
  Serial.println("HTTP webserver started\n");
  webserverVersion = getWebserverVersion();
}

String getWebserverVersion() {
  File file = SPIFFS.open("/main.js", FILE_READ);
  String ver = "";
  uint8_t saveState = 0;
  while(file.available()) {
    char c = file.read();
    if (c == ';') break;
    else if (saveState == 0 && c == '"') saveState = 1;
    else if (saveState == 1 && c == 'v') saveState = 2;
    else if (saveState == 2 && c == '"') break;
    else if (saveState == 2) ver += c;
  }
  file.close();
  return ver;
}
