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
#ifndef __M2M_MIRAONE_h__
#define __M2M_MIRAONE_h__

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "Arduino.h"
#include <Stream.h>
#include "M2M_MiraOneMessage.h"

#define MIRA_NETWORK_ID   42
#define MIRA_AES_KEY   "o#VDMJhtp0N2ZY&s"

////////////////////////////////////////////////////////////////////////////////////////////////
//
// External defines
//
#ifdef MIRA_DEBUG
#define LOG_ERROR(...) Log.error(__VA_ARGS__)
#define LOG_INFO(...) Log.info(__VA_ARGS__)
#define LOG_DEBUG(...) Log.debug(__VA_ARGS__)
#define LOG_TRACE(...) Log.trace(__VA_ARGS__)
#define LOG_TRACE_START(...) Log.traceStart(__VA_ARGS__)
#define LOG_TRACE_PART(...) Log.tracePart(__VA_ARGS__)
#define LOG_TRACE_END(...) Log.traceEnd(__VA_ARGS__)
#else
#define LOG_ERROR(...)
#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_TRACE(...)
#define LOG_TRACE_START(...)
#define LOG_TRACE_PART(...)
#define LOG_TRACE_END(...)
#endif

#ifndef NOT_A_PIN
#define NOT_A_PIN	0
#endif 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal defines
//
#define MIRA_BUFFER_SIZE	128 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class definitions
//
class MiraOne
{
public:
	// Constructor
	MiraOne(Stream& port, uint8_t resetPin = NOT_A_PIN, MiraAntenna antenna = MiraAntenna::internal);

	// Infrastructure
	void begin(uint16_t networkId, const char* aesKey, const char* name, bool root);
	void update();
	uint8_t getNextMessageId();
	IEEE_EUI64 getAddress();
	void flush();
	void reset();

	// Management
	bool setNetworkCredentials(const uint16_t networkId, const char* aesKey);
	bool becomeNetworkRoot();
	bool setName(const char* name);
	bool setAntenna(MiraAntenna antenna);
	bool commitSettings();
	bool getVersion(VersionInfo& version);
	bool getEUI64Info(IEEE_EUI64* buffer);

	// Network statistics
	bool getNetworkStatistics(uint8_t interval);
	bool networkPing(IEEE_EUI64 address);

	// Messaging
	bool available();
	bool send(MiraOneMessage* message);
	MiraOneMessage* getNextMessage();
	bool getNextMessage(MiraOneMessage* result);

protected:
	Stream* _stream;
	uint8_t _messageBuffer[255];
	uint16_t _networkId;
	const char* _aesKey;
	const char* _name;
	IEEE_EUI64 _address;
	bool _coordinator;
	MiraAntenna _antenna;
	uint8_t _resetPin;
	uint8_t _currentMessageId;	
};

#endif