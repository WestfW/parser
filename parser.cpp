/*
 * parser.cpp
 * Implement a command-line parser.
 * Written 2014 by Bill Westfield (WestfW)
 * Released to the public domain.
 */

#include "Arduino.h"
#include "parser.h"
#include <avr/pgmspace.h>
#include "WString.h"

char linebuffer[PARSER_LINELEN];
static byte inptr, parseptr, termchar;

char getcwait(void)
{
    int c;
    do {
	c = Serial.read();
    } while (c <= 0);
    return c;
}

/*
 * parserGetline
 * Read a line of text from Serial into the internal line buffer.
 * With echoing and editing!
 */
uint8_t parseGetline(void)
{
    char c;

    /*
     * Reset the line buffer
     */
    parseReset();

    do {
	c = getcwait();
	switch (c) {
	case 127:
	case CTRL('H'):
	    /*
	     * Destructive backspace: remove last character
	     */
	    if (inptr > 0) {
		Serial.print("\010 \010");
		linebuffer[--inptr] = 0;
	    }
	    break;
	case CTRL('R'):
	    /*
	     * Ctrl-R retypes the line
	     */
	    Serial.print("\r\n");
	    Serial.print(linebuffer);
	    break;
	case CTRL('U'):
	    /*
	     * Ctrl-U deletes the entire line and starts over.
	     */
	    Serial.println("XXX");
	    memset(linebuffer, 0, sizeof(linebuffer));
	    inptr = 0;
	    break;
	default:
	    /*
	     * Otherwise, echo the character and put it into the buffer
	     */
	    linebuffer[inptr++] = c;
	    Serial.write(c);
	    break;
	}
    } while (c != CTRL('M') && inptr < PARSER_LINELEN);
    Serial.println();			/* Echo newline too. */
    return(inptr);
}

/*
 * parseGetline_nb
 * Read a line of text from Serial into the internal line buffer.
 * With echoing and editing!
 * Non-blocking.  Returns 0 until end-of-line seen.
 */
void parseReset()
{
    /*
     * Reset the line buffer
     */
    memset(linebuffer, 0, sizeof(linebuffer));
    inptr = 0;
    parseptr = 0;
    termchar = 0;
}

uint8_t parseGetline_nb(void)
{
    int c;

    c = Serial.read();
    switch (c) {
    case 127:
    case CTRL('H'):
	/*
	 * Destructive backspace: remove last character
	 */
	if (inptr > 0) {
	    Serial.print("\010 \010");
	    linebuffer[--inptr] = 0;
	}
    break;
    case CTRL('R'):
	/*
	 * Ctrl-R retypes the line
	 */
	Serial.print("\r\n");
	Serial.print(linebuffer);
	break;
    case CTRL('U'):
	/*
	 * Ctrl-U deletes the entire line and starts over.
	 */
	Serial.println("XXX");
	memset(linebuffer, 0, sizeof(linebuffer));
	inptr = 0;
	break;
    case CTRL('M'):
	Serial.println();			/* Echo newline too. */
	return inptr;
    case -1:
	/*
	 * No character present; don't do anything.
	 */
	return 0;
    default:
	/*
	 * Otherwise, echo the character and put it into the buffer
	 */
	linebuffer[inptr++] = c;
	Serial.write(c);
    }
    return 0;
}


bool parseIsWhitespace(char c)
{
    return (c == ' ' || c == CTRL('I'));
}


const char parseDelimiters[] PROGMEM = "\r\n ,;:=\t";
bool parseDelim(char c)
{
    if (c == 0 || strchr_P(parseDelimiters, c))
	return true;
    return false;
}

int parseNumber()
{
    return atoi(parseToken());
}

/*
 * parseTermChar
 * return the termination character of the last token
 */
uint8_t parseTermChar()
{
    return termchar;
}

/*
 * parseCharacter
 */

/*
 * parseToken
 * A token is a set of non-delimiter characters ending at a delimiter.
 * As a line is parsed from the internal buffer, parseptr is advanced, and
 * the delimiters of parsed tokens are replaced with nulls.
 * Note that a line always ends with the newline character AND a null.
 */
char * parseToken(void)
{
    uint8_t i;

    while (parseIsWhitespace(linebuffer[parseptr])) { /* skip leading whitespace */
	parseptr++;
    }
    if (linebuffer[parseptr] == 0) {  // reached the end of the line?
	return NULL;
    }
    i = parseptr;  // save start position of token
    while ((!parseDelim(linebuffer[parseptr])) && (parseptr < PARSER_LINELEN)) {
	parseptr++; // advance pointer till we hit a delimiter.
    }
    termchar = linebuffer[parseptr];
    linebuffer[parseptr++] = 0;  // replace the delimiter with null
    return &linebuffer[i];  // convert position to pointer for retval
}

/*
 * Match the next token with a list of keywords.
 * The list of keywords is in PROGMEM, separated by spaces.
 * returns either the position of the found keyword (0..n), 
 * PARSER_NOMATCH, or PARSER_EOL at the end of line
 */
int8_t parseKeyword(const char *keys)
{
    char *p = parseToken();
    char *key = (char *)keys;
    int8_t i = 0;
    if (p) {
	while (pgm_read_byte(key)) {
	    key = tokcasecmp(p, key);
	    if (key == 0) {
		return i;  // match
	    }
	    key++;  // skip delimiter
	    i++; // next keyword
	}
    } else {
	return PARSER_EOL;
    }
    return PARSER_NOMATCH;
}


/*
 * tokcasecmp
 * tokcasecmp is like strcasecmp_P, except that the strings are terminated
 * by any char < 32.  Return value is 0 for match, or pointer to the delimiter
 * for non-match (to expedite comparing against strings of targets.)
 */

char *tokcasecmp(char *tok, char *target)
{
    char c1, c2;
    char *t = (char *)target;

    do {
	c1 = TOUPPER(*tok);
	c2 = TOUPPER(pgm_read_byte(t));
	if (c2 <= ' ') {
	    /* Match */
	    if (c1 != 0) {
		return t;  // oops; both strings didn't end.
	    }
	    return 0; //match!
	}
	tok++; t++;
    } while (c1 == c2);
    /* Non-match.  Advance target ptr */
    while (pgm_read_byte(t) > ' ') {
	t++;
    }
    return t;
}
