#include <virtuabotixRTC.h>

#include <SimpleDHT.h> //Biblioteca do sensor de temperatura
#include <LiquidCrystal_I2C.h> //Biblioteca do display LCD
#define sensorUmidade 2 // sinal do sensor de umidade do solo
#define pinLed 8 // Bomba d'água
#define controleDaTela 3 // entrada que irá receber os sinais para mudança na tela

#define   clk   11
#define   dat   10
#define   rst   9


// ========================================================================================================
// --- Constantes Auxiliares ---
#define   segL       00
#define   minL       26
#define   horL       12
#define   d_semL      2
#define   d_mesL     27
#define   mesL       11
#define   anoL     2023
// ========================================================================================================
// --- Declaração de Objetos ---
virtuabotixRTC   myRTC(clk, dat, rst);         //declara objeto para o RTC
// ========================================================================================================


int pinDHT11 = A0; //Pino que faz a leitura de umidade e temperatura
int modoDeApresentacao = 0;
int diasParaColheita = 50;
int comparador = 50;
int diasQuePassaram = 0;
SimpleDHT11 dht11(pinDHT11);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  // put your setup code here, to run once:
  pinMode(controleDaTela, INPUT); //recebe o sinal responsável por alterar o que é mostrado no LCD
  pinMode(pinLed, OUTPUT); //Saída responsável por acionar a bomba d'água
  pinMode(sensorUmidade, INPUT); //recebe o sinal digital do sensor LOW para molhado e HIGH para terra seca
  lcd.init(); //função responsável por iniciar o LCD
  lcd.backlight(); //função responsável por manter a luz de fundo do LCD ligado
  //myRTC.setDS1302Time(segL, minL, horL, d_semL, d_mesL, mesL, anoL);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  umidadeDaTerra();
  temperatura();
  tela();
  
  myRTC.updateTime();
  if(myRTC.hours == 23 && myRTC.minutes == 59){
    if(comparador != diasQuePassaram){
      diasParaColheita = diasParaColheita - 1;
      diasQuePassaram ++;
    }
  }  
}

void umidadeDaTerra() {
  if (digitalRead(sensorUmidade) == LOW){ //condição que identifica quando o solo está molhado
    digitalWrite(pinLed,HIGH);
    //Serial.print("1");
  } else {
    digitalWrite(pinLed, LOW);
    //Serial.print("0");
  }

}


void temperatura() {
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }
  
  //Serial.print((int)temperature); Serial.print(" *C, "); 
  //Serial.print((int)humidity); Serial.println(" H");
  
  // DHT11 sampling rate is 1HZ.
  delay(500);
}


void tela() {
  
  if(digitalRead(controleDaTela) == LOW) { //função que muda o que é mostrado no display
    modoDeApresentacao ++;
    Serial.print(modoDeApresentacao);
    delay(100);
  }

  if(modoDeApresentacao == 3){
    modoDeApresentacao = 0;
    Serial.print(modoDeApresentacao);
  }

  while(digitalRead(controleDaTela) == LOW){ //impede que o pressionar do botão continue contando

  }


  if(modoDeApresentacao == 0) { //tela responsável por exibir as informaçoes de umidade do solo
    if(digitalRead(sensorUmidade) == LOW){
      lcd.clear();
      lcd.setCursor(0, 0);
        lcd.print("umidade(solo):");
      lcd.setCursor(1, 1);
        lcd.print("nivel bom"); 
    }
    if(digitalRead(sensorUmidade) == HIGH){
      lcd.clear();
      lcd.setCursor(0, 0);
        lcd.print("umidade(solo):");
      lcd.setCursor(1, 1);
        lcd.print("nivel baixo");
    }
  }
  
  if(modoDeApresentacao == 1) { //tela responsável por apresentar as informações de temperatura e umidade relativa do ar
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
      Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
      return;
    }
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 0);
      lcd.print("temperatura:");
      lcd.print((int)temperature);
      lcd.print("*C");
    lcd.setCursor(0, 1);
      lcd.print("umidade:");
      lcd.print((int)humidity);
      lcd.print("%");
  }

  if(modoDeApresentacao == 2) { //tela responsável por apresentar quantos dias faltam para a colheita
    lcd.clear();
    lcd.setCursor(0, 0);
      lcd.print("colheita:");
    lcd.setCursor(0, 1);
      lcd.print("faltam ");
      lcd.print(diasParaColheita);
      lcd.print(" dias");
  }
  
  
}
