#include <cassert>
#include <cmath>
#include <nmea0183_connector.hpp>

using namespace nmea0183_connector;

int main() {
    float v = 0;
    assert(parse_lat_lon("4807.038", "N", v));
    assert(std::fabs(v - 48.1173f) < 0.0002f);
    assert(parse_lat_lon("01131.000", "W", v));
    assert(std::fabs(v + 11.516666f) < 0.0002f);
    assert(parse_knots("10.0", "N", v) && std::fabs(v - 10.0f) < 0.0001f);
    assert(parse_knots("18.52", "K", v) && std::fabs(v - 10.0f) < 0.01f);
    assert(parse_knots("5.144", "M", v) && std::fabs(v - 10.0f) < 0.01f);
    assert(parse_left_right_signed("3.2", "L", v) && std::fabs(v + 3.2f) < 0.0001f);
    assert(parse_left_right_signed("3.2", "R", v) && std::fabs(v - 3.2f) < 0.0001f);
    return 0;
}
