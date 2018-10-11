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
#include <Arduino.h>
#include <Stream.h>
#include <M2M_Logger.h>
#include "M2M_MiraOneMessage.h"

#define M2M_MIRA_NETWORK_ID   42
#define M2M_MIRA_AES_KEY   "o#VDMJhtp0N2ZY&s"

////////////////////////////////////////////////////////////////////////////////////////////////
//
// External defines
//
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
// Logging defines
//
#define MIRA_DEBUG

#ifdef MIRA_DEBUG
#define MO_LOG_ERROR(...) if (_logger != nullptr) _logger->error(__VA_ARGS__)
#define MO_LOG_INFO(...) if (_logger != nullptr) _logger->info(__VA_ARGS__)
#define MO_LOG_DEBUG(...) if (_logger != nullptr) _logger->debug(__VA_ARGS__)
#define MO_LOG_TRACE(...) if (_logger != nullptr) _logger->trace(__VA_ARGS__)
#define MO_LOG_TRACE_START(...) if (_logger != nullptr) _logger->traceStart(__VA_ARGS__)
#define MO_LOG_TRACE_PART(...) if (_logger != nullptr) _logger->tracePart(__VA_ARGS__)
#define MO_LOG_TRACE_END(...) if (_logger != nullptr) _logger->traceEnd(__VA_ARGS__)
#else
#define MO_LOG_ERROR(...)
#define MO_LOG_INFO(...)
#define MO_LOG_DEBUG(...)
#define MO_LOG_TRACE(...)
#define MO_LOG_TRACE_START(...)
#define MO_LOG_TRACE_PART(...)
#define MO_LOG_TRACE_END(...)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class definitions
//
#define WATCHDOG_CALLBACK_SIGNATURE void (*watchdogcallback)()

class MiraOne
{
public:
	// Constructor
	MiraOne(Stream& port, uint8_t resetPin = NOT_A_PIN, MiraAntenna antenna = MiraAntenna::internal);

	// Infrastructure
	void begin(bool root, const char* name, uint16_t networkId = M2M_MIRA_NETWORK_ID, const char* aesKey = M2M_MIRA_AES_KEY);
	void update();
	uint8_t getNextMessageId();
	IEEE_EUI64 getAddress();
	void flush();
	void reset();

	// Logging
	void setLogger(Logger* logger);

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

	// Watchdog
	void setWatchdogCallback(WATCHDOG_CALLBACK_SIGNATURE);

protected:
    void callWatchdog();

	Logger* _logger = nullptr;
	Stream* _stream;
	uint8_t _messageBuffer[255];
	uint16_t _networkId;
	const char* _aesKey;
	const char* _name;
	bool _coordinator;
	MiraAntenna _antenna;
	uint8_t _resetPin;
	uint8_t _currentMessageId;
	WATCHDOG_CALLBACK_SIGNATURE;
};

#endif