/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include <stdlib.h>
#include <stdbool.h>

#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
//#include "graficos.c"
//#include "graficos.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define MAX_BOMBAS 3

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
extern uint8_t inicio1[]; // La misma que buffer2
extern uint8_t bloque_sin_romper[]; // buffer3
extern uint8_t bloque_a_romper[];
extern uint8_t bomberman[];
extern uint8_t manup[];
extern uint8_t mandown[];
extern uint8_t bloques[];
extern uint8_t manside2[];
extern uint8_t manup2[];
extern uint8_t mandown2[];
extern uint8_t dead[];
extern uint8_t dead2[];
extern uint8_t bomb[];
extern uint8_t fuego[];
extern uint8_t manside_negro [];
extern uint8_t mandown_negro [];
extern uint8_t manside2_negro [];
extern uint8_t mandown2_negro [];
unsigned char buffer2[184 * 100 * 4]; // Pantalla de incio
unsigned char buffer3[13 * 13 * 4]; // Bloque que no se rompe
unsigned char buffer4[20 * 20 * 4]; // Bloque que no se rompe contorno
unsigned char buffer5[18 * 26 * 4]; // Sprite de Mandown
unsigned char buffer6[18 * 26 * 4]; // Sprite de Mandown 2

char Buffer[100];
uint8_t cambiomenu = 0;
uint8_t rxData;
uint8_t buffer[1];
uint8_t rxData;
uint8_t ciclos = 0;
uint8_t comando2 = 0;
uint8_t comando = 0; // Variable para guardar el comando recibido
uint8_t musica = 0;

//char buffer2[1] ;
int limite_izq = 24;  // Límite izquierdo (jugador 1)
int limite_der = 274;  // Límite derecho (jugador 2)
int limite_arriba = 14;  // Límite superior (jugador 1)
int limite_abajo = 194;  // Límite inferior (jugador 2)
uint8_t muerte = 0;     //Que jugador murió.
// Reducir els tamaño de la explosión
int fuego_escala = 13;  // Ajusta el tamaño de la explosión (más pequeño)
int pos_x = 24;
int pos_y = 14;
int pos_x2 = 274;
int pos_y2 = 194;

int x2 = 5;
int y2 = 10;
int lineHeight = 15;  // Separación entre líneas


int monitoreo1 = 0;
int monitoreo2 = 0;
int monitoreo3 = 0;
int monitoreo4 = 0;
char msg[50];
char rxDato;

uint32_t tiempo_bomba1[MAX_BOMBAS];
uint32_t tiempo_bomba2[MAX_BOMBAS];

int puedeSalir1[MAX_BOMBAS];
int puedeSalir2[MAX_BOMBAS];

int bombas_x1[MAX_BOMBAS] = {-1, -1, -1};
int bombas_y1[MAX_BOMBAS] = {-1, -1, -1};

int bombas_x2[MAX_BOMBAS] = {-1, -1, -1};
int bombas_y2[MAX_BOMBAS] = {-1, -1, -1};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
int hayBloqueEn(int px, int py);
void propagarFuego(int bombas_x, int bombas_y);
int colision_fuego(int fuego_x, int fuego_y, int jugador_x, int jugador_y);
int colision(int x1, int y1, int x2, int y2);
void pantallamuerte(int muerte);
void transmit_uart(char *string);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void UART_Send_String(char *string){
	uint8_t len = strlen(string);
	HAL_UART_Transmit(&huart2,(uint8_t*)string, len, 200);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_UART5_Init();
  MX_FATFS_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

	LCD_Init();
	HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_RESET);
	LCD_Clear(0x00);
	FillRect(0, 0, 319, 239, 0x0000);


	  //Waiting for the Micro SD module to initialize
	  HAL_Delay(500);
	  fres = f_mount(&fs, "/", 0);
	      if (fres != FR_OK) {
	          char err[50];
	          sprintf(err, "Mount error: %d\n", fres);
	          UART_Send_String(err);
	          while (1);
	      }
	      UART_Send_String("Micro SD mounted.\n");

	      // Abrimos el archivo en modo lectura
	      fres = f_open(&fil, "prueba.txt", FA_READ);
	      if (fres == FR_OK) {
	    	  UART_Send_String("\n\r");
	    	  UART_Send_String("File opened for reading.\n");
	      } else if (fres != FR_OK) {
	    	  UART_Send_String("File was not opened for reading!\n");
	      }

	      // Leemos el archivo
	      char line[3500];
	      uint32_t index = 0;

	      while (f_gets(line, sizeof(line), &fil) && index < 18400 * 4) {
	              char *token = strtok(line, ",\n\r");
	              while (token != NULL && index < 18400 * 4) {
	                  while (*token == ' ') token++;
	                  if (strncmp(token, "0x", 2) == 0) token += 2;
	                  uint8_t value = (uint8_t)strtol(token, NULL, 16);
	                  buffer2[index++] = value;

	                  token = strtok(NULL, ",\n\r");
	              }
	          }

	// Imprimir texto en pantalla
	LCD_Bitmap(68, 30, 184, 100, buffer2);
	fres = f_close(&fil);
		      if (fres == FR_OK) {
		    	  UART_Send_String("The file is closed.\n");
		      } else if (fres != FR_OK) {
		    	  UART_Send_String("The file was not closed.\n");

		      }

	LCD_Print("J1 pulse cuadrado", 23, 170, 2, 0xFFFF, 0x0000);
	HAL_UART_Receive_IT(&huart3, buffer, 1);
	//HAL_UART_Receive_IT(&huart5, rxData, 1);
	while(cambiomenu != 1){
		HAL_Delay(100);
	}
    HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_RESET);
	//Aquí debería de estar la condición, si se oprime start en el J1, entonces que cambie el fondo.
	FillRect(0, 0, 319, 239, 0x04e0);

	// Abrimos el archivo en modo lectura
		      fres = f_open(&fil, "bloque_sin_romper.txt", FA_READ);
		      if (fres == FR_OK) {
		    	  UART_Send_String("\n\r");
		    	  UART_Send_String("File opened for reading.\n");
		      } else if (fres != FR_OK) {
		    	  UART_Send_String("File was not opened for reading!\n");
		      }

		      index = 0;
		      // Leemos el archivo de los bloques
		      while (f_gets(line, sizeof(line), &fil) && index < 169 * 4) {
		              char *token = strtok(line, ",\n\r");
		              while (token != NULL && index < 169 * 4) {
		                  while (*token == ' ') token++;
		                  if (strncmp(token, "0x", 2) == 0) token += 2;
		                  uint8_t value = (uint8_t)strtol(token, NULL, 16);
		                  buffer3[index++] = value;

		                  token = strtok(NULL, ",\n\r");
		              }
		          }
		      // Cerramos el archivo
		      	fres = f_close(&fil);
		      			      if (fres == FR_OK) {
		      			    	  UART_Send_String("The file is closed.\n");
		      			      } else if (fres != FR_OK) {
		      			    	  UART_Send_String("The file was not closed.\n");

		      			      }
		      // Abrimos el archivo en modo lectura
		      		      fres = f_open(&fil, "bloques.txt", FA_READ);
		      		      if (fres == FR_OK) {
		      		    	  UART_Send_String("\n\r");
		      		    	  UART_Send_String("File opened for reading.\n");
		      		      } else if (fres != FR_OK) {
		      		    	  UART_Send_String("File was not opened for reading!\n");
		      		      }

		      index = 0;
		      // Leemos el archivo
		      while (f_gets(line, sizeof(line), &fil) && index < 400 * 4) {
		    	  	  char *token = strtok(line, ",\n\r");
		    	  	  while (token != NULL && index < 400 * 4) {
		    	  		  while (*token == ' ') token++;
		    	  		  if (strncmp(token, "0x", 2) == 0) token += 2;
		    	  		  uint8_t value = (uint8_t)strtol(token, NULL, 16);
		    	  		  buffer4[index++] = value;

		    	  		  token = strtok(NULL, ",\n\r");
		    	  	  }
		      	  }

		     // Cerramos el archivo
		     fres = f_close(&fil);
		     if (fres == FR_OK) {
		    	 UART_Send_String("The file is closed.\n");
		     } else if (fres != FR_OK) {
		    	 UART_Send_String("The file was not closed.\n");

		     }

// Acá se hacen los bloques que van en medio del mapa que no se rompen
	for(int y = 0; y<239; y++){
		LCD_Bitmap(0, y, 13, 13,buffer3);
		LCD_Bitmap(10, y, 13, 13,buffer3);
		LCD_Bitmap(295, y, 13, 13,buffer3);
		LCD_Bitmap(307, y, 13, 13,buffer3);
		y +=12;
	}

	for(int x = 0; x<307; x++){
		LCD_Bitmap(x, 0, 13, 13,buffer3);
		LCD_Bitmap(x, 221, 13, 13,buffer3); //227
		LCD_Bitmap(x, 227, 13, 13,buffer3); //227
		x +=12;
	}


	for(int y = 42; y<96; y++){ //Aqui solo se hacen todos los bloques menos la fila de en medio
		LCD_Bitmap(42,y, 20, 20,buffer4);
		LCD_Bitmap(82,y, 20, 20,buffer4);
		LCD_Bitmap(123,y, 20, 20,buffer4);
		LCD_Bitmap(163,y, 20, 20,buffer4);
		LCD_Bitmap(204,y, 20, 20,buffer4);
		LCD_Bitmap(250,y, 20, 20,buffer4);
		y +=50;
	}

	for(int y = 147; y<302; y++){ //Aqui solo se hacen todos los bloques menos la fila de en medio
		LCD_Bitmap(42,y, 20, 20,buffer4);
		LCD_Bitmap(82,y, 20, 20,buffer4);
		LCD_Bitmap(123,y, 20, 20,buffer4);
		LCD_Bitmap(163,y, 20, 20,buffer4);
		LCD_Bitmap(204,y, 20, 20,buffer4);
		LCD_Bitmap(250,y, 20, 20,buffer4);
		y +=52;
	}
	LCD_Sprite(pos_x, pos_y, 18, 26, mandown, 3, 1, 0, 0);
	LCD_Sprite(pos_x2, pos_y2, 18, 26, mandown2, 3, 2, 0, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		if(muerte == 0){
		if (comando =='d') { // derecha
		    FillRect(pos_x, pos_y, 18, 26, 0x04e0);
		    int nuevo_x = pos_x + 10;
		    if (nuevo_x <= 274 && !hayBloqueEn(nuevo_x, pos_y)) {
		      pos_x = nuevo_x;
		    }

		    int anim = (pos_x / 10) % 3;
		    LCD_Sprite(pos_x, pos_y, 18, 26, bomberman, 3, anim, 1, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando = 0;
		  }

		  else if (comando == 'a') { // izquierda
		    FillRect(pos_x, pos_y, 18, 26, 0x04e0);
		    int nuevo_x = pos_x - 10;
		    if (nuevo_x >= 24 && !hayBloqueEn(nuevo_x, pos_y)) {
		      pos_x = nuevo_x;
		    }

		    int anim = (pos_x / 10) % 3;
		    LCD_Sprite(pos_x, pos_y, 18, 26, bomberman, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando = 0;
		  }

		  else if (comando == 's') { // abajo
		    FillRect(pos_x, pos_y, 18, 26, 0x04e0);
		    int nuevo_y = pos_y + 10;
		    if (nuevo_y <= 200 && !hayBloqueEn(pos_x, nuevo_y)) {
		      pos_y = nuevo_y;
		    }

		    int anim = (pos_y / 10) % 3;
		    LCD_Sprite(pos_x, pos_y, 18, 26, mandown, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando = 0;
		  }

		  else if (comando == 'w') { // arriba
		    FillRect(pos_x, pos_y, 18, 26, 0x04e0);
		    int nuevo_y = pos_y - 10;
		    if (nuevo_y >= 14 && !hayBloqueEn(pos_x, nuevo_y)) {
		      pos_y = nuevo_y;
		    }

		    int anim = (pos_y / 10) % 3;
		    LCD_Sprite(pos_x, pos_y, 18, 26, manup, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando = 0;
		  }
		  else if (comando == 'x'){ //coloco la bomba
			   for (int i = 0; i < MAX_BOMBAS; i++) {
			        if (bombas_x1[i] == -1 && bombas_y1[i] == -1) {
			            bombas_x1[i] = pos_x;
			            bombas_y1[i] = pos_y;
			            tiempo_bomba1[i] = HAL_GetTick();
			            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
			            break;
			        }
			    }
			    HAL_Delay(15);
			    comando = 0;
		        HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_RESET);
		        HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_SET);

		  }
		//Bomber2
		  else if (comando2 == 5){ //coloco la bomba
			    for (int i = 0; i < MAX_BOMBAS; i++) {
			        if (bombas_x2[i] == -1 && bombas_y2[i] == -1) {
			            bombas_x2[i] = pos_x2;
			            bombas_y2[i] = pos_y2;
			            tiempo_bomba2[i] = HAL_GetTick();
			            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
			            break;
			        }
			    }
			    HAL_Delay(15);
			    comando2 = 0;
	            HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_RESET);
	            HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_SET);
		  }
		  else if (comando2 == 4) { // derecha
		    FillRect(pos_x2, pos_y2, 18, 26, 0x04e0);
		    int nuevo_x = pos_x2 + 10;
		    if (nuevo_x <= 274 && !hayBloqueEn(nuevo_x, pos_y2)) {
		      pos_x2 = nuevo_x;
		    }

		    int anim = (pos_x2 / 10) % 3;
		    LCD_Sprite(pos_x2, pos_y2, 18, 26, manside2, 3, anim, 1, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando2 = 0;
		  }

		  else if (comando2 == 3) { // izquierda
		    FillRect(pos_x2, pos_y2, 18, 26, 0x04e0);
		    int nuevo_x = pos_x2 - 10;
		    if (nuevo_x >= 24 && !hayBloqueEn(nuevo_x, pos_y2)) {
		      pos_x2 = nuevo_x;
		    }

		    int anim = (pos_x2 / 10) % 3;
		    LCD_Sprite(pos_x2, pos_y2, 18, 26, manside2, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando2 = 0;
		  }

		  else if (comando2 == 2) { // abajo
		    FillRect(pos_x2, pos_y2, 18, 26, 0x04e0);
		    int nuevo_y = pos_y2 + 10;
		    if (nuevo_y <= 200 && !hayBloqueEn(pos_x2, nuevo_y)) {
		      pos_y2 = nuevo_y;
		    }

		    int anim = (pos_y2 / 10) % 3;
		    LCD_Sprite(pos_x2, pos_y2, 18, 26, mandown2, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando2 = 0;
		  }

		  else if (comando2 == 1 ) { // arriba
		    FillRect(pos_x2, pos_y2, 18, 26, 0x04e0);
		    int nuevo_y = pos_y2 - 10;
		    if (nuevo_y >= 14 && !hayBloqueEn(pos_x2, nuevo_y)) {
		      pos_y2 = nuevo_y;
		    }

		    int anim = (pos_y2 / 10) % 3;
		    LCD_Sprite(pos_x2, pos_y2, 18, 26, manup2, 3, anim, 0, 0);
		    for (int i = 0; i < MAX_BOMBAS; i++) {
		        if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		            LCD_Bitmap(bombas_x1[i], bombas_y1[i], 18, 18, bomb);
		        }
		        if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		            LCD_Bitmap(bombas_x2[i], bombas_y2[i], 18, 18, bomb);
		        }
		    }
		    HAL_Delay(15);
		    comando2 = 0;
		  }
		for (int i = 0; i < MAX_BOMBAS; i++) {
		    // Explosión para el jugador 1
		    if (bombas_x1[i] != -1 && bombas_y1[i] != -1) {
		        if (HAL_GetTick() - tiempo_bomba1[i] >= 1000) {
		            FillRect(bombas_x1[i], bombas_y1[i], 18, 18, 0x04e0); // borrar bomba
		            HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_RESET);
		            HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_RESET);

		            for (int offset = -fuego_escala; offset <= fuego_escala; offset += fuego_escala) {
		                // Comprobar fuego a la izquierda y derecha
		                if (bombas_x1[i] + offset >= limite_izq && bombas_x1[i] + offset <= limite_der) {
		                    if (!hayBloqueEn(bombas_x1[i] + offset, bombas_y1[i])) {
		                        LCD_Bitmap(bombas_x1[i] + offset, bombas_y1[i], 13, 18, fuego);
		                        HAL_Delay(20);
		                        FillRect(bombas_x1[i] + offset, bombas_y1[i], 13, 18, 0x04e0);

		                        // Comprobar colisión
		                        if (colision_fuego(bombas_x1[i] + offset, bombas_y1[i], pos_x, pos_y)) {
		                            int anim = (pos_y / 10) % 3;
		                            LCD_Sprite(pos_x, pos_y, 18, 25, dead, 2, anim, 0, 0); //Tentativamente 3
		                            //LCD_Bitmap(pos_x, pos_y, 18, 25, dead); // Jugador 1 muere
		                            muerte = 1;
		                            //pantallamuerte(muerte);
		                        }
		                        if (colision_fuego(bombas_x1[i] + offset, bombas_y1[i], pos_x2, pos_y2)) {
		                            //LCD_Bitmap(pos_x2, pos_y2, 18, 25, dead2); // Jugador 2 muere
		                        	int anim = (pos_y2 / 10) % 3;
		                        	LCD_Sprite(pos_x2, pos_y2, 18, 25, dead2, 2, anim, 0, 0); //Tentativamente 3
		                            muerte = 2;
		                            //pantallamuerte(muerte);
		                        }
		                    }
		                }

		                // Comprobar fuego arriba y abajo
		                if (bombas_y1[i] + offset >= limite_arriba && bombas_y1[i] + offset <= limite_abajo) {
		                    if (!hayBloqueEn(bombas_x1[i], bombas_y1[i] + offset)) {
		                        LCD_Bitmap(bombas_x1[i], bombas_y1[i] + offset, 13, 18, fuego);
		                        HAL_Delay(20);
		                        FillRect(bombas_x1[i], bombas_y1[i] + offset, 13, 18, 0x04e0);

		                        // Comprobar colisión
		                        if (colision_fuego(bombas_x1[i], bombas_y1[i] + offset, pos_x, pos_y)) {
		                            //LCD_Bitmap(pos_x, pos_y, 18, 25, dead); // Jugador 1 muere
		                            int anim = (pos_y / 10) % 3;
		                            LCD_Sprite(pos_x, pos_y, 18, 25, dead, 2, anim, 0, 0); //Tentativamente 3
		                            muerte = 1;
		                            //pantallamuerte(muerte);
		                        }
		                        if (colision_fuego(bombas_x1[i], bombas_y1[i] + offset, pos_x2, pos_y2)) {
		                            LCD_Bitmap(pos_x2, pos_y2, 18, 25, dead2); // Jugador 2 muere
		                        	int anim = (pos_y2 / 10) % 3;
		                        	LCD_Sprite(pos_x2, pos_y2, 18, 25, dead2, 2, anim, 0, 0); //Tentativamente 3
		                            muerte = 2;
		                            //pantallamuerte(muerte);
		                        }
		                    }
		                }
		            }


		            // Liberar espacio de la bomba
		            bombas_x1[i] = -1;
		            bombas_y1[i] = -1;
		        }
		    }

		    // Explosión para el jugador 2
		    if (bombas_x2[i] != -1 && bombas_y2[i] != -1) {
		        if (HAL_GetTick() - tiempo_bomba2[i] >= 1000) {
		            FillRect(bombas_x2[i], bombas_y2[i], 18, 18, 0x04e0); // borrar bomba
		            HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_RESET);
		            HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_RESET);
		            for (int offset = -fuego_escala; offset <= fuego_escala; offset += fuego_escala) {
		                // Comprobar fuego a la izquierda y derecha
		                if (bombas_x2[i] + offset >= limite_izq && bombas_x2[i] + offset <= limite_der) {
		                    if (!hayBloqueEn(bombas_x2[i] + offset, bombas_y2[i])) {
		                        LCD_Bitmap(bombas_x2[i] + offset, bombas_y2[i], 13, 18, fuego);
		                        HAL_Delay(20);
		                        FillRect(bombas_x2[i] + offset, bombas_y2[i], 13, 18, 0x04e0);

		                        // Comprobar colisión
		                        if (colision_fuego(bombas_x2[i] + offset, bombas_y2[i], pos_x, pos_y)) {
		                            //LCD_Bitmap(pos_x, pos_y, 18, 25, dead); // Jugador 1 muere
		                            int anim = (pos_y / 10) % 3;
		                            LCD_Sprite(pos_x, pos_y, 18, 25, dead, 2, anim, 0, 0); //Tentativamente 3
		                            muerte = 1;
		                            //pantallamuerte(muerte);
		                        }
		                        if (colision_fuego(bombas_x2[i] + offset, bombas_y2[i], pos_x2, pos_y2)) {
		                            //LCD_Bitmap(pos_x2, pos_y2, 18, 25, dead2); // Jugador 2 muere
		                        	int anim = (pos_y2/ 10) % 3;
		                        	LCD_Sprite(pos_x2, pos_y2, 18, 25, dead2, 2, anim, 0, 0); //Tentativamente 3
		                            muerte = 2;
		                            //pantallamuerte(muerte);
		                        }
		                    }
		                }

		                // Comprobar fuego arriba y abajo
		                if (bombas_y2[i] + offset >= limite_arriba && bombas_y2[i] + offset <= limite_abajo) {
		                    if (!hayBloqueEn(bombas_x2[i], bombas_y2[i] + offset)) {
		                        LCD_Bitmap(bombas_x2[i], bombas_y2[i] + offset, 13, 18, fuego);
		                        HAL_Delay(20);
		                        FillRect(bombas_x2[i], bombas_y2[i] + offset, 13, 18, 0x04e0);

		                        // Comprobar colisión
		                        if (colision_fuego(bombas_x2[i], bombas_y2[i] + offset, pos_x, pos_y)) {
		                            int anim = (pos_y / 10) % 3;
		                            LCD_Sprite(pos_x, pos_y, 18, 25, dead, 2, anim, 0, 0); //Tentativamente 3
		                            //LCD_Bitmap(pos_x, pos_y, 18, 25, dead); // Jugador 1 muere
		                            muerte = 1;
		                            //pantallamuerte(muerte);
		                        }
		                        if (colision_fuego(bombas_x2[i], bombas_y2[i] + offset, pos_x2, pos_y2)) {
		                        	int anim = (pos_y2 / 10) % 3;
		                        	LCD_Sprite(pos_x2, pos_y2, 18, 25, dead2, 2, anim, 0, 0); //Tentativamente 3
		                        	//LCD_Bitmap(pos_x2, pos_y2, 18, 25, dead2); // Jugador 2 muere
		                            muerte = 2;
		                            //pantallamuerte(muerte);
		                        }
		                    }
		                }
		            }

		            // Liberar espacio de la bomba
		            bombas_x2[i] = -1;
		            bombas_y2[i] = -1;
		        }
		    }
		}
	//pantallamuerte(muerte);
		}
		else if (muerte != 0){
			if (ciclos == 0){
				pantallamuerte(muerte);
			}
		}

		//}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|PC6_Pin|LCD_D1_Pin|PC8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
                           LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6_Pin PC8_Pin */
  GPIO_InitStruct.Pin = PC6_Pin|PC8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_SS_Pin */
  GPIO_InitStruct.Pin = SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	comando2 = atoi((char*)buffer);  // Casting a char*
	HAL_UART_Receive_IT(&huart3, buffer, 1);
    comando = rxData; // Guardamos el comando
    HAL_UART_Receive_IT(&huart5, &rxData, 1); // Reanudamos la recepción
    if(comando2 == 6){
    	cambiomenu = 1;
    }

    if (comando2 == 6 && muerte > 0){
    	NVIC_SystemReset();
    };
	//HAL_UART_Receive_IT(&huart5, (uint8_t*)buffer2, 1);
	//comando = atoi((char*)buffer);  // Casting a char*
	//comando = buffer;
}

int colision(int x1, int y1, int x2, int y2) {
    return (x1 < x2 + 18) && (x1 + 18 > x2) && (y1 < y2 + 18) && (y1 + 26 > y2);
}
int colision_fuego(int fuego_x, int fuego_y, int jugador_x, int jugador_y) {
    // Revisa si la explosión (fuego) toca el jugador (solo revisamos la parte visible de los jugadores)
    if (fuego_x < jugador_x + 18 && fuego_x + 13 > jugador_x &&  // Colisión en X
        fuego_y < jugador_y + 26 && fuego_y + 18 > jugador_y) { // Colisión en Y
        return 1; // Hay colisión
    }
    return 0; // No hay colisión
}

void propagarFuego(int bombas_x, int bombas_y) {
    // Propagar a la izquierda
    for (int i = 0; i <= 200; i += 13) {
        if (bombas_x - i >= 0 && !hayBloqueEn(bombas_x - i, bombas_y)) {
            LCD_Bitmap(bombas_x - i, bombas_y, 13, 13, fuego);
        } else {
            break;
        }
    }
    // Propagar a la derecha
    for (int i = 0; i <= 200; i += 13) {
        if (bombas_x + i <= 307 && !hayBloqueEn(bombas_x + i, bombas_y)) {
            LCD_Bitmap(bombas_x + i, bombas_y, 13, 13, fuego);
        } else {
            break;
        }
    }
    // Propagar hacia arriba
    for (int i = 0; i <= 200; i += 13) {
        if (bombas_y - i >= 0 && !hayBloqueEn(bombas_x, bombas_y - i)) {
            LCD_Bitmap(bombas_x, bombas_y - i, 13, 13, fuego);
        } else {
            break;
        }
    }
    // Propagar hacia abajo
    for (int i = 0; i <= 200; i += 13) {
        if (bombas_y + i <= 239 && !hayBloqueEn(bombas_x, bombas_y + i)) {
            LCD_Bitmap(bombas_x, bombas_y + i, 13, 13, fuego);
        } else {
            break;
        }
    }
}

int hayBloqueEn(int px, int py) {
  int bloques_x[] = {42, 82, 123, 163, 204, 250};
  int bloques_y[] = {42, 92, 146, 200};

  for (int i = 0; i < sizeof(bloques_x)/sizeof(bloques_x[0]); i++) {
    for (int j = 0; j < sizeof(bloques_y)/sizeof(bloques_y[0]); j++) {
      int bx = bloques_x[i];
      int by = bloques_y[j];
      if (
        px + 18 > bx && px < bx + 20 &&  // colisión en X
        py + 26 > by && py < by + 20     // colisión en Y
      ) {
        return 1; // hay colisión
      }
    }
  }

  return 0; // libre
}

void pantallamuerte(int muerte){
	HAL_GPIO_WritePin(PC6_GPIO_Port, PC6_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PC8_GPIO_Port, PC8_Pin, GPIO_PIN_SET);

	// Abrimos el archivo en modo lectura
			fres = f_open(&fil, "mandown.txt", FA_READ);
			if (fres == FR_OK) {
				UART_Send_String("\n\r");
				UART_Send_String("File opened for reading.\n");
			} else if (fres != FR_OK) {
				UART_Send_String("File was not opened for reading!\n");
			}

			 char line[3500];
			 uint32_t index = 0;
			// Leemos el archivo
			while (f_gets(line, sizeof(line), &fil) && index < 468 * 4) { // 468 sale de multiplicar 18x26 (dimensiones del frame)
				char *token = strtok(line, ",\n\r");
				while (token != NULL && index < 468 * 4) {
					while (*token == ' ') token++;
					if (strncmp(token, "0x", 2) == 0) token += 2;
					uint8_t value = (uint8_t)strtol(token, NULL, 16);
					buffer5[index++] = value;

					token = strtok(NULL, ",\n\r");
					}
	     	  }

			// Cerramos el archivo
			fres = f_close(&fil);
			if (fres == FR_OK) {
				UART_Send_String("The file is closed.\n");
			} else if (fres != FR_OK) {
				UART_Send_String("The file was not closed.\n");

			}

			// Abrimos el archivo en modo lectura del otro jugador
					fres = f_open(&fil, "mandown2.txt", FA_READ);
					if (fres == FR_OK) {
						UART_Send_String("\n\r");
						UART_Send_String("File opened for reading.\n");
					} else if (fres != FR_OK) {
						UART_Send_String("File was not opened for reading!\n");
					}

			index = 0;
		    // Leemos el archivo
				while (f_gets(line, sizeof(line), &fil) && index < 468 * 4) { // 468 sale de multiplicar 18x26 (dimensiones del frame)
					char *token = strtok(line, ",\n\r");
					while (token != NULL && index < 468 * 4) {
						while (*token == ' ') token++;
						if (strncmp(token, "0x", 2) == 0) token += 2;
						uint8_t value = (uint8_t)strtol(token, NULL, 16);
						buffer6[index++] = value;

						token = strtok(NULL, ",\n\r");
					}
				}

			// Cerramos el archivo
			fres = f_close(&fil);
        	if (fres == FR_OK) {
			UART_Send_String("The file is closed.\n");
        	} else if (fres != FR_OK) {
            UART_Send_String("The file was not closed.\n");

        	}

	if(muerte == 1){

		HAL_Delay(100);
		int anim = (pos_y / 10) % 3;
		FillRect(0, 0, 319, 239, 0x0000);
		LCD_Print("J1 Gana", 23, 170, 2, 0xFFFF, 0x0000);
		pos_y = 100;
		pos_x = 0;
		while (pos_x <= 160){

		    LCD_Sprite(pos_x, pos_y, 18, 26, manside_negro, 3, anim, 1, 0);
		    FillRect(0, 100, 18, 26, 0x0000);
		    FillRect(pos_x - 18, pos_y, 18, 26, 0x0000);
		    pos_x = pos_x + 10;
		    HAL_Delay(100);
			//LCD_Sprite(159, 100, 18, 26, buffer6, 3, 2, 0, 0);
		}
		LCD_Sprite(pos_x - 10, pos_y, 18, 26, mandown_negro, 3, anim, 1, 0);

		while(1){

			LCD_Print("J1 Gana", 23, 170, 2, 0xFFFF, 0x0000);
			HAL_Delay(1000);
			FillRect(0, 150, 160, 40, 0x0000);
			HAL_Delay(400);
		}

	}
	else {

		FillRect(0, 0, 319, 239, 0x0000);
		int anim = (pos_y2 / 10) % 3;
		pos_y2 = 100;
		pos_x2 = 0;
		while (pos_x2 <= 160){
			LCD_Print("J2 Gana", 23, 170, 2, 0xFFFF, 0x0000);
		    LCD_Sprite(pos_x2, pos_y2, 18, 26, manside2_negro, 3, anim, 1, 0);
		    FillRect(0, 100, 18, 26, 0x0000);
		    FillRect(pos_x2 - 18, pos_y2, 18, 26, 0x0000);
		    pos_x2 = pos_x2 + 10;
		    HAL_Delay(100);
		}
		LCD_Sprite(pos_x2 - 10, pos_y2, 18, 26, mandown2_negro, 3, anim, 1, 0);

		while(1){

			LCD_Print("J2 Gana", 23, 170, 2, 0xFFFF, 0x0000);
			HAL_Delay(1000);
			FillRect(0, 150, 160, 40, 0x0000);
			HAL_Delay(400);
		}
	}
	ciclos = 1;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
