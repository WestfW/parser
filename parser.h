#include "Arduino.h"

/*
 * API
 */
uint8_t parseGetline(void);
char *parseToken(void);
int parseNumber();
int8_t parseKeyword(const char *keys);
/*
 * Constants
 */
#define PARSER_LINELEN 80
#define PARSER_NOMATCH -1
#define PARSER_EOL -2


/*
 * Internals
 */
char getcwait(void);
bool substrdcmp(char *cmd,  char *line);
bool parseIsWhitespace(char c);
bool parseDelim(char c);
bool parserEOL();
char *tokcasecmp(char *tok, char * target);

#define TOUPPER(a) (a & ~('a'-'A'))
#define CTRL(x) (x-64)


