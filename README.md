# pypilot-nmea0183-connector

Header-only C++11 NMEA 0183 connector for pypilot ports on Arduino and Linux.

This project converts between NMEA 0183 byte streams/sentences and typed `pypilot-data-model` fields. It is transport-agnostic and uses fixed buffers in the core.

It deliberately does **not** contain:

- serial drivers
- TCP/UDP sockets
- WiFi/Ethernet client objects
- pypilot TCP client protocol
- servo UART protocol
- autopilot control loops

## Public include

Use the same include on Linux and Arduino:

```cpp
#include <pypilot_nmea0183_connector.hpp>
```

The public include exposes NMEA 0183 helpers, sentence parsing, stream parsing, first-class input application, and output formatting through one compact surface.

Internal source headers are intentionally coarse-grained:

```text
nmea0183_helpers.hpp       checksum and scalar/field parse helpers; no sentence dependency
sentence_parser.hpp        NmeaSentence plus Nmea0183StreamParser
apply_sentence_pypilot.hpp pypilot-data-model input applier
sentence_formatter.hpp     NMEA output formatter
```

All supported input sentences are applied through `Nmea0183Connector::apply_sentence()`; there is no separate optional-input applier.

## Dependencies

The connector expects this sibling project or include path:

- `pypilot-data-model`

For local CMake builds, default is:

```text
../pypilot-data-model/src
```

## Build on Linux

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
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

GPL-3.0-or-later.
