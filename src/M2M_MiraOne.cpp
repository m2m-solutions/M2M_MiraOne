
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
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "M2M_MiraOne.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
MiraOne::MiraOne(Stream& stream, uint8_t resetPin, MiraAntenna antenna)
{
	_stream = &stream;
	_resetPin = resetPin;
	_antenna = antenna;
	if (resetPin != NOT_A_PIN)
	{
		pinMode(resetPin, OUTPUT);
		digitalWrite(resetPin, HIGH);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Infrastructure
//
void MiraOne::begin(bool root, const char* name, uint16_t networkId, const char* aesKey)
{
	_networkId = networkId;
	_aesKey = aesKey;
	_name = name;
	setNetworkCredentials(networkId, aesKey);
	setAntenna(_antenna);
	if (root)
	{
		becomeNetworkRoot();
	}
	commitSettings();
}

void MiraOne::update()
{
	// Nothing for now
}

uint8_t MiraOne::getNextMessageId()
{
	return _currentMessageId++;
}

void MiraOne::flush()
{
	MO_LOG_TRACE_START(F("Flush "));
	while (_stream->available())
	{
		char ch = _stream->read();
		MO_LOG_TRACE_PART("%c", ch);
	}
	MO_LOG_TRACE_END("");
	callWatchdog();
}

void MiraOne::reset()
{
	MO_LOG_TRACE(F("Resetting Mira module"));
	digitalWrite(_resetPin, LOW);
	delay(200);
	digitalWrite(_resetPin, HIGH);
	delay(500);
	callWatchdog();
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Logging
//
void MiraOne::setLogger(Logger* logger)
{
	_logger = logger;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Management
//
bool MiraOne::setNetworkCredentials(const uint16_t networkId, const char* aesKey)
{
	MO_LOG_TRACE("Setting network credentials");
	MiraOneMessage* message = MiraOneMessage::getSetCredentialsMessage(networkId, aesKey);
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR(F("setNetworkCredentials: Send failure"));
		callWatchdog();
		return false;
	}
	callWatchdog();
	delete message;
	MO_LOG_TRACE(F("Waiting for response"));
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		MO_LOG_ERROR(F("setNetworkCredentials: Receive failure"));
		callWatchdog();
		return false;
	}
	callWatchdog();
	// Check result
	delete response;
	return true;
}

bool MiraOne::becomeNetworkRoot()
{
	MiraOneMessage* message = MiraOneMessage::getBecomeNetworkRootMessage();
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR(F("becomeNetworkRoot: Send failure"));
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		MO_LOG_ERROR(F("becomeNetworkRoot: Receive failure"));
		callWatchdog();
		return false;
	}
	// Check status
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::setName(const char* name)
{
	MiraOneMessage* message = MiraOneMessage::getSetNameMessage(name);
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR(F("setName: Send failure"));
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MO_LOG_TRACE(F("Waiting for response"));
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		MO_LOG_ERROR(F("setName: Receive failure"));
		callWatchdog();
		return false;
	}
	// Check status
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::setAntenna(MiraAntenna antenna)
{
	MiraOneMessage* message = MiraOneMessage::getSetAntennaMessage(antenna);
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR(F("commitSettings: Send failure"));
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		MO_LOG_ERROR(F("commitSettings: Receive failure"));
		callWatchdog();
		return false;
	}
	// Check status
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::commitSettings()
{
	MiraOneMessage* message = MiraOneMessage::getCommitSettingsMessage();
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR(F("commitSettings: Send failure"));
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		MO_LOG_ERROR(F("commitSettings: Receive failure"));
		callWatchdog();
		return false;
	}
	// Check status
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::getVersion(VersionInfo& version)
{
	MiraOneMessage* message = MiraOneMessage::getGetVersionMessage();
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR("Send failure");
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MO_LOG_TRACE("Waiting for response");

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}
	MO_LOG_TRACE("Got response message");
	delete response;
	callWatchdog();

	// This is the version message
	response = getNextMessage();
	if (!response)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}	
	MO_LOG_TRACE("Got reply message");
	uint8_t* data = response->getData();
	version.major = *data++;
	version.minor = *data;
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::getEUI64Info(IEEE_EUI64* buffer)
{
	MiraOneMessage* message = MiraOneMessage::getGetEUI64InfoMessage();
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR("Send failure");
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();
	MO_LOG_TRACE("Waiting for response");

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}
	MO_LOG_TRACE("Got response message");
	uint8_t* data = response->getData();
	memcpy(buffer, data, 8);
	delete response;
	callWatchdog();

	// This is the version message
	response = getNextMessage();
	if (!response)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}	
	MO_LOG_TRACE("Got reply message");
	data = response->getData();
	memcpy(buffer, data, 8);
	delete response;
	callWatchdog();
	return true;	
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Network statistics
//

bool MiraOne::getNetworkStatistics(uint8_t interval)
{
	MiraOneMessage* message = MiraOneMessage::getNetworkGetStatisticsMessage(interval);	
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR("Send failure");
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();

	MO_LOG_TRACE(F("Waiting for response"));
	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}
	MO_LOG_TRACE("Got response message");
	delete response;
	callWatchdog();
	return true;
}

bool MiraOne::networkPing(IEEE_EUI64 address)
{
	// Network ping can only be sent to a specific node, so the UEI64 address is mandatory	
	MiraOneMessage* message = MiraOneMessage::getNetworkPingMessage(address);
	if (!send(message))
	{
		delete message;
		MO_LOG_ERROR("Send failure");
		callWatchdog();
		return false;
	}
	delete message;
	callWatchdog();

	MO_LOG_TRACE(F("Waiting for response"));
	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response || response->getMessageType() != MIRA_MESSAGE_TYPE_ACK)
	{
		delete response;
		MO_LOG_ERROR("Receive failure");
		callWatchdog();
		return false;
	}
	MO_LOG_TRACE("Got response message");
	delete response;
	callWatchdog();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Messaging
//
bool MiraOne::available()
{
	return _stream->available();
}

bool MiraOne::send(MiraOneMessage* message)
{
	message->setMessageIndex(_currentMessageId++);
	message->dumpToLog(_logger);
	return message->write(_stream, getNextMessageId(), _logger);
	callWatchdog();
}

MiraOneMessage* MiraOne::getNextMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_SEND);
	if (result->read(_stream, _logger))
	{
		result->dumpToLog(_logger);
		callWatchdog();
		return result;
	}
	delete result;
	callWatchdog();
	return nullptr;
}

bool MiraOne::getNextMessage(MiraOneMessage* result)
{
	if (result->read(_stream, _logger))
	{
		callWatchdog();
#ifdef MIRA_DEBUG		
		result->dumpToLog(_logger);
#endif		
		callWatchdog();
		return true;
	}
	callWatchdog();
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Watchdog
//
void MiraOne::callWatchdog()
{
    if (watchdogcallback != nullptr)
    {
        (watchdogcallback)();
    }
}

void MiraOne::setWatchdogCallback(WATCHDOG_CALLBACK_SIGNATURE)
{
    this->watchdogcallback = watchdogcallback;
}




