# SocketCandecodeSignals

Filter to decode CAN signals in combination with candump (socketcan tools).

Tiny C tool, no dependencies beyond libc and the `candump -L` log format —
made for shell pipelines and embedded devices where a python environment
is not available.

## Clone and build

```
git clone https://github.com/ebroecker/SocketCandecodeSignals
cd SocketCandecodeSignals
make
```

No submodules — the signal encode/decode helpers (`can_encode_decode_inl.h`)
are part of the repository (originally from
[reinzor/libcan-encode-decode](https://github.com/reinzor/libcan-encode-decode)).

## Usage

```
candump -L canBus | ./socketcanDecodeSignal dbc-file frameName[.signalName] [frame2[.signal2] ...]
```

A `frameName` argument decodes all signals of that frame, a
`frameName.signalName` argument decodes only the given signal. Input lines
which cannot be parsed are skipped with a warning on stderr, so the filter
keeps running on incomplete logs.

Example (no CAN bus needed):

```
$ echo "(0.0) vcan0 001#8d00100100820100" | ./socketcanDecodeSignal ccl_test.dbc testFrame1.sig0
Trying to find: Frame: testFrame1.sig0, Signal: sig0
-- testFrame1 (0x001)  sig0 (0 [2]) --
(0000.000000) vcan0 sig0: 0x01 01
```

## Multiplexed frames

Simple DBC multiplexing is supported. For

```
BO_ 1 testFrame1: 8 TEST_ECU
 SG_ sig0 M : 1|2@0+ (1.0,0.0) [0|0] "" CCL_TEST
 SG_ sig1 m1 : 7|6@0+ (1.0,0.0) [0|0] "" CCL_TEST
 SG_ sig3 m2 : 20|12@0+ (1.0,0.0) [0|0] "" CCL_TEST
```

the multiplexer signal (`M`) is always reported, `m`-group signals are only
decoded when their group matches the current multiplexer value. Signals of
other groups and frames with an undefined multiplexer value are skipped:

```
$ echo "(0.0) vcan0 001#8d00100100820100" | ./socketcanDecodeSignal ccl_test.dbc testFrame1
...
(0000.000000) vcan0 sig0: 0x01 01
(0000.000000) vcan0 sig1: 0x23 35
(0000.000000) vcan0 sig2: 0x00 00
(0000.000000) vcan0 sig5: 0x01 01
...
```

Here `sig0 = 1` selects group `m1`: `sig1`/`sig2` (group `m1`) are decoded,
`sig3`/`sig4` (group `m2`) are skipped, plain signals (`sig5`, ...) are
always decoded.

## Tests

```
make test
```

runs the smoke tests (`test.sh`) by piping log lines to stdin — no CAN bus
required. The expected values were verified against
[cantools](https://github.com/cantools/cantools).

For live testing on a virtual bus:

```
sudo modprobe vcan
sudo ip link add type vcan
sudo ifconfig vcan0 up
candump -L vcan0 | ./socketcanDecodeSignal ccl_test.dbc testFrame1 testFrame2
# another terminal:
cansend vcan0 001#8d00100100820100
cansend vcan0 002#0C00057003CD1F83
```

## Alternatives

If python is available, [cantools](https://github.com/cantools/cantools)
covers a superset of this use case — simple *and* extended multiplexing,
many database formats, plotting:

```
candump -L vcan0 | cantools decode ccl_test.dbc
```

[canmatrix](https://github.com/ebroecker/canmatrix) (same author) reads and
converts most database formats. This tool is the slim C option when neither
is available.
