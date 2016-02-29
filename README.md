# SocketCandecodeSignals
filter to decode signals in combination with candump (socketcan tools)

#clone#
```
git clone  https://github.com/ebroecker/SocketCandecodeSignals
git submodule update --init
```
or
```
git clone --recursive https://github.com/ebroecker/SocketCandecodeSignals
```

#Compile#
```gcc -g -o socketcanDecodeSignal main.c datenbasis.c processFrame.c lib.c```

##Usage##
```
candump -L canBus | ./socketcanDecodeSignal dbc-file frameName[.signalName] [secondFrame[.someSignal] ...]
```
***Example***
```
candump -L vcan0 | ./socketcanDecodeSignal ccl_test.dbc testFrame1.sig0 testFrame2
```

#Test#
***prepare virtual can:***
```
sudo modprobe vcan
sudo ip link add type vcan
sudo ifconfig vcan0 up
```

***dump vcan0 with signal decoding:***
```
candump -L vcan0 | ./socketcanDecodeSignal ccl_test.dbc testFrame1 testFrame2
```

***send some can frames (other terminal)***
```
cansend vcan0 001#8d00100100820100
cansend vcan0 002#0C00057003CD1F83
```

***will result:***
```
Trying to find: Frame: testFrame1
-- testFrame1 (0x001) 
Trying to find: Frame: testFrame2
-- testFrame2 (0x002) 
(1456687759.303127) vcan0 sig0: 0x01 01 
(1456687759.303127) vcan0 sig1: 0x23 35 
(1456687759.303127) vcan0 sig2: 0x00 00 
(1456687759.303127) vcan0 sig3: 0x800 2048 
(1456687759.303127) vcan0 sig4: 0x100 256 
(1456687759.303127) vcan0 sig5: 0x01 01 
(1456687759.303127) vcan0 sig6: 0x00 00 
(1456687759.303127) vcan0 sig7: 0x208 520 
(1456687759.303127) vcan0 sig8: 0x00 00 
(1456687759.303127) vcan0 sig9: 0x00 00 
(1456687759.303127) vcan0 sig10: 0x00 00 
(1456687763.319899) vcan0 secSig1: 0x00 00 
(1456687763.319899) vcan0 secSig2: 0x00 00 
(1456687763.319899) vcan0 secSig3: 0x00 00 
(1456687763.319899) vcan0 secSig4: 0x02 02 
(1456687763.319899) vcan0 secSig5: 0x00 00 
(1456687763.319899) vcan0 secSig6: 0x00 00 
(1456687763.319899) vcan0 secSig7: 0x00 00 
(1456687763.319899) vcan0 secSig8: 0x03 03 
(1456687763.319899) vcan0 secSig9: 0x01 01 
(1456687763.319899) vcan0 secSig10: 0x500 1280 
(1456687763.319899) vcan0 secSig11: 0xffffffffffffff70 -144 
(1456687763.319899) vcan0 secSig12: 0x0c 12 
```


***Test without virtual can:***
```
echo "(0.0) vcan0 001#8d00100100820100" | ./socketcanDecodeSignal ccl_test.dbc testFrame1
```

```
Trying to find: Frame: testFrame1
-- testFrame1 (0x001) 
(0000.000000) vcan0 sig0: 0x01 01 
(0000.000000) vcan0 sig1: 0x23 35 
(0000.000000) vcan0 sig2: 0x00 00 
(0000.000000) vcan0 sig3: 0x800 2048 
(0000.000000) vcan0 sig4: 0x100 256 
(0000.000000) vcan0 sig5: 0x01 01 
(0000.000000) vcan0 sig6: 0x00 00 
(0000.000000) vcan0 sig7: 0x208 520 
(0000.000000) vcan0 sig8: 0x00 00 
(0000.000000) vcan0 sig9: 0x00 00 
(0000.000000) vcan0 sig10: 0x00 00 
```

```
echo "(0.0) vcan0 001#8d00100100820100" | ./socketcanDecodeSignal ccl_test.dbc testFrame1
```

```
Trying to find: Frame: testFrame2
-- testFrame2 (0x002) 
(0000.000001) vcan0 secSig1: 0x00 00 
(0000.000001) vcan0 secSig2: 0x00 00 
(0000.000001) vcan0 secSig3: 0x00 00 
(0000.000001) vcan0 secSig4: 0x02 02 
(0000.000001) vcan0 secSig5: 0x00 00 
(0000.000001) vcan0 secSig6: 0x00 00 
(0000.000001) vcan0 secSig7: 0x00 00 
(0000.000001) vcan0 secSig8: 0x03 03 
(0000.000001) vcan0 secSig9: 0x01 01 
(0000.000001) vcan0 secSig10: 0x500 1280 
(0000.000001) vcan0 secSig11: 0xffffffffffffff70 -144 
(0000.000001) vcan0 secSig12: 0x0c 12 
```
