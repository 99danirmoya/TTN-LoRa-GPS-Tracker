/* ***********************************************************************************************************************************************************
ARCHIVO DE MEDICION DEL NIVEL DE BATERIA

Archivo para medir la bateria (18650) en la unidad necesaria.
Inspirado en: https://gist.github.com/jenschr/dfc765cb9404beb6333a8ea30d2e78a1?permalink_comment_id=3446268
*********************************************************************************************************************************************************** */
#include "configuration.h"                                                           // Se usan macros declaradas en dicho archivo
#include <Wire.h>
#include <axp20x.h>                                                                  // Libreria para usar el "Power Management Unit" (PMU) I2C modelo AXP192 incluido en placas de LilyGO

AXP20X_Class axp;                                                                    // Se crea el objeto del PMU

float batVolt;                                                                       // Variable para calcular el voltaje a partir del valor leido por el ADC
uint16_t analogValue;
uint8_t batPercent;

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Setup del PMU AXP192
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
void AXP_setup(){
  Wire.begin(SDA_PIN, SCL_PIN);

  if(axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL){                             // Attempt to initialize the AXP192 with the correct I2C address (0x34)
    Serial.println("AXP192 not found at address 0x34!");
    while (1);
  }else{
    Serial.println("AXP192 found!");
  }

  axp.adc1Enable(AXP202_BATT_VOL_ADC1, true);                                        // Enable the ADCs for battery voltage measurement
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion medir el valor en bits de la batería (bits). SE DEBE HACER UN DIVISOR RESISTIVO ENTRE LA BATERÍA Y EL "VBAT_PIN" CON DOS RESISTENCIAS DE 100K
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t battery_value(){
  analogValue = analogRead(PMU_IRQ);

  return analogValue;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion medir el voltaje de la batería (voltios). SE DEBE HACER UN DIVISOR RESISTIVO ENTRE LA BATERÍA Y EL "VBAT_PIN" CON DOS RESISTENCIAS DE 100K
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
float battery_voltage(){
  batVolt = (float)(analogRead(PMU_IRQ)) / 4095 * 2 * 3.3 * 1.1;                    // Formula para calcular el voltaje de la bateria teniendo en cuenta el divisor resistivo de doble 100K ohm y el factor de correccion 1.1 del ADC

  return batVolt;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Funcion medir el nivel de batería (%)
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t battery_level(){
  analogValue = analogRead(PMU_IRQ);                                                // AQUI NO PONGO LA FORMULA QUE USO EN 'battery_value()' YA QUE LO CALIBRE VARIANDO LOS VOLTAJES CON UNA FUENTE DE ALIMENTACION, SE DEBE CAMBIAR EL DIVISOR A UNA RESISTENCIA DE 22K Y OTRA DE 47K
  
  if(analogValue >= 3200) batPercent = 100;                                          // Cualquier cosa por encima, 100%
  else if(analogValue >= 2015) batPercent = map(analogValue, 2015, 3200, 90, 100);   // Rango 90-100%, 3.7-4.2V
  else if(analogValue >= 1720) batPercent = map(analogValue, 1720, 2015, 10, 90);    // Rango 10-90%, 3.2-3.7V
  else if(analogValue >= 1300) batPercent = map(analogValue, 1300, 1720, 0, 10);     // Rango 0-10%, 2.5-3.2V
  else batPercent = 0;                                                               // Cualquier cosa por debajo, 0%
  
  return batPercent;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------