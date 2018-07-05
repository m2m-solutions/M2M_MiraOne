#ifndef __LOGGER__h
#define __LOGGER__h

// Based on code from https://github.com/mrRobot62/Arduino-logging-library

//#include <inttypes.h>
#include <stdarg.h>
//#if defined(ARDUINO)
    #include "Arduino.h"    
//#endif
#if defined(ARDUINO_ARCH_AVR)
#include "SoftwareSerial.h"
#endif
#if defined(ARDUINO_ARCH_SAMD)
#define vsnprintf_P vsnprintf
#endif

// TODO: Add support for Flash strings

enum class LogLevel: uint8_t
{
    NoOutput,
    Error,
    Info,
    Debug,
    Trace
};

class Logger
{
public:
#if defined(ARDUINO_ARCH_SAMD)
    void begin(Uart* stream, uint32_t baudrate, LogLevel logLevel);
#endif
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_AVR_LEONARDO)
	void begin(Serial_* serial, uint32_t baudrate, LogLevel logLevel);
#endif
#if defined(ARDUINO_ARCH_AVR)
	void begin(HardwareSerial* stream, uint32_t baudrate, LogLevel logLevel);
	void begin(SoftwareSerial* serial, uint32_t baudrate, LogLevel logLevel);
#endif
    void setLogLevel(LogLevel logLevel);
	void setIncludeTimestamp(bool value);
	void setIncludeLogLevel(bool value);
	void error(const __FlashStringHelper* message, ...);
    void error(const char* message, ...);
	void info(const __FlashStringHelper* message, ...);
    void info(const char* message, ...);
	void debug(const __FlashStringHelper* message, ...);
	void debug(const char* message, ...);
	void trace(const __FlashStringHelper * message, ...);
	void trace(const char* message, ...);
	void traceStart(const __FlashStringHelper* message, ...);
	void traceStart(const char*, ...);
	void tracePart(const __FlashStringHelper* message, ...);
	void tracePart(const char*, ...);
	void traceEnd(const __FlashStringHelper* message, ...);
	void traceEnd(const char*, ...);

private:
	LogLevel _logLevel = LogLevel::Info;
	Print* _print = nullptr;
	bool _includeTimestamp = false;
	bool _includeLogLevel = false;

    void log(LogLevel logLevel, bool isPart, bool writeLinefeed, const char* message, va_list args);
	void log(LogLevel logLevel, bool isPart, bool writeLinefeed, const __FlashStringHelper* message, va_list args);
	void logPrefix(LogLevel logLevel, bool isPart);
	bool isActive();
};

extern Logger Log;
#endif
