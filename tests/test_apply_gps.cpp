#include <cassert>
#include <cmath>
#include <nmea0183_connector.hpp>
#include <ship_data_model.hpp>

using namespace nmea0183_connector;
using namespace ship_data_model;

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
    assert(parse("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n", s));
    assert(c.apply_sentence(s, model, 100));
    assert(std::fabs(model.navigation.gps.fix_lat_deg.value - 48.1173f) < 0.0002f);
    assert(std::fabs(model.navigation.gps.fix_lon_deg.value - 11.516666f) < 0.0002f);
    assert(std::fabs(model.navigation.gps.speed_kn.value - 22.4f) < 0.0001f);
    assert(std::fabs(model.navigation.gps.track_deg.value - 84.4f) < 0.0001f);
    assert(std::fabs(model.navigation.gps.declination_deg.value + 3.1f) < 0.0001f);

    assert(parse("$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48\r\n", s));
    assert(c.apply_sentence(s, model, 200));
    assert(std::fabs(model.navigation.gps.track_deg.value - 54.7f) < 0.0001f);
    assert(std::fabs(model.navigation.gps.speed_kn.value - 5.5f) < 0.0001f);

    assert(parse("$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*7D\r\n", s));
    assert(!c.apply_sentence(s, model, 300));
    return 0;
}
