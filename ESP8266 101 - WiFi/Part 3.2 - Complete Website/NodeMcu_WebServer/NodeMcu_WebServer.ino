#include <ESP8266WiFi.h>

//Change WiFi credentials here if required
const char* ssid     = "Hello";
const char* password = "World";

//IO Port Constants 
String outputGPIOState = "off";
const int analog_ip = A0;
const int outputGPIO = 5;

WiFiServer server(80);
String header;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(outputGPIO, OUTPUT);
  // Set outputs to LOW
  digitalWrite(outputGPIO, LOW);

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

        // Read the first line of the request: Concat char to the header variable to form a request string        
        char c = client.read();          
        Serial.write(c);
                         
        header += c;
        if (c == '\n') {  //This checks if the request data is complete                          
          if (currentLine.length() == 0) {

            //Tell the client the request is succesfully recieved and to close the connection (important)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //Handle the AJAX request for the LED Brightness slider
            if(header.indexOf("slider") > 0){
  
              String myCommand = header.substring(header.indexOf("slider/")+7);
              analogWrite(outputGPIO, myCommand.toInt());
              break;
              
            }
            //Handle the AJAX request for the PhotoResistor Ticker
            else if(header.indexOf("phStaus") > 0){

              float lightVal = analogRead (analog_ip); // Analog Values 0 to 1024
              int normlightVal = 100- ((lightVal/1024)*100); //Normalize it to values between 0 and 100%
              client.println(normlightVal); //Send the data to client
              break;
            }
            // turns the GPIO on and off
            //Alternate way to do things. Not done through AJAX. Use GET Path to toggle
            else if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              outputGPIOState = "on";
              digitalWrite(outputGPIO, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              outputGPIOState = "off";
              digitalWrite(outputGPIO, LOW);
            } 
            
            
            //Some Javascipt to handle realtime updates through AJAX
            //You can send response either as a string literal or through console.println 
            String html_1 = R"=====(
            <script>
                var ajaxRequest = null;
                if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
                else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }

               window.onload=getStatus;
               function getStatus(){
                 setTimeout(getStatus, 4000);
                 ajaxLoad("phStaus","");
                 
               }
                function updateSlider(slideAmount) {
                    var sliderDiv = document.getElementById("sliderAmount");
                    sliderDiv.innerHTML = slideAmount;
                    ajaxLoad("slider",slideAmount);
                }
                function updatePHValue(myVal) {
                    var phButton = document.getElementById("phresistor01");
                    phButton.innerHTML = myVal;
                }
                function ajaxLoad(type,value)
                {
                  if(value !="") value = "/" + value;
                  var serverReq = type + value;
                  if(!ajaxRequest){ alert("AJAX is not supported."); return; }
                 
                  ajaxRequest.open("GET",serverReq,true);
                  ajaxRequest.onreadystatechange = function()
                  {
                    if(ajaxRequest.readyState == 4 && ajaxRequest.status==200)
                    {
                      var ajaxResult = ajaxRequest.responseText;
                      if( type=="phStaus") updatePHValue(ajaxResult + "%");
                    }
                  }
                  ajaxRequest.send();
                }
                
            </script>
            )=====";
            //Read the analogue value: for initialization
            float lightVal = analogRead (analog_ip); // Analog Values 0 to 1024
            int normlightVal = 100-((lightVal/1024.0)*100);
  
            // Display the HTML web page
            client.println(html_1);
           
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: powderblue; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            client.println("<body style=""background-color:#195B6A;""><b><h1>ESP8266 Web Server</h1></b></br></br></br>");
            client.println("<b><p style=\"color: white;\">GPIO 5 - State " + outputGPIOState + "</p></b>");
            // If the outputGPIOState is off, it displays the ON button       
            if (outputGPIOState=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            client.println("</br><div class=\"slidecontainer\"><b><p style=\"color: white;\">LED Brightness Slider: PWM</p></b><input style=\"width: 400px;\" type=\"range\" min=\"0\" max=\"1024\" value=\"0\" onchange=\"updateSlider(this.value)\"><div style=\"color: white;\" id=\"sliderAmount\"></div></div>");
            client.println("</br><b><p style=\"color: white;\">Room Brightness: Analog Input</p></b>");
            client.println("<b><p><button id=\"phresistor01\" class=\"button\">"+ String(normlightVal) + " %</button></p></b>");
            client.println("</body></html>");
            client.println();
            
            break;
            
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;
        }
      }
    }
    header = "";
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
