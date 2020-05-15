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

#ifdef DEBUG
static const int debug = DEBUG;
#else
static const int debug = 0;
#endif

#define DEBUGPRINT(...) if (debug) printf(__VA_ARGS__)

bool terminator(char c);
uint8_t keywd_cmp (const char *in, const char *s);
uint8_t keywd_len (const char *s);
char * keywd_complete (const char *in, const char *list);


class cmd_t {
private:
    const char *text;
    void *(*func)(char *);
    cmd_t *next;

public:
    cmd_t() { text=NULL; func = NULL; }
    
    cmd_t(cmd_t *&head, const char *t, void*(*f)(char *)) {
	text = t;
	func = f;
	if (head == NULL) {
	    head = this;
	} else {
	    cmd_t *p = (cmd_t *)head;
	    while (p->next != NULL) {
		p = p->next;
	    }
	    p->next = this;
	}
    }

    void dump() {
	cmd_t *p = this;
	while (p) {
	    printf("%s\n", p->text);
	    p = p->next;
	}
    }
    
    cmd_t *matchany (cmd_t *head, const char *in) {
	uint8_t matchlen,	// number of characters matched
	    keylength,		// full size of keyword.
	    nmatches = 0;	// number of matches
	cmd_t *pmatch=NULL, *match = head;	// candidate partialmatch

	DEBUGPRINT("\n cmd_t matchany '%s' in '%s'", in, match->next->text);
	do { // for all keyword choices
	    keylength = keywd_len(match->text);
	    matchlen = keywd_cmp(in, match->text);
	    DEBUGPRINT("\n matched %d of %d chars in '%s'", matchlen, keylength, match->text);
	    if (matchlen == keylength) {
		return match;  // exact match
	    } else if (matchlen) {
		// partial match.  Remember it.
		nmatches++;
		pmatch = match;
		DEBUGPRINT("\npartial match %d at '%s'", nmatches, match->text);
	    }
	    /* Next possible keyword */
	    match = match->next;
	} while (match);
	if (nmatches == 1) {
	    return pmatch;
	}
	DEBUGPRINT("\nno match");
	return NULL;  // no match
    }

    char *complete (const char *in) {
	return keywd_complete(in, text);
    }

};

bool terminator(char c) {
    return c == 0 || c == ' ';
}

/*
 * Compare keyword
 * compare an input keyword (in) with a string in a table (s)
 * terminate on any non-printing character (including space)
 * return the number of matching characters:
 *   strlen(in): the input is a partial match of the string
 *   strlen(s): exact match
 *   0: no match (0 chars matched OR in longer than s)
 */
uint8_t keywd_cmp (const char *in, const char *s)
{
    uint8_t count = 0;
    while (!terminator(*s)) {
	if (terminator(*in))
	    return count;
	if (*in++ != *s++)
	    return 0;
	count++;
    }
    if (terminator(*in)) // "in" same length as "s" ?
	return count;    //   exact match
    return 0;
}

/*
 * keywd_len
 * like strlen(), only the string ends with any terminator
 */
uint8_t keywd_len (const char *s)
{
    uint8_t len = 0;
    while (!terminator(*s++))
	len++;
    return len;
}

/*
 * keywd_complete
 * return string pointer to "completion" of a singly-matched keyword.
 */
char * keywd_complete (const char *in, const char *list)
{
    uint8_t matchlen,		       // number of characters matched
	keylen,			       // keyword length
	inlen;			       // full size of keyword.
    static char compstr[10];

    inlen = keywd_len(in);
    do { // for all keyword choices
	matchlen = keywd_cmp(in, list);
	if (matchlen == inlen) {
	    /* this is our partial match */
	    char *p=compstr;
	    list += inlen;
	    while (!terminator(*list)) {
		*p++ = *list++;
	    }
	    *p = 0;
	    return compstr;
	}
	list += keywd_len(list) + 1;
    } while (*(list-1));
    return 0;  // no match	
}

/*
 * keywd_matchany
 * see if the "in" string matches any of the keywords in "list" string
 * return ... index val of exact match???
 */
uint8_t keywd_matchany (const char *in, const char *list)
{
    uint8_t index = 0;
    uint8_t matchlen,		       // number of characters matched
	keylength,		       // full size of keyword.
	nmatches = 0,		       // number of matches
	partialmatch = 0;	       // candidate partialmatch

    DEBUGPRINT("\nmatchany '%s' in '%s'", in, list);
    do { // for all keyword choices
	keylength = keywd_len(list);
	matchlen = keywd_cmp(in, list);
	DEBUGPRINT("\n matched %d of %d chars in '%s'", matchlen, keylength, list);
	if (matchlen == keylength) {
	    return index+1;  // exact match
	} else if (matchlen) {
	    // partial match.  Remember it.
	    nmatches++;
	    partialmatch = index+1;
	    DEBUGPRINT("\npartial match %d at %d", nmatches, partialmatch);
	}
	/* Next possible keyword */
	index++;
	list += keylength+1;
    } while (*(list-1));
    if (nmatches == 1) {
        return partialmatch;
    }
    return 0;  // no match	
}


#if defined(TEST) && TEST
const char keywords[]= "test terminate end start";
const char *testinputs[] = {
    "a",
    "mult",
    "mul",
    "test",
    "te",
    "ter",
    "try",
    "exit",
    "testing",
    "add",
    "end",
    "start"
};


void *add_f (char *c) {
    printf("%s\n", c);
    return NULL;
}

void *sub_f (char *c) {
    printf("%s\n", c);
    return NULL;
}
void *mul_f (char *c) {
    printf("%s\n", c);
    return NULL;
}

cmd_t *mathcmd = NULL;

cmd_t add(mathcmd, "add", add_f);
cmd_t sub(mathcmd, "sub", sub_f);
cmd_t mul(mathcmd, "mul", mul_f);


void test_keywd_cmp()
{
    printf("\ntest_keywd_cmp");
    for (uint8_t i=0; i < sizeof(testinputs)/sizeof(testinputs[0]); i++) {
	printf("\n  Input is \"%s\", string \"%s\".  output is %d",
		testinputs[i], keywords, keywd_cmp(testinputs[i], keywords));
    }
}

void test_keywd_match()
{
    int index;
    printf("\ntest_keywd_match");
    for (uint8_t i=0; i < sizeof(testinputs)/sizeof(testinputs[0]); i++) {
	index = keywd_matchany(testinputs[i], keywords);
	printf("\n  Input is \"%s\", string \"%s\".  output is %d",
	       testinputs[i], keywords, index);
	if (index) {
	    char *p = keywd_complete(testinputs[i], keywords);
	    if (p && *p) {
		printf(" (complete with '%s')", p);
	    }
	}
    }
}

void test_cmd_match()
{
    cmd_t *match;
    printf("\ntest_cmd_match");
    for (uint8_t i=0; i < sizeof(testinputs)/sizeof(testinputs[0]); i++) {
	match = mathcmd->matchany(mathcmd, testinputs[i]);
	printf("\n  Input is \"%s\".  output is %p",
	       testinputs[i], match);
	if (match) {
	    char *p = match->complete(testinputs[i]);
	    if (p && *p) {
		printf(" (complete with '%s')", p);
	    }
	}

    }
}
#endif


int main() {
    printf("\nBuilt up command list:\n");
    mathcmd->dump();
    printf("\n");
#if defined(TEST) && TEST
    test_keywd_cmp();
    test_keywd_match();
    test_cmd_match();
    printf("\n");
#endif
}

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

#ifDEF NOT_READY_FOR_PRIME_TIME
cmd_t *cmd_t::head = NULL;
cmd_t::cmd_t() { text=NULL; func = NULL; }
    
cmd_t::cmd_t(const char *t, void*(*f)(char *)) {
    text = t;
    func = f;
    if (head == NULL) {
	head = this;
    } else {
	cmd_t *p = head;
	while (p->next != NULL) {
	    p = p->next;
	}
	p->next = this;
    }
}

cmd_t::cmd_t(cmd_t *head, const char *t, void*(*f)(char *)) {
    text = t;
    func = f;
    if (head == NULL) {
	head = this;
    } else {
	cmd_t *p = head;
	while (p->next != NULL) {
	    p = p->next;
	}
	p->next = this;
    }
}

cmd_t::void dump() {
    cmd_t *p = head;
    while (p) {
	printf("%s\n", p->text);
	p = p->next;
    }
}

#endif
