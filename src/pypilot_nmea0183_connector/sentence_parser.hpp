#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "nmea0183_helpers.hpp"

namespace pypilot_nmea0183_connector {

static const uint8_t NMEA_MAX_SENTENCE_LEN = 96;
static const uint8_t NMEA_MAX_FIELDS = 32;

struct NmeaSentence {
    char raw[NMEA_MAX_SENTENCE_LEN];
    char body[NMEA_MAX_SENTENCE_LEN];
    char talker[3];
    char formatter[6];
    char* fields[NMEA_MAX_FIELDS];
    uint8_t field_count;
    bool valid_checksum;
    char start_char;

    void clear() {
        raw[0] = '\0';
        body[0] = '\0';
        talker[0] = '\0'; talker[1] = '\0'; talker[2] = '\0';
        formatter[0] = '\0';
        field_count = 0;
        valid_checksum = false;
        start_char = '$';
        for (uint8_t i = 0; i < NMEA_MAX_FIELDS; ++i) fields[i] = 0;
    }

    const char* field(uint8_t index) const {
        return index < field_count ? fields[index] : "";
    }
};

inline bool formatter_is(const NmeaSentence& s, const char* fmt) {
    return fmt && strcmp(s.formatter, fmt) == 0;
}

inline bool talker_is(const NmeaSentence& s, const char* talker) {
    return talker && strcmp(s.talker, talker) == 0;
}

class Nmea0183StreamParser {
public:
    Nmea0183StreamParser() { reset(); }

    void reset() {
        pos_ = 0;
        collecting_ = false;
        last_error_ = "";
        buffer_[0] = '\0';
    }

    const char* last_error() const { return last_error_; }

    bool push(char c, NmeaSentence& out) {
        if (c == '$' || c == '!') {
            collecting_ = true;
            pos_ = 0;
            buffer_[pos_++] = c;
            buffer_[pos_] = '\0';
            return false;
        }
        if (!collecting_) return false;
        if (c == '\r') return false;
        if (c == '\n') {
            buffer_[pos_] = '\0';
            collecting_ = false;
            return parse_line(buffer_, out);
        }
        if (pos_ + 1 >= NMEA_MAX_SENTENCE_LEN) {
            reset();
            last_error_ = "sentence too long";
            return false;
        }
        buffer_[pos_++] = c;
        buffer_[pos_] = '\0';
        return false;
    }

    bool parse_line(const char* line, NmeaSentence& out) {
        out.clear();
        if (!line || (line[0] != '$' && line[0] != '!')) { last_error_ = "bad start"; return false; }
        const char* star = strchr(line, '*');
        if (!star) { last_error_ = "missing checksum"; return false; }
        uint8_t supplied = 0;
        if (!parse_checksum_hex(star + 1, supplied)) { last_error_ = "bad checksum field"; return false; }
        uint8_t computed = nmea_checksum_range(line + 1, star);
        if (computed != supplied) { last_error_ = "checksum mismatch"; return false; }

        size_t raw_len = strlen(line);
        if (raw_len >= NMEA_MAX_SENTENCE_LEN) raw_len = NMEA_MAX_SENTENCE_LEN - 1;
        memcpy(out.raw, line, raw_len);
        out.raw[raw_len] = '\0';
        out.start_char = line[0];
        out.valid_checksum = true;

        size_t body_len = static_cast<size_t>(star - (line + 1));
        if (body_len >= NMEA_MAX_SENTENCE_LEN) body_len = NMEA_MAX_SENTENCE_LEN - 1;
        memcpy(out.body, line + 1, body_len);
        out.body[body_len] = '\0';

        if (body_len < 5) { last_error_ = "short body"; return false; }
        out.talker[0] = out.body[0];
        out.talker[1] = out.body[1];
        out.talker[2] = '\0';
        out.formatter[0] = out.body[2];
        out.formatter[1] = out.body[3];
        out.formatter[2] = out.body[4];
        out.formatter[3] = '\0';

        char* p = out.body;
        char* comma = strchr(p, ',');
        if (!comma) return true;
        *comma = '\0';
        p = comma + 1;
        while (out.field_count < NMEA_MAX_FIELDS) {
            out.fields[out.field_count++] = p;
            comma = strchr(p, ',');
            if (!comma) break;
            *comma = '\0';
            p = comma + 1;
        }
        last_error_ = "";
        return true;
    }

private:
    char buffer_[NMEA_MAX_SENTENCE_LEN];
    size_t pos_;
    bool collecting_;
    const char* last_error_;
};

} // namespace pypilot_nmea0183_connector
