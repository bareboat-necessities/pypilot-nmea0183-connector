#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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

} // namespace pypilot_nmea0183_connector
