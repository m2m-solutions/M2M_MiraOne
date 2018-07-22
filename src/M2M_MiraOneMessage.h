//---------------------------------------------------------------------------------------------
//
// Library for the Lumenradio MiraOne radio module.
//
// Copyright 2018, M2M Solutions AB
// Written by Jonny Bergdahl, 2018-07-05
//
// Licensed under the MIT license, see the LICENSE.txt file.
//
//---------------------------------------------------------------------------------------------
#ifndef __M2M_MIRAONEMESSAGE_h__
#define __M2M_MIRAONEMESSAGE_h__

// MiraOne message structure
//
// +-------------------+--------------+---------------
// | Description       | Type         | Value
// +-------------------+--------------+------------
// | Start  character  | uint8_t      | 0xE1
// | Message header    | uint8_t      | Se Message header section
// | Message index     | uint8_t      | Sequence number
// | Data size         | uint8_t      | Payload data size
// | Address           | None/char[8] | 
// | Data              | Variable     |
// | CRC               | unit16_t     |
// +-------------------+--------------+

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include <Arduino.h>
#include <M2M_Logger.h>
#include "M2M_MiraOneMessage.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal defines
//
#define MIRA_CHAR_STC    0xE1
#define MIRA_CHAR_ESC   0xE2

#define MIRA_SERIAL_TIMEOUT			1000

#define MIRA_MESSAGE_RESPONSE_FLAG    0x80
#define MIRA_MESSAGE_ADDRESS_FLAG   0x40
#define MIRA_MESSAGE_CLASS_FLAGS    0x0f

#define MIRA_ADDRESSING_MODE_NONE         0xff
#define MIRA_ADDRESSING_MODE_ADDRESS      0x01
#define MIRA_ADDRESSING_MODE_TONETWORKROOT    0x02
#define MIRA_ADDRESSING_MODE_LINKLOCAL      0x04
#define MIRA_ADDRESSING_MODE_BROADCAST      0x08

#define MIRA_ADDRESS_TYPE_NOADDRESS       0x01
#define MIRA_ADDRESS_TYPE_EUI64         0x02

#define MIRA_ADDRESS_ADDRESS				0x12
#define MIRA_ADDRESS_NETWORK_ROOT 			0x21
#define MIRA_ADDRESS_LINKLOCAL				0x41
#define MIRA_ADDRESS_BROADCAST 				0x21

#define MIRA_MESSAGE_CLASS_DATAMESSAGE      0x03
#define MIRA_MESSAGE_CLASS_FWUPMESSAGE      0x04
#define MIRA_MESSAGE_CLASS_NETSTATMESSAGE     0x07
#define MIRA_MESSAGE_CLASS_SETTINGSMESSAGE    0x08

#define MIRA_MESSAGE_TYPE_ACK				0x01
#define MIRA_MESSAGE_TYPE_ERROR				0x02
#define MIRA_MESSAGE_TYPE_NETWORK_PONG		0x0a
#define MIRA_MESSAGE_TYPE_STATISTICS		0x04
#define MIRA_MESSAGE_TYPE_DATA_RECEIVED		0x04
#define MIRA_MESSAGE_TYPE_SLEEPY_DATA_RECEIVED	0x05

#define MESSAGE_GET_VERSION					0x01, 0x03

#define MESSAGE_DATA_SEND					0x03, 0x03
#define MESSAGE_SLEEPY_DATA_RECEIVED		0x03, 0x05
#define MESSAGE_DATA_MAIL					0x03, 0x06
#define MESSAGE_DATA_NET          			0x07, 0x03

#define MESSAGE_NETWORK_GET_STATISTICS		0x07, 0x03
#define MESSAGE_NETWORK_PING				0x07, 0x09

#define MESSAGE_SETTINGS_SET_CREDENTIALS	0x08, 0x03
#define MESSAGE_SETTINGS_BECOME_ROOT		0x08, 0x04
#define MESSAGE_SETTINGS_SET_ANTENNA		0x08, 0x05
#define MESSAGE_SETTINGS_SET_NAME			0x08, 0x09
#define MESSAGE_SETTINGS_COMMIT				0x08, 0x0a

#define MESSAGE_GET_EUI64INFO				0x01, 0x09

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Logging defines
//
#define MIRA_DEBUG

#ifdef MIRA_DEBUG
#define MOM_LOG_ERROR(...) if (logger != nullptr) logger->error(__VA_ARGS__)
#define MOM_LOG_INFO(...) if (logger != nullptr) logger->info(__VA_ARGS__)
#define MOM_LOG_DEBUG(...) if (logger != nullptr) logger->debug(__VA_ARGS__)
#define MOM_LOG_TRACE(...) if (logger != nullptr) logger->trace(__VA_ARGS__)
#define MOM_LOG_TRACE_START(...) if (logger != nullptr) logger->traceStart(__VA_ARGS__)
#define MOM_LOG_TRACE_PART(...) if (logger != nullptr) logger->tracePart(__VA_ARGS__)
#define MOM_LOG_TRACE_END(...) if (logger != nullptr) logger->traceEnd(__VA_ARGS__)
#else
#define MOM_LOG_ERROR(...)
#define MOM_LOG_INFO(...)
#define MOM_LOG_DEBUG(...)
#define MOM_LOG_TRACE(...)
#define MOM_LOG_TRACE_START(...)
#define MOM_LOG_TRACE_PART(...)
#define MOM_LOG_TRACE_END(...)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Struct definitions
//
struct CredentialsData
{
	uint16_t net_id;
	uint8_t aes_key;
};

enum class MiraAntenna: uint8_t
{
  internal = 0,
  external = 1
};

struct IEEE_EUI64 {
	uint8_t data[8];
};

struct VersionInfo
{
	uint8_t major;
	uint8_t minor;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class definitions
//
class MiraOneMessage
{
public:
	// Constructor/Destructor
	MiraOneMessage();
	MiraOneMessage(uint8_t a, uint8_t b);
	~MiraOneMessage();

	// Static message factories
	static MiraOneMessage* getDataSendMessageForNode(IEEE_EUI64 address, uint8_t* data, uint8_t size);
	static MiraOneMessage* getDataSendMessageForRoot(uint8_t* data, uint8_t size);		
	static MiraOneMessage* getDataSendMessageForBroadcast(uint8_t* data, uint8_t size);		
	static MiraOneMessage* getDataMailMessage();

	static MiraOneMessage* getNetworkGetStatisticsMessage(uint8_t interval);
	static MiraOneMessage* getNetworkPingMessage(IEEE_EUI64 address);

	static MiraOneMessage* getSetCredentialsMessage(const uint16_t networkId, const char* aesKey);

	static MiraOneMessage* getBecomeNetworkRootMessage();
	static MiraOneMessage* getSetAntennaMessage(MiraAntenna antenna);
	static MiraOneMessage* getSetNameMessage(const char* name);
	static MiraOneMessage* getCommitSettingsMessage();
	static MiraOneMessage* getGetVersionMessage();

	static MiraOneMessage* getGetEUI64InfoMessage();
	
	static MiraOneMessage* readFromNetwork();

	// Property getters
	bool isResponse();
	bool hasAddress();
	uint8_t getMessageClass();
	uint8_t getMessageType();
	uint8_t getAddressingMode();
	uint8_t getAddressType();
	uint8_t getMessageIndex();
	uint8_t getDataSize();
	uint8_t* getData();

	// Property setters
	void setData(const uint8_t* data, uint8_t length);
	void setMessageIndex(uint8_t index);

	// Message handling
	bool write(Stream* stream, uint8_t messageId, Logger* logger);
	bool read(Stream* stream, Logger* logger);
	void dumpToLog(Logger* logger);

	// CRC - Not used here
	static uint16_t crc16Kermit(char *data, uint16_t len);
	static uint16_t addToCrc(uint16_t& currentValue, uint8_t value);

	// Logging
	static void setLogger(Logger* logger);

private:
	uint8_t _messageHeader = 0;
	uint8_t _messageType = 0;
	uint8_t _messageIndex = 0;
	uint8_t _dataSize = 0;
	uint8_t* _address = nullptr;
	uint8_t* _data = nullptr;
	uint16_t _crc = 0;

	// Private functions
	void writeSTC(Stream* stream, Logger* logger);
	void writeEscapedData(Stream* stream, uint8_t data, uint16_t& crc, Logger* logger);
};

#endif