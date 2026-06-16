#include <cassert>
#include <pypilot_nmea0183_connector.hpp>

using namespace pypilot_nmea0183_connector;

int main() {
    assert(nmea_checksum_body("GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W") == 0x6A);
    assert(verify_nmea_checksum("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A"));
    assert(!verify_nmea_checksum("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*00"));
    return 0;
}
