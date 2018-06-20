#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif
#define SPEAKER_RELAY_ACTIVATE 7
#define SPEAKER_RELAY_RESET 8
#define PUSHER 2
#define AMPLIFIEER_TRIGGER 4

/*
Speakers are connected to 2 output of a bistable relay, the pusle to
activate or reset the state have a duration of minimum 5ms

The amplier trigger is an input coming from the homecinema amplifier set to 1
when it's on, so this input activate the relay

The push button force the relay to switch state until the next change from
amplifier trigger input

The builtin led is on when the homecinema is active

At startup the hifispeaker should be activated
*/

unsigned long previousMillis = 0;
const long interval = .5 * 1000;
int relay_activation_duration = 10;

unsigned long last_debonce_time_pusher = 0;  // the last time the output pin was toggled
unsigned long last_debonce_time_amplifier_trigger = 0;  // the last time the output pin was toggled
unsigned long debounce_delay = 200;    // the debounce time; increase if the output flickers

int pusher_state = 0;         // current state of the button
int last_pusher_state = 0;     // previous state of the button

int amplifier_trigger_state = 0;         // current state of the button
int last_amplifier_trigger_state = 0;     // previous state of the button

bool speaker_relay_activated = false;

void activate_homecinema_speakers() {
  digitalWrite(SPEAKER_RELAY_ACTIVATE, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(relay_activation_duration);
  digitalWrite(SPEAKER_RELAY_ACTIVATE, LOW);
  speaker_relay_activated = true;
  Serial.println("HOME CINEMA speakers activated");
}

void activate_hifi_speakers() {
  digitalWrite(SPEAKER_RELAY_RESET, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(relay_activation_duration);
  digitalWrite(SPEAKER_RELAY_RESET, LOW);
  speaker_relay_activated = false;
  Serial.println("HIFI speakers activated");
}

void toggle_speakers() {
  if (speaker_relay_activated) {
    activate_hifi_speakers();
  } else {
    activate_homecinema_speakers();
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPEAKER_RELAY_ACTIVATE, OUTPUT);
  pinMode(SPEAKER_RELAY_RESET, OUTPUT);
  pinMode(PUSHER, INPUT);
  pinMode(AMPLIFIEER_TRIGGER, INPUT);

  Serial.begin(9600);
  Serial.println("Arduino is initiliazed");
  activate_hifi_speakers();
}

void loop() {
  int read_pusher = digitalRead(PUSHER);
  int read_amplifier_trigger = digitalRead(AMPLIFIEER_TRIGGER);

  if (read_pusher != last_pusher_state) {
    last_debonce_time_pusher = millis();
  }
  if (read_amplifier_trigger != last_amplifier_trigger_state) {
    last_debonce_time_amplifier_trigger = millis();
  }

  if ((millis() - last_debonce_time_pusher) > debounce_delay) {
    if (read_pusher != pusher_state) {
      pusher_state = read_pusher;
      Serial.println("Toggle switch have been pressed");
      toggle_speakers();
    }
  }
  if ((millis() - last_debonce_time_amplifier_trigger) > debounce_delay) {
    if (read_amplifier_trigger != amplifier_trigger_state) {
      amplifier_trigger_state = read_amplifier_trigger;
      Serial.println("Amplifier trigger has changed state");
      if (amplifier_trigger_state) {
        activate_homecinema_speakers();
      } else {
        activate_hifi_speakers();
      }
    }
  }

  // save the current state as the last state, for next time through the loop
  last_pusher_state = read_pusher;
  last_amplifier_trigger_state = read_amplifier_trigger;
}
