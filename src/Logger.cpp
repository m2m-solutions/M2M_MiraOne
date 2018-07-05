#include "Logger.h"

#if defined(ARDUINO_ARCH_SAMD)
void Logger::begin(Uart* stream, uint32_t baudrate, LogLevel logLevel)
{
	_print = stream;
    _logLevel = logLevel;
    stream->begin(baudrate);
}
#endif
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_AVR_LEONARDO)

void Logger::begin(Serial_* serial, uint32_t baudrate, LogLevel logLevel)
{
	_print = serial;
	_logLevel = logLevel;
	serial->begin(baudrate);
}
#endif
#if defined(ARDUINO_ARCH_AVR)

void Logger::begin(HardwareSerial* serial, uint32_t baudrate, LogLevel logLevel)
{
	_print = serial;
	_logLevel = logLevel;
	serial->begin(baudrate);
}

void Logger::begin(SoftwareSerial* serial, uint32_t baudrate, LogLevel logLevel)
{
	_print = serial;
	_logLevel = logLevel;
	serial->begin(baudrate);
}

#endif

void Logger::setLogLevel(LogLevel logLevel)
{
    _logLevel = logLevel;
}

void Logger::setIncludeTimestamp(bool value)
{
	_includeTimestamp = value;
}

void Logger::setIncludeLogLevel(bool value)
{
	_includeLogLevel = value;
}

void Logger::error(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Error, false, true, message, args);
	va_end(args);
}

void Logger::error(const char* message, ...)
{
	va_list args;
    va_start(args, message);
    log(LogLevel::Error, false, true, message, args);
	va_end(args);
}
void Logger::info(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Info, false, true, message, args);
	va_end(args);
}

void Logger::info(const char* message, ...)
{
	va_list args;
    va_start(args, message);
    log(LogLevel::Info, false, true, message, args);
	va_end(args);
}

void Logger::debug(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Debug, false, true, message, args);
	va_end(args);
}

void Logger::debug(const char* message, ...)
{
	va_list args;
    va_start(args, message);
    log(LogLevel::Debug, false, true, message, args);
	va_end(args);
}

void Logger::trace(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, false, true, message, args);
	va_end(args);
}

void Logger::trace(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, false, true, message, args);
	va_end(args);
}

void Logger::traceStart(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, false, false, message, args);
	va_end(args);
}

void Logger::traceStart(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, false, false, message, args);
	va_end(args);
}

void Logger::tracePart(const __FlashStringHelper *message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, true, false, message, args);
	va_end(args);
}

void Logger::tracePart(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, true, false, message, args);
	va_end(args);
}

void Logger::traceEnd(const __FlashStringHelper* message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, true, true, message, args);
	va_end(args);
}

void Logger::traceEnd(const char* message, ...)
{
	va_list args;
	va_start(args, message);
	log(LogLevel::Trace, true, true, message, args);
	va_end(args);
}

void Logger::logPrefix(LogLevel logLevel, bool isPart)
{
	char buffer[20];
	if (_includeTimestamp)
	{
		uint32_t timestamp = millis();
		snprintf(buffer, 128, "%010ul ", timestamp);
		_print->print(buffer);
	}
	if (_includeLogLevel)
	{
		switch (logLevel)
		{
		case LogLevel::Debug:
			_print->print(F("DBG "));
			break;
		case LogLevel::Error:
			_print->print(F("ERR "));
			break;
		case LogLevel::Info:
			_print->print(F("INF "));
			break;
		case LogLevel::Trace:
			_print->print(F("TRC "));
			break;
		}
	}
}

void Logger::log(LogLevel logLevel, bool isPart, bool writeLinefeed, const char* format, va_list args)
{
	char buffer[128];
	if (!isActive() || _logLevel < logLevel)
	{
		return;
	}
	if (!isPart)
	{
		logPrefix(logLevel, isPart);
	}

	vsnprintf(buffer, 128, format, args);
	_print->print(buffer);
	if (writeLinefeed)
	{
		_print->print("\r\n");
	}
}

void Logger::log(LogLevel logLevel, bool isPart, bool writeLinefeed, const __FlashStringHelper* format, va_list args)
{
	char buffer[128];
	if (!isActive() || _logLevel < logLevel)
	{
		return;
	}
	if (!isPart)
	{
		logPrefix(logLevel, isPart);
	}

	vsnprintf_P(buffer, 128, reinterpret_cast<const char*>(format), args);
	_print->print(buffer);
	if (writeLinefeed)
	{
		_print->print("\r\n");
	}
}

bool Logger::isActive()
{
	return (_print);
}


Logger Log;
