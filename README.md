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
RMC, GGA, VTG
HDT, HDM, HDG
MWV, VWR, VWT
VHW, LWY, RSA
APB, XTE
```

pypilot-specific compatibility details:

```text
RSA input is sign-inverted into pypilot rudder angle.
APB XTE is clamped to +/-0.15 nmi and remembers sender ID plus compass/gps mode hint.
apply_sentence has a source-aware overload for serial/tcp/gpsd/signalk provenance.
```

## Supported output

Generic helpers:

```text
HDT, HDM, MWV, RSA, VHW
```

pypilot-style output helpers:

```text
APXDR pitch/roll
APROT heading-rate
APHDM heading
APMWV apparent/true wind
APRSA rudder with pypilot sign convention
```

## License

GPL-3.0-or-later.
