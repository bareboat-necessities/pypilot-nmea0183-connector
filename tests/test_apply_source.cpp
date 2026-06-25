#include <cassert>
#include <pypilot_nmea0183_connector.hpp>
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
    assert(parse("$IIMWV,45.0,R,12.3,N,A*3C\r\n", s));
    assert(c.apply_sentence(s, model, 77, SensorSource::tcp));
    assert(model.wind.apparent.source.value == SensorSource::tcp);
    assert(model.wind.apparent.last_update_us == 77);
    return 0;
}
