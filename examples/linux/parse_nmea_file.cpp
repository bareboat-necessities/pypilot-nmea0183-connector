#include <fstream>
#include <iostream>
#include <pypilot_data_model.hpp>
#include <pypilot_nmea0183_connector.hpp>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " file.nmea\n";
        return 2;
    }
    std::ifstream in(argv[1]);
    if (!in) return 1;

    pypilot_data_model::DataModel<> model;
    pypilot_nmea0183_connector::Nmea0183StreamParser parser;
    pypilot_nmea0183_connector::Nmea0183RxConnector<> connector;
    pypilot_nmea0183_connector::NmeaSentence sentence;

    uint64_t now_us = 0;
    char c = 0;
    while (in.get(c)) {
        if (parser.push(c, sentence)) {
            connector.apply_sentence(sentence, model, now_us += 100000);
        }
    }

    std::cout << "gps.speed_kn=" << model.navigation.gps.speed_kn.value << "\n";
    std::cout << "gps.track_deg=" << model.navigation.gps.track_deg.value << "\n";
    std::cout << "wind.apparent.speed_kn=" << model.wind.apparent.speed_kn.value << "\n";
    return 0;
}
