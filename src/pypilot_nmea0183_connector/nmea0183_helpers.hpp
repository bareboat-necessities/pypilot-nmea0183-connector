#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace pypilot_nmea0183_connector {

inline uint8_t from_hex(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + c - 'A');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + c - 'a');
    return 0xff;
}

inline char to_hex(uint8_t v) {
    static const char* hex = "0123456789ABCDEF";
    return hex[v & 0x0f];
}

inline uint8_t nmea_checksum_body(const char* body) {
    uint8_t cs = 0;
    if (!body) return cs;
    while (*body) cs ^= static_cast<uint8_t>(*body++);
    return cs;
}

inline uint8_t nmea_checksum_range(const char* begin, const char* end) {
    uint8_t cs = 0;
    while (begin && begin < end) cs ^= static_cast<uint8_t>(*begin++);
    return cs;
}

inline bool parse_checksum_hex(const char* two_hex, uint8_t& out) {
    if (!two_hex || !two_hex[0] || !two_hex[1]) return false;
    uint8_t hi = from_hex(two_hex[0]);
    uint8_t lo = from_hex(two_hex[1]);
    if (hi > 0x0f || lo > 0x0f) return false;
    out = static_cast<uint8_t>((hi << 4) | lo);
    return true;
}

inline bool verify_nmea_checksum(const char* line) {
    if (!line || (*line != '$' && *line != '!')) return false;
    const char* star = strchr(line, '*');
    if (!star) return false;
    uint8_t expected = 0;
    if (!parse_checksum_hex(star + 1, expected)) return false;
    return nmea_checksum_range(line + 1, star) == expected;
}

inline bool parse_real(const char* field, float& out) {
    if (!field || !field[0]) return false;
    char* end = 0;
    double v = strtod(field, &end);
    if (end == field) return false;
    out = static_cast<float>(v);
    return true;
}

inline bool parse_char_field(const char* field, char& out) {
    if (!field || !field[0]) return false;
    out = field[0];
    return true;
}

inline bool parse_degrees(const char* field, float& out_deg) {
    return parse_real(field, out_deg);
}

inline bool parse_knots(const char* value, const char* unit, float& out_kn) {
    float v = 0.0f;
    if (!parse_real(value, v)) return false;
    char u = unit && unit[0] ? unit[0] : 'N';
    if (u == 'N') out_kn = v;
    else if (u == 'K') out_kn = v * 0.539956803f;
    else if (u == 'M') out_kn = v * 1.94384449f;
    else return false;
    return true;
}

inline bool parse_lat_lon(const char* value, const char* hemi, float& out_deg) {
    if (!value || !value[0] || !hemi || !hemi[0]) return false;
    float raw = 0.0f;
    if (!parse_real(value, raw)) return false;
    int degrees = static_cast<int>(raw / 100.0f);
    float minutes = raw - static_cast<float>(degrees * 100);
    float deg = static_cast<float>(degrees) + minutes / 60.0f;
    if (hemi[0] == 'S' || hemi[0] == 'W') deg = -deg;
    else if (!(hemi[0] == 'N' || hemi[0] == 'E')) return false;
    out_deg = deg;
    return true;
}

inline bool parse_left_right_signed(const char* magnitude, const char* side, float& out) {
    float v = 0.0f;
    if (!parse_real(magnitude, v) || !side || !side[0]) return false;
    if (side[0] == 'L') out = -v;
    else if (side[0] == 'R') out = v;
    else return false;
    return true;
}

inline bool parse_east_west_signed(const char* magnitude, const char* side, float& out) {
    float v = 0.0f;
    if (!parse_real(magnitude, v) || !side || !side[0]) return false;
    if (side[0] == 'E') out = v;
    else if (side[0] == 'W') out = -v;
    else return false;
    return true;
}

} // namespace pypilot_nmea0183_connector
