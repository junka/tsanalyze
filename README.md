# TS Analyze

use basic rules from ISO 13818 and ETSI EN 300 468 to decode MPEG2 TS

use EN 300 743 to decode subtitle

Pinrt descriptors for DVB/ATSC/ISDB standard.

Print PSI tables infomation and some other subsystem infomation


# Compile

Two methods: start with 
```
- autogen.sh
- cmake
```
and then do ```make``` for both ways

# Usage
- Most simple way 
```
./tsanalyze tsfile
```
- support file and udp stream analyze
```
./tsanalyze -f udp udp://url 
```
`Ctrl + C` to stop and show received ts info 

- print table selected
```
./tsanalyze tsfile -s pat -s cat 
```
- show stats about ts stream
 ```
 ./tsanalyze tsfile -S
 ```


# descriptor
not all descriptor implemented now, see ```doc/descriptor.md``` to add new descriptors