# pypilot-nmea0183-connector

Header-only C++11 NMEA 0183 connector for applying marine NMEA 0183 sentences to the typed pypilot data model and formatting NMEA output from that model.

The repository name and CMake target remain `pypilot_nmea0183_connector`, but the public include is:

```cpp
#include <nmea0183_connector.hpp>
```

Public C++ symbols live in:

```cpp
namespace nmea0183_connector
```

The connector is transport-agnostic. It works with byte streams and complete NMEA sentences supplied by caller code, but it does not own serial ports, sockets, WiFi, Ethernet, or pypilot TCP sessions.

It deliberately does **not** contain:

- serial drivers
- TCP/UDP sockets
- WiFi/Ethernet client objects
- pypilot TCP client protocol
- servo UART protocol
- autopilot control loops

## Dependency

The connector depends on the sibling `pypilot-data-model` headers. For local CMake builds, the default include path is:

```text
../pypilot-data-model/src
```

Application code normally includes the connector public header and lets it include the data model headers it needs.

## Public surface

The public include exposes:

- checksum helpers
- fixed-buffer NMEA sentence formatting
- scalar and span parsing helpers
- `NmeaSentence`
- `Nmea0183StreamParser`
- `Nmea0183RxConnector`
- NMEA TX formatter helpers

Internal implementation headers are intentionally coarse-grained:

```text
src/nmea0183_connector/nmea0183_helpers.hpp
src/nmea0183_connector/sentence_parser.hpp
src/nmea0183_connector/nmea_rx_connector.hpp
src/nmea0183_connector/nmea_tx_connector.hpp
src/nmea0183_connector.hpp
```

Applications, examples, and tests should include only:

```cpp
#include <nmea0183_connector.hpp>
```

## Parser model

`NmeaSentence` stores one raw sentence buffer. Body, talker, sentence id, and fields are `NmeaSpan` pointer/length views into that raw buffer. Tokenization does not copy the body and does not write temporary null terminators into fields. Numeric parsing uses a small local temporary only when converting a span to a number.

Use `sentence_is(sentence, "RMC")` to match the three-letter NMEA sentence id.

All supported input sentences are applied through `Nmea0183RxConnector::apply_sentence()`.

## Build on Linux

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Arduino example

```bash
arduino-cli compile --fqbn arduino:avr:mega --libraries . examples/arduino/Nmea0183ConnectorExample
```

The Arduino library metadata advertises the same public include:

```cpp
#include <nmea0183_connector.hpp>
```

## Supported input

```text
RMC, GGA, GLL, VTG
HDT, HDM, HDG
MWV, MWD, VWR, VWT
VHW, LWY, DBT, DPT, RSA
APB, RMB, XTE
XDR, ROT
```

pypilot-specific compatibility details:

```text
RSA input is sign-inverted into pypilot rudder angle.
APB XTE is clamped to +/-0.15 nmi and writes mode_hint plus sender_id into model.navigation.apb.
RMB writes route/waypoint fields into model.navigation.rmb and updates APB track/XTE hints.
XDR pitch/roll and ROT heading-rate are first-class inputs.
apply_sentence has a source-aware overload for serial/tcp/gpsd/signalk provenance.
```

## Supported output

Generic helpers:

```text
HDT, HDM, MWV, RSA, VHW, RMC
```

AP/pypilot-style output helpers use the `make_ap_` prefix:

```text
make_xdr_pitch / make_xdr_roll -> APXDR pitch/roll
make_rot                       -> APROT heading-rate
make_ap_hdm                    -> APHDM heading
make_ap_mwv                    -> APMWV apparent/true wind
make_ap_rsa                    -> APRSA rudder with pypilot sign convention
make_ap_rmc                    -> APRMC / GPRMC-compatible RMC output
```

## License

Apache-2.0.
