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
    assert(parse("$IILWY,A,3.2*2C\r\n", s));
    assert(c.apply_sentence(s, model, 100, SensorSource::serial));
    assert(std::fabs(model.water.leeway_deg.value - 3.2f) < 0.0001f);
    assert(model.water.leeway_source.value == SensorSource::serial);
    assert(model.water.last_update_us == 100);
    return 0;
}
