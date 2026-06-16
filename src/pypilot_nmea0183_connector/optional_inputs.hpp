#pragma once

#include <string.h>
#include <pypilot_data_model.hpp>
#include "sentence.hpp"
#include "parse_helpers.hpp"

namespace pypilot_nmea0183_connector {

struct OptionalInputOptions { bool enable_optional_inputs = false; };

inline float optional_wrap_360(float v) { while (v >= 360.0f) v -= 360.0f; while (v < 0.0f) v += 360.0f; return v; }
inline float optional_wrap_180(float v) { v = optional_wrap_360(v + 180.0f) - 180.0f; if (v <= -180.0f) v += 360.0f; return v; }
inline void optional_copy_id(char* out, size_t out_size, const char* value) { if (!out || out_size == 0) return; size_t i = 0; for (; i + 1 < out_size && value && value[i]; ++i) out[i] = value[i]; out[i] = '\0'; }
template<typename Setting> inline void optional_set_source(Setting& setting, pypilot_data_model::SensorSource source) { if (source != pypilot_data_model::SensorSource::none) setting.value = source; }

template<typename Real = float>
class OptionalInputApplier {
public:
    OptionalInputApplier() : options_(), last_error_("") {}
    explicit OptionalInputApplier(const OptionalInputOptions& options) : options_(options), last_error_("") {}
    void set_enable_optional_inputs(bool enabled) { options_.enable_optional_inputs = enabled; }
    bool optional_inputs_enabled() const { return options_.enable_optional_inputs; }
    const char* last_error() const { return last_error_; }

    bool apply_optional_sentence(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source = pypilot_data_model::SensorSource::none) {
        last_error_ = "";
        if (!options_.enable_optional_inputs) { last_error_ = "optional input disabled"; return false; }
        if (!s.valid_checksum) { last_error_ = "invalid checksum"; return false; }
        if (formatter_is(s, "GLL")) return apply_gll(s, model, now_us, source);
        if (formatter_is(s, "RMB")) return apply_rmb(s, model, now_us, source);
        if (formatter_is(s, "XDR")) return apply_xdr(s, model, now_us);
        if (formatter_is(s, "ROT")) return apply_rot(s, model, now_us);
        if (formatter_is(s, "MWD")) return apply_mwd(s, model, now_us, source);
        if (formatter_is(s, "DBT")) return apply_dbt(s, model, now_us, source);
        if (formatter_is(s, "DPT")) return apply_dpt(s, model, now_us, source);
        last_error_ = "unsupported optional sentence";
        return false;
    }

private:
    OptionalInputOptions options_;
    const char* last_error_;

    bool apply_gll(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 6 || s.field(5)[0] != 'A') { last_error_ = "bad GLL"; return false; }
        float lat = 0, lon = 0;
        if (parse_lat_lon(s.field(0), s.field(1), lat)) model.navigation.gps.fix_lat_deg.set(static_cast<Real>(lat), now_us);
        if (parse_lat_lon(s.field(2), s.field(3), lon)) model.navigation.gps.fix_lon_deg.set(static_cast<Real>(lon), now_us);
        optional_set_source(model.navigation.gps.source, source);
        model.navigation.gps.last_update_us = now_us;
        return true;
    }

    bool apply_rmb(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 13 || s.field(0)[0] != 'A') { last_error_ = "bad RMB"; return false; }
        float xte = 0, lat = 0, lon = 0, range = 0, bearing = 0, vmg = 0;
        if (parse_left_right_signed(s.field(1), s.field(2), xte)) {
            model.navigation.rmb.xte_nmi.set(static_cast<Real>(xte), now_us);
            model.navigation.apb.xte_nmi.set(static_cast<Real>(xte), now_us);
        }
        optional_copy_id(model.navigation.rmb.origin_id, sizeof(model.navigation.rmb.origin_id), s.field(3));
        optional_copy_id(model.navigation.rmb.destination_id, sizeof(model.navigation.rmb.destination_id), s.field(4));
        if (parse_lat_lon(s.field(5), s.field(6), lat)) model.navigation.rmb.destination_lat_deg.set(static_cast<Real>(lat), now_us);
        if (parse_lat_lon(s.field(7), s.field(8), lon)) model.navigation.rmb.destination_lon_deg.set(static_cast<Real>(lon), now_us);
        if (parse_real(s.field(9), range)) model.navigation.rmb.range_nmi.set(static_cast<Real>(range), now_us);
        if (parse_real(s.field(10), bearing)) {
            model.navigation.rmb.bearing_deg.set(static_cast<Real>(optional_wrap_360(bearing)), now_us);
            model.navigation.apb.track_deg.set(static_cast<Real>(optional_wrap_360(bearing)), now_us);
        }
        if (parse_real(s.field(11), vmg)) model.navigation.rmb.closing_velocity_kn.set(static_cast<Real>(vmg), now_us);
        model.navigation.rmb.arrived.value = s.field(12)[0] == 'A';
        optional_set_source(model.navigation.rmb.source, source);
        optional_set_source(model.navigation.apb.source, source);
        model.navigation.rmb.last_update_us = now_us;
        model.navigation.apb.last_update_us = now_us;
        return true;
    }

    bool apply_xdr(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us) {
        bool any = false;
        for (uint8_t i = 0; i + 3 < s.field_count; i += 4) {
            float v = 0;
            if (s.field(i)[0] == 'A' && parse_real(s.field(i + 1), v) && s.field(i + 2)[0] == 'D') {
                if (strcmp(s.field(i + 3), "PTCH") == 0) { model.imu.pitch_deg.set(static_cast<Real>(v), now_us); any = true; }
                else if (strcmp(s.field(i + 3), "ROLL") == 0) { model.imu.roll_deg.set(static_cast<Real>(v), now_us); any = true; }
            }
        }
        if (!any) last_error_ = "bad XDR";
        return any;
    }

    bool apply_rot(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us) {
        float rate_min = 0;
        if (s.field_count < 2 || s.field(1)[0] != 'A' || !parse_real(s.field(0), rate_min)) { last_error_ = "bad ROT"; return false; }
        model.imu.heading_rate_lowpass_deg_s.set(static_cast<Real>(rate_min / 60.0f), now_us);
        return true;
    }

    bool apply_mwd(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float direction = 0, speed = 0;
        if (s.field_count < 7 || !parse_real(s.field(0), direction) || !parse_knots(s.field(4), s.field(5), speed)) { last_error_ = "bad MWD"; return false; }
        model.wind.truewind.direction_deg.set(static_cast<Real>(optional_wrap_180(direction)), now_us);
        model.wind.truewind.speed_kn.set(static_cast<Real>(speed), now_us);
        optional_set_source(model.wind.truewind.source, source);
        model.wind.truewind.last_update_us = now_us;
        return true;
    }

    bool apply_dbt(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float depth = 0;
        if (s.field_count >= 4 && parse_real(s.field(2), depth)) {
            model.water.depth_m.set(static_cast<Real>(depth), now_us);
            optional_set_source(model.water.depth_source, source);
            model.water.last_update_us = now_us;
            return true;
        }
        last_error_ = "bad DBT";
        return false;
    }

    bool apply_dpt(const NmeaSentence& s, pypilot_data_model::DataModel<Real>& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float depth = 0, offset = 0;
        if (!parse_real(s.field(0), depth)) { last_error_ = "bad DPT"; return false; }
        model.water.depth_m.set(static_cast<Real>(depth), now_us);
        if (parse_real(s.field(1), offset)) model.water.depth_offset_m.set(static_cast<Real>(offset), now_us);
        optional_set_source(model.water.depth_source, source);
        model.water.last_update_us = now_us;
        return true;
    }
};

} // namespace pypilot_nmea0183_connector
