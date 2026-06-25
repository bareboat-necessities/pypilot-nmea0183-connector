# NMEA 0183 UDP Address and Port Reference

This file is a practical discovery reference for NMEA 0183 data sent over UDP by commercial marine devices, gateways, chartplotters, and Wi-Fi bridges.

It is not a formal NMEA standard. Defaults vary by vendor, firmware version, product mode, and user configuration. For receive code, the safest pattern is usually to bind to `0.0.0.0:<port>` and accept datagrams from whichever source IP appears.

## Standards and conventions

- `10110/tcp` and `10110/udp` are the IANA-registered ports for `nmea-0183`.
- `10111/udp` is registered for NMEA OneNet and should not be treated as normal NMEA 0183.
- NMEA 0183 over IP usually appears as UDP broadcast, UDP unicast, or TCP. True multicast is uncommon and not standardized for NMEA 0183.
- When multicast is manually configured, use administratively scoped multicast ranges rather than public/reserved groups. A practical private range to scan or support is `239.192.0.0/14`; for example, `239.194.4.4`.

## Highest-value UDP ports to listen on

| UDP port | Address/source pattern to expect | Common devices / families | Notes |
|---:|---|---|---|
| `10110` | Listen on `0.0.0.0:10110`; destination may be `255.255.255.255` or subnet broadcast such as `192.168.4.255` | Generic NMEA 0183 gateways, OpenCPN/kplex repeaters, ShipModul MiniPlex, SWAY Breeze, Actisense W2K when configured, many Navico/GoFree setups | De facto standard NMEA 0183 over IP port. |
| `2000` | Often source/AP `192.168.1.1`; listen on `0.0.0.0:2000` | Digital Yacht WLN/NavLink/iAIS families, Navionics-oriented gateways, some Yacht Devices configurations, some Quark setups | Very common in mobile chart app integrations. |
| `11101` | Source/AP often `10.10.10.1`; listen on `0.0.0.0:11101` | PredictWind DataHub / DataHub Pro | PredictWind commonly broadcasts NMEA 0183 over UDP `11101`; TCP companion is often `11102`. |
| `5000` | Device/AP dependent | NKE WiFi Box | NKE WiFi Box UDP input/output examples commonly use port `5000`. |
| `10021` | Furuno network range; often destination broadcast `172.31.255.255:10021` | Furuno NavNet / NavNet3D style networks | May contain Furuno framing/prefix before actual NMEA-style sentences. |
| `1456` | Often Yacht Devices AP `192.168.4.1`; TCP by factory on many models, UDP if configured | Yacht Devices YDWN, YDWR, YDWG, YDEN, YDNR | Yacht Devices examples often use `1456` as the server port. UDP behavior is configurable. |
| `1496` | Usually Yacht Devices gateway output when configured | Yacht Devices YDWG examples | Seen in OpenCPN/Yacht Devices configuration examples. Treat as configured, not universal default. |
| `60001` | Device/app dependent | Some generic marine Wi-Fi gateways, TIMEZERO/iBoat-style setups, Actisense examples | Often appears in app setup guides as a common alternate NMEA-over-Wi-Fi port. |

## Practical receive scan order

For a discovery daemon, start UDP listeners in this order:

```text
10110
2000
11101
5000
10021
1456
1496
60001
```

For each port, bind to all local interfaces where possible:

```text
0.0.0.0:<port>
```

Then classify incoming payloads by sentence prefix:

```text
$GPRMC, $GNRMC, $IIRMC
$GPGGA, $GNGGA
$IIMWV, $WIMWV
$IIVWR, $WIVWR
$IIVWT, $WIVWT
$IIDBT, $SDDBT
$IIDPT, $SDDPT
!AIVDM, !AIVDO
```

## Common broadcast and multicast destinations

When receiving broadcast UDP, normally bind to `0.0.0.0:<port>` rather than binding to the broadcast address. These addresses are still useful when sniffing traffic, configuring devices, or matching datagrams:

| Address | Why it matters |
|---|---|
| `255.255.255.255` | Limited broadcast. Simple gateways often use this when they do not know the local subnet. |
| `192.168.1.255` | Common broadcast address for devices on `192.168.1.0/24`. |
| `192.168.4.255` | Common broadcast address for ESP-style, Yacht Devices, SWAY, and other AP-mode networks on `192.168.4.0/24`. |
| `192.168.0.255` | Common home/router/MFD subnet broadcast. |
| `10.0.0.255` | Seen with some marine Wi-Fi bridges and APs. |
| `10.10.10.255` | Useful around devices using `10.10.10.1`, including some PredictWind-style networks. |
| `172.31.255.255` | Important for Furuno NavNet3D-style UDP traffic on port `10021`. |
| `239.192.0.0/14` | Administratively scoped multicast range suitable for private manually configured multicast. |
| `239.194.4.4` | Example private multicast group sometimes used in OpenCPN-style examples. Not an NMEA standard default. |

## Vendor and device families

| Vendor / device family | Likely target | Notes |
|---|---|---|
| Digital Yacht WLN10 / WLN30 / NavLink2 / iAIS / iAIS TX | UDP `2000`; often AP/source `192.168.1.1` | Some models also support TCP `2000`. |
| PredictWind DataHub / DataHub Pro | UDP `11101`, TCP `11102`; often AP/source `10.10.10.1` | Broadcasts NMEA 0183 over Wi-Fi/Ethernet. |
| SWAY Breeze | UDP `10110`, TCP `2001`; often AP/source `192.168.4.1` | Common NMEA Wi-Fi gateway pattern. |
| ShipModul MiniPlex Wi-Fi models | Usually `10110`; AP/source may be `10.0.0.1` or configured LAN IP | MiniPlex-2Wi and MiniPlex-3Wi setups vary by mode. |
| Actisense W2K-1 / W2K-2 | Configurable; check UDP/TCP `10110`, `2000`, `60001` | W2K products expose data servers and are commonly configured for chart apps. |
| Yacht Devices YDWN / YDWR / YDWG / YDEN / YDNR | Factory/server examples often use `1456`; also check UDP `2000` and `1496` | UDP and TCP behavior are configurable. |
| NKE WiFi Box | UDP `5000` | Check both input and output modes. |
| Navico GoFree / Simrad / B&G / Lowrance | Often UDP/TCP `10110` | Older GoFree/Lowrance integrations are frequently UDP. Newer units may expose configurable TCP/Ethernet output. |
| Furuno NavNet / NavNet3D | UDP broadcast `172.31.255.255:10021`; also check `10110` for bridge/app integrations | Payload can include non-standard Furuno wrapping. |
| Rose Point Nemo Gateway | UDP broadcast supported; port configurable | Start with `10110`, then scan alternates. |
| Quark-elec QK-A024 / A026 / A031 / A034 / A036 | Often `192.168.1.100:2000`; many examples are TCP-first | Check UDP `2000`, but do not assume it is broadcasting. |
| Vesper XB-8000 / Cortex / WatchMate Vision | Usually TCP, commonly `39150`, often AP/source `192.168.15.1` | Include in discovery tooling, but do not treat as UDP broadcast default. |
| Comar i300W / i320W | Check UDP `10110`, `2000`, `60001` | Defaults vary by firmware/configuration. |
| AMEC / Alltek B600W and similar AIS Wi-Fi products | Check UDP `10110`, `2000`, `60001` | AIS sentences usually appear as `!AIVDM` / `!AIVDO`. |
| MailASail RedBox | Configurable; check `10110`, `2000`, `60001` | Often acts as a router/gateway rather than a fixed single-port sender. |
| SailTimer Air Link | Check `10110`, `2000`, `60001` | Defaults vary by app/device configuration. |

## TCP ports worth checking separately

Many products advertised as "NMEA over Wi-Fi" are actually TCP-only or TCP-default. Do not rely only on UDP scanning.

```text
10110
2000
11102
1456
39150
2001
60001
8080
23
```

Typical meanings:

| TCP port | Common use |
|---:|---|
| `10110` | NMEA 0183 over TCP, generic standard/de facto port. |
| `2000` | Digital Yacht, Quark, Navionics-style integrations. |
| `11102` | PredictWind DataHub TCP companion to UDP `11101`. |
| `1456` | Yacht Devices server examples. |
| `39150` | Vesper XB-8000 / WatchMate / Cortex family. |
| `2001` | SWAY Breeze TCP example. |
| `60001` | Alternate chart app / gateway configurations. |
| `8080` | Device web UI or sometimes websocket/app bridge, not guaranteed NMEA stream. |
| `23` | Telnet-style debug/config/data stream on some older devices, not guaranteed NMEA stream. |

## Discovery implementation notes

1. Join multicast groups only when the user configured a group or when the application has an explicit multicast scan mode. Most devices will not require multicast membership.
2. Enable UDP broadcast receive on sockets where the platform requires it.
3. Bind with address reuse where supported so multiple chart applications can coexist.
4. Capture the source IP and source port for every valid NMEA sentence. Many gateways transmit from an ephemeral source port even when the destination port is fixed.
5. Do not assume one datagram contains exactly one sentence. Some devices bundle multiple NMEA sentences in one UDP datagram.
6. Do not assume sentences are newline-terminated. Accept `\r\n`, `\n`, `\r`, and datagram boundary as possible separators.
7. Validate NMEA checksums when present, but keep a debug mode that records invalid sentences because some gateways emit useful proprietary data with weak formatting.
8. For AIS, support both `!AIVDM` and `!AIVDO`, including multipart messages.
9. Treat `$GPRMC`, `$GNRMC`, `$GPGGA`, and `$GNGGA` as strong evidence of GNSS position data.
10. Treat `$IIMWV` / `$WIMWV` as strong evidence of wind data.
11. Treat `$SDDPT`, `$IIDPT`, `$SDDBT`, and `$IIDBT` as depth data.

## Minimal UDP listener example

A robust connector should open all expected UDP ports at the same time and classify streams by observed sentences, not by vendor name.

```text
for port in [10110, 2000, 11101, 5000, 10021, 1456, 1496, 60001]:
    bind UDP 0.0.0.0:port
    receive datagram
    split into candidate NMEA sentences
    if sentence starts with '$' or '!':
        validate optional checksum
        record source_ip, source_port, local_port, first_seen, last_seen
        publish stream candidate
```

## References

- IANA Service Name and Transport Protocol Port Number Registry: `nmea-0183` on `10110` and `nmea-onenet` on `10111`.
- OpenCPN documentation for NMEA over TCP/UDP, port `10110`, and multicast configuration notes.
- Digital Yacht NavLink/WLN/iAIS support documentation for UDP port `2000` app integrations.
- PredictWind DataHub documentation for UDP `11101` and TCP `11102`.
- NKE WiFi Box user documentation for UDP `5000`.
- ShipModul MiniPlex manuals and setup examples for Wi-Fi/NMEA IP behavior.
- Yacht Devices documentation and OpenCPN examples for `1456`, `1496`, and configurable UDP/TCP behavior.
- Furuno NavNet3D community and integration notes for UDP broadcast `172.31.255.255:10021`.
- TIMEZERO/iBoat documentation listing common NMEA gateway ports including `10110`, `2000`, and `60001`.
- SeaIQ and Aqua Map integration notes for Vesper, Comar, AMEC, MailASail, SailTimer, and other Wi-Fi NMEA gateway families.
