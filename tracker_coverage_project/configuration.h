/* ***********************************************************************************************************************************************************
TTGO LoRa32 V2.1 for The Things Network

Copyright (C) 2019 by Roel van Wanrooy (www.connectix.nl)

This code requires LMIC library by Matthijs Kooijman: https://github.com/matthijskooijman/arduino-lmic

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************************************************************************************** */
#pragma once

#include <Arduino.h>
#include <lmic.h>
void ttn_register(void (*callback)(uint8_t message));

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Configuration
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define SINGLE_CHANNEL_GATEWAY  0           // If using a single-channel gateway, uncomment this next option and set to your gateway's channel
#define CLOCK_ERROR             1           // If you are having difficulty sending messages to TTN after the first successful send, uncomment the next option and experiment with values (~ 1 - 5)
#define DEBUG_PORT              Serial      // Serial debug port
#define SERIAL_BAUD             115200      // Serial debug baud rate
#define SLEEP_BETWEEN_MESSAGES  1           // Do sleep between messages
#define ENABLE_DEBUG            1           // Serial debug on/off
#define ENABLE_VSI              0           // Habilitar el VSI
#define ENABLE_DOWNLINK         1           // Habilita los downlinks personales
#define SEND_INTERVAL_STATIC    300000      // Tiempo de espera entre mensajes SI NO SE HABILITA EL VSI
#define SEND_INTERVAL_RELAXED   600000      // Tiempo de espera entre mensajes cuando la distancia varia poco entre medidas SI SE HABILITA EL VSI
#define SEND_INTERVAL_INTENSIVE 300000      // Tiempo de espera entre mensajes cuando la distancia varia mucho entre medidas SI SE HABILITA EL VSI
#define MESSAGE_TO_SLEEP_DELAY  1000        // Time after message before going to sleep
#define LORAWAN_PORT            1           // Port the messages will be sent to
#define LORAWAN_CONFIRMED_EVERY 0           // Send confirmed message every these many messages (0 means never)
#define LORAWAN_SF              DR_SF12     // Spreading factor
#define LORAWAN_ADR             0           // Enable ADR
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// DEBUG
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef DEBUG_PORT
#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// Custom messages
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
#define EV_QUEUED       100
#define EV_PENDING      101
#define EV_ACK          102
#define EV_RESPONSE     103
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// LoRa SPI
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
#define SCK_GPIO        5
#define MISO_GPIO       19
#define MOSI_GPIO       27
#define NSS_GPIO        18
#define RESET_GPIO      23
#define DIO0_GPIO       26
#define DIO1_GPIO       33
#define DIO2_GPIO       32
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
// AXP192 (Rev1-specific options)
// -----------------------------------------------------------------------------------------------------------------------------------------------------------
#define LORA_POWER_CTRL_CH  2
#define PMU_IRQ             35
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Aportes (sensores, variables, ...)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LED_PIN             4               // BUILTIN LED de la LilyGO T3 v1.6
#define BUTTON_PIN          38
#define GPS_RX_PIN          34
#define GPS_TX_PIN          12
#define SerialGPS           Serial1
#define GPS_BAUD_RATE       9600
#define SDA_PIN             21
#define SCL_PIN             22
#define TX_BUFFER_SIZE      23              // Tamaño del buffer de bytes a enviar a TTN
#define RX_BUFFER_SIZE      1               // Tamaño del buffer de bytes a recibir desde TTN
#define TARGET_ARRAY_LENGTH 5               // Tamaño del Buffer Circular
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~