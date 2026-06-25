#include <cassert>
#include <cmath>
#include <cstring>
#include <pypilot_nmea0183_connector.hpp>
#include <pypilot_data_model.hpp>

using namespace pypilot_nmea0183_connector;
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
    char line[128];
    make_sentence(line, sizeof(line), "GPAPB,A,A,0.200,L,N,A,A,011.0,T,WPT,123.4,T,234.5,M");
    assert(parse(line, s));
    assert(c.apply_sentence(s, model, 100, SensorSource::serial));
    assert(std::fabs(model.navigation.apb.xte_nmi.value + 0.15f) < 0.0001f);
    assert(std::fabs(model.navigation.apb.track_deg.value - 234.5f) < 0.0001f);
    assert(model.navigation.apb.source.value == SensorSource::serial);
    assert(model.navigation.apb.last_update_us == 100);
    assert(c.last_apb_mode() == AutopilotMode::compass);
    assert(std::strcmp(c.last_apb_sender_id(), "GP") == 0);
    assert(model.navigation.apb.mode_hint.value == AutopilotMode::compass);
    assert(std::strcmp(model.navigation.apb.sender_id, "GP") == 0);
    return 0;
}
