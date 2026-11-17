# SocketCandecodeSignals
filter to decode signals in combination with candump (socketcan tools)

Tiny C tool, no dependencies beyond libc and can-utils log format —
made for shell pipelines and embedded devices where a python environment
is not available.

# clone #
```
git clone  https://github.com/ebroecker/SocketCandecodeSignals
```

# Compile #
```make
```
or
```gcc -g -o socketcanDecodeSignal main.c datenbasis.c processFrame.c lib.c```

## Usage ##
```
candump -L canBus | ./socketcanDecodeSignal dbc-file frameName[.signalName] [secondFrame[.someSignal] ...]
```
***Example***
```
candump -L vcan0 | ./socketcanDecodeSignal ccl_test.dbc testFrame1.sig0 testFrame2
```

Subscribing a whole frame decodes all its signals; a `frame.signal`
argument decodes only that signal. Lines in the input which cannot be
parsed are skipped with a warning on stderr, so the filter keeps
running on partial logs.

## Multiplexed frames ##
Simple DBC multiplexing (`SG_ muxer M ...`, `SG_ signal m2 ...`) is
supported: the multiplexer signal is always reported, m-group signals
are only decoded when their group matches the current multiplexer
value. Signals of other groups and frames with an undefined multiplexer
value are skipped.

## Tests ##
`./test.sh` (or `make test`) runs the smoke tests, no CAN bus needed —
the expected values were verified against
[cantools](https://github.com/cantools/cantools).
For live testing with a virtual bus:
```
sudo modprobe vcan
sudo ip link add type vcan
sudo ifconfig vcan0 up
candump -L vcan0 | ./socketcanDecodeSignal ccl_test.dbc testFrame1 testFrame2
cansend vcan0 001#8d00100100820100
cansend vcan0 002#0C00057003CD1F83
```

## Alternatives ##
If python is available, [cantools](https://github.com/cantools/cantools)
offers a superset of this functionality, including simple *and* extended
signal multiplexing plus plotting:
```
candump -L vcan0 | cantools decode ccl_test.dbc
```
[canmatrix](https://github.com/ebroecker/canmatrix) (same author) reads
and converts most database formats. This tool is the slim C option when
neither is an option.
