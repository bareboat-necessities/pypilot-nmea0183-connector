#include <cassert>
#include <cstring>
#include <pypilot_nmea0183_connector.hpp>

using namespace pypilot_nmea0183_connector;

static void expect_body(const char* line, const char* body) {
    char expected[128];
    make_sentence(expected, sizeof(expected), body);
    assert(std::strcmp(line, expected) == 0);
    assert(verify_nmea_checksum(line));
}

int main() {
    char line[128];
    make_xdr_pitch(line, sizeof(line), 1.25f);
    expect_body(line, "APXDR,A,1.250,D,PTCH");

    make_xdr_roll(line, sizeof(line), -2.5f);
    expect_body(line, "APXDR,A,-2.500,D,ROLL");

    make_rot(line, sizeof(line), 3.75f);
    expect_body(line, "APROT,3.750,A");

    make_ap_hdm(line, sizeof(line), 123.4f);
    expect_body(line, "APHDM,123.4,M");

    make_ap_mwv(line, sizeof(line), -10.0f, 12.3f, true);
    expect_body(line, "APMWV,350.0,R,12.3,N,A");

    make_ap_rsa(line, sizeof(line), 2.5f);
    expect_body(line, "APRSA,-2.5,A,,V");

    pypilot_data_model::DataModel<> model;
    model.navigation.gps.fix_lat_deg.set(48.1173f, 100);
    model.navigation.gps.fix_lon_deg.set(11.516666f, 100);
    model.navigation.gps.speed_kn.set(22.4f, 100);
    model.navigation.gps.track_deg.set(84.4f, 100);
    make_ap_rmc(line, sizeof(line), model, "123519", "230394");

    char expected_body[128];
    std::strcpy(expected_body, "AP");
    std::strcat(expected_body, "RMC,123519,A,4807.0379,N,1131.0000,E,22.40,84.40,230394,,,A");
    expect_body(line, expected_body);
    return 0;
}
