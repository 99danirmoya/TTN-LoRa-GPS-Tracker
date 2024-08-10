/* ***********************************************************************************************************************************************************
SENSOR COOPER V4 - MediaLab_ IoT, UNIVERSIDAD DE OVIEDO

Este archivo ha sido modificado de manera considerable para implementar el sensor ultrasónico de distancia JSN-SR04T en vez del original BME280.
El enlace para consultar el proyecto original del usuario de GitHub "rwanrooy" es el siguiente:
https://github.com/rwanrooy/TTGO-PAXCOUNTER-LoRa32-V2.1-TTN.git

A LO LARGO DEL PROYECTO, SE INDICA CON LINEAS HECHAS CON "~" AQUELLAS SECCIONES DE CODIGO QUE DEBEN SER EDITADAS A GUSTO, EL RESTO NO SE DEBE DE MODIFICAR.
*********************************************************************************************************************************************************** */

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// LIBRERIAS
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Wire.h>
#include "configuration.h"                                               // Libreria de macros
#include "bat.h"                                                                                  // Se llama a la funcion de medicion de bateria para añadirlo al txBuffer
#include "sensor.h"                                                      // Libreria de funciones de sensores y construccion del "txBuffer" con las medidas a enviar
#include "sleep.h"                                                       // Libreria de funciones para activar el deep sleep
#include "vsi.h"                                                         // Libreria de funciones para activar el Variable Send Interval
#include "rom/rtc.h"                                                     // Libreria para usar la memoria RTC del ESP32, donde se pueden guardar variables cuyos valores sobreviven al deep sleep
#include "driver/gpio.h"                                                 // GPIO driver
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// VARIABLES A GUARDAR EN LA MEMORIA RTC (sobrevive al deep sleep)
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
RTC_DATA_ATTR uint32_t bootCount = 0;                                    // Contador de despertares tras deep sleep, almacenado en la memoria RTC para sobrevivir deep sleep. Se usa para el control de los mensajes confirmados
RTC_DATA_ATTR uint16_t bufferCircular[TARGET_ARRAY_LENGTH];              // Lista donde guardo los últimos 5 valores enviados a TTN. LA ALOJO EN LA MEMORIA RTC PARA QUE SOBREVIVA AL DEEP SLEEP
RTC_DATA_ATTR bool onLED = false;                                        // Booleano para indicar en el Uplink el estado del LED que se modifica desde Downlink
RTC_DATA_ATTR volatile bool buttonPressed = false;                       // Declare the boolean variable as volatile since it will be modified in an ISR
RTC_DATA_ATTR int rssi;                                                  // Variable para guardar el parametro de calidad de señal RSSI. Se guarda en la memoria RTC ya que se obtiene al final de cada retransmision y se envia en la siguiente
RTC_DATA_ATTR float snr;
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// VARIABLES A GUARDAR EN LA MEMORIA FLASH (se reinician tras el deep sleep)
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t txBuffer[TX_BUFFER_SIZE];                                 // Como el array de bytes que se envía a TTN se calcula en otro archivo, lo declaro como static para que sea visible. AJUSTAR EL NUMERO DE BYTES AL DEL PAYLOAD
static uint8_t rxBuffer[RX_BUFFER_SIZE];

uint32_t tiempoPrevio = 0;                                               // Variable para almacenar el tiempo previo en el que se ejecuto el envio de datos
uint32_t sendInterval;                                                   // Variable donde se guarda el tiempo entre mensajes
bool first = true;                                                       // Booleano para entrar una vez en el bloque de envio de datos si se recibe confirmacion de TX
bool confirmed;                                                          // Variable booleana donde se guarda si se mandan o no mensajes confirmados a TTN
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// INTERRUPCION PARA ACTIVAR EL MODO ALARMA DEL TRACKER
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;                                                  // Set the boolean variable to true when the interrupt is triggered
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para enviar el paquete de datos LoRa
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void do_send(){
  build_packet(txBuffer);
  
  // Gestión del Send Interval (AQUÍ PARA EVITAR ENÉSIMAS ITERACIONES) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  #if ENABLE_VSI == 1
    uint16_t medidaBuffer = txBuffer[1] + (txBuffer[2] << 8);            // Variable en la que se guarda el último valor de distancia enviado a TTN y almacenado en el buffer circular. SE DEBE EDITAR EL TIPO DE VARIABLE (p.e.: int, uint8_t, long, ...), TENIENDOSE EN CUENTA EL NUMERO DE BYTES
    carga_valores(medidaBuffer); print_array();                          // Se cargan el valor actual del sensor al buffer circular y se imprime por pantalla el estado actual del buffer
    sendInterval = variable_send_interval();                             // Se asigna el valor del "Variable Send Interval" a la variable "sendInterval"
  #else
    sendInterval = static_send_interval();                               // Se asigna el valor del "Static Send Interval" a la variable "sendInterval"
  #endif
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Mensajes confirmados ------------------------------------------------------------------------------------------------------------------------------------
  #if LORAWAN_CONFIRMED_EVERY > 0
    confirmed = (bootCount % LORAWAN_CONFIRMED_EVERY == 0);
  #else
    confirmed = false;
  #endif
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------

  ttn_cnt(bootCount);
  ttn_send(txBuffer, sizeof(txBuffer), LORAWAN_PORT, confirmed);

  bootCount++;                                                           // Se le suma uno al contador de arranques tras cada ciclo de envío de datos por LoRa
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion para mostrar mensajes por monitor serial segun se interactue con TTN por medio de "ttn.ino"
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void callback(uint8_t message){
  // Callbacks simples ---------------------------------------------------------------------------------------------------------------------------------------
  if(EV_JOINING       == message){ Serial.println(F("Conectando con TTN..."));                       Serial.println(F("-------------------")); }
  if(EV_JOINED        == message){ Serial.println(F("¡Conectado con TTN!"));                         Serial.println(F("-------------------")); }
  if(EV_JOIN_FAILED   == message){ Serial.println(F("¡Conexion con TTN fallida!"));                  Serial.println(F("-------------------")); }
  if(EV_REJOIN_FAILED == message){ Serial.println(F("¡Nuevo intento de conexion con TTN fallido!")); Serial.println(F("-------------------")); }
  if(EV_RESET         == message){ Serial.println(F("Reseteo de conexion con TTN"));                 Serial.println(F("-------------------")); }
  if(EV_LINK_DEAD     == message){ Serial.println(F("¡Link con TTN muerto!"));                       Serial.println(F("-------------------")); }
  if(EV_ACK           == message){ Serial.println(F("¡ACK recibido (mensaje confirmado)!"));         Serial.println(F("-------------------")); }
  if(EV_PENDING       == message){ Serial.println(F("Mensaje descartado"));                          Serial.println(F("-------------------")); }
  if(EV_QUEUED        == message){ Serial.println(F("Mensaje en cola..."));                          Serial.println(F("-------------------")); }
  
  // Callbacks complejas -------------------------------------------------------------------------------------------------------------------------------------
  if(EV_TXCOMPLETE    == message){ Serial.println(F("¡TRANSMISION COMPLETADA, ABIERTO RX! (si el Downlink está activado)"));
    #if ENABLE_DOWNLINK == 1
      if(LMIC.txrxFlags & TXRX_ACK){                                     // Si se recibe 'TXRX_ACK' en LMIC.txrxFlags, se confirma que hubo un ACK
        Serial.println(F("¡ACK recibido!"));
      }

      if(LMIC.dataLen){                                                  // Si hay algo en la funcion 'LMIC.dataLen', que contiene la cantidad de bytes de downlink, se procede a su procesamiento
        Serial.print(F("Recibidos "));
        Serial.print(LMIC.dataLen);                                      // Se muestra el tamaño del downlink
        Serial.println(F(" bytes de payload (downlink)"));
        
        Serial.print(F("Payload (downlink) recibido: "));
        for(int i = 0; i < LMIC.dataLen; i++){                           // Bucle para barrer el numero de bytes del downlink
          if(LMIC.frame[LMIC.dataBeg + i] < 0x10){                       // Este 'if' se encarga de añadir un 0 a los bytes menores de 16 en hexadecimal ('0A' en vez de 'A')
            Serial.print(F("0"));
          }
          Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);               // Se imprimen por pantalla los bytes recibidos
        }
        Serial.println();                                                // Se introduce un salto de linea cuando se han imprimido todos los bytes de downlink

        // Acciones en funcion del downlink ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if(LMIC.frame[LMIC.dataBeg] == 1){                               // Si el primer byte recibido es "1", se enciende el BUILTIN LED, si no, no
          Serial.println("LED ON");
          digitalWrite(LED_PIN, HIGH);
          gpio_hold_en(GPIO_NUM_4);                                      // Enable hold on GPIO04
          gpio_deep_sleep_hold_en();                                     // Enable deep sleep hold
          onLED = true;
        }
        else if (LMIC.frame[LMIC.dataBeg] == 0){
          gpio_hold_dis(GPIO_NUM_4);                                     // Disable hold on GPIO04
          digitalWrite(LED_PIN, LOW);
          Serial.println("LED OFF");
          onLED = false;
        }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      }

      rssi = LMIC.rssi;
      snr = LMIC.snr;
    #endif

    Serial.println(F("-------------------"));
    sleep();                                                             // Me voy a dormir aquí, cuando se confirma el envío del payload y se recibe el downlink
  }

  #if ENABLE_DOWNLINK == 1
    if(EV_RESPONSE == message){
      Serial.println(F("Respuesta de TTN: "));

      size_t len = ttn_response_len();                                   // Variable para guardar el tamaño de la respuesta
      uint8_t data[len];                                                 // Array de tamaño de la respuesta
      ttn_response(data, len);                                           // Funcion para guardar el numero de bytes 'len' en el array 'data'

      char buffer[3];                                                    // Buffer para imprimir el mensaje por serial monitor. El tamaño que se le reserva es 3 ya que se esperan valores hexadecimales del tipo "0xAB"
      for(uint8_t i = 0; i < len; i++){                                  // Se barre el mensaje en bytes
        snprintf(buffer, sizeof(buffer), "%02X", data[i]);               // Se formatean los bytes en formato hexadecimal de 2 digitos y lo guardamos en buffer
        Serial.print(buffer);                                            // Se imprime el buffer por serial monitor
      }
      Serial.println();
      Serial.println(F("-------------------"));
    }
  #endif
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// ===========================================================================================================================================================
// Setup main - MODIFICAR PIN MODES Y SUS CONDICIONES INICIALES
// ===========================================================================================================================================================
void setup(){
  #if ENABLE_DEBUG == 1                                                  // Activar o desactivar desde "configuration.h" el monitor serial para debugging
    DEBUG_PORT.begin(SERIAL_BAUD);
  #endif

  // Setup de los perifericos que envian por LoRa ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  GPS_setup();
  LED_setup();
  button_setup();
  boton_despertador();
  AXP_setup();
  
  // TTN setup -----------------------------------------------------------------------------------------------------------------------------------------------
  if(!ttn_setup()){
    Serial.println(F("[ERR] Chip de radio LoRa no encontrado, ¡ACTIVANDO DEEP SLEEP HASTA REINICIO MANUAL!"));
    delay(MESSAGE_TO_SLEEP_DELAY);
    esp_deep_sleep_start();                                              // Aquí me voy a dormir para siempre, como si fuese un while(1) pero de bajo consumo
  }

  // TTN register --------------------------------------------------------------------------------------------------------------------------------------------
  ttn_register(callback);                                                // Funcion de eventos de TTN
  ttn_join();
  ttn_sf(LORAWAN_SF);                                                    // Spreading Factor
  ttn_adr(LORAWAN_ADR);
}
// ===========================================================================================================================================================

// ===========================================================================================================================================================
// Loop main
// ===========================================================================================================================================================
void loop(){
  ttn_loop();                                                            // Función definida en "ttn.ino" en la que se ejecuta la función principal "os_runloop_once()" de la librería LMIC. GESTION DE CALLBACKS
  
  if(buttonPressed && !onLED){                                           // Check if the button press has been detected
    Serial.println("¡ALARMA ACTIVA!");
    digitalWrite(LED_PIN, HIGH);
    gpio_hold_en(GPIO_NUM_4);                                            // Enable hold on GPIO04
    gpio_deep_sleep_hold_en();                                           // Enable deep sleep hold
    onLED = true;
  }

  if(tiempoPrevio == 0 || millis() - tiempoPrevio > sendInterval){       // 'if' en el que solo se entra la primera vez que se ejecuta el loop al arrancar o despertar de deep sleep para tomar medidas. Despues, se encarga la funcion 'ttn_loop()'
    tiempoPrevio = millis();
    first = false;
    Serial.println(F("===================")); Serial.print(F("TRANSMISION NUMERO ")); Serial.println(bootCount + 1); Serial.println(F("==================="));
    do_send();                                                           // Llamamos a la función "send()", encargada de enviar los datos por LoRa
  }
}
// ===========================================================================================================================================================
