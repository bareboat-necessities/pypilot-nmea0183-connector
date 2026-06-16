#include <Arduino.h>
#include <pypilot_data_model.hpp>
#include <pypilot_nmea0183_connector.hpp>

pypilot_data_model::DataModel<> model;
pypilot_nmea0183_connector::Nmea0183StreamParser parser;
pypilot_nmea0183_connector::Nmea0183Connector<> connector;

void setup() {
  Serial.begin(115200);
  Serial1.begin(4800);
}

void loop() {
  while (Serial1.available()) {
    pypilot_nmea0183_connector::NmeaSentence sentence;
    if (parser.push((char)Serial1.read(), sentence)) {
      connector.apply_sentence(sentence, model, micros());
    }
  }
}
