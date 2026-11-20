#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

#define MODO_SIMULADO 1
#define INVERTER_POT 0
#define ESTADO_CONTROLA_MOTOR 0   // 0 = motor segue so o pot | 1 = estado limita o motor
#define TESTE_MOTOR 0             // 1 = motor sempre a 100% (testa alimentacao/MOSFET)

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int ledR = 3;
const int ledG = 5;
const int ledB = 6;

const int pinoPot   = A0;
const int pinoPiezo = A1;
const int pinoMotor = 10;

const int PWM_ENSAIO = 255;

const int N_AMOSTRAS = 100;
const unsigned long INTERVALO_US = 1000;
const int JANELAS_CALIBRACAO = 10;

const float LIMITE_ATENCAO = 1.20;
const float LIMITE_CRITICO = 1.50;

const float HISTERESE = 0.05;   // evita ficar alternando entre estados

const float BASELINE_MINIMO = 1.0;

const int PWM_NORMAL  = 255;
const int PWM_ATENCAO = 140;
const int PWM_CRITICO = 0;

const float AMP_BASE = 40.0;
const float FREQ_SIM = 50.0;

float rmsBase = 1.0;
float rmsAtual = 0.0;
float razao = 1.0;

int variacao = 0;
float fatorDesgaste = 1.0;
int pwmPot = 0;     // pedido do potenciometro
int pwmAtual = 0;   // aplicado no motor
int pwmAlvo = 0;    // limite do estado

const char* estado = "";
int nivelEstado = 0;   // 0 = NORMAL, 1 = ATENCAO, 2 = CRITICO

void definirCor(int vermelho, int verde, int azul) {
  analogWrite(ledR, vermelho);
  analogWrite(ledG, verde);
  analogWrite(ledB, azul);
}

void desligarLEDs() {
  definirCor(0, 0, 0);
}

int lerSinal() {
#if MODO_SIMULADO
  // Fase avanca um passo fixo por amostra: 100 amostras = exatamente 5 ciclos,
  // entao o RMS simulado nao oscila por jitter de tempo.
  static float fase = 0.0;
  fase += 2.0 * PI * FREQ_SIM * ((float)INTERVALO_US / 1000000.0);
  if (fase > 2.0 * PI) fase -= 2.0 * PI;

  float v = 512.0 +
            AMP_BASE * fatorDesgaste * sin(fase) +
            random(-3, 4);

  return (int)v;
#else
  return analogRead(pinoPiezo);
#endif
}

float medirRMS() {
  float soma = 0;
  float somaQuad = 0;

  unsigned long proximo = micros();

  for (int i = 0; i < N_AMOSTRAS; i++) {
    while ((long)(micros() - proximo) < 0) {}

    proximo += INTERVALO_US;

    float v = (float)lerSinal();

    soma += v;
    somaQuad += v * v;
  }

  float media = soma / N_AMOSTRAS;
  float variancia = (somaQuad / N_AMOSTRAS) - (media * media);

  if (variancia < 0) variancia = 0;

  return sqrt(variancia);
}

void calibrar() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CALIBRANDO");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");

  fatorDesgaste = 1.0;
  nivelEstado = 0;
  pwmAtual = PWM_ENSAIO;
  pwmAlvo = PWM_ENSAIO;

  analogWrite(pinoMotor, PWM_ENSAIO);
  delay(1500);

  float soma = 0;

  for (int i = 0; i < JANELAS_CALIBRACAO; i++) {
    soma += medirRMS();
  }

  rmsBase = soma / JANELAS_CALIBRACAO;

  if (rmsBase < BASELINE_MINIMO) {
    rmsBase = BASELINE_MINIMO;
  }

  rmsAtual = rmsBase;

  Serial.print("Referencia RMS: ");
  Serial.println(rmsBase, 2);

  lcd.clear();
}

void analisarEstado() {
  razao = rmsAtual / rmsBase;
  variacao = (int)round((razao - 1.0) * 100.0);

  // Sobe com margem acima do limite, desce com margem abaixo (histerese)
  if (nivelEstado == 0 && razao > LIMITE_ATENCAO + HISTERESE) nivelEstado = 1;
  else if (nivelEstado >= 1 && razao < LIMITE_ATENCAO - HISTERESE) nivelEstado = 0;

  if (nivelEstado == 1 && razao > LIMITE_CRITICO + HISTERESE) nivelEstado = 2;
  else if (nivelEstado == 2 && razao < LIMITE_CRITICO - HISTERESE) nivelEstado = 1;

  if (nivelEstado == 0) {
    estado = "NORMAL";
    pwmAlvo = PWM_NORMAL;
    definirCor(0, 255, 0);       // Verde
  }
  else if (nivelEstado == 1) {
    estado = "ATENCAO";
    pwmAlvo = PWM_ATENCAO;
    definirCor(255, 80, 0);      // Laranja
  }
  else {
    estado = "CRITICO";
    pwmAlvo = PWM_CRITICO;
    definirCor(255, 0, 0);       // Vermelho
  }
}

void aplicarMotor() {
#if TESTE_MOTOR
  pwmAtual = 255;
#elif ESTADO_CONTROLA_MOTOR
  pwmAtual = min(pwmPot, pwmAlvo);
#else
  pwmAtual = pwmPot;
#endif
  analogWrite(pinoMotor, pwmAtual);
}

void atualizarLCD() {
  lcd.setCursor(0, 0);
  lcd.print("RMS:");
  lcd.print(rmsAtual, 1);
  lcd.print(" ");

  if (variacao >= 0) lcd.print("+");

  lcd.print(variacao);
  lcd.print("%   ");

  lcd.setCursor(0, 1);
  lcd.print("Estado: ");
  lcd.print(estado);
  lcd.print("    ");
}

void monitorSerial() {
  Serial.print("PWM POT: ");
  Serial.print(pwmPot);

  Serial.print(" | PWM MOTOR: ");
  Serial.print(pwmAtual);

  Serial.print(" | PWM ESTADO: ");
  Serial.print(pwmAlvo);

  Serial.print(" | RMS: ");
  Serial.print(rmsAtual, 2);

  Serial.print(" | BASE: ");
  Serial.print(rmsBase, 2);

  Serial.print(" | VARIACAO: ");
  Serial.print(variacao);
  Serial.print("%");

#if MODO_SIMULADO
  Serial.print(" | DESGASTE: x");
  Serial.print(fatorDesgaste, 2);
#endif

  Serial.print(" | ESTADO: ");
  Serial.println(estado);
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A2));

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  desligarLEDs();

  pinMode(pinoMotor, OUTPUT);
  analogWrite(pinoMotor, 0);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("TOOLGUARD");
  lcd.setCursor(0, 1);
  lcd.print("Sistema Online");

  delay(2000);
  calibrar();
}

void loop() {
  int leituraPot = analogRead(pinoPot);

#if INVERTER_POT
  leituraPot = 1023 - leituraPot;
#endif

  float nivel = leituraPot / 1023.0;
  fatorDesgaste = 3.0 - (2.0 * nivel);

  pwmPot = map(leituraPot, 0, 1023, 0, 255);

  float novo = medirRMS();
  rmsAtual = 0.6 * rmsAtual + 0.4 * novo;

  analisarEstado();
  aplicarMotor();
  atualizarLCD();
  monitorSerial();

  delay(100);
}