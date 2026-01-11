#include <Arduino.h>

/* ---------- PINOUT ---------- */
const int PIN_SPEED_OUT = 9;
const int PIN_FLOW_OUT  = 8;

/* ---------- GLOBALS ---------- */
long target_speed_hz = 0;
long target_flow_hz  = 0;

unsigned long prev_micros_speed = 0;
unsigned long prev_micros_flow  = 0;
bool state_speed = LOW;
bool state_flow  = LOW;

String input_string = "";
bool string_complete = false;

/* ---------- PROTOTYPES ---------- */
void readSerialData(void);
void parseCommand(String);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  input_string.reserve(200);

  pinMode(PIN_SPEED_OUT, OUTPUT);
  pinMode(PIN_FLOW_OUT, OUTPUT);

  digitalWrite(PIN_SPEED_OUT, LOW);
  digitalWrite(PIN_FLOW_OUT, LOW);
}

void loop() {
  unsigned long current_micros = micros();

  readSerialData();

  if (string_complete) {
    parseCommand(input_string);
    input_string = "";
    string_complete = false;
  }

  /* Handle Speed Output */
  if (target_speed_hz > 0) {
    unsigned long interval_speed = 1000000UL / (target_speed_hz * 2);
    
    if (current_micros - prev_micros_speed >= interval_speed) {
      prev_micros_speed = current_micros;
      state_speed = !state_speed;
      digitalWrite(PIN_SPEED_OUT, state_speed);
    }
  } else {
    digitalWrite(PIN_SPEED_OUT, LOW);
    state_speed = LOW;
  }

  /* Handle Flow Output */
  if (target_flow_hz > 0) {
    unsigned long interval_flow = 1000000UL / (target_flow_hz * 2);
    
    if (current_micros - prev_micros_flow >= interval_flow) {
      prev_micros_flow = current_micros;
      state_flow = !state_flow;
      digitalWrite(PIN_FLOW_OUT, state_flow);
    }
  } else {
    digitalWrite(PIN_FLOW_OUT, LOW);
    state_flow = LOW;
  }
}

void readSerialData() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      string_complete = true;
    } else {
      if (c != '\r') {
        input_string += c;
      }
    }
  }
}

void parseCommand(String cmd) {
  int s_index = cmd.indexOf("S");
  int f_index = cmd.indexOf("F");

  if (s_index != -1 && f_index != -1) {
    String s_val = cmd.substring(s_index + 1, f_index);
    String f_val = cmd.substring(f_index + 1);

    long new_speed = s_val.toInt();
    long new_flow  = f_val.toInt();

    if (new_speed >= 0) target_speed_hz = new_speed;
    if (new_flow >= 0)  target_flow_hz  = new_flow;
  }

  Serial.print("Set -> Speed: ");
  Serial.print(target_speed_hz);
  Serial.print(" Hz | Flow to: ");
  Serial.print(target_flow_hz);
  Serial.println(" Hz");
}