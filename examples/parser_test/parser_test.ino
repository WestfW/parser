#include <parser.h>

void setup() {
  while (!Serial)
    delay(500);
  Serial.begin(115200);
  // put your setup code here, to run once:
}

int red, blue, green;

void loop() {
  char *p;
  int8_t cmd;
  int n;

  Serial.print(F("Enter command: "));

  int i = parseGetline();  // read a line of text

  do {
    enum {
      CMD_RED, CMD_GREEN, CMD_BLUE, CMD_RESET  // make sure this matches the string
    };
    cmd = parseKeyword(PSTR("red green blue reset")); // look for a command.

    if (cmd >= 0) {
      n = parseNumber();
    }
    switch (cmd) {
      case CMD_RED:
        red = n;
        break;
      case CMD_BLUE:
        blue = n;
        break;
      case CMD_GREEN:
        green = n;
        break;
      case CMD_RESET:
        red = green = blue = 0;
        break;
      case PARSER_EOL:
        Serial.print("RED = "); Serial.print(red);
        Serial.print(" GREEN = "); Serial.print(green);
        Serial.print(" BLUE= "); Serial.println(blue);
        break;
      default:
        Serial.println("Invalid command");
        break;
    }
  } while (cmd >= 0);
}

