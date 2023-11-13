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

float aceZ = 0.00;

int medRot;
int medAcel;

float medRotX, medRotY, medRotZ; // Variables para almacenar mediciones de giroscopio en cada eje
float medAcelX, medAcelY, medAcelZ; // Variables para almacenar mediciones de aceleración en cada eje


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
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

extern uint8_t giro1[];
extern uint8_t acel[];


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
        Serial2.print('b');
        delay(100);
        
        Serial.println("Solicitando medición");
        delay(1000);
        Serial2.print("on");
        String response = Serial2.readStringUntil('\n'); // Leer la respuesta hasta el salto de línea
        Serial.println("Respuesta recibida: " + response);

        if (response.length() > 0) {
            int startIndex, endIndex;

            // Extraer aceleración X
            startIndex = response.indexOf("X: ") + 3;
            endIndex = response.indexOf(", Y: ");
            medAcelX = response.substring(startIndex, endIndex).toFloat();

            // Extraer aceleración Y
            startIndex = endIndex + 4;
            endIndex = response.indexOf(", Z: ", startIndex);
            medAcelY = response.substring(startIndex, endIndex).toFloat();

            // Extraer aceleración Z
            startIndex = endIndex + 4;
            endIndex = response.indexOf(" m/s^2", startIndex);
            medAcelZ = response.substring(startIndex, endIndex).toFloat();

            // Extraer rotación X
            startIndex = response.indexOf("X: ", endIndex) + 3;
            endIndex = response.indexOf(", Y: ", startIndex);
            medRotX = response.substring(startIndex, endIndex).toFloat();

            // Extraer rotación Y
            startIndex = endIndex + 4;
            endIndex = response.indexOf(", Z: ", startIndex);
            medRotY = response.substring(startIndex, endIndex).toFloat();

            // Extraer rotación Z
            startIndex = endIndex + 4;
            endIndex = response.indexOf(" rad/s", startIndex);
            medRotZ = response.substring(startIndex, endIndex).toFloat();

            auto floatToString = [](float value) -> String {
              int entero = static_cast<int>(value);
              int decimal = static_cast<int>((value - entero) * 100);
              return String(entero) + "." + (decimal < 10 ? "0" : "") + String(abs(decimal));
              };
              
            // Convertir los valores a String
            String strRotX = floatToString(medRotX);
            String strRotY = floatToString(medRotY);
            String strRotZ = floatToString(medRotZ);
            String strAcelX = floatToString(medAcelX);
            String strAcelY = floatToString(medAcelY);
            String strAcelZ = floatToString(medAcelZ);
            
            tone(B_PIN, 500, 500);
            delay(200);
            noTone(B_PIN);
            // Imprimir los valores en la pantalla LCD
            LCD_Clear(0x0000); // Limpia la pantalla antes de imprimir nuevos datos
            LCD_Print("Rotacion(rad/s):", 10, 20, 2, 0x07FF, 0x0000);
            LCD_Print("X: ", 10, 50, 2, 0x07E0, 0x0000);
            LCD_Print(strRotX, 60, 50, 2, 0xffff, 0x0000);
            LCD_Print("Y: ", 10, 70, 2, 0x07E0, 0x0000);
            LCD_Print(strRotY, 60, 70, 2, 0xffff, 0x0000);
            LCD_Print("Z: ", 10, 90, 2, 0x07E0, 0x0000);
            LCD_Print(strRotZ, 60, 90, 2, 0xffff, 0x0000);    
            LCD_Bitmap(200, 50, 60, 60, giro1);

                    
            LCD_Print("Aceleracion(m/s^2): ", 10, 120, 2, 0x07FF, 0x0000);
            LCD_Print("X: ", 10, 150, 2, 0x07E0, 0x0000);
            LCD_Print(strAcelX, 60, 150, 2, 0xffff, 0x0000);
            LCD_Print("Y: ", 10, 170, 2, 0x07E0, 0x0000);
            LCD_Print(strAcelY, 60, 170, 2, 0xffff, 0x0000);
            LCD_Print("Z: ", 10, 190, 2, 0x07E0, 0x0000);
            LCD_Print(strAcelZ, 60, 190, 2, 0xffff, 0x0000);
            LCD_Bitmap(200, 150, 60, 60, acel);

            delay(500);
            Serial2.print('g'); // Datos recibidos e impresos correctamente - NeoPixel Verde
            } else {
              // Si hubo un error
              Serial.print("Dato no recibido");
              Serial2.print('r'); // Error en la recepción o procesamiento de datos - NeoPixel Rojo
         }
      }
    }
  }
  ultimoestadoBoton = SW1med;

//Funcion para el boton de guardado
  if (SW2guard != ultimoestadoBoton2) {
    ultimoTiempoRebote2 = millis();
  }

  if ((millis() - ultimoTiempoRebote2) > retrasoRebote2) {
    if (SW2guard != estadoBoton2) {
      estadoBoton2 = SW2guard;

      if (estadoBoton2 == LOW) {
      
        Serial2.println('a');
        tone(B_PIN, 1000, 1000);
        
        myFile = SD.open("datafile.txt", FILE_WRITE);
        if (myFile) {
          myFile.print("Rotacion: ");
          myFile.print("X: ");
          myFile.print(medRotX);
          myFile.print("Y: ");
          myFile.print(medRotY);
          myFile.print("Z: ");
          myFile.print(medRotZ);
          myFile.println("Aceleracion: "); // Guarda ambos valores separados por una coma
          myFile.print("X: ");
          myFile.print(medAcelX);
          myFile.print("Y: ");
          myFile.print(medAcelY);
          myFile.print("Z: ");
          myFile.print(medAcelZ);
          myFile.close();

          Serial.println("Medición guardada");
          Serial2.print('g');
          delay(200);
          
        } else {
          Serial2.print('r');
          tone(B_PIN, 200, 500);
         
          Serial.println("Error al escribir en la tarjeta");
        }
      }
    }
   }
ultimoestadoBoton2 = SW2guard;
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


//----------------------- Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits) -----------------------
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

// ----------------------- Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta -----------------------
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
