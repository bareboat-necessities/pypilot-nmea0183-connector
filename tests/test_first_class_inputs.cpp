#include <cassert>
#include <cmath>
#include <pypilot_nmea0183_connector.hpp>
#include <pypilot_data_model.hpp>

using namespace nmea0183_connector;
using namespace pypilot_data_model;

int main() {
    DataModel<> model;
    Nmea0183RxConnector<> connector;
    NmeaSentence s;
    s.clear();
    s.valid_checksum = true;
    s.talker = NmeaSpan("II", 2);
    s.sentence = NmeaSpan("GLL", 3);
    s.formatter = s.sentence;
    s.field_count = 6;
    s.fields[0] = NmeaSpan("4807.038", 8);
    s.fields[1] = NmeaSpan("N", 1);
    s.fields[2] = NmeaSpan("01131.000", 9);
    s.fields[3] = NmeaSpan("E", 1);
    s.fields[4] = NmeaSpan("123519", 6);
    s.fields[5] = NmeaSpan("A", 1);
    assert(connector.apply_sentence(s, model, 20, SensorSource::serial));
    assert(std::fabs(model.navigation.gps.fix_lat_deg.value - 48.1173f) < 0.0002f);
    return 0;
}
