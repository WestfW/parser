#include <parser.h>

void setup() {
  while (!Serial)
    delay(500);
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.print(F("Enter command: "));
  parseReset();
  pinMode(13, OUTPUT);
}

boolean delay_without_delaying(unsigned long &since, unsigned long time) {
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time) {
    since = currentmillis;
    return true;
  }
  return false;
}

int red, blue, green;
unsigned long ledtime;

void loop() {
  char *p;
  int8_t cmd;
  int n;


  if (parseGetline_nb()) {
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
    parseReset();
    Serial.print(F("Enter command: "));
  } // if line
  
  static int ledstate = false;
  if (delay_without_delaying(ledtime, 500)) {
    ledstate = !ledstate;
    digitalWrite(13, ledstate);
  }
}

