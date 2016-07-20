/* Modulo de prueba de la placa de 4 relevadores */
int rel1 = A2;
int rel2 = A3;
int rel3 = A4;
int rel4 = 13;

void setup() {
    pinMode(rel1, OUTPUT);
    pinMode(rel2, OUTPUT);
    pinMode(rel3, OUTPUT);
    pinMode(rel4, OUTPUT);
}

void relOn(rel) {
    digitalWrite(rel,HIGH); //cierra el relevador
    delay 2000;
    digitalWrite(rel,DOWN); //abre el relevador
}

void loop() {
    relOn(rel1);
    relOn(rel2);
    relOn(rel3);
    relOn(rel4);
}