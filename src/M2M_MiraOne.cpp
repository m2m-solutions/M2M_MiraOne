
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
#include "Logger.h"

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
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Infrastructure
//
void MiraOne::begin(uint16_t networkId, const char* aesKey, const char* name, bool root)
{
	_networkId = networkId;
	_aesKey = aesKey;
	_name = name;
	getEUI64Info(&_address);
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
	LOG_TRACE_START(F("Flush "));
	while (_stream->available())
	{
		char ch = _stream->read();
		LOG_TRACE_PART("%c", ch);
	}
	LOG_TRACE_END("");
}

void MiraOne::reset()
{
	LOG_TRACE(F("Resetting Mira module"));
	digitalWrite(_resetPin, LOW);
	delay(200);
	digitalWrite(_resetPin, HIGH);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Management
//
bool MiraOne::setNetworkCredentials(const uint16_t networkId, const char* aesKey)
{
	LOG_TRACE(F("setNetworkCredentials()"));
	MiraOneMessage* message = MiraOneMessage::getSetCredentialsMessage(networkId, aesKey);
	if (!send(message))
	{
		delete message;
		LOG_ERROR(F("setNetworkCredentials: Send failure"));
		return false;
	}
	delete message;
	LOG_TRACE(F("Waiting for response"));
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		LOG_ERROR(F("setNetworkCredentials: Receive failure"));
		return false;
	}
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
		LOG_ERROR(F("becomeNetworkRoot: Send failure"));
		return false;
	}
	delete message;
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		LOG_ERROR(F("becomeNetworkRoot: Receive failure"));
		return false;
	}
	// Check status
	delete response;
	return true;
}

bool MiraOne::setName(const char* name)
{
	MiraOneMessage* message = MiraOneMessage::getSetNameMessage(name);
	if (!send(message))
	{
		delete message;
		LOG_ERROR(F("setName: Send failure"));
		return false;
	}
	delete message;
	LOG_TRACE(F("Waiting for response"));
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		LOG_ERROR(F("setName: Receive failure"));
		return false;
	}
	// Check status
	delete response;
	return true;
}

bool MiraOne::setAntenna(MiraAntenna antenna)
{
	MiraOneMessage* message = MiraOneMessage::getSetAntennaMessage(antenna);
	if (!send(message))
	{
		delete message;
		LOG_ERROR(F("commitSettings: Send failure"));
		return false;
	}
	delete message;
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		LOG_ERROR(F("commitSettings: Receive failure"));
		return false;
	}
	// Check status
	delete response;
	return true;
}

bool MiraOne::commitSettings()
{
	MiraOneMessage* message = MiraOneMessage::getCommitSettingsMessage();
	if (!send(message))
	{
		delete message;
		LOG_ERROR(F("commitSettings: Send failure"));
		return false;
	}
	delete message;
	MiraOneMessage* response = getNextMessage();
	if (response == nullptr)
	{
		delete response;
		LOG_ERROR(F("commitSettings: Receive failure"));
		return false;
	}
	// Check status
	delete response;
	return true;
}

bool MiraOne::getVersion(VersionInfo& version)
{
	MiraOneMessage* message = MiraOneMessage::getGetVersionMessage();
	if (!send(message))
	{
		delete message;
		LOG_ERROR("Send failure");
		return false;
	}
	delete message;
	LOG_TRACE("Waiting for response");

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}
	LOG_TRACE("Got response message");
	delete response;

	// This is the version message
	response = getNextMessage();
	if (!response)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}	
	LOG_TRACE("Got reply message");
	uint8_t* data = response->getData();
	version.major = *data++;
	version.minor = *data;
	delete response;
	return true;
}

bool MiraOne::getEUI64Info(IEEE_EUI64* buffer)
{
	MiraOneMessage* message = MiraOneMessage::getGetEUI64InfoMessage();
	if (!send(message))
	{
		delete message;
		LOG_ERROR("Send failure");
		return false;
	}
	delete message;
	LOG_TRACE("Waiting for response");

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}
	LOG_TRACE("Got response message");
	uint8_t* data = response->getData();
	memcpy(buffer, data, 8);
	delete response;

	// This is the version message
	response = getNextMessage();
	if (!response)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}	
	LOG_TRACE("Got reply message");
	data = response->getData();
	memcpy(buffer, data, 8);
	delete response;
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
		LOG_ERROR("Send failure");
		return false;
	}
	delete message;
	LOG_TRACE(F("Waiting for response"));

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}
	LOG_TRACE("Got response message");
	delete response;
	return true;
}

bool MiraOne::networkPing(IEEE_EUI64 address)
{
	// Network ping can only be sent to a specific node, so the UEI64 address is mandatory	
	MiraOneMessage* message = MiraOneMessage::getNetworkPingMessage(address);
	if (!send(message))
	{
		delete message;
		LOG_ERROR("Send failure");
		return false;
	}
	delete message;
	LOG_TRACE(F("Waiting for response"));

	// This is the ack message
	MiraOneMessage* response = getNextMessage();
	if (!response || response->getMessageType() != MIRA_MESSAGE_TYPE_ACK)
	{
		delete response;
		LOG_ERROR("Receive failure");
		return false;
	}
	LOG_TRACE("Got response message");
	delete response;
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
	message->dumpToLog();
	return message->write(_stream, getNextMessageId());
}

MiraOneMessage* MiraOne::getNextMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_SEND);
	if (result->read(_stream))
	{
		result->dumpToLog();
		return result;
	}
	delete result;
	return nullptr;
}

bool MiraOne::getNextMessage(MiraOneMessage* result)
{
	if (result->read(_stream))
	{
#ifdef MIRA_DEBUG		
		result->dumpToLog();
#endif		
		return true;
	}
	return false;
}






