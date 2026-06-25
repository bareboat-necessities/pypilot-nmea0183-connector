#include <cassert>
#include <cmath>
#include <cstring>
#include <pypilot_nmea0183_connector.hpp>
#include <pypilot_data_model.hpp>

using namespace pypilot_nmea0183_connector;
using namespace pypilot_data_model;

static void make_sentence_object(NmeaSentence& s, const char* formatter, const char* const* fields, uint8_t count) {
    s.clear();
    s.valid_checksum = true;
    s.talker = NmeaSpan("II", 2);
    s.formatter = NmeaSpan(formatter, 3);
    s.field_count = count;
    for (uint8_t i = 0; i < count; ++i) s.fields[i] = NmeaSpan(fields[i], std::strlen(fields[i]));
}

int main() {
    DataModel<> model;
    Nmea0183RxConnector<> connector;
    NmeaSentence s;

    const char* gll_fields[] = {"4807.038", "N", "01131.000", "E", "123519", "A"};
    make_sentence_object(s, "GLL", gll_fields, 6);
    assert(connector.apply_sentence(s, model, 20, SensorSource::serial));
    assert(std::fabs(model.navigation.gps.fix_lat_deg.value - 48.1173f) < 0.0002f);

    const char* rmb_fields[] = {"A", "0.20", "L", "ORIG", "DEST", "4807.038", "N", "01131.000", "E", "2.5", "123.4", "5.6", "A"};
    make_sentence_object(s, "RMB", rmb_fields, 13);
    assert(connector.apply_sentence(s, model, 30, SensorSource::serial));
    assert(std::strcmp(model.navigation.rmb.destination_id, "DEST") == 0);
    assert(std::fabs(model.navigation.rmb.range_nmi.value - 2.5f) < 0.001f);

    const char* xdr_fields[] = {"A", "1.5", "D", "PTCH", "A", "-2.5", "D", "ROLL"};
    make_sentence_object(s, "XDR", xdr_fields, 8);
    assert(connector.apply_sentence(s, model, 40));
    assert(std::fabs(model.imu.pitch_deg.value - 1.5f) < 0.001f);
    assert(std::fabs(model.imu.roll_deg.value + 2.5f) < 0.001f);

    const char* rot_fields[] = {"120.0", "A"};
    make_sentence_object(s, "ROT", rot_fields, 2);
    assert(connector.apply_sentence(s, model, 50));
    assert(std::fabs(model.imu.heading_rate_lowpass_deg_s.value - 2.0f) < 0.001f);

    const char* mwd_fields[] = {"270.0", "T", "271.0", "M", "12.5", "N", "6.4", "M"};
    make_sentence_object(s, "MWD", mwd_fields, 8);
    assert(connector.apply_sentence(s, model, 60, SensorSource::serial));
    assert(std::fabs(model.wind.truewind.speed_kn.value - 12.5f) < 0.001f);

    const char* dbt_fields[] = {"36.0", "f", "10.9", "M", "6.0", "F"};
    make_sentence_object(s, "DBT", dbt_fields, 6);
    assert(connector.apply_sentence(s, model, 70, SensorSource::serial));
    assert(std::fabs(model.water.depth_m.value - 10.9f) < 0.001f);

    const char* dpt_fields[] = {"11.2", "0.4"};
    make_sentence_object(s, "DPT", dpt_fields, 2);
    assert(connector.apply_sentence(s, model, 80, SensorSource::serial));
    assert(std::fabs(model.water.depth_m.value - 11.2f) < 0.001f);
    assert(std::fabs(model.water.depth_offset_m.value - 0.4f) < 0.001f);
    return 0;
}
