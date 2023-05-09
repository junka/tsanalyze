# TS Analyze

A TS packet start with 4 bytes header, total length could be 188, 192 or 204 bytes.
Most system will use 188 bytes.

Construct TS packets we will get PES/PSI packets. PSI is structured for search and
other auxiliary functions. PES contains ES packets which could be audio or vedio
streams.

With this understanding, you can start decoding a TS stream or file now.


Use basic rules from ISO 13818 and ETSI EN 300 468 to decode MPEG2 TS
Use EN 300 743 to decode subtitle
Use ETS 300 706 to decode teletext

Print descriptors for DVB/ATSC/ISDB standard.
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
- Support file and udp stream analyze
```
./tsanalyze -f udp udp://url_of_stream
```
`Ctrl + C` to stop and show received ts information 

- Print table selected
```
./tsanalyze tsfile -s pat -s cat 
```
- Print stats about ts streams
 ```
 ./tsanalyze tsfile -S
 ```


# descriptor
Not all descriptor implemented now, see ```doc/descriptor.md``` to add new descriptors