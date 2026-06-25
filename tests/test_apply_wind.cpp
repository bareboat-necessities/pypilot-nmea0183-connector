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
    assert(parse("$IIMWV,45.0,R,12.3,N,A*3C\r\n", s));
    assert(c.apply_sentence(s, model, 100));
    assert(std::fabs(model.wind.apparent.direction_deg.value - 45.0f) < 0.0001f);
    assert(std::fabs(model.wind.apparent.speed_kn.value - 12.3f) < 0.0001f);

    assert(parse("$IIMWV,220.0,T,10.0,N,A*0A\r\n", s));
    assert(c.apply_sentence(s, model, 200));
    assert(std::fabs(model.wind.truewind.direction_deg.value + 140.0f) < 0.0001f);
    assert(std::fabs(model.wind.truewind.speed_kn.value - 10.0f) < 0.0001f);
    return 0;
}
