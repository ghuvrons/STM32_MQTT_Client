/*
 * mqtt.c
 *
 *  Created on: Sep 29, 2021
 *      Author: janoko
 */

#include "mqtt_packet.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debugger.h"

MQTT_Packet MQTT_Packet_New(MQTT_PacketType packetType, uint8_t *buffer)
{
  MQTT_Packet packet;
  memset((void *) &(packet), 0, sizeof(MQTT_Packet));
  *buffer = packetType;
  packet.buffer = buffer;
  packet.bufferPtr = buffer + 5;            // 1 byte packet type + 4 bytes length
  return packet;
}


uint8_t* MQTT_Packet_Buffer(MQTT_Packet *packet)
{
  uint8_t sizeOflen;
  uint8_t *buffer;

  buffer = packet->buffer + 5;              // 1 byte packet type + 4 bytes length
  packet->bufferPtr = packet->buffer + 1;

  sizeOflen = MQTT_Packet_AddVarInt(packet, packet->length);
  packet->bufferLen = packet->length+1;
  packet->length -= sizeOflen;
  
  if(sizeOflen < 4)
  {
    sizeOflen++; // add 1 byte for packet type
    
    // shift to right (packet type and length)
    while((sizeOflen)){
      sizeOflen--;
      packet->bufferPtr--;
      buffer--;
      *buffer = *(packet->bufferPtr);
    }
  }
  return buffer;
}


uint8_t MQTT_Packet_AddInt8(MQTT_Packet *packet, int8_t data)
{
  *(packet->bufferPtr) = data;
  packet->bufferPtr++;
  packet->length++;

  return 1;
}


uint8_t MQTT_Packet_AddInt16(MQTT_Packet *packet, int16_t data)
{
  // data save to buffer in big endian
  packet->bufferPtr += 2;
  for (uint8_t i = 0; i < 2; i++)
  {
    packet->bufferPtr--;
    *(packet->bufferPtr) = data & 0xFF;
    data >>= 8;
  }
  packet->bufferPtr += 2;
  packet->length += 2;

  return 2;
}


uint8_t MQTT_Packet_AddInt32(MQTT_Packet *packet, int32_t data)
{
  // data save to buffer in big endian
  packet->bufferPtr += 4;
  for (uint8_t i = 0; i < 4; i++)
  {
    packet->bufferPtr--;
    *(packet->bufferPtr) = data & 0xFF;
    data >>= 8;
  }
  packet->bufferPtr += 4;
  packet->length += 4;

  return 4;
}


uint8_t MQTT_Packet_AddVarInt(MQTT_Packet *packet, int data)
{
  uint8_t result;
  uint8_t tmp[4];
  int8_t i = 4;

  memset(tmp, 0, 4);
  do {
    i--;
    tmp[i] = (uint8_t)(data % 128);
    data /= 128;
    if(data > 0){
      tmp[i] |= 128;
    }
  } while (data && i);

  result = 4-i;

  while (i < 4)
  {
    *(packet->bufferPtr) = tmp[i];
    packet->bufferPtr++;
    i++;
  }
  packet->length += result;
  
  return result;
}


uint16_t MQTT_Packet_AddBytes(MQTT_Packet *packet, const uint8_t *data, uint16_t length)
{
  uint16_t result;

  result = MQTT_Packet_AddInt16(packet, (int16_t)length);

  for (uint16_t i = 0; i < length; i++)
  {
    *(packet->bufferPtr) = *data;
    packet->bufferPtr++;
    data++;
  }
  packet->length += length;

  result += length;
  return result;
}


void MQTT_Packet_StartAddProperties(MQTT_Packet *packet)
{
  packet->tmpPropLen = 0;
  packet->bufferPtr += 4;     // set free space for length of properties
}


void MQTT_Packet_StopAddProperties(MQTT_Packet *packet)
{
  uint8_t sizeOflen;
  uint8_t *tmpPropPtr;
  // pointer back into before properties start;
  packet->bufferPtr -= (packet->tmpPropLen + 4);
  // pointer of properties
  tmpPropPtr = packet->bufferPtr + 4;

  sizeOflen = MQTT_Packet_AddVarInt(packet, packet->tmpPropLen);
  if(sizeOflen < 4)
  {
    for (uint16_t i = 0; i < packet->tmpPropLen; i++)
    {
      *(packet->bufferPtr) = *(tmpPropPtr);
      packet->bufferPtr++;
      tmpPropPtr++;
    }
  }
}


void MQTT_Packet_AddProperties(
    MQTT_Packet *packet, 
    MQTT_PacketPropType propType, 
    const uint8_t *data, uint16_t length
){
  MQTT_Packet_AddInt8(packet, propType);
  packet->tmpPropLen++;

  switch (propType)
  {
  case MQTT_PROP_PL_FORMAT_ID:
  case MQTT_PROP_REQ_PROB_INFO:
  case MQTT_PROP_REQ_RESP_INFO:
  case MQTT_PROP_MAX_QOS:
  case MQTT_PROP_RETAIN_AV:
  case MQTT_PROP_WILL_SUBSCR_AV:
  case MQTT_PROP_SUBSCR_ID_AV:
  case MQTT_PROP_SHARED_SUBSCR_AV:
    // byte
    packet->tmpPropLen += MQTT_Packet_AddInt8(packet, *data);
    break;

  case MQTT_PROP_SVR_KEEP_ALIVE:
  case MQTT_PROP_RECV_MAX:
  case MQTT_PROP_TOPIC_ALIAS_MAX:
  case MQTT_PROP_TOPIC_ALIAS:
    // two bytes
    packet->tmpPropLen += MQTT_Packet_AddInt16(packet, *((int16_t *)data));
    break;

  case MQTT_PROP_MSG_EXP_INTV:
  case MQTT_PROP_SESS_EXP_INTV:
  case MQTT_PROP_WILL_DELAY_INTV:
  case MQTT_PROP_MAX_PACKET_SZ:
    // four bytes
    packet->tmpPropLen += MQTT_Packet_AddInt32(packet, *((int32_t *)data));
    break;

  case MQTT_PROP_SUBSCR_ID:
    // var integer
    packet->tmpPropLen += MQTT_Packet_AddVarInt(packet, *((int *)data));
    break;

  case MQTT_PROP_CONTENT_TYPE:
  case MQTT_PROP_RESP_TOPIC:
  case MQTT_PROP_CORRELATION_DATA:
  case MQTT_PROP_ASSG_CLIENT_ID:
  case MQTT_PROP_AUTH_METHOD:
  case MQTT_PROP_AUTH_DATA:
  case MQTT_PROP_RESP_INFO:
  case MQTT_PROP_SVR_REF:
  case MQTT_PROP_REASON_STR:
  case MQTT_PROP_USER_PROP:
    // n bytes
    packet->tmpPropLen += MQTT_Packet_AddBytes(packet, data, length);
    break;

  default:
    break;
  }
}

