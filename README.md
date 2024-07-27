# TS Analyze

A TS packet start with 4 bytes header, total length could be 188, 192 or 204 bytes which are specified in standard DVB/ATSC/ISDB.


```
Construct TS packets we will get PES/PSI packets.
PSI is structured for search and other auxiliary functions. 
PES contains ES packets which could be audio or vedio streams.
```

With this understanding, you can start decoding a TS stream or file now.


- Use basic rules from ISO 13818 and ETSI EN 300 468 to decode MPEG2 TS
- Use EN 300 743 to decode subtitle
- Use ETS 300 706 to decode teletext

Print descriptors for DVB/ATSC/ISDB standard.
Print PSI tables infomation and some other subsystem infomation


# Compile

We could install pybind11 to build package for python.

for Linux & Macos
```
git clone https://github.com/junka/tsanalyze.git
cd tsanalyze
mkdir build
cd build
cmake ..
make
```

for Windows with MSVC
```
git clone https://github.com/junka/tsanalyze.git
cd tsanalyze
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build .
```

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