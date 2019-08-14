const int analog_ip = A0;
const int outputGPIO = 5;
float baseValue = 0;

void setup() { 
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(outputGPIO, OUTPUT);
  // Set output to HIGH
  digitalWrite(outputGPIO, HIGH);
  delay(500);              // wait for half second

  for(int x = 700; x> 0; x=x-10){
    analogWrite(outputGPIO, x); //PWM
    delay(20);
  }
 
  baseValue = analogRead (analog_ip);
}

void loop() {

  float lightVal = analogRead (analog_ip); // Analog Values 0 to 1024
  int scaledVal = abs(1024*((lightVal-baseValue)/(1024-baseValue)));
  analogWrite(outputGPIO, scaledVal);
  Serial.println("Photoresistor Value: " + String(lightVal) + " | Scaled Value: "+ String(scaledVal));
  delay(10);

}
