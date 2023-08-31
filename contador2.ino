#include <map>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Button.h"
#include "logo.h"

#define TFT_CS    D2
#define TFT_DC    D4
#define TFT_MOSI  D7
#define TFT_CLK   D5
#define TFT_RST   D3
#define TFT_MISO  D8
#define BOTON     D1

#define LARGO_PILETA    50
#define ANCHO_PANTALLA  240 
#define ALTO_PANTALLA   320

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

Button boton(BOTON);

boolean contando = false;
boolean esperandoIncrementoSerie = false;

unsigned long cronometroPiletas= 0;
unsigned long cronometroTotal = 0;

int contadorPiletas = 0;
int contadorTotal = 0;
int contadorSeries = 0;

unsigned long timestampUltimoCronometroImpreso = 0;
int unSegundo = 1000; // un segundo = mil milisegundos
int tiempoParaIncrementarSerie = 3000; // el tiempo que tiene el usuario para incrementar series
int tiempoParaResetar = 3500;
int pressReset = 4000; // 4 segundos para resetear
unsigned long timestampBotonPresionado = 0;

typedef struct
{
  int piletas;
  int tiempo;
} datos_serie;

datos_serie series[20];

// coordenadas y tamaños de las cosas impresas.
int16_t xContador, yContador, xMensajeMtsSerie, yMensajeMtsSerie, xCronometro, yCronometro;
uint16_t wContador, hContador, wMensajeMtsSerie, hMensajeMtsSerie, wCronometro, hCronometro;

void setup() {
  boton.begin();
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_WHITE);
  tft.fillScreen(0x7DFA);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  
  tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, ILI9341_NAVY);
}

void loop() {

  if (!contando)
  {
    if (boton.pressed())
    {
      contando = true;
      timestampBotonPresionado = millis();
      tft.fillScreen(ILI9341_BLACK);
      imprimeContador(contadorPiletas, ILI9341_WHITE);
    }
  }
  else // comenzó a funcionar el aparato
  {
    if (boton.pressed())
    {
      if(!esperandoIncrementoSerie)
      {
        esperandoIncrementoSerie = true;
        timestampBotonPresionado = millis();
        contadorPiletas++;
        contadorTotal++;
      }
      else // ventana temporal para aumentar la serie
      {
        series[contadorSeries].piletas = contadorPiletas;
        series[contadorSeries].tiempo = cronometroPiletas;
        contadorSeries++;

        contadorPiletas = 0;
        cronometroPiletas = 0;
        imprimeSeries();
      }
      imprimeContador(contadorPiletas, ILI9341_WHITE);
    }
    imprimeCronometros(ILI9341_WHITE);
    imprimeMetros(ILI9341_GREEN, ILI9341_GREENYELLOW);
    timerIncrementoSerie();

    // RESET
    if(boton.released()) {
      if (millis() - timestampBotonPresionado >= tiempoParaResetar) {
        contando = false;
        cronometroPiletas= 0;
        cronometroTotal = 0;
        contadorPiletas = 0;
        contadorTotal = 0;
        
        tft.fillScreen(ILI9341_WHITE);
        tft.drawXBitmap(0,0, logo_bits, logo_w, logo_h, ILI9341_NAVY);
      }
    }
  }
}

void timerIncrementoSerie()
{
  if (millis() - timestampBotonPresionado >= tiempoParaIncrementarSerie)
  {
    esperandoIncrementoSerie = false;
  }
}

void imprimeCronometros(uint16_t color)
{
  if (millis() - timestampUltimoCronometroImpreso >= unSegundo)
  {
    timestampUltimoCronometroImpreso = millis();
    cronometroPiletas++;
    cronometroTotal++;

    String relojSerie = reloj(cronometroPiletas);
    String relojTotal = reloj(cronometroTotal);

    tft.setTextColor(color, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.getTextBounds((String)relojSerie, 0, 0, &xCronometro, &yCronometro, &wCronometro, &hCronometro);
    tft.setCursor(0, hContador);
    tft.write(0x0F); // sibolo del reloj
    tft.println(" " + relojSerie);

    tft.setCursor(0, ALTO_PANTALLA - hCronometro);
    tft.write(0xE7); // sibolo del reloj
    tft.println(" " + relojTotal);
  }
}

String reloj(unsigned int tiempo)
{
    String segundos = String(tiempo % 60);
    segundos = segundos.length() == 1 ? "0" + segundos : segundos;

    String minutos = String((int)floor(tiempo / 60));
    minutos = minutos.length() == 1 ? "0" + minutos : minutos;

    String reloj = minutos + ":" + segundos;

    return reloj;
}

void imprimeContador(int numero, uint16_t colorTexto)
{
  tft.setTextWrap(false);
  tft.setTextColor(colorTexto, ILI9341_BLACK);
  tft.setTextSize(15);
  tft.getTextBounds((String)numero, 0, 0, &xContador, &yContador, &wContador, &hContador);
  tft.setCursor(ANCHO_PANTALLA - wContador, 0);
  tft.println(numero);
}

void imprimeMetros(uint16_t colorSerie, uint16_t colorTotal)
{
  String m = "m";
  String mtsSerie = String(contadorPiletas * LARGO_PILETA);
  String mtsTotal = String(contadorTotal * LARGO_PILETA);
  String mensajeSerie = mtsSerie + m;
  String mensajeTotal = mtsTotal + m;

  tft.setTextSize(3);
  
  tft.getTextBounds((String)cuatroChars(mensajeSerie), 0, 0, &xMensajeMtsSerie, &yMensajeMtsSerie, &wMensajeMtsSerie, &hMensajeMtsSerie);
  tft.setCursor(ANCHO_PANTALLA - wMensajeMtsSerie, hContador);
  tft.setTextColor(colorSerie, ILI9341_BLACK);
  tft.println(cuatroChars(mensajeSerie));

  tft.setTextSize(4);
  tft.getTextBounds((String)mensajeTotal, 0, 0, &xMensajeMtsSerie, &yMensajeMtsSerie, &wMensajeMtsSerie, &hMensajeMtsSerie);
  tft.setCursor(ANCHO_PANTALLA - wMensajeMtsSerie, ALTO_PANTALLA - hMensajeMtsSerie);
  tft.setTextColor(colorTotal, ILI9341_BLACK);
  tft.println(mensajeTotal);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
}

String cuatroChars(const String& input)
{
  String result;
    
  if (input.length() >= 4) {
    return input;
  } else {
      for (int i = 0; i < (4 - input.length()); ++i) {
          result += " ";
      }
      result += input;
  }
  return result;
}

void imprimeSeries()
{
  tft.setCursor(0, 150);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  std::map<int, int> counts;  // Using a map to store counts

  // Count occurrences of each unique 'piletas' value
  for (int i = 0; i < 10; ++i) {
    if (series[i].piletas == 0) continue;
    int piletas = series[i].piletas;
    counts[piletas]++;
  }

  // Print the counts in the desired format
  for (const auto &entry : counts) {
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(entry.second);
    tft.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
    tft.print("x");
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print(entry.first);
    tft.print(" ");
    tft.setTextColor(0x90D5, ILI9341_BLACK);
    tft.print(entry.second * entry.first * LARGO_PILETA);
    tft.println("m  ");
    tft.setCursor(0, tft.getCursorY()+2);
  }


}
