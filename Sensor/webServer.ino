String insertString(String indexString, String startString, int startOffset, String endString, int endOffset, String inString, String insert) {
  int index = inString.indexOf(indexString);
  int startLocation = inString.indexOf(startString,index) + startOffset;
  int endLocation = inString.indexOf(endString,index) + endOffset;
  String beginString = inString;
  beginString.remove(startLocation);
  inString.remove(0,endLocation);
  return beginString + insert + inString;
}

void updateServer() {
  
  char path[14] = "/variables.js";
  String f = readFile(SPIFFS,path);
  
  String message = "";
     
  //set ipAddress
  message = WiFi.isConnected() ? WiFi.localIP().toString().c_str() : WiFi.softAPIP().toString().c_str();
  f = insertString("ipAddress", "\"", 1, ";", -1, f, message);

  //set wsPort
  message = (String)wsPort;
  f = insertString("wsPort", "\"", 1, ";", -1, f, message);

  char fChar[f.length()];
  stringToChar(f,fChar);
  writeFile(SPIFFS, path, fChar);
}

void handleRoot() {
  //webServer.send(200, "text/plain", "hello from ESP32!");
  handleFileRead("/");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i=0; i<webServer.args(); i++){
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".jpg")) return "image/jpg";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if(debug) Serial.println("handleFileRead: " + path);

  if (path.endsWith("/")) {
    path += "index.html";         // If a folder is requested, send the index file
    updateServer();
  }
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = webServer.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  if(debug) Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

String readFile(fs::FS &fs, const char * path){
  if(debug) Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");

  if(!file || file.isDirectory()){
    if(debug) Serial.println("- empty file or failed to open file");
    if (path == "/") {
      file.close();
      return getEmptyIndex();
    }
    else {
      file.close();
      return String();
    }
  }
  if(debug) Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  //Serial.println(fileContent);
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  if(debug) Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }

  int bytesWritten = file.print(message);
  
  if (debug) {
    Serial.println("Bytes Written: " + (String)bytesWritten);
    if(bytesWritten >= 0){
      Serial.println("- file written");
    } else {
      Serial.println("- write failed");
    }
  }
  
  file.close();
}

String getEmptyIndex() {
  String message = "Could not find the requested page, please make sure you have uploaded the latest webserver files";
}
