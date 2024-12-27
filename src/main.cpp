#include <Arduino.h>
#include <array>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDR 0x3C
#define SCL 5
#define SDA 4
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

int checkButton(int index, unsigned long currentMillis);
void updateState(int index);

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);

struct State {
  int x, y;
  char currentChar;
  char message[9]; // Max 8 characters
  unsigned long lastBlink;
  bool showCursor;
};

struct Button {
  int pin;
  int state;
  int lastState;
  unsigned long lastDebounceTime;
};

int BTN_PINS[5] = {
  13, // UP
  12, // DOWN
  11, // LEFT
  10, // RIGHT
  9 // CONFIRM
};

unsigned long debounceDelay = 50;

State state = {0, 0, 'A', ""};
std::array<Button, 5> buttons;

std::array<char, 40> chars = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', ',', '!', '?'
};

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA, SCL);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  for (int i = 0; i < 5; i++){
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    buttons[i] = {BTN_PINS[i], HIGH, HIGH, 0};
  }
}

void loop() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < 5; i++){
    int reading = checkButton(i, currentMillis);

    if (reading) {
      updateState(i); 
    }
  }
  
  display.clearDisplay();

  // Draw keyboard
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print(" A B C D E F G H I J ");
  display.setCursor(0, 32);
  display.print(" K L M N O P Q R S T ");
  display.setCursor(0, 44);
  display.print(" U V W X Y Z 0 1 2 3 ");
  display.setCursor(0, 56);
  display.print(" 4 5 6 7 8 9 . , ! ? ");


  display.drawRect(state.x * 12 - 3 + 6, state.y * 12 - 3 + 20, 12, 13, WHITE);

  // Draw cursor
  display.setTextSize(2);
  display.setCursor(0, 0);
  char letter = chars[state.y * 10 + state.x];
  state.currentChar = letter;
  display.print(state.message);

  if (currentMillis - state.lastBlink > 500) {
    state.showCursor = !state.showCursor;
    state.lastBlink = currentMillis;
  }

  if (state.showCursor) {
    display.setCursor(strlen(state.message) * 12, 0);
    display.print("_");
  }

  display.display();
}

void updateState(int index) {
  switch (index) {
    case 0:
      state.y--;
      break;
    case 1:
      state.y++;
      break;
    case 2:
      state.x--;
      break;
    case 3:
      state.x++;
      break;
    case 4:
      if (strlen(state.message) < sizeof(state.message) - 1) {
        Serial.println(strlen(state.message));
        state.message[strlen(state.message)] = state.currentChar;
      }
      break;
  }
}

int checkButton(int index, unsigned long currentMillis) {
  Button *btn = &buttons.at(index);
  int reading = digitalRead(btn->pin);

  if (reading != btn->lastState) {
    btn->lastDebounceTime = currentMillis;
  }

  if ((currentMillis - btn->lastDebounceTime) > debounceDelay) {
    if (reading == LOW && btn->state == HIGH) {
      btn->state = LOW;
      return 1;  // Button was pressed
    } else if (reading == HIGH && btn->state == LOW) {
      btn->state = HIGH;
    }
  }

  btn->lastState = reading;
  return 0;  // No button press detected
}
