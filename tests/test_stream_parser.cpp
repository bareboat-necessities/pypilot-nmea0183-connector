#include <cassert>
#include <cstring>
#include <pypilot_nmea0183_connector.hpp>

using namespace pypilot_nmea0183_connector;

int main() {
    Nmea0183StreamParser parser;
    NmeaSentence s;
    const char* line = "$IIMWV,45.0,R,12.3,N,A*3C\r\n";
    bool got = false;
    for (const char* p = line; *p; ++p) got = parser.push(*p, s) || got;
    assert(got);
    assert(std::strcmp(s.talker, "II") == 0);
    assert(std::strcmp(s.formatter, "MWV") == 0);
    assert(s.field_count == 5);
    assert(std::strcmp(s.field(0), "45.0") == 0);
    assert(std::strcmp(s.field(1), "R") == 0);

    parser.reset();
    got = false;
    const char* bad = "$IIMWV,45.0,R,12.3,N,A*00\r\n";
    for (const char* p = bad; *p; ++p) got = parser.push(*p, s) || got;
    assert(!got);
    return 0;
}
