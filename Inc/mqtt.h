/*
 * mqtt.h
 *
 *  Created on: Sep 29, 2021
 *      Author: janoko
 */

#ifndef MQTT_CLIENT_INC_MQTT_H_
#define MQTT_CLIENT_INC_MQTT_H_

#include "stm32f4xx_hal.h"
#include "mqtt_packet.h"

#define MQTT_BUFFER_SIZE 512
#define MQTT_PROTOCOL_VERSION 5
#define MQTT_ID_SZ_MAX 32
#define MQTT_USERNAME_SZ_MAX 32
#define MQTT_PASSWORD_SZ_MAX 32

/* MQTT Status */
#define MQTT_STAT_RUNNING_CMD   0x01    // bit 0 : check wheater command is runnung
#define MQTT_STAT_CONNECTED     0x02    // bit 1 : check wheater client conneccted to server

#define MQTT_IS_STATUS(mqttc, stat) (((mqttc)->status & (stat)) != 0)
#define MQTT_SET_STATUS(mqttc, stat) {(mqttc)->status |= (stat);}
#define MQTT_UNSET_STATUS(mqttc, stat) {(mqttc)->status &= ~(stat);}

typedef struct {
  char ID[MQTT_ID_SZ_MAX];
  uint8_t status;
  uint8_t txBuffer[MQTT_BUFFER_SIZE];
  uint8_t rxBuffer[MQTT_BUFFER_SIZE];
  uint16_t txBufferLen;
  uint16_t rxBufferLen;
  MQTT_Packet tmpPacket;

  struct {
    uint16_t keepAlive;
    uint32_t sessionExpInterval;
  } options;

  struct {
    char username[MQTT_USERNAME_SZ_MAX];
    char password[MQTT_PASSWORD_SZ_MAX];
  } auth;
} MQTT_Client;

void MQTT_PacketSend(MQTT_Client*, uint8_t*data, uint16_t size);
void MQTT_PacketRecieve(MQTT_Client*);
void MQTT_LockCMD(MQTT_Client*);
void MQTT_UnlockCMD(MQTT_Client*);

void MQTT_SetAuth(MQTT_Client*, const char *username, const char *password);

// mqtt commands
void MQTT_Connect(MQTT_Client*, const char *id);

#endif /* MQTT_CLIENT_INC_MQTT_H_ */
