void loop {
  //Pausa hasta que los botones se liberen
  while (leerBotones() != 0) {}

  lcd.clear();
  Serial.print(opEstados);

  switch (opEstados) {
    case APAGAR:
      Off();
      break;

    case SELEC:
      selecCiclo();
      break;

    case PROGR:
      programar();
      break;

    case INICIO:
      iniciar();
      break;
  }
}
