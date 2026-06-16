#pragma once

#include <stdio.h>
#include <string.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "checksum.hpp"
#include "parse_helpers.hpp"

namespace pypilot_nmea0183_connector {

inline void format_real(char* out, size_t out_size, float value, unsigned decimals = 1) {
    if (!out || out_size == 0) return;
#ifdef ARDUINO
    dtostrf(value, 0, decimals, out);
    char* first = out;
    while (*first == ' ') ++first;
    if (first != out) memmove(out, first, strlen(first) + 1);
#else
    snprintf(out, out_size, "%.*f", static_cast<int>(decimals), static_cast<double>(value));
#endif
}

inline size_t make_sentence(char* out, size_t out_size, const char* body, char start_char = '$') {
    if (!out || out_size == 0 || !body) return 0;
    uint8_t cs = nmea_checksum_body(body);
    int n = snprintf(out, out_size, "%c%s*%c%c\r\n", start_char, body, to_hex(cs >> 4), to_hex(cs));
    if (n < 0) return 0;
    return static_cast<size_t>(n) < out_size ? static_cast<size_t>(n) : out_size - 1;
}

inline size_t make_hdt(char* out, size_t out_size, float heading_deg, const char* talker = "II") {
    char h[24];
    char body[64];
    format_real(h, sizeof(h), heading_deg, 1);
    snprintf(body, sizeof(body), "%sHDT,%s,T", talker, h);
    return make_sentence(out, out_size, body);
}

inline size_t make_hdm(char* out, size_t out_size, float heading_deg, const char* talker = "II") {
    char h[24];
    char body[64];
    format_real(h, sizeof(h), heading_deg, 1);
    snprintf(body, sizeof(body), "%sHDM,%s,M", talker, h);
    return make_sentence(out, out_size, body);
}

inline size_t make_mwv(char* out, size_t out_size, float angle_deg, float speed_kn, bool apparent = true, const char* talker = "II") {
    char a[24];
    char s[24];
    char body[80];
    format_real(a, sizeof(a), angle_deg, 1);
    format_real(s, sizeof(s), speed_kn, 1);
    snprintf(body, sizeof(body), "%sMWV,%s,%c,%s,N,A", talker, a, apparent ? 'R' : 'T', s);
    return make_sentence(out, out_size, body);
}

inline size_t make_rsa(char* out, size_t out_size, float rudder_deg, const char* talker = "II") {
    char r[24];
    char body[80];
    format_real(r, sizeof(r), rudder_deg, 1);
    snprintf(body, sizeof(body), "%sRSA,%s,A,,V", talker, r);
    return make_sentence(out, out_size, body);
}

inline size_t make_vhw(char* out, size_t out_size, float water_speed_kn, const char* talker = "II") {
    char s[24];
    char body[80];
    format_real(s, sizeof(s), water_speed_kn, 1);
    snprintf(body, sizeof(body), "%sVHW,,,,,%s,N,,K", talker, s);
    return make_sentence(out, out_size, body);
}

} // namespace pypilot_nmea0183_connector
