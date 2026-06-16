#pragma once

#include <pypilot_data_model.hpp>
#include "parse_helpers.hpp"

namespace pypilot_nmea0183_connector {

inline float wrap_360_deg(float angle_deg) {
    while (angle_deg >= 360.0f) angle_deg -= 360.0f;
    while (angle_deg < 0.0f) angle_deg += 360.0f;
    return angle_deg;
}

inline float wrap_180_deg(float angle_deg) {
    angle_deg = wrap_360_deg(angle_deg + 180.0f) - 180.0f;
    if (angle_deg <= -180.0f) angle_deg += 360.0f;
    return angle_deg;
}

template<typename Real = float>
class Nmea0183Connector {
public:
    Nmea0183Connector()
        : last_error_(""),
          last_apb_mode_(pypilot_data_model::AutopilotMode::gps) {
        last_apb_sender_id_[0] = '\0';
        last_apb_sender_id_[1] = '\0';
        last_apb_sender_id_[2] = '\0';
    }

    const char* last_error() const { return last_error_; }
    pypilot_data_model::AutopilotMode last_apb_mode() const { return last_apb_mode_; }
    const char* last_apb_sender_id() const { return last_apb_sender_id_; }

    bool apply_sentence(const NmeaSentence& sentence,
                        pypilot_data_model::DataModel<Real>& model,
                        uint64_t now_us) {
        return apply_sentence(sentence, model, now_us, pypilot_data_model::SensorSource::none);
    }

    bool apply_sentence(const NmeaSentence& sentence,
                        pypilot_data_model::DataModel<Real>& model,
                        uint64_t now_us,
                        pypilot_data_model::SensorSource source) {
        last_error_ = "";
        if (!sentence.valid_checksum) { last_error_ = "invalid checksum"; return false; }
        if (formatter_is(sentence, "RMC")) return apply_rmc(sentence, model, now_us, source);
        if (formatter_is(sentence, "GGA")) return apply_gga(sentence, model, now_us, source);
        if (formatter_is(sentence, "VTG")) return apply_vtg(sentence, model, now_us, source);
        if (formatter_is(sentence, "HDT")) return apply_hdt(sentence, model, now_us);
        if (formatter_is(sentence, "HDM")) return apply_hdm(sentence, model, now_us);
        if (formatter_is(sentence, "HDG")) return apply_hdg(sentence, model, now_us);
        if (formatter_is(sentence, "MWV")) return apply_mwv(sentence, model, now_us, source);
        if (formatter_is(sentence, "VWR")) return apply_vwr(sentence, model, now_us, source);
        if (formatter_is(sentence, "VWT")) return apply_vwt(sentence, model, now_us, source);
        if (formatter_is(sentence, "VHW")) return apply_vhw(sentence, model, now_us, source);
        if (formatter_is(sentence, "LWY")) return apply_lwy(sentence, model, now_us, source);
        if (formatter_is(sentence, "RSA")) return apply_rsa(sentence, model, now_us, source);
        if (formatter_is(sentence, "APB")) return apply_apb(sentence, model, now_us, source);
        if (formatter_is(sentence, "XTE")) return apply_xte(sentence, model, now_us, source);
        last_error_ = "unsupported sentence";
        return false;
    }

private:
    const char* last_error_;
    pypilot_data_model::AutopilotMode last_apb_mode_;
    char last_apb_sender_id_[3];

    template<typename Setting>
    void set_source(Setting& setting, pypilot_data_model::SensorSource source) {
        if (source != pypilot_data_model::SensorSource::none) setting.value = source;
    }

    void remember_sender(const NmeaSentence& s) {
        last_apb_sender_id_[0] = s.talker[0];
        last_apb_sender_id_[1] = s.talker[1];
        last_apb_sender_id_[2] = '\0';
    }

    template<typename Model>
    bool apply_rmc(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 8 || s.field(1)[0] != 'A') { last_error_ = "invalid RMC"; return false; }
        float lat = 0, lon = 0, sog = 0, cog = 0;
        if (parse_lat_lon(s.field(2), s.field(3), lat)) model.navigation.gps.fix_lat_deg.set(static_cast<Real>(lat), now_us);
        if (parse_lat_lon(s.field(4), s.field(5), lon)) model.navigation.gps.fix_lon_deg.set(static_cast<Real>(lon), now_us);
        if (parse_real(s.field(6), sog)) model.navigation.gps.speed_kn.set(static_cast<Real>(sog), now_us);
        if (parse_real(s.field(7), cog)) model.navigation.gps.track_deg.set(static_cast<Real>(wrap_360_deg(cog)), now_us);
        if (s.field_count >= 11) {
            float var = 0;
            if (parse_east_west_signed(s.field(9), s.field(10), var)) model.navigation.gps.declination_deg.set(static_cast<Real>(var), now_us);
        }
        set_source(model.navigation.gps.source, source);
        model.navigation.gps.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_gga(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 9 || s.field(5)[0] == '0') { last_error_ = "invalid GGA"; return false; }
        float lat = 0, lon = 0, alt = 0;
        if (parse_lat_lon(s.field(1), s.field(2), lat)) model.navigation.gps.fix_lat_deg.set(static_cast<Real>(lat), now_us);
        if (parse_lat_lon(s.field(3), s.field(4), lon)) model.navigation.gps.fix_lon_deg.set(static_cast<Real>(lon), now_us);
        if (parse_real(s.field(8), alt)) model.navigation.gps.fix_alt_m.set(static_cast<Real>(alt), now_us);
        set_source(model.navigation.gps.source, source);
        model.navigation.gps.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_vtg(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 7) { last_error_ = "short VTG"; return false; }
        float track = 0, speed = 0;
        if (parse_real(s.field(0), track)) model.navigation.gps.track_deg.set(static_cast<Real>(wrap_360_deg(track)), now_us);
        if (parse_knots(s.field(4), s.field(5), speed)) model.navigation.gps.speed_kn.set(static_cast<Real>(speed), now_us);
        else if (parse_knots(s.field(6), s.field(7), speed)) model.navigation.gps.speed_kn.set(static_cast<Real>(speed), now_us);
        set_source(model.navigation.gps.source, source);
        model.navigation.gps.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_hdt(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float heading = 0;
        if (!parse_real(s.field(0), heading)) { last_error_ = "bad HDT"; return false; }
        model.imu.heading_deg.set(static_cast<Real>(wrap_360_deg(heading)), now_us);
        return true;
    }

    template<typename Model>
    bool apply_hdm(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float heading = 0;
        if (!parse_real(s.field(0), heading)) { last_error_ = "bad HDM"; return false; }
        model.imu.heading_deg.set(static_cast<Real>(wrap_360_deg(heading)), now_us);
        return true;
    }

    template<typename Model>
    bool apply_hdg(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float heading = 0;
        if (!parse_real(s.field(0), heading)) { last_error_ = "bad HDG"; return false; }
        model.imu.heading_deg.set(static_cast<Real>(wrap_360_deg(heading)), now_us);
        float var = 0;
        if (s.field_count >= 5 && parse_east_west_signed(s.field(3), s.field(4), var)) {
            model.navigation.gps.declination_deg.set(static_cast<Real>(var), now_us);
        }
        return true;
    }

    template<typename Model>
    bool apply_mwv(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 5 || s.field(4)[0] != 'A') { last_error_ = "invalid MWV"; return false; }
        float angle = 0, speed = 0;
        if (!parse_real(s.field(0), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad MWV"; return false; }
        if (s.field(1)[0] == 'T') {
            model.wind.truewind.direction_deg.set(static_cast<Real>(wrap_180_deg(angle)), now_us);
            model.wind.truewind.speed_kn.set(static_cast<Real>(speed), now_us);
            set_source(model.wind.truewind.source, source);
            model.wind.truewind.last_update_us = now_us;
        } else {
            model.wind.apparent.direction_deg.set(static_cast<Real>(wrap_180_deg(angle)), now_us);
            model.wind.apparent.speed_kn.set(static_cast<Real>(speed), now_us);
            set_source(model.wind.apparent.source, source);
            model.wind.apparent.last_update_us = now_us;
        }
        return true;
    }

    template<typename Model>
    bool apply_vwr(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float angle = 0, speed = 0;
        if (!parse_left_right_signed(s.field(0), s.field(1), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad VWR"; return false; }
        model.wind.apparent.direction_deg.set(static_cast<Real>(angle), now_us);
        model.wind.apparent.speed_kn.set(static_cast<Real>(speed), now_us);
        set_source(model.wind.apparent.source, source);
        model.wind.apparent.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_vwt(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float angle = 0, speed = 0;
        if (!parse_left_right_signed(s.field(0), s.field(1), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad VWT"; return false; }
        model.wind.truewind.direction_deg.set(static_cast<Real>(angle), now_us);
        model.wind.truewind.speed_kn.set(static_cast<Real>(speed), now_us);
        set_source(model.wind.truewind.source, source);
        model.wind.truewind.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_vhw(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float speed = 0;
        if (parse_knots(s.field(4), s.field(5), speed)) {
            model.water.speed_kn.set(static_cast<Real>(speed), now_us);
            set_source(model.water.source, source);
            model.water.last_update_us = now_us;
            return true;
        }
        last_error_ = "bad VHW";
        return false;
    }

    template<typename Model>
    bool apply_lwy(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float leeway = 0;
        if (s.field_count >= 2 && s.field(0)[0] == 'A' && parse_real(s.field(1), leeway)) {
            model.water.leeway_deg.set(static_cast<Real>(leeway), now_us);
            set_source(model.water.leeway_source, source);
            model.water.last_update_us = now_us;
            return true;
        }
        last_error_ = "bad LWY";
        return false;
    }

    template<typename Model>
    bool apply_rsa(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float angle = 0;
        if (s.field_count >= 2 && s.field(1)[0] == 'A' && parse_real(s.field(0), angle)) {
            model.rudder.angle_deg.set(static_cast<Real>(-angle), now_us);
            set_source(model.rudder.source, source);
            model.rudder.last_update_us = now_us;
            return true;
        }
        if (s.field_count >= 4 && s.field(3)[0] == 'A' && parse_real(s.field(2), angle)) {
            model.rudder.angle_deg.set(static_cast<Real>(-angle), now_us);
            set_source(model.rudder.source, source);
            model.rudder.last_update_us = now_us;
            return true;
        }
        last_error_ = "bad RSA";
        return false;
    }

    template<typename Model>
    bool apply_xte(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        float xte = 0;
        if (s.field_count < 5 || s.field(0)[0] != 'A' || s.field(1)[0] != 'A' || !parse_left_right_signed(s.field(2), s.field(3), xte)) { last_error_ = "bad XTE"; return false; }
        model.navigation.apb.xte_nmi.set(static_cast<Real>(xte), now_us);
        set_source(model.navigation.apb.source, source);
        model.navigation.apb.last_update_us = now_us;
        return true;
    }

    template<typename Model>
    bool apply_apb(const NmeaSentence& s, Model& model, uint64_t now_us, pypilot_data_model::SensorSource source) {
        if (s.field_count < 14) { last_error_ = "short APB"; return false; }
        bool any = false;
        float xte = 0;
        if (parse_left_right_signed(s.field(2), s.field(3), xte)) {
            if (xte > 0.15f) xte = 0.15f;
            if (xte < -0.15f) xte = -0.15f;
            model.navigation.apb.xte_nmi.set(static_cast<Real>(xte), now_us);
            any = true;
        }
        float track = 0;
        if (parse_real(s.field(12), track)) {
            model.navigation.apb.track_deg.set(static_cast<Real>(wrap_360_deg(track)), now_us);
            any = true;
        }
        last_apb_mode_ = s.field(13)[0] == 'M' ? pypilot_data_model::AutopilotMode::compass : pypilot_data_model::AutopilotMode::gps;
        remember_sender(s);
        set_source(model.navigation.apb.source, source);
        model.navigation.apb.last_update_us = now_us;
        if (!any) last_error_ = "bad APB";
        return any;
    }
};

} // namespace pypilot_nmea0183_connector
