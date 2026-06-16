#include <cassert>
#include <cstring>
#include <pypilot_nmea0183_connector.hpp>

using namespace pypilot_nmea0183_connector;

int main() {
    char line[96];
    make_hdt(line, sizeof(line), 123.4f);
    assert(std::strcmp(line, "$IIHDT,123.4,T*26\r\n") == 0);
    assert(verify_nmea_checksum(line));

    make_mwv(line, sizeof(line), 45.0f, 12.3f, true);
    assert(verify_nmea_checksum(line));
    assert(std::strstr(line, "$IIMWV,45.0,R,12.3,N,A*") == line);

    make_rsa(line, sizeof(line), 2.5f);
    assert(std::strcmp(line, "$IIRSA,2.5,A,,V*7E\r\n") == 0);
    return 0;
}
