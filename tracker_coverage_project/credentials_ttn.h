/* ***********************************************************************************************************************************************************
ARCHIVO DE CLAVES OTAA PARA TTN

Se debe tener especial cuidado copiando las claves desde la consola de TTN ya que algunas deben venir en formato LSB y, otras, en MSB
*********************************************************************************************************************************************************** */
#pragma once

// Only one of these settings must be defined
//#define USE_ABP
#define USE_OTAA

#ifdef USE_ABP  // UPDATE WITH YOUR TTN KEYS AND ADDR. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //static const PROGMEM u1_t NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //static const u1_t PROGMEM APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //static const u4_t DEVADDR = 0x26010000 ;                                                   // <-- Change this address for every node!
#endif

#ifdef USE_OTAA  // UPDATE WITH YOUR TTN KEYS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  static const u1_t PROGMEM APPEUI[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Debe ir en formato LSB (least-significant-byte first)
  static const u1_t PROGMEM DEVEUI[8]  = { 0x52, 0x97, 0x06, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };  // Debe ir en formato LSB (least-significant-byte first)
  static const u1_t PROGMEM APPKEY[16] = { 0x0B, 0x6F, 0x58, 0x21, 0x95, 0xC1, 0x91, 0x26, 0x92, 0x4D, 0xA6, 0x9A, 0x1D, 0xBC, 0x46, 0xC2 };  // Debe ir en formato MSB (most-significant-byte first)
#endif