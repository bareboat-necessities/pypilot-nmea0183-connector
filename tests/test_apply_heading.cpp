#include <cassert>
#include <cmath>
#include <nmea0183_connector.hpp>
#include <pypilot_data_model.hpp>

using namespace nmea0183_connector;
using namespace pypilot_data_model;

static bool parse(const char* line, NmeaSentence& s) {
    Nmea0183StreamParser p;
    bool got = false;
    for (const char* c = line; *c; ++c) got = p.push(*c, s) || got;
    return got;
}

int main() {
    DataModel<> model;
    Nmea0183RxConnector<> c;
    NmeaSentence s;
    assert(parse("$IIHDT,123.4,T*26\r\n", s));
    assert(c.apply_sentence(s, model, 100));
    assert(std::fabs(model.imu.heading_deg.value - 123.4f) < 0.0001f);

    assert(parse("$IIRSA,2.5,A,,V*7E\r\n", s));
    assert(c.apply_sentence(s, model, 200));
    assert(std::fabs(model.rudder.angle_deg.value + 2.5f) < 0.0001f);
    return 0;
}
