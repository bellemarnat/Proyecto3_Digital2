// ----------------------- Librerias -----------------------
#include <SD.h>
#include <SPI.h>
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

//----------------------- Definiciones ----------------------- 
#define LCD_RST PD_0 // PINES PARA LCD TFT
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1

int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7}; 

#define CS_PIN PA_3 // PIN PARA SD
#define B_PIN PF_2 // PIN PARA BUZZER PASIVO

// PIN PARA BOTONES
#define SW1_PIN PF_4 //Bandera para realizar la medición
#define SW2_PIN PF_0 //Bandera para guardar dato

// ----------------------- Variables Globales ----------------------- 
char receivedData[50]; // Almacena los datos recibidos como cadena de caracteres
bool dataReceived = false;

float temp = 0.00;
float hum = 0.00;

int medTemp;
int medHum;

int SW1med;
int SW2guard;

File myFile;

int estadoBoton = HIGH; // Estado del botón en la última lectura
int ultimoestadoBoton = HIGH; // Estado del botón en la última lectura
unsigned long ultimoTiempoRebote = 0; // Tiempo de la última transición del botón
unsigned long retrasoRebote = 50; // Tiempo de rebote (ajústalo según tus necesidades)

int estadoBoton2 = HIGH; // Estado del botón en la última lectura
int ultimoestadoBoton2 = HIGH; // Estado del botón en la última lectura
unsigned long ultimoTiempoRebote2 = 0; // Tiempo de la última transición del botón
unsigned long retrasoRebote2 = 50; // Tiempo de rebote (ajústalo según tus necesidades)

//----------------------- Prototipos de funciones ----------------------- 
void actualizarPantalla(int temp, int hum);
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
//void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
//void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

extern uint8_t fondo[];

//----------------------- Void Setup ----------------------- 
void setup() {
  // LCD TFT
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(115200);
  Serial2.begin(115200);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  SPI.setModule(0);
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(B_PIN, OUTPUT);
  
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);
  
  FillRect(0, 0, 320, 240, 0x421b);

  //SD
  Serial.println("Bienvenido");
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS_PIN)) {
    Serial.println("initialization failed!");
    return;
    }
    Serial.println("initialization done");
}

// ----------------------- Void Loop ----------------------- 
void loop() {
  SW1med = digitalRead(SW1_PIN);
  SW2guard = digitalRead(SW2_PIN);

  if (SW1med != ultimoestadoBoton) {
    ultimoTiempoRebote = millis();
  }

  if ((millis() - ultimoTiempoRebote) > retrasoRebote) {
    if (SW1med != estadoBoton) {
      estadoBoton = SW1med;

      if (estadoBoton == LOW) {
        Serial.println("Solicitando medición");
        Serial2.print("on");
        delay(100); // Espera un momento para asegurar que los datos se envíen completamente
        if (Serial2.available()) {
          medHum = Serial2.parseFloat();
          medTemp = Serial2.parseFloat();
          Serial.print("Medición recibida:");
          Serial.print("Medición de temperatura: ");
          Serial.print((String) medTemp);
          Serial.print("˚C");
          Serial.print("  ");
          Serial.print("Medición de humedad: ");
          Serial.print((String) medHum);
          Serial.print("%");



          Serial.print("\n");
          
            tone(B_PIN, 1000, 500);
            delay(200);
            noTone(B_PIN);
            String datoTemp = "Temperatura: ";
            datoTemp += String(medTemp);
            LCD_Print(datoTemp, 20, 90, 2, 0xffff, 0x421b);
            delay(1000);
            
            String datoHum = "Humedad: ";
            datoHum += String(medHum);
            LCD_Print(datoHum, 20, 150, 2, 0xffff, 0x421b);
            delay(1000);

        } else {
          Serial.println("Dato no recibido");
        }
      }
    }
  }
  ultimoestadoBoton = SW1med;
        
  if (SW2guard != ultimoestadoBoton2) {
    ultimoTiempoRebote2 = millis();
  }

  if ((millis() - ultimoTiempoRebote2) > retrasoRebote2) {
    if (SW2guard != estadoBoton2) {
      estadoBoton2 = SW2guard;

      if (SW2guard == 0) {
        tone(B_PIN, 200, 500);
        myFile = SD.open("datafile.txt", FILE_WRITE);
        if (myFile) {
          myFile.print(medTemp);
          myFile.println("," + String(medHum)); // Guarda ambos valores separados por una coma
          Serial.println("Medición guardada:");
          Serial.println("Temperatura: " + medTemp);
          Serial.println("Humedad: " + medHum);
          tone(B_PIN, 200, 500);
          
          delay(200);
          myFile.close();
        } else {
          Serial.println("Error al escribir en la tarjeta");
        }
      }
    }
  }
  ultimoestadoBoton2 = SW2guard;
}


/*
void loop() {
  SW1med = digitalRead(SW1_PIN);
  SW2guard = digitalRead(SW2_PIN);
  
  if (SW1med != ultimoestadoBoton) {
    ultimoTiempoRebote = millis();
  }

  if ((millis() - ultimoTiempoRebote) > retrasoRebote) {
    if (SW1med != estadoBoton) {
      estadoBoton = SW1med;

      if (estadoBoton == 0) {
        Serial.println("Solicitando medición");
        Serial2.print("on");
        delay(100); // Espera un momento para asegurar que los datos se envíen completamente
        if (Serial2.available()) {
          String data = Serial2.readStringUntil('\n'); // Lee una línea completa
          // Divide los datos en temperatura y humedad
          int separatorIndex = data.indexOf(',');
          if (separatorIndex != -1) {
            String tempStr = data.substring(0, separatorIndex);
            String humStr = data.substring(separatorIndex + 1);
            medTemp = tempStr.toInt();
            medHum = humStr.toInt();
            Serial.println("Medición recibida");
            Serial.println("Temperatura: " + medTemp);
            Serial.println("Humedad: " + medHum);
            tone(B_PIN, 2093, 200);
            delay(200);
            noTone(B_PIN);
            // Resto del procesamiento de datos
          } else {
            Serial.println("Formato de datos incorrecto: " + data);
          }
        } else {
          Serial.println("Dato no recibido");
        }
      }
    }
  }
  ultimoestadoBoton = SW1med;

  if (SW2guard != ultimoestadoBoton2) {
    ultimoTiempoRebote2 = millis();
  }

  if ((millis() - ultimoTiempoRebote2) > retrasoRebote2) {
    if (SW2guard != estadoBoton2) {
      estadoBoton2 = SW2guard;

      if (SW2guard == 0) {
        myFile = SD.open("datafile.txt", FILE_WRITE);
        if (myFile) {
          myFile.print(medTemp);
          myFile.println("," + String(medHum)); // Guarda ambos valores separados por una coma
          Serial.println("Medición guardada:");
          Serial.println("Temperatura: " + medTemp);
          Serial.println("Humedad: " + medHum);
          tone(B_PIN, 2093, 200);
          delay(200);
          noTone(B_PIN);
          myFile.close();
        } else {
          Serial.print("Error al escribir en la tarjeta");
        }
      }
    }
  }
  ultimoestadoBoton2 = SW2guard;
}*/


//----------------------- SD ----------------------- 
void saveDataToSD(void) {
  myFile = SD.open("dataFile.txt", FILE_WRITE); // Abrir Archivo de datos como Escritura
    myFile.print("Temperatura: ");
    myFile.print(temp);
    myFile.println(" C ");
    myFile.print("Humedad: ");
    myFile.print(hum);
    myFile.println(" % ");
    myFile.close();
} 

//----------------------- Inicio LCD ----------------------- 
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  
  // Secuencia de Inicialización
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); // Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    // Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    // Display on
  digitalWrite(LCD_CS, HIGH);
}

// ----------------------- Función para enviar comandos a la LCD - parámetro (comando) ----------------------- 
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

// ----------------------- Función para enviar datos a la LCD - parámetro (dato) ----------------------- 
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//-----------------------  Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana) ----------------------- 
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}

//-----------------------  Función para borrar la pantalla - parámetros (color) ----------------------- 
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 

// ----------------------- Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color) ----------------------- 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}

//-----------------------  Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color) ----------------------- 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}

// ----------------------- Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color) ----------------------- 
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

//----------------------- Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color) ----------------------- 
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//-----------------------  Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) ----------------------- 
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  //Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    //Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}