#define _GNU_SOURCE 1

#include "crc16.h"
#include "p1-lib.h"

#include <string.h>

uint16_t crc_telegram (const uint8_t *data, unsigned int length)
{
	// Calculate the CRC16 of a telegram, for verification
	
	if (data[length - 3] == '!') {
		
		// Old-style telegrams end with "!\r\n" and do not contain a CRC, so there's no point in checking it
		
		return 0;
		
	} else if (data[length - 7] == '!') {
		
		// Calculate CRC16 from start of telegram until '!' (inclusive)
		// Length is full telegram length minus 2 bytes CR + LF minus 4 bytes hex-encoded CRC16
		
		return crc16(data, length - 6);
	}
	
	// Invalid telegram
	
	return 0;
}

void telegram_parser_init (telegram_parser *obj)
{
	parser_init(&(obj->parser));	// Initialise Ragel state machine
	
	obj->data = &(obj->parser.data);
	obj->status = 0;
	obj->len = 0;
	obj->onLastLine = false;
	return true;
}

int telegram_parser_read (telegram_parser *obj, const uint8_t* buff, size_t bufLen)
{
	if(buff == NULL) {
		buff = obj->buffer;
		bufLen = obj->len;
	}

	parser_execute(&(obj->parser), (const char *)(buff), bufLen, 1);
	obj->status = parser_finish(&(obj->parser));
	if (obj->status == 1) {
		uint16_t crc = crc_telegram(obj->buffer, obj->len);
		// TODO: actually report CRC error
		
	}
	if (obj->parser.parse_errors) {
		return TELEFRAM_PARSER_FAILED;
	}
	
	return TELEGRAM_OK;
}

int telegram_parser_read_partial (telegram_parser *obj, const uint8_t* buff, const size_t bufLen, size_t *numRead)
{
	size_t index = 0;

	if(obj->len == 0) {
		//search for beginning of telegram
		while(index < bufLen) {
			if(buff[index++] == '/') {
				index--;
				break;
			}
		}
	}


	// Search for the ending of a telegram
	size_t lastSearchedIndex = index;
	for(; lastSearchedIndex < bufLen; lastSearchedIndex++) {
		if(buff[lastSearchedIndex] == '!') {
			obj->onLastLine = true;
			break;
		}
	}

	bool foundEnd = false;
	*numRead = bufLen;
	if(obj->onLastLine) {
		for(; lastSearchedIndex < bufLen; lastSearchedIndex++) {
			if(buff[lastSearchedIndex] == '\n') {
				*numRead = ++lastSearchedIndex;
				foundEnd = true;
				break;
			}
		}
	}

	if(obj->len + (*numRead - index) >= BUFSIZE_TELEGRAM) {
		obj->len = 0;
		obj->status = 0;
		obj->onLastLine = false;
		return TELEGRAM_BUFFER_TO_SMALL;
	}

	size_t bytesToCopy = *numRead - index;
	memcpy(&(obj->buffer[obj->len]), &(buff[index]), bytesToCopy);
	obj->len += bytesToCopy;

	if(foundEnd) {
		obj->buffer[obj->len] = 0;
		return telegram_parser_read(obj, NULL, 0);
	}
	return (obj->len == 0 || foundEnd) ? TELEGRAM_OK : TELEGRAM_INCOMPLETE;
}