#pragma once

#include <string.h>
#include "parse_helpers.hpp"
#include "sentence.hpp"

namespace pypilot_nmea0183_connector {

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
