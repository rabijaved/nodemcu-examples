#include <ESP8266WiFi.h>

//Change WiFi credentials here if required
const char* ssid     = "Hello";
const char* password = "World";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connect then proceed
    Serial.println("New Client."); 
    String currentLine = "";     
    while (client.connected()) { 
      if (client.available()) {    

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head>");
            client.println("<body><b><h1>Hello World</h1></b></body>");
            client.println("</html>");
            client.println();
            
            break;
        }
      }
    

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  
  }
}
