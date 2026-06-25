#include <cassert>
#include <pypilot_nmea0183_connector.hpp>

using namespace pypilot_nmea0183_connector;

int main() {
    Nmea0183StreamParser parser;
    NmeaSentence s;
    const char* line = "$IIMWV,45.0,R,12.3,N,A*3C\r\n";
    bool got = false;
    for (const char* p = line; *p; ++p) got = parser.push(*p, s) || got;
    assert(got);
    assert(nmea_span_equals(s.talker, "II"));
    assert(nmea_span_equals(s.formatter, "MWV"));
    assert(s.field_count == 5);
    assert(nmea_span_equals(s.field(0), "45.0"));
    assert(nmea_span_equals(s.field(1), "R"));
    assert(s.body.data == s.raw + 1);
    assert(s.field(0).data > s.raw && s.field(0).data < s.raw + s.raw_length);

    parser.reset();
    got = false;
    const char* bad = "$IIMWV,45.0,R,12.3,N,A*00\r\n";
    for (const char* p = bad; *p; ++p) got = parser.push(*p, s) || got;
    assert(!got);
    return 0;
}
