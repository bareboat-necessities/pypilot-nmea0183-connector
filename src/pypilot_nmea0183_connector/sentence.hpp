#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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

} // namespace pypilot_nmea0183_connector
