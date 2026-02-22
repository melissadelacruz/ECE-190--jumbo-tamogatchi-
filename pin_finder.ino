void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCLK: ");
  Serial.println(SCK);
  Serial.print("CS: ");
  Serial.println(SS);  
}

void loop() { 
  
}
