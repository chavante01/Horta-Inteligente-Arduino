#include <virtuabotixRTC.h>

#include <SimpleDHT.h> //Biblioteca do sensor de temperatura
#include <LiquidCrystal_I2C.h> //Biblioteca do display LCD
#define sensorUmidade 2 // sinal do sensor de umidade do solo
#define pinLed 8 // indicador de solo molhado
#define ledAlerta 7 //Bomba d'água
#define controleDaTela 3 // entrada que irá receber os sinais para mudança na tela
#define ledAtencao 6 // avisa que a bomba pode ligar


#define   clk   11
#define   dat   10
#define   rst   9


// ========================================================================================================
// --- Constantes Auxiliares ---
#define   segL       00
#define   minL       24
#define   horL       00
#define   d_semL      6
#define   d_mesL     01
#define   mesL       12
#define   anoL     2023
// ========================================================================================================
// --- Declaração de Objetos ---
virtuabotixRTC   myRTC(clk, dat, rst);         //declara objeto para o RTC
// ========================================================================================================


int pinDHT11 = A0; //Pino que faz a leitura de umidade e temperatura
int modoDeApresentacao = 0;
int diasParaColheita = 47;
int comparador = 50;
int diasQuePassaram = 0;
int temporizadorParaAcionarBomba = 0;//essa variável é utilizada para segurança da bomba d'água, ela só irá acionar depois de 5 minutos que o sistema tiver passado para o estado "seco"
int temporizadorParaDesligarBomba = 0;//essa variável é utilizada para segurança da bomba d'água, ela só irá acionar depois de 1 minuto que o sistema tiver passado para o estado "molhado"
SimpleDHT11 dht11(pinDHT11);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  // put your setup code here, to run once:
  pinMode(controleDaTela, INPUT); //recebe o sinal responsável por alterar o que é mostrado no LCD
  pinMode(ledAtencao, OUTPUT); //ativa o LED de atenção para o acionamento da bomba d'água
  pinMode(ledAlerta, OUTPUT); //ativa o LED que indica que a bomba d'água está em funcionamento
  pinMode(pinLed, OUTPUT); //Saída responsável por acionar a bomba d'água
  pinMode(sensorUmidade, INPUT); //recebe o sinal digital do sensor LOW para molhado e HIGH para terra seca
  lcd.init(); //função responsável por iniciar o LCD
  lcd.backlight(); //função responsável por manter a luz de fundo do LCD ligado
  //myRTC.setDS1302Time(segL, minL, horL, d_semL, d_mesL, mesL, anoL);   //é de suma importância que você descomente essa linha e rode o código 
  Serial.begin(9600);                                                      //uma única vez para setar esse valor no módulo RTC, após isso comente esta linha novamente
}

void loop() {
  // put your main code here, to run repeatedly:
  umidadeDaTerra();
  temperatura();
  tela();
  
  myRTC.updateTime();
  if(myRTC.hours == 23 && myRTC.minutes == 59 && myRTC.seconds == 59){ // se passarem 24h, então decresça um dia do tempo para a colheita
    if(comparador != diasQuePassaram){
      delay(1000);
      diasParaColheita = diasParaColheita - 1;
      diasQuePassaram ++;
    }
  }  
}

void umidadeDaTerra() {
  //myRTC.updateTime();
  //Serial.print(myRTC.seconds);
  //Serial.print(" //");
  //Serial.print(temporizadorParaDesligarBomba);
  //Serial.print(" //");

  if(digitalRead(sensorUmidade) == LOW){
    if(myRTC.seconds == 59){
      temporizadorParaDesligarBomba ++;
    }
    if(temporizadorParaDesligarBomba < 2) {
      digitalWrite(pinLed, LOW); //indicador de solo molhado
      digitalWrite(ledAlerta, HIGH); //bomba d'água
      digitalWrite(ledAtencao, HIGH); //indica que o estado vai passar de "seco" para "molhado"
     }

    if(temporizadorParaDesligarBomba == 2){
      temporizadorParaAcionarBomba = 0; //reseta o contador de segurança
      digitalWrite(pinLed, HIGH); //indicador de solo molhado
      digitalWrite(ledAlerta, LOW); //bomba d'água
      digitalWrite(ledAtencao, LOW);
    }
  }
  
  if(digitalRead(sensorUmidade) == HIGH){ //condição que identifica solo seco
    if(myRTC.seconds == 59) {  // esse contador tem uma imprecisão de alguns segundos, mas serão suficientes para que a bomba só acione quando de fato necessário
      temporizadorParaAcionarBomba ++;
    }
    if( (temporizadorParaAcionarBomba > 0) && (temporizadorParaAcionarBomba < 5) ) {
      digitalWrite(pinLed, LOW);
      digitalWrite(ledAlerta, LOW); // bomba d'água
      digitalWrite(ledAtencao, HIGH);
    }
    if(temporizadorParaAcionarBomba == 5) {
      temporizadorParaDesligarBomba = 0;
      digitalWrite(ledAtencao, LOW);
      digitalWrite(pinLed, LOW); //indicador de solo molhado
      digitalWrite(ledAlerta, HIGH); // bomba d'água 
    }
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
  
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" H");
  
  // DHT11 sampling rate is 1HZ.
  delay(500);
}


void tela() {

  if(digitalRead(controleDaTela) == LOW) { //função que muda o que é mostrado no display
    modoDeApresentacao ++;
    //Serial.print(modoDeApresentacao);
    delay(500);
  }

  if(modoDeApresentacao == 4){
    modoDeApresentacao = 0;
    //Serial.print(modoDeApresentacao);
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
  
  if(modoDeApresentacao == 3){
    lcd.clear();
    lcd.setCursor(3, 0);
      if(myRTC.hours < 10) lcd.print("0");
      lcd.print(myRTC.hours);
      lcd.print(":");
      if(myRTC.minutes < 10) lcd.print("0");
      lcd.print(myRTC.minutes);
      lcd.print(":");
      if(myRTC.seconds < 10) lcd.print("0");
      lcd.print(myRTC.seconds);
    lcd.setCursor(2, 1);
      if(myRTC.dayofmonth < 10) lcd.print("0");
      lcd.print(myRTC.dayofmonth);
      lcd.print("/");
      if(myRTC.month < 10) lcd.print("0");
      lcd.print(myRTC.month);
      lcd.print("/");
      lcd.print(myRTC.year);
  }
  
}
