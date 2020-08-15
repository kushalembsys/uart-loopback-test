# uart-loopback-test
Linux UART loopback test from userspace

### Compilation
`gcc uart-loopback.c -o uart-loopback`

### Run
Loopback test for ttyO2
`sudo ./uart-loopback /dev/ttyO2`

### Tested on
Beaglebone (All variants)
Raspberry Pi (All variants)
