#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>

#include "p1-parser.h"
#include "dsmr-data.h"

uint16_t crc_telegram (const uint8_t *data, unsigned int length);


// Default size of the buffer used to read and store telegrams, determines maximum telegram size

#define BUFSIZE_TELEGRAM 4096

#define TELEGRAM_OK 0
#define TELEGRAM_INCOMPLETE -1
#define TELEGRAM_BUFFER_TO_SMALL -2
#define TELEGRAM_CRC_ERROR -3
#define TELEFRAM_PARSER_FAILED -4

typedef struct telegram_parser_struct {
	int status;				// Ragel parser status
	struct parser parser;	// Ragel state machine structure
	
	struct dsmr_data_struct *data;			// Smart meter data structure
	
	size_t len;								// Telegram length
	uint8_t buffer[BUFSIZE_TELEGRAM+1];		// Telegram buffer pointer

	bool onLastLine;
} telegram_parser;


void telegram_parser_init (telegram_parser *obj);
int telegram_parser_read (telegram_parser *obj, const uint8_t* buff, size_t bufLen);
int telegram_parser_read_partial (telegram_parser *obj, const uint8_t* buff, const size_t bufLen, size_t *numRead);