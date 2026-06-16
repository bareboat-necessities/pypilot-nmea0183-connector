#include <cassert>
#include <pypilot_nmea0183_connector.hpp>
#include <pypilot_nmea0183_connector/optional_inputs.hpp>
#include <pypilot_data_model.hpp>

using namespace pypilot_nmea0183_connector;
using namespace pypilot_data_model;

int main() {
    DataModel<> model;
    OptionalInputApplier<> optional;
    NmeaSentence sentence;
    sentence.clear();
    sentence.valid_checksum = true;
    sentence.talker[0] = 'G';
    sentence.talker[1] = 'P';
    sentence.talker[2] = '\0';
    sentence.formatter[0] = 'G';
    sentence.formatter[1] = 'L';
    sentence.formatter[2] = 'L';
    sentence.formatter[3] = '\0';
    assert(!optional.optional_inputs_enabled());
    assert(!optional.apply_optional_sentence(sentence, model, 10, SensorSource::serial));
    optional.set_enable_optional_inputs(true);
    assert(optional.optional_inputs_enabled());
    return 0;
}
