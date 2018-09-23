
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
#include "M2M_MiraOneMessage.h"
#include "M2M_MiraOne.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constructor/Destructor
//
MiraOneMessage::MiraOneMessage()
{
}

MiraOneMessage::MiraOneMessage(uint8_t header, uint8_t type)
{
	_messageHeader = header;
	_messageType = type;
	_address = nullptr;
	_data = nullptr;
	_crc = 0;	
}

MiraOneMessage::~MiraOneMessage()
{
	if (_address)
	{
		delete _address;
	}
	if (_data)
	{
		delete _data;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static message factories
//
MiraOneMessage* MiraOneMessage::getDataSendMessageForRoot(uint8_t* data, uint8_t size)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_SEND);
	result->_messageHeader |= MIRA_MESSAGE_ADDRESS_FLAG;
	result->_address = new uint8_t(MIRA_ADDRESS_NETWORK_ROOT);
	result->setData(data, size);
	return result;
}

MiraOneMessage* MiraOneMessage::getDataSendMessageForNode(IEEE_EUI64 address, uint8_t* data, uint8_t size)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_SEND);
	result->_messageHeader |= MIRA_MESSAGE_ADDRESS_FLAG;
	uint8_t* buffer = new uint8_t[9];
	buffer[0] = MIRA_ADDRESSING_MODE_ADDRESS << 4 | MIRA_ADDRESS_TYPE_EUI64;
	memcpy(&buffer[1], &address, 8);
	result->_address = buffer;
	result->setData(data, size);
	return result;
}

MiraOneMessage* MiraOneMessage::getDataSendMessageForBroadcast(uint8_t* data, uint8_t size)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_SEND);
	result->_messageHeader |= MIRA_MESSAGE_ADDRESS_FLAG;
	result->_address = new uint8_t(MIRA_ADDRESS_BROADCAST);
	result->setData(data, size);
	return result;	
}

MiraOneMessage* MiraOneMessage::getDataMailMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_MAIL);

	return result;
}

MiraOneMessage* MiraOneMessage::getNetworkGetStatisticsMessage(uint8_t interval)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_DATA_NET);
	//result->_messageHeader |= MIRA_MESSAGE_ADDRESS_FLAG;
	//result->_address = new uint8_t(MIRA_ADDRESS_BROADCAST);
	result->setData(new uint8_t(interval), 1);
	return result;
}

MiraOneMessage* MiraOneMessage::getNetworkPingMessage(IEEE_EUI64 address)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_NETWORK_PING);
	result->_messageHeader |= MIRA_MESSAGE_ADDRESS_FLAG;
	uint8_t* buffer = new uint8_t[9];
	*buffer = MIRA_ADDRESSING_MODE_ADDRESS << 4 | MIRA_ADDRESS_TYPE_EUI64;
	memcpy(buffer + 1, &address, 8);
	result->_address = buffer;
	buffer = new uint8_t(32);
	memset(buffer, 0xa5, 32);
	result->setData(buffer, 33);
	delete buffer;
	return result;
}

MiraOneMessage* MiraOneMessage::getSetCredentialsMessage(const uint16_t networkId, const char* aesKey)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_SETTINGS_SET_CREDENTIALS);
	uint8_t* buffer = new uint8_t[18];
	memcpy(buffer, &networkId, 2);
	memcpy(buffer + 2, aesKey, 16);
	result->setData(buffer, 18);
	return result;
}

MiraOneMessage* MiraOneMessage::getBecomeNetworkRootMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_SETTINGS_BECOME_ROOT);
	return result;
}

MiraOneMessage* MiraOneMessage::getSetAntennaMessage(MiraAntenna antenna)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_SETTINGS_SET_ANTENNA);
	result->setData(new uint8_t((uint8_t)antenna), 1);
	return result;
}

MiraOneMessage* MiraOneMessage::getSetNameMessage(const char* name)
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_SETTINGS_SET_NAME);
	uint8_t length = strlen(name);
	char* buffer = new char[length + 1];
	strcpy(buffer, name);
	result->setData((uint8_t*)buffer, length);
	return result;
}

MiraOneMessage* MiraOneMessage::getCommitSettingsMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_SETTINGS_COMMIT);
	return result;
}

MiraOneMessage* MiraOneMessage::getGetVersionMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_GET_VERSION);
	return result;
}

MiraOneMessage* MiraOneMessage::getGetEUI64InfoMessage()
{
	MiraOneMessage* result = new MiraOneMessage(MESSAGE_GET_EUI64INFO);
	return result;
}

MiraOneMessage* MiraOneMessage::readFromNetwork()
{
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property getters
//
bool MiraOneMessage::isResponse()
{
	return (_messageHeader & MIRA_MESSAGE_RESPONSE_FLAG) == MIRA_MESSAGE_RESPONSE_FLAG;
}

bool MiraOneMessage::hasAddress()
{
	return (_messageHeader & MIRA_MESSAGE_ADDRESS_FLAG) == MIRA_MESSAGE_ADDRESS_FLAG;
}

uint8_t MiraOneMessage::getMessageClass()
{
	return static_cast<uint8_t>(_messageHeader & 0b00001111);
}

uint8_t MiraOneMessage::getMessageType()
{
	return _messageType;
}

uint8_t MiraOneMessage::getAddressingMode()
{
	if (!hasAddress())
	{
		return MIRA_ADDRESSING_MODE_NONE;
	}
	return static_cast<uint8_t>(_address[0] >> 4);
}

uint8_t MiraOneMessage::getAddressType()
{
	return static_cast<uint8_t>(_address[0] & 0b00001111);
}

uint8_t MiraOneMessage::getMessageIndex()
{
	return static_cast<uint8_t>(_messageIndex);
}

uint8_t MiraOneMessage::getDataSize()
{
	return static_cast<uint8_t>(_dataSize);
}

uint8_t* MiraOneMessage::getData()
{
	return _data;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property setters
//
void MiraOneMessage::setData(const uint8_t* data, uint8_t size)
{
	uint8_t* buffer = new uint8_t[size];
	for (int i = 0; i < size; i++)
	{
		buffer[i] = data[i];
	}
	//memcpy(&buffer, data, size);
	_data = buffer;	
	_dataSize = size;
}

void MiraOneMessage::setMessageIndex(uint8_t index)
{
	_messageIndex = index;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Message handling
//
bool MiraOneMessage::write(Stream* stream, uint8_t messageIndex, Logger* logger)
{
	MOM_LOG_TRACE_START(F("Write message: "));
	uint16_t temp = 0;
	_messageIndex = messageIndex;
	_crc = 0;
	writeSTC(stream, logger);
	writeEscapedData(stream, _messageHeader, _crc, logger);
	writeEscapedData(stream, _messageType, _crc, logger);
	writeEscapedData(stream, _messageIndex, _crc, logger);
	writeEscapedData(stream, _dataSize, _crc, logger);
	if (hasAddress())
	{
		writeEscapedData(stream, _address[0], _crc, logger);
		if (getAddressType() == MIRA_ADDRESS_TYPE_EUI64)
		{
			for (int i = 1; i < 9; i++)
			{
				writeEscapedData(stream, _address[i], _crc, logger);
			}
		}
	}
	for (int i = 0; i < _dataSize; i++)
	{
		writeEscapedData(stream, _data[i], _crc, logger);
	}
	writeEscapedData(stream, static_cast<uint8_t>(_crc >> 8), temp, logger);
	writeEscapedData(stream, static_cast<uint8_t>(_crc & 0xff), temp, logger);
	stream->flush();
	MOM_LOG_TRACE_END(F("[end]"));
	return true;
}

void MiraOneMessage::writeSTC(Stream* stream, Logger* logger)
{
	MOM_LOG_TRACE_PART(F("0x%02x "), (uint8_t)MIRA_CHAR_STC);
	stream->print((char)MIRA_CHAR_STC);
}

void MiraOneMessage::writeEscapedData(Stream* stream, uint8_t data, uint16_t& crc, Logger* logger)
{
	addToCrc(crc, data);
	switch (data)
	{
		case MIRA_CHAR_STC:
		case MIRA_CHAR_ESC:
			MOM_LOG_TRACE_PART(F("0x%02x 0x0%2x"), (char)MIRA_CHAR_ESC, (char)~data);
			stream->print((char)MIRA_CHAR_ESC);
			stream->print((char)~data);
			break;
		default:
			MOM_LOG_TRACE_PART(F("0x%02x "), (uint8_t)data);
			stream->print((char)data);
			break;
	}
}

bool MiraOneMessage::read(Stream* stream, Logger* logger)
{

	int16_t index = -1;
	int ch;
	bool inAddress = false;
	uint8_t dataCount = 0;
	uint8_t crcCount = 0;
	uint16_t crc = 0;
	uint32_t timeout = millis();

	MOM_LOG_TRACE_START(F("Reading message: "));
	while (true)
	{
		while (!stream->available())
		{
			//LOG_TRACE_PART(F("."));
			if (millis() - timeout > MIRA_SERIAL_TIMEOUT)
			{
				MOM_LOG_TRACE_END("");
				MOM_LOG_ERROR(F("Timout waiting for data"));
				return false;
			}
			delay(2);
		}
		timeout = millis();
		ch = stream->read();
		if (ch == -1)
		{
			MOM_LOG_TRACE_END(F(" - Read failed"));
			return NULL;
		}
		if (ch == MIRA_CHAR_ESC)		// Unescape
		{
			ch = stream->read();
			if (ch == -1)
			{
				MOM_LOG_TRACE_END(F(" - Read failed"));
				// Failed to read pending data, just return
				return NULL;
			}
			ch = ~ch;
		}
		if (index == -1)		
		{
			if (ch != MIRA_CHAR_STC)
			{
				continue;
			}			
			else
			{
				index = 0;
				continue;
			}
		}
		index++;
		switch (index)
		{
			case 1:
				_messageHeader = ch;
				addToCrc(crc, ch);
				continue;
			case 2:
				_messageType = ch;
				addToCrc(crc, ch);
				continue;
			case 3:
				_messageIndex = ch;
				addToCrc(crc, ch);
				continue;
			case 4:
				_dataSize = static_cast<uint8_t>(ch);
				dataCount = 0;
				addToCrc(crc, ch);
				inAddress = hasAddress();
				continue;
		}
		if (inAddress)
		{
			addToCrc(crc, ch);
			switch (index)
			{
				// Index 7-15 is the EUI64 address
				case 15:
					inAddress = false;
					// Intentional fall through
				default:
					_address[index - 6] = ch;
			}				
			continue;
		}
		if (dataCount < _dataSize)
		{
			if (dataCount == 0)
			{
				_data = new uint8_t[_dataSize];
			}
			_data[dataCount] = ch;
			addToCrc(crc, ch);
			dataCount++;			
			continue;
		}

		switch (crcCount)
		{
			case 0: 
				_crc = static_cast<uint16_t>(ch << 8);
				crcCount++;
				break;
			case 1:
				_crc = _crc | static_cast<uint16_t>(ch % 0xff);
				crcCount++;
				if (_crc == crc)
				{
					MOM_LOG_TRACE_END(F("(CRC OK)"));
				}
				else
				{
					MOM_LOG_TRACE_END(F("(CRC FAIL 0x%04x, 0x%04x)"), _crc, crc);
				}
				return _crc == crc;
		}
	}
	return false;
}

void MiraOneMessage::dumpToLog(Logger* logger)
{
	if (logger->getLogLevel() != LogLevel::Trace )
	{
		return;
	}
	MOM_LOG_TRACE(F("======== Mira message ========="));
	MOM_LOG_TRACE_START(F("Message header  : 0x%02x (Response: "), _messageHeader);
	if (isResponse())
	{
		MOM_LOG_TRACE_PART(F("True"));
	}
	else
	{
		MOM_LOG_TRACE_PART(F("False"));
	}
	MOM_LOG_TRACE_PART(F(", Address flag: "));
	if (hasAddress())
	{
		MOM_LOG_TRACE_PART(F("True"));
	}
	else
	{
		MOM_LOG_TRACE_PART(F("False"));
	}
	MOM_LOG_TRACE_PART(F(", Message class: "));
	switch (getMessageClass())
	{
		case 0x01:
			MOM_LOG_TRACE_END(F("ACK)"));
			break;
		case 0x02:
			MOM_LOG_TRACE_END(F("ERROR)"));
			break;
		case 0x03:
			MOM_LOG_TRACE_END(F("DATA_MESSAGES)"));
			break;
		case 0x04:
			MOM_LOG_TRACE_END(F("FWUP_MESSAGES)"));
			break;
		case 0x07:
			MOM_LOG_TRACE_END(F("NETSTAT_MESSAGES)"));
			break;
		case 0x08:
			MOM_LOG_TRACE_END(F("SETTINGS_MESSAGES)"));
			break;
		default:
			MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
	}
	MOM_LOG_TRACE_START(F("Message type    : 0x%02x ("), _messageType);
	switch (getMessageClass())
	{
		case 0x03:	// DATA_MESSAGES
			switch(_messageType)
			{
				case 0x01:
					MOM_LOG_TRACE_END(F("ACK)"));
					break;
				case 0x02:
					MOM_LOG_TRACE_END(F("ERROR)"));
					break;			
				case 0x03:
					MOM_LOG_TRACE_END(F("DATA_SEND)"));
					break;
				case 0x04:
					MOM_LOG_TRACE_END(F("DATA_RECEIVED)"));
					break;
				case 0x05:
					MOM_LOG_TRACE_END(F("SLEEPY_DATA_RECEIVED)"));
					break;
				case 0x07:
					MOM_LOG_TRACE_END(F("SLEEPY_DATA_MAIL)"));
					break;
				default:
					MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
					break;				
			}
			break;
		case 0x04:	// FWUP_MESSAGES
			switch(_messageType)
			{
				case 0x01:
					MOM_LOG_TRACE_END(F("ACK)"));
					break;
				case 0x02:
					MOM_LOG_TRACE_END(F("ERROR)"));
					break;						
				case 0x03:
					MOM_LOG_TRACE_END(F("FWUP_OPEN_SESSION)"));
					break;
				case 0x04:
					MOM_LOG_TRACE_END(F("FWUP_CLOSE_SESSION)"));
					break;
				case 0x05:
					MOM_LOG_TRACE_END(F("FWUP_SUBSCRIBE)"));
					break;
				case 0x06:
					MOM_LOG_TRACE_END(F("FWUP_STATUS_REQUEST)"));
					break;
				case 0x07:
					MOM_LOG_TRACE_END(F("FWUP_STATUS)"));
					break;
				case 0x08:
					MOM_LOG_TRACE_END(F("FWUP_DATA_REQUEST)"));
					break;
				case 0x09:
					MOM_LOG_TRACE_END(F("FWUP_DATA)"));
					break;
				case 0x0D:
					MOM_LOG_TRACE_END(F("FWUP_ROLLBACK_REQUEST)"));
					break;
				default:
					MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
					break;				
			}
			break;
		case 0x07:	// NETSTAT_MESSAGES
			switch(_messageType)
			{
				case 0x01:
					MOM_LOG_TRACE_END(F("ACK)"));
					break;
				case 0x02:
					MOM_LOG_TRACE_END(F("ERROR)"));
					break;						
				case 0x03:
					MOM_LOG_TRACE_END(F("NETWORK_GET_STATISTICS)"));
					break;
				case 0x04:
					MOM_LOG_TRACE_END(F("NETWORK_STATISTICS)"));
					break;
				case 0x09:
					MOM_LOG_TRACE_END(F("NETWORK_PING)"));
					break;
				case 0x0A:
					MOM_LOG_TRACE_END(F("NETWORK_PONG)"));
					break;
				default:
					MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
					break;
			}		
			break;
		case 0x08:
			switch(_messageType)
			{
				case 0x01:
					MOM_LOG_TRACE_END(F("ACK)"));
					break;
				case 0x02:
					MOM_LOG_TRACE_END(F("ERROR)"));
					break;						
				case 0x03:
					MOM_LOG_TRACE_END(F("SETTINGS_SET_CREDENTIALS)"));
					break;
				case 0x04:
					MOM_LOG_TRACE_END(F("SETTINGS_BECOME_ROOT)"));
					break;
				case 0x05:
					MOM_LOG_TRACE_END(F("SETTINGS_SET_ANTENNA)"));
					break;
				case 0x0A:
					MOM_LOG_TRACE_END(F("SETTINGS_COMMIT)"));
					break;
				default:
					MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
					break;				
			}
			break;
		default:
			MOM_LOG_TRACE_END(F("Unknown: 0x%02x)"), _messageType);
			break;
	}
	
	MOM_LOG_TRACE(F("Message index   : 0x%02x"), _messageIndex);
	MOM_LOG_TRACE(F("Data size       : %u bytes"), _dataSize);
	if (hasAddress())
	{
		MOM_LOG_TRACE_START(F("Addressing mode : 0x%02x "), getAddressingMode());
		MOM_LOG_TRACE_END(F(", Address type: 0x%02x"), getAddressType());
		if (getAddressType() == MIRA_ADDRESS_TYPE_EUI64)
		{
			MOM_LOG_TRACE_START(F("Address         : "));
			for (int i = 1; i < 9; i++)
			{
				MOM_LOG_TRACE_PART(F("%02x"), _address[i]);
			}
			MOM_LOG_TRACE_END("");
		}
	}
	if (_dataSize > 0)
	{
		MOM_LOG_TRACE_START(F("Data            : "));
		for (int i = 0; i < getDataSize(); i++)
		{
			MOM_LOG_TRACE_PART(F("0x%02x "), (uint8_t)_data[i]);
		}
		MOM_LOG_TRACE_END("");
	}
	/*
	if (getMessageClass() == 0x07 && _messageType == 0x04)
	{
		Log.trace(F("  Network statistics: "));
		Log.traceStart(F("  node_eui          : "));
		for (int i = 0; i < 8; i++)
		{
			Log.tracePart(F("%02x "), (uint8_t)_data[i]);
		}
		Log.traceEnd("");
		Log.traceStart(F("  parent_eui        : "));
		for (int i = 8; i < 16; i++)
		{
			Log.tracePart(F("%02x "), (uint8_t)_data[i]);
		}
		Log.traceEnd("");
		Log.traceStart(F("  os_version_major  : "));
		Log.traceEnd(F("0x%02x "), (uint8_t)_data[16]);
		Log.traceStart(F("  os_version_minor  : "));
		Log.traceEnd(F("0x%02x "), (uint8_t)_data[17]);
		Log.traceStart(F("  link_quality      : "));
		Log.traceEnd(F("0x%02x "), (uint8_t)_data[18]);
		Log.traceStart(F("  channel_error_rate : "));
		for (int i = 19; i < 51; i = i +2)
		{
			Log.tracePart(F("0x%0002x "), (uint16_t)_data[i]);
		}
		Log.traceEnd("");	
	}
	*/
	MOM_LOG_TRACE(F("==============================="));
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Message handling
//
uint16_t MiraOneMessage::crc16Kermit(char *data, uint16_t len)
{
	unsigned int i;
	unsigned char carry;
	uint16_t poly = 0x8408;	//reversed 0x1021
	uint16_t crc = 0;

	if (len == 0)
	{
		return (~crc);
	}

	for (i = 0; i < len; i++)
	{
		crc ^= data[i];
		for (int i = 0; i < 8; i++)
		{
			carry = crc & 1;
			crc >>= 1;
			if (carry)
			{
				crc ^= poly;
			}
		}
	}
	return crc;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Private functions
//
uint16_t MiraOneMessage::addToCrc(uint16_t& currentValue, uint8_t value)
{
	const uint16_t poly = 0x8408;	//reversed 0x1021
	currentValue ^= value;
	for (int i = 0; i < 8; i++)
	{
		bool carry = currentValue & 1;
		currentValue >>= 1;
		if (carry)
		{
			currentValue ^= poly;
		}
	}
	return currentValue;
}
