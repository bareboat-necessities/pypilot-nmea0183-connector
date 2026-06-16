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
    Nmea0183Connector() : last_error_("") {}

    const char* last_error() const { return last_error_; }

    bool apply_sentence(const NmeaSentence& sentence,
                        pypilot_data_model::DataModel<Real>& model,
                        uint64_t now_us) {
        last_error_ = "";
        if (!sentence.valid_checksum) { last_error_ = "invalid checksum"; return false; }
        if (formatter_is(sentence, "RMC")) return apply_rmc(sentence, model, now_us);
        if (formatter_is(sentence, "GGA")) return apply_gga(sentence, model, now_us);
        if (formatter_is(sentence, "VTG")) return apply_vtg(sentence, model, now_us);
        if (formatter_is(sentence, "HDT")) return apply_hdt(sentence, model, now_us);
        if (formatter_is(sentence, "HDM")) return apply_hdm(sentence, model, now_us);
        if (formatter_is(sentence, "HDG")) return apply_hdg(sentence, model, now_us);
        if (formatter_is(sentence, "MWV")) return apply_mwv(sentence, model, now_us);
        if (formatter_is(sentence, "VWR")) return apply_vwr(sentence, model, now_us);
        if (formatter_is(sentence, "VWT")) return apply_vwt(sentence, model, now_us);
        if (formatter_is(sentence, "VHW")) return apply_vhw(sentence, model, now_us);
        if (formatter_is(sentence, "RSA")) return apply_rsa(sentence, model, now_us);
        if (formatter_is(sentence, "APB")) return apply_apb(sentence, model, now_us);
        if (formatter_is(sentence, "XTE")) return apply_xte(sentence, model, now_us);
        last_error_ = "unsupported sentence";
        return false;
    }

private:
    const char* last_error_;

    template<typename Model>
    bool apply_rmc(const NmeaSentence& s, Model& model, uint64_t now_us) {
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
        return true;
    }

    template<typename Model>
    bool apply_gga(const NmeaSentence& s, Model& model, uint64_t now_us) {
        if (s.field_count < 9 || s.field(5)[0] == '0') { last_error_ = "invalid GGA"; return false; }
        float lat = 0, lon = 0, alt = 0;
        if (parse_lat_lon(s.field(1), s.field(2), lat)) model.navigation.gps.fix_lat_deg.set(static_cast<Real>(lat), now_us);
        if (parse_lat_lon(s.field(3), s.field(4), lon)) model.navigation.gps.fix_lon_deg.set(static_cast<Real>(lon), now_us);
        if (parse_real(s.field(8), alt)) model.navigation.gps.fix_alt_m.set(static_cast<Real>(alt), now_us);
        return true;
    }

    template<typename Model>
    bool apply_vtg(const NmeaSentence& s, Model& model, uint64_t now_us) {
        if (s.field_count < 7) { last_error_ = "short VTG"; return false; }
        float track = 0, speed = 0;
        if (parse_real(s.field(0), track)) model.navigation.gps.track_deg.set(static_cast<Real>(wrap_360_deg(track)), now_us);
        if (parse_knots(s.field(4), s.field(5), speed)) model.navigation.gps.speed_kn.set(static_cast<Real>(speed), now_us);
        else if (parse_knots(s.field(6), s.field(7), speed)) model.navigation.gps.speed_kn.set(static_cast<Real>(speed), now_us);
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
    bool apply_mwv(const NmeaSentence& s, Model& model, uint64_t now_us) {
        if (s.field_count < 5 || s.field(4)[0] != 'A') { last_error_ = "invalid MWV"; return false; }
        float angle = 0, speed = 0;
        if (!parse_real(s.field(0), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad MWV"; return false; }
        if (s.field(1)[0] == 'T') {
            model.wind.truewind.direction_deg.set(static_cast<Real>(wrap_180_deg(angle)), now_us);
            model.wind.truewind.speed_kn.set(static_cast<Real>(speed), now_us);
        } else {
            model.wind.apparent.direction_deg.set(static_cast<Real>(wrap_180_deg(angle)), now_us);
            model.wind.apparent.speed_kn.set(static_cast<Real>(speed), now_us);
        }
        return true;
    }

    template<typename Model>
    bool apply_vwr(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float angle = 0, speed = 0;
        if (!parse_left_right_signed(s.field(0), s.field(1), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad VWR"; return false; }
        model.wind.apparent.direction_deg.set(static_cast<Real>(angle), now_us);
        model.wind.apparent.speed_kn.set(static_cast<Real>(speed), now_us);
        return true;
    }

    template<typename Model>
    bool apply_vwt(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float angle = 0, speed = 0;
        if (!parse_left_right_signed(s.field(0), s.field(1), angle) || !parse_knots(s.field(2), s.field(3), speed)) { last_error_ = "bad VWT"; return false; }
        model.wind.truewind.direction_deg.set(static_cast<Real>(angle), now_us);
        model.wind.truewind.speed_kn.set(static_cast<Real>(speed), now_us);
        return true;
    }

    template<typename Model>
    bool apply_vhw(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float speed = 0;
        if (parse_knots(s.field(4), s.field(5), speed)) {
            model.water.speed_kn.set(static_cast<Real>(speed), now_us);
            return true;
        }
        last_error_ = "bad VHW";
        return false;
    }

    template<typename Model>
    bool apply_rsa(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float angle = 0;
        if (s.field_count >= 2 && s.field(1)[0] == 'A' && parse_real(s.field(0), angle)) {
            model.rudder.angle_deg.set(static_cast<Real>(angle), now_us);
            return true;
        }
        if (s.field_count >= 4 && s.field(3)[0] == 'A' && parse_real(s.field(2), angle)) {
            model.rudder.angle_deg.set(static_cast<Real>(angle), now_us);
            return true;
        }
        last_error_ = "bad RSA";
        return false;
    }

    template<typename Model>
    bool apply_xte(const NmeaSentence& s, Model& model, uint64_t now_us) {
        float xte = 0;
        if (s.field_count < 5 || s.field(0)[0] != 'A' || s.field(1)[0] != 'A' || !parse_left_right_signed(s.field(2), s.field(3), xte)) { last_error_ = "bad XTE"; return false; }
        model.navigation.apb.xte_nmi.set(static_cast<Real>(xte), now_us);
        return true;
    }

    template<typename Model>
    bool apply_apb(const NmeaSentence& s, Model& model, uint64_t now_us) {
        bool any = false;
        float xte = 0;
        if (s.field_count >= 5 && parse_left_right_signed(s.field(2), s.field(3), xte)) {
            model.navigation.apb.xte_nmi.set(static_cast<Real>(xte), now_us);
            any = true;
        }
        float track = 0;
        if (s.field_count >= 14 && parse_real(s.field(11), track)) {
            model.navigation.apb.track_deg.set(static_cast<Real>(wrap_360_deg(track)), now_us);
            any = true;
        } else if (s.field_count >= 12 && parse_real(s.field(9), track)) {
            model.navigation.apb.track_deg.set(static_cast<Real>(wrap_360_deg(track)), now_us);
            any = true;
        }
        if (!any) last_error_ = "bad APB";
        return any;
    }
};

} // namespace pypilot_nmea0183_connector
