/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ARCHIVO PARA INCLUIR FUNCIONES DE SENSORES Y AÑADIR MEDICIONES A "txBuffer"

Este archivo ha sido modificado de manera considerable para implementar el sensor ultrasónico de distancia JSN-SR04T en vez del original BME280.
Enlace para consultar el proyecto original del usuario de GitHub "rwanrooy": https://github.com/rwanrooy/TTGO-PAXCOUNTER-LoRa32-V2.1-TTN.git
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Librerias de sensores
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <TinyGPS++.h>
#include "configuration.h"                                                                        // Se usan macros declarados en dicho archivo
#include "bat.h"                                                                                  // Se llama a la funcion de medicion de bateria para añadirlo al txBuffer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Macros, constructores, variables... de sensores
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TinyGPSPlus gps;

float lat, lon;
uint16_t axpVolt;
uint8_t sat;
int alt, snrInt;
long latInt, lonInt;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup inicial del sensor
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void GPS_setup(){
  Serial.println("LoRa tracker 99danirmoya");
  SerialGPS.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(3000);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup inicial del LED controlado por downlink
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LED_setup(){
  pinMode(LED_PIN, OUTPUT);                                                                       // Suelo tener la costumbre de inicializar los pines de luces a 'LOW' pero, como cuando se despierta de deep sleep, 'setup()' vuelve a ejecturse, se apagaria sin yo quererlo
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void button_setup(){
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Setup inicial del LED controlado por downlink
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void medidas_GPS(){
  while(SerialGPS.available() > 0){
    if(gps.encode(SerialGPS.read())){
      if(gps.location.isValid()){
        lat = gps.location.lat();
        lon = gps.location.lng();
        alt = gps.altitude.meters();
        sat = gps.satellites.value();
      }else{
        Serial.println(F("INVALID GPS"));
      }
    }
  }
}

// ===========================================================================================================================================================
// FUNCION PRINCIPAL DE "sensor.ino" - Construir el paquete de datos que se enviara por LoRa, SE DEBE EDITAR PARA INCLUIR LAS VARIABLES DE OTROS SENSORES, PERO MANTENER LA ESTRUCTURA DE DESCOMPOSICION EN BYTES PARA SER AÑADIDOS AL "txBuffer"
// ===========================================================================================================================================================
void build_packet(uint8_t txBuffer[TX_BUFFER_SIZE]){                                              // Uso 'uint8_t' para que todos los datos sean del tamaño de 1byte (8bits)  
  #if ENABLE_DOWNLINK == 1
    // Estado del LED (EJEMPLO PARA AÑADIR DOWNLINK, DEBE ESTAR ACTIVADO EN 'configuration.h') ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if(onLED == true){
      txBuffer[0] = byte(1);                                                                      // Se manda el byte 0x01
      Serial.println("LED On");
    }else{
      txBuffer[0] = byte(0);                                                                      // Se manda el byte 0x00
      Serial.println("LED Off");
    }
  #else
    // SI EL DOWNLINK NO ESTÁ ACTIVADO EN 'configuration.h', MANDO SIEMPRE 0) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    txBuffer[0] = byte(0);
    Serial.println("LED Off");
  #endif
  
  Serial.println(F("-------------------"));

  // GPS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  medidas_GPS();

  Serial.print("Latitud: "); Serial.print(lat, 6); Serial.print("\t");
  latInt = lat * 1000000;
  txBuffer[1] = (byte)((latInt & 0X000000FF));
  txBuffer[2] = (byte)((latInt & 0X0000FF00) >> 8 );
  txBuffer[3] = (byte)((latInt & 0X00FF0000) >> 16);
  txBuffer[4] = (byte)((latInt & 0XFF000000) >> 24);
  
  Serial.print("Longitud: "); Serial.print(lon, 6); Serial.print("\t");
  lonInt = lon * 1000000;
  txBuffer[5] = (byte)((lonInt & 0X000000FF));
  txBuffer[6] = (byte)((lonInt & 0X0000FF00) >> 8 );
  txBuffer[7] = (byte)((lonInt & 0X00FF0000) >> 16);
  txBuffer[8] = (byte)((lonInt & 0XFF000000) >> 24);

  Serial.print("Altitud: "); Serial.print(alt); Serial.print("\t");
  txBuffer[9] = lowByte(alt);
  txBuffer[10] = highByte(alt);
  
  Serial.print("Satelites: "); Serial.println(sat);
  txBuffer[11] = (byte)(sat);

  Serial.println(F("-------------------"));

  // LoRa ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  txBuffer[12] = (byte)((rssi & 0X000000FF));
  txBuffer[13] = (byte)((rssi & 0X0000FF00) >> 8 );
  txBuffer[14] = (byte)((rssi & 0X00FF0000) >> 16);
  txBuffer[15] = (byte)((rssi & 0XFF000000) >> 24);
  
  snrInt = snr * 100;
  txBuffer[16] = (byte)((snrInt & 0X000000FF));
  txBuffer[17] = (byte)((snrInt & 0X0000FF00) >> 8 );
  txBuffer[18] = (byte)((snrInt & 0X00FF0000) >> 16);
  txBuffer[19] = (byte)((snrInt & 0XFF000000) >> 24);

  Serial.print("RSSI: "); Serial.print(rssi); Serial.print(" dBm"); Serial.print("\tSNR: "); Serial.print(snr); Serial.println(" dB");
  Serial.println(F("-------------------"));

  // Bateria ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  axpVolt = axp.getBattVoltage();
  txBuffer[20] = lowByte(axpVolt);
  txBuffer[21] = highByte(axpVolt);

  Serial.print(F("Voltaje de la batería: ")); Serial.print(axpVolt); Serial.println(F(" mV"));
  Serial.println(F("==================="));                                                       // Separador visual entre bloques de datos debug

  // ALARMA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  txBuffer[22] = buttonPressed ? (byte)(1) : (byte)(0);

  Serial.println(F("Bytes a enviar por LoRa:"));
  for(uint8_t i = 0; i < TX_BUFFER_SIZE; i++){
    Serial.print(txBuffer[i]); Serial.print(F(" "));                                                // Print in decimal format
  }
  Serial.println();
  
  Serial.println(F("==================="));                                                         // Separador visual entre bloques de datos debug
}