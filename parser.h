/* 
 * Written 2014 by Bill Westfield (WestfW)
 * Released to the public domain.
 */
#include "Arduino.h"

/*
 * API
 */
uint8_t parseGetline(void);
uint8_t parseGetline_nb(void);
void parseReset(void);
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

#define TOUPPER(a) ((a >= 'a' && a <= 'z') ? (a & ~('a'-'A')) : a)
#define CTRL(x) (x-64)

#ifdef NOT_READY
class cmd_t {
private:
    static cmd_t *head;
    const char *text;
    void *(*func)(char *);
    cmd_t *next;

public:
    cmd_t();
    cmd_t(const char *t, void*(*f)(char *));
    void dump();
};
#endif
