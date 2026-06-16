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

## Supported phase-0 input

```text
RMC, GGA, VTG
HDT, HDM, HDG
MWV, VWR, VWT
VHW, RSA
APB, XTE
```

## Supported phase-0 output

```text
HDT, HDM, MWV, RSA, VHW
```

## License

GPL-3.0-or-later.
