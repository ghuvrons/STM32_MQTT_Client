/*
 * mqtt.c
 *
 *  Created on: Sep 29, 2021
 *      Author: janoko
 */

#include "mqtt_packet.h"
#include "mqtt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debugger.h"

__weak void MQTT_PacketSend(MQTT_Client *packet, uint8_t *data, uint16_t size)
{
  return;
}


__weak void MQTT_PacketRecieve(MQTT_Client *packet)
{
  return;
}

__weak void MQTT_LockCMD(MQTT_Client *mqttClient)
{
  while(MQTT_IS_STATUS(mqttClient, MQTT_STAT_RUNNING_CMD));
  MQTT_SET_STATUS(mqttClient, MQTT_STAT_RUNNING_CMD);
}


__weak void MQTT_UnlockCMD(MQTT_Client *mqttClient)
{
  MQTT_UNSET_STATUS(mqttClient, MQTT_STAT_RUNNING_CMD);
}


void MQTT_SetAuth(MQTT_Client *mqttClient, const char *username, const char *password)
{
  uint16_t i;
  
  for (i = 0; i < MQTT_USERNAME_SZ_MAX-1; i++)
  {
    mqttClient->auth.username[i] = *username;
    username++;
    if(!mqttClient->auth.username[i]) break;
  }
  for (i = 0; i < MQTT_PASSWORD_SZ_MAX-1; i++)
  {
    mqttClient->auth.password[i] = *password;
    password++;
    if(!mqttClient->auth.password[i]) break;
  }
  
}


void MQTT_Connect(MQTT_Client *mqttClient, const char *id)
{
  MQTT_Packet packet;
  uint8_t *packetBuffer;
  uint8_t cFlag;

  MQTT_LockCMD(mqttClient);

  cFlag = 2; // clean set
  if(mqttClient->auth.username[0] != 0) cFlag |= 0x80;
  if(mqttClient->auth.password[0] != 0) cFlag |= 0x40;

  packet = MQTT_Packet_New(MQTT_PACKET_TYPE_CONNECT, mqttClient->txBuffer);
  MQTT_Packet_AddBytes(&packet, (uint8_t *) "MQTT", 4);
  MQTT_Packet_AddInt8(&packet, MQTT_PROTOCOL_VERSION);
  MQTT_Packet_AddInt8(&packet, cFlag);
  MQTT_Packet_AddInt16(&packet, mqttClient->options.keepAlive);

  // Properties
  MQTT_Packet_StartAddProperties(&packet);
  {
    if(mqttClient->options.sessionExpInterval){
      MQTT_Packet_AddProperties(&packet, 
        MQTT_PROP_SESS_EXP_INTV, 
        (uint8_t *) &(mqttClient->options.sessionExpInterval), 1
      );
    }
  }
  MQTT_Packet_StopAddProperties(&packet);
  
  // Payload
  MQTT_Packet_AddBytes(&packet, (const uint8_t *) id, strlen(id));

  if(mqttClient->auth.username[0] != 0){
    MQTT_Packet_AddBytes(&packet,
      (uint8_t *) mqttClient->auth.username,
      strlen(mqttClient->auth.username)
    );
  }

  if(mqttClient->auth.password[0] != 0){
    MQTT_Packet_AddBytes(&packet,
      (uint8_t *) mqttClient->auth.password,
      strlen(mqttClient->auth.password)
    );
  }

  packetBuffer = MQTT_Packet_Generate(&packet);

  DBG_Log((uint8_t*) "Connect Buffer", 14);
  DBG_Log(packetBuffer, packet.bufferLen);

  MQTT_PacketSend(mqttClient, packetBuffer, packet.bufferLen);
  MQTT_UnlockCMD(mqttClient);
}
