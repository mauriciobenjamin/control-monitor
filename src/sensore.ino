int uvSen = A2;
float rangoBruto = 1024;
float rangoLog = 5.0;

void setup() {
  Serial.begin(9600);
  Serial.println("Prueba del sensor UV");
}

void loop() {
  int valorBruto = analogRead(uvSen);

  Serial.print("Bruto = ");
  Serial.print(valorBruto);
  Serial.print(" - mW/cm² = ");
  Serial.println(brutoaW(valorBruto));
  delay(1000);
}

float brutoaW(int bruto) {
  float watt = bruto * 3.3 / rangoBruto;
  return (watt * 4.77);
}
