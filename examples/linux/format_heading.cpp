#include <iostream>
#include <pypilot_nmea0183_connector.hpp>

int main() {
    char line[96];
    pypilot_nmea0183_connector::make_hdt(line, sizeof(line), 123.4f);
    std::cout << line;
    return 0;
}
