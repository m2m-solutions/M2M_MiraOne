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
// This file defines payloads that can be used to send data in a MiraOne mesh network.
//
// MiraOnePayloadv1 contains a field that should be set to the address of the sending node, 
// this can be used by the receiving node to send back a reply. 
// This is needed as the MiraOne UART API does not report the address in messages received by
// normal nodes. It does however report the address for sleepy nodes.
//
// MiraOnePayloadv2 can be used where the address of the sending node doesn't matter.
//
// 
//---------------------------------------------------------------------------------------------
#ifndef __M2M_MIRAONEPAYLOAD_h__
#define __M2M_MIRAONEPAYLOAD_h__

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include <Arduino.h>
#include "M2M_MiraOneMessage.h"
////////////////////////////////////////////////////////////////////////////////////////////////
//
// Struct definitions
//


// ---------------------------------------------------------------------------------------------
// Version 1
// This payload contains the MiraOne node miraAddress
struct __attribute__((packed)) MiraOnePayloadv1
{
	uint8_t payloadVersion = 1;
	IEEE_EUI64 miraAddress;
	char serialNumber[12];
	uint8_t dataLength;
	uint8_t data[];
	uint8_t getLength() 
	{ 
		return sizeof(MiraOnePayloadv1) + dataLength; 
	}
};

// ---------------------------------------------------------------------------------------------
// Version 2
// This payload omits the MiraOne node miraAddress
struct __attribute__((packed)) MiraOnePayloadv2
{
	uint8_t payloadVersion = 2;
	char serialNumber[12];
	uint8_t dataLength;
	uint8_t data[];
	uint8_t getLength() 
	{ 
		return sizeof(MiraOnePayloadv2) + dataLength; 
	}
};

#endif
