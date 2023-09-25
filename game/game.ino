#include <LiquidCrystal.h>
#include <stdlib.h>


//******* CONFIGURACAO DO PROJETO *********
#define pinBotoes A0

#define pinRs 8
#define pinEn 9
#define pinD4 4
#define pinD5 5
#define pinD6 6
#define pinD7 7
#define pinBackLight 10
//*****************************************

#define btNENHUM 0
#define btSELECT 1
#define btLEFT   2
#define btUP     3
#define btDOWN   4
#define btRIGHT  5

#define tempoDebounce 50

byte jogadorPe[8] = {
  B11111,
  B10101,
  B11111,
  B11111,
  B01110,
  B01010,
  B01010,
  B11011
};

byte jogadorInv[8] = {
  B11011,
  B01010,
  B01010,
  B01110,
  B11111,
  B11111,
  B10101,
  B11111
};

byte obs[8] = {
  B00000,
  B01110,
  B10101,
  B11011,
  B01110,
  B01110,
  B00000,
  B00000
};

// byte coracao[8] = {
//   B00000,
//   B00000,
//   B01010,
//   B11111,
//   B11111,
//   B01110,
//   B00100,
//   B00000
// };

byte jogadorPeMorte[8] = {
  B11111,
  B10101,
  B11111,
  B10001,
  B11111,
  B01110,
  B01010,
  B11011
};

byte jogadorInvMorte[8] = {
  B11011,
  B01010,
  B01010,
  B01110,
  B10001,
  B11111,
  B10101,
  B11111
};

struct Obstaculo {
  int x;
  int y;
  bool visivel;
  unsigned long ultimoObstaculo;
  int wait;
};


unsigned long intervaloBase = 300; 
unsigned long diminuicaoPorPontuacao = 5; 
unsigned long intervaloMinimo = 150; 

unsigned long delayBotao;
unsigned long ultimoObstaculo = 0;
unsigned long pontuacao = 0;

unsigned long intervalo = 250;

int estadoBotaoAnt = btNENHUM;

void estadoBotao(int botao);
void botaoApertado(int botao);
void botaoSolto(int botao);
void done(Obstaculo ob);
int calculaProbabilidade(int pontuacao);
unsigned long getIntervalo(int pontuacao);

LiquidCrystal lcd(pinRs, pinEn, pinD4, pinD5, pinD6, pinD7);

// Para o exemplo de uso
String descBotao[6] = {"", "Select", "Esquerda", "Abaixo", "Acima", "Direita"};
int contador = 0;
bool allWaiting = true;



//quadrado
int posX = 1; 
int posY = 1; 

//Obstaculo

const int maxObstaculos = 8;
Obstaculo obstaculos[maxObstaculos];



bool perdeu = false;

void setup() {
  pinMode(pinBackLight, OUTPUT);
  digitalWrite(pinBackLight, HIGH);

  lcd.createChar(0, jogadorPe);
  lcd.createChar(1, obs);
  lcd.createChar(2, jogadorInv);
// //  lcd.createChar(3, coracao);
  lcd.createChar(4, jogadorPeMorte);
  lcd.createChar(5, jogadorInvMorte);


  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.clear();
  lcd.setCursor(posX, posY);
  lcd.write(byte(0)); // Desenha o quadrado

  randomSeed(analogRead(millis()));

  for (int i = 0; i < maxObstaculos; i++) {
    obstaculos[i].x = 15; // Posição inicial no canto direito
    obstaculos[i].y = random(2); // Aleatoriamente na primeira ou segunda linha
    obstaculos[i].visivel = false;
    obstaculos[i].ultimoObstaculo = 0;
    obstaculos[i].wait = -1;
  }
  
  

}

void loop() {
  randomSeed(analogRead(millis()));
  Serial.println(pontuacao);
  int valBotoes = analogRead(pinBotoes);

  if ((valBotoes < 800) && (valBotoes >= 600)) {
    estadoBotao(btSELECT);
  } else if ((valBotoes < 600) && (valBotoes >= 400)) {
    estadoBotao(btLEFT);
  } else if ((valBotoes < 400) && (valBotoes >= 200)) {
    estadoBotao(btUP);
  } else if ((valBotoes < 200) && (valBotoes >= 60)) {
    estadoBotao(btDOWN);
  } else if (valBotoes < 60) {
    estadoBotao(btRIGHT);
  } else {
    estadoBotao(btNENHUM);
  }



  for(int i=0; i<maxObstaculos; i++){
    
    if (!obstaculos[i].visivel) {
      int probabilidade = calculaProbabilidade(pontuacao);

      if (random(100) < probabilidade){
        obstaculos[i].y = (random(0, 2)); // Aleatoriamente na primeira ou segunda linha
        obstaculos[i].wait =random(2,7);
        int w = obstaculos[i].wait;
        obstaculos[i].visivel = true;


        for(int j=0; j<maxObstaculos; j++){
          int w2 = obstaculos[j].wait;
          if((w2!=-1)&&(i!=j)&&(obstaculos[j].y!=obstaculos[i].y)){
            if((w2==w) || (w2==w-1) || (w2==w+1)){
              obstaculos[i].wait=-1;
              obstaculos[i].visivel = false;
            }
          }
        }


      }
    }
    else{
      
      if((millis() - obstaculos[i].ultimoObstaculo >= intervalo)){
        if(obstaculos[i].wait>0){
          obstaculos[i].wait--;
          obstaculos[i].ultimoObstaculo = millis();
          
          continue;
        }
        else{
          lcd.setCursor(obstaculos[i].x, obstaculos[i].y);

          lcd.print(" "); // Apaga o obstáculo da posição atual
          obstaculos[i].x--; // Move o obstáculo uma coluna para a esquerda

          if (obstaculos[i].x >= 0) {

            lcd.setCursor(obstaculos[i].x, obstaculos[i].y);

            
            lcd.write(byte(1)); // Redesenha o obstáculo na nova posição
            
            if((obstaculos[i].x==posX) && (obstaculos[i].y==posY)){
              done(obstaculos[i]);
            }
          }else{
            obstaculos[i].visivel = false; // O obstáculo desaparece quando chega à esquerda
            obstaculos[i].x = 15; // Reinicia no canto direito
            obstaculos[i].wait = -1;
            pontuacao++;
          }
          obstaculos[i].ultimoObstaculo = millis();
        }
      }
    }
    
  }
}

void estadoBotao(int botao) {
  // Quando um botao estiver apertado
  if (botao != btNENHUM) {
    //Serial.println(botao);
  }

  // Quando o botao for apertado ou solto
  if ((millis() - delayBotao) > tempoDebounce) {
    if ((botao != btNENHUM) && (estadoBotaoAnt == btNENHUM)) {
      botaoApertado(botao);
      delayBotao = millis();
    }

    if ((botao == btNENHUM) && (estadoBotaoAnt != btNENHUM)) {
      botaoSolto(estadoBotaoAnt);
      delayBotao = millis();
    }
  }
  estadoBotaoAnt = botao;
}


unsigned long getIntervalo(int pontuacao) {
unsigned long intervalo = intervaloBase - (pontuacao * diminuicaoPorPontuacao);
  
  // Certifique-se de que o intervalo não seja menor que o intervalo mínimo
  if (intervalo < intervaloMinimo) {
    intervalo = intervaloMinimo;
  }
  
  return intervalo;
}

void done(Obstaculo ob){

  lcd.clear();
  lcd.setCursor(0, 0); // Define o cursor para a primeira linha
  lcd.print("Game Over");
  lcd.setCursor(0, 1); // Define o cursor para a segunda linha
  lcd.print("Pontuacao: ");
  lcd.print(pontuacao);
  while(true);

}

void botaoApertado(int botao) {
  // Quando um botão for apertado

  // Para o exemplo de uso
  if (botao == btDOWN && posY > 0) {
    lcd.setCursor(posX, posY);
    lcd.print(" "); // Apaga o quadrado da posição atual
    posY--; // Move o quadrado uma linha para cima
    lcd.setCursor(posX, posY);
    for(int i=0; i<maxObstaculos; i++){
      if (obstaculos[i].wait != 1 && (obstaculos[i].x == posX) && (obstaculos[i].y == posY)){
        done(obstaculos[i]);
      }
    }

    if(random(100)<10)
      lcd.write(byte(5)); // Redesenha
    else
      lcd.write(byte(2)); // Redesenha


  } else if (botao == btUP && posY < 1) {
    lcd.setCursor(posX, posY);
    lcd.print(" "); // Apaga o quadrado da posição atual
    posY++; // Move o quadrado uma linha para baixo
    lcd.setCursor(posX, posY);
    for(int i=0; i<maxObstaculos; i++){
      if (obstaculos[i].wait != 1 && (obstaculos[i].x == posX) && (obstaculos[i].y == posY)){
        done(obstaculos[i]);
      }
    }
    if(random(100)<10)
      lcd.write(byte(4)); // Redesenha
    else
      lcd.write(byte(0)); // Redesenha
    
  }
}

int calculaProbabilidade(int pontuacao) {
  // Ajuste esses valores de acordo com a sua preferência
  int probabilidadeBase = 20; // Probabilidade inicial
  int aumentoPorPontuacao = 5; // Aumento na probabilidade por ponto

  // Calcula a probabilidade com base na pontuação
  int probabilidade = probabilidadeBase + (pontuacao * aumentoPorPontuacao);

  // Limite a probabilidade para um valor máximo (opcional)
  int probabilidadeMaxima = 90; // Por exemplo, 90%
  if (probabilidade > probabilidadeMaxima) {
    probabilidade = probabilidadeMaxima;
  }

  return probabilidade;
}

void botaoSolto(int botao) {
  // Quando um botão for solto

  // Para o exemplo de uso
  lcd.setCursor(0, 1);
  //lcd.print(descBotao[botao]);
}