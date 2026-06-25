#pragma once

#include "pypilot_nmea0183_connector/nmea0183_helpers.hpp"
#include "pypilot_nmea0183_connector/sentence_parser.hpp"
#include "pypilot_nmea0183_connector/nmea_rx_connector.hpp"
#include "pypilot_nmea0183_connector/nmea_tx_connector.hpp"

namespace pypilot_nmea0183_connector {

template<typename Real = float>
using Nmea0183Connector = Nmea0183RxConnector<Real>;

} // namespace pypilot_nmea0183_connector
