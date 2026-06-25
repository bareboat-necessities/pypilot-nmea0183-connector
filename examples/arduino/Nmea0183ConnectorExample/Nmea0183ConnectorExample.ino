#include <Arduino.h>
#include <pypilot_data_model.hpp>
#include <nmea0183_connector.hpp>

pypilot_data_model::DataModel<> model;
nmea0183_connector::Nmea0183StreamParser parser;
nmea0183_connector::Nmea0183RxConnector<> connector;

void setup() {
  Serial.begin(115200);
  Serial1.begin(4800);
}

void loop() {
  while (Serial1.available()) {
    nmea0183_connector::NmeaSentence sentence;
    if (parser.push((char)Serial1.read(), sentence)) {
      connector.apply_sentence(sentence, model, micros());
    }
  }
}
