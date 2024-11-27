#include "FastLED.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Setup Led
#define LED_COUNT 15
#define BUTTON_LEN 5
#define LEDS_LEN 5
#define FLOWERS_LEN 15

// Setup LCD
#define col 16
#define lin 2
#define ende 0x27
LiquidCrystal_I2C lcd(ende, col, lin);

// Setup SD
#define CS 4
/*
Modulo cartão SD:
    CS 4
    SCK 13
    MOSI 11
    MISO 12
*/

File file;

// Pinos para os botões
short buttonPins[BUTTON_LEN] = {8, 9, 10, A0, A1};

bool lastState[LEDS_LEN] = {false, false, false, false, false};

// Cores de cada botão
const CRGB buttonsColors[BUTTON_LEN] = {
    CRGB(255, 0, 0),
    CRGB(0, 255, 0),
    CRGB(0, 0, 255),
    CRGB(255, 255, 0),
    CRGB(255, 0, 255)};

// Emoções para serem gravadas no cartão SD
const char *emotions[BUTTON_LEN] = {
    "Feliz",
    "Confuso",
    "Triste",
    "Aborrecido",
    "Zangado"};

// Ordem em que as flores serão acesas
// Primeiro índice corresponde ao índice da fita led e o segundo ao led da flor
const short flowersOrder[FLOWERS_LEN][2] = {
    {0, 14},
    {2, 13},
    {4, 14},
    {1, 12},
    {3, 13},
    {0, 11},
    {4, 11},
    {3, 10},
    {2, 9},
    {1, 9},
    {4, 8},
    {1, 7},
    {3, 7},
    {0, 8},
    {2, 6}};

// Variável que armazena todas as fitas led
CRGB leds[LEDS_LEN][LED_COUNT];

// Indica qual flor será direcionada a animação
short order;

bool state;

// Função que realiza a animação da cor subindo até chegar na flor
void animate(short order, CRGB color)
{
  const short ledsIdx = flowersOrder[order][0];
  const short idx = flowersOrder[order][1];

  CRGB temp = leds[ledsIdx][0];

  for (short i = 0; i <= idx; i++)
  {
    if (i > 0)
    {
      leds[ledsIdx][i - 1] = temp;
      temp = leds[ledsIdx][i];
    }
    leds[ledsIdx][i] = color;

    delay(100);
    FastLED.show();
  }
}

// Função que abre o arquivo das emoções e adiciona uma nova emoção
void writeSD(short i)
{
  file = SD.open("registro_emocoes/emocoes.txt", FILE_WRITE);
  if (!file)
    return;
  file.println(emotions[i]);
  file.close();
  Serial.println(emotions[i]);
}

void resetLCD()
{
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("COMO VOCE SE");
  lcd.setCursor(2, 1);
  lcd.print("SENTE HOJE?");
}

void emotionLCD(short i)
{
  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print(emotions[i]);
}

void setup()
{
  // Inicializando a comunicação serial
  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.backlight();
  resetLCD();

  /*
    // Inicializando a comunicação com o cartão SD
    if (!SD.begin(CS)) {
      Serial.println("Não foi possível acessar o cartão.");
    } else {
      Serial.println("Cartão inicializado.");
      //Verifica se a pasta existe, caso não, cria a pasta.
      const bool file_exists = SD.exists("registro_emocoes");

      if (!file_exists) {
        Serial.println("Criando pasta registro_emocoes.");
        SD.mkdir("registro_emocoes");
      }

      //Verifica se o arquivo existe, caso não, cria o arquivo.
      const bool archive_exists = SD.exists("registro_emocoes/emocoes.txt");

      if (!archive_exists) {
        Serial.println("Criando arquivo registro_emocoes/emocoes.txt.");
        file = SD.open("registro_emocoes/emocoes.txt", FILE_WRITE);
        file.close();
      }
    }
    */

  // Inicializando uma seed geradora de números pseudoaleatórios
  randomSeed(analogRead(A3));

  // Declarando os pinos dos botões como INPUT
  for (int i = 0; i < BUTTON_LEN; i++)
  {
    pinMode(buttonPins[i], INPUT);
  }

  // Declarando os pinos das fitas led para serem gerenciados pela biblioteca FastLED
  FastLED.addLeds<WS2812, 2, RGB>(leds[0], LED_COUNT);
  FastLED.addLeds<WS2812, 3, RGB>(leds[1], LED_COUNT);
  FastLED.addLeds<WS2812, 5, RGB>(leds[2], LED_COUNT);
  FastLED.addLeds<WS2812, 6, RGB>(leds[3], LED_COUNT);
  FastLED.addLeds<WS2812, 7, RGB>(leds[4], LED_COUNT);
  FastLED.clear();
  FastLED.show();

  // Inicializando uma flor inicial
  order = random(15);
}

void loop()
{
  for (int i = 0; i < BUTTON_LEN; i++)
  {
    state = digitalRead(buttonPins[i]);

    if (state && !(lastState[i]))
    {
      const CRGB color = buttonsColors[i];
      emotionLCD(i);
      animate(order, color);
      resetLCD();

      // writeSD(emotions[i]);
      order = (order + 1) % FLOWERS_LEN;
    }
    lastState[i] = state;
  }
}
