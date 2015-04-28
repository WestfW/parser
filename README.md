#Parser library
***

Simple command line parser (CLI) for Arduino/etc.

This project is at its initial stages of development, had little to no documentation, and is not really ready for public consumption

There are two parts to this "parser."  First, it provides functions to read "line-at-a-time" input given a simple non-blocking single character input function (`Serial.read()` on Arduino.)  Simple command-line editing (delete, erase line, retype) is provided, and the text is collected in an internal buffer until a newline is typed.

Second, the library provides functions for separating the line into simple "tokens" and interpretting several common types of tokens (keywords, numbers, simple fields.)

***
### Basic operation
The basic flow using this library looks like:

    parseReset();  // Reset the parser
    parseGetLine(); // read a line of text
    do {
       results = parseSomeTokenTypes(args);
       // act on results
    } while (results != PARSER_EOL); // until end of line
***
###Details

> ` void parseReset(void); `

Zero and reset the internal buffers.


> ` uint8_t parseGetline(void); `

Read a line of text into the interal buffer.
Returns the length of the line.


> ` uint8_t parseGetline_nb(void); `

A non-blocking version of the GetLine function.  The function will always return instantly, with a value of 0 if a full line has not been read yet, or the length of the line if the read is complete.

> ` char *parseToken(void); `

"eat" the next token from the line buffer and return a pointer to the start of that token.  Parsing a token is destructive of the line buffer; field separators are converted to nulls, and can't be retrieved.  Most of the parse functions do their own parseToken; you can use the function when you want to get a text field without additional interpretation.

> ` int parseNumber(); `

Parse a decimal number from the line buffer.

> ` int8_t parseKeyword(const char PROGMEM *keys); `

Parse a keyword.  Given a space-separated list of keywords, return either the index of a matching keyword, or PARSER_NOMATCH.
