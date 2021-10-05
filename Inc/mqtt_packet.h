/*
 * mqtt.h
 *
 *  Created on: Sep 29, 2021
 *      Author: janoko
 */

#ifndef MQTT_CLIENT_INC_MQTT_PACKET_H_
#define MQTT_CLIENT_INC_MQTT_PACKET_H_

#include "stm32f4xx_hal.h"

/* PACKET TYPES */
#define MQTT_PACKET_TYPE_CONNECT  0x10    // packet type : connect
#define MQTT_PACKET_TYPE_CONNACK  0x20    // packet type : Connect acknowledgment
#define MQTT_PACKET_TYPE_PUBLISH  0x30    // packet type : Publish message
#define MQTT_PACKET_TYPE_PUBACK   0x40    // packet type : Publish acknowledgment (QoS 1)
#define MQTT_PACKET_TYPE_PUBREC   0x50    // packet type : Publish received (QoS 2 delivery part 1)
#define MQTT_PACKET_TYPE_PUBREL   0x60    // packet type : Publish release (QoS 2 delivery part 2)
#define MQTT_PACKET_TYPE_PUBCOMP  0x70    // packet type : Publish complete (QoS 2 delivery part 3)

/* Packet Properties */
#define MQTT_PROP_PL_FORMAT_ID          0x01
#define MQTT_PROP_MSG_EXP_INTV          0x02
#define MQTT_PROP_CONTENT_TYPE          0x03
#define MQTT_PROP_RESP_TOPIC            0x08
#define MQTT_PROP_CORRELATION_DATA      0x09
#define MQTT_PROP_SUBSCR_ID             0x0B
#define MQTT_PROP_SESS_EXP_INTV         0x11
#define MQTT_PROP_ASSG_CLIENT_ID        0x12
#define MQTT_PROP_SVR_KEEP_ALIVE        0x13
#define MQTT_PROP_AUTH_METHOD           0x15
#define MQTT_PROP_AUTH_DATA             0x16
#define MQTT_PROP_REQ_PROB_INFO         0x17
#define MQTT_PROP_WILL_DELAY_INTV       0x18
#define MQTT_PROP_REQ_RESP_INFO         0x19
#define MQTT_PROP_RESP_INFO             0x1A
#define MQTT_PROP_SVR_REF               0x1C
#define MQTT_PROP_REASON_STR            0x1F
#define MQTT_PROP_RECV_MAX              0x21
#define MQTT_PROP_TOPIC_ALIAS_MAX       0x22
#define MQTT_PROP_TOPIC_ALIAS           0x23
#define MQTT_PROP_MAX_QOS               0x24
#define MQTT_PROP_RETAIN_AV             0x25
#define MQTT_PROP_USER_PROP             0x26
#define MQTT_PROP_MAX_PACKET_SZ         0x27
#define MQTT_PROP_WILL_SUBSCR_AV        0x28
#define MQTT_PROP_SUBSCR_ID_AV          0x29
#define MQTT_PROP_SHARED_SUBSCR_AV      0x2A


typedef uint8_t MQTT_PacketType;
typedef uint8_t MQTT_PacketPropType;


typedef struct {
  MQTT_PacketType type;
  uint16_t length;
  uint16_t bufferLen;
  uint8_t *buffer;
  uint8_t *bufferPtr;
  uint16_t tmpPropLen;        // length of properties
} MQTT_Packet;

MQTT_Packet MQTT_Packet_New(MQTT_PacketType, uint8_t *buffer);
uint8_t MQTT_Packet_AddInt8(MQTT_Packet*, int8_t);
uint8_t MQTT_Packet_AddInt16(MQTT_Packet*, int16_t);
uint8_t MQTT_Packet_AddInt32(MQTT_Packet*, int32_t);
uint8_t MQTT_Packet_AddVarInt(MQTT_Packet*, int);
uint16_t MQTT_Packet_AddBytes(MQTT_Packet*, const uint8_t*, uint16_t length);
void MQTT_Packet_StartAddProperties(MQTT_Packet*);
void MQTT_Packet_StopAddProperties(MQTT_Packet*);
void MQTT_Packet_AddProperties(
    MQTT_Packet*, MQTT_PacketPropType, 
    const uint8_t *data, uint16_t length
);
uint8_t *MQTT_Packet_Encode(MQTT_Packet*);
MQTT_Packet MQTT_Packet_Decode(uint8_t*, uint16_t length);

#endif /* MQTT_CLIENT_INC_MQTT_PACKET_H_ */
