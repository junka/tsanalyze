# TS Analyze

A command-line MPEG-2 **TS** (transport stream) analyzer. It reads a TS file or a live UDP stream and decodes the tables and bitstreams defined by the DVB / ATSC / ISDB broadcasting standards.

A TS packet starts with a 4-byte header; the total packet length is normally 188 bytes, and may be 192 or 204 bytes depending on the standard.

```
Construct TS packets we will get PES / PSI packets.
PSI is structured for search and other auxiliary functions.
PES contains ES packets which could be audio or video streams.
```

With this understanding you can start decoding a TS stream or file.

- Use basic rules from ISO 13818-1 and ETSI EN 300 468 to decode MPEG-2 TS (PAT / PMT / CAT / TSDT / NIT / SDT / BAT / EIT / TDT / TOT)
- Use EN 300 743 to decode subtitle
- Use ETSI ETS 300 706 to decode teletext
- Print PSI/SI/PSIP table information and several subsystem statistics
- Print descriptors for the DVB / ATSC / ISDB standards
- Output the result as plain text, JSON, YAML, or an interactive HTML viewer

---

## Compile

Optionally install [pybind11] to also build the Python binding.

### Linux & macOS

```sh
git clone https://github.com/junka/tsanalyze.git
cd tsanalyze
cmake -S . -B build
cmake --build build
```

### Windows (MSVC)

```sh
git clone https://github.com/junka/tsanalyze.git
cd tsanalyze
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build .
```

After building, the `tsanalyze` executable sits in the `build` directory.

---

## Usage

Basic invocation:

```sh
./tsanalyze /path/to/some.ts
```

The CLI accepts the following options:

| Option             | Description                                                    |
| ------------------ | -------------------------------------------------------------- |
| `-h, --help`       | Show this help                                                 |
| `-f, --format`     | Select input format: `file` or `udp`                           |
| `-o, --output`     | Save output to `stdout`, `txt`, `json`, `yaml`, or `html`      |
| `-s, --table`      | Show only the selected table(s): `pat`, `cat`, `pmt`, `tsdt`, `nit`, `sdt`, `bat`, `tdt` |
| `-p, --pid`        | Show a selected PID only                                       |
| `-S, --stats`      | Show all TS stats                                              |
| `-d, --details`    | Show all PES infos                                             |
| `-b, --brief`      | Show all infos in brief (default)                              |
| `-v, --version`    | Show version                                                   |

### Analyze a file

```sh
./tsanalyze tsfile.ts
```

### Analyze a live UDP stream

```sh
./tsanalyze -f udp udp://url_of_stream
```

Press `Ctrl + C` to stop and print the information received so far.

### Select tables to print

```sh
./tsanalyze tsfile.ts -s pat -s cat
```

### Show TS statistics

```sh
./tsanalyze tsfile.ts -S
```

### Show a single PID

```sh
./tsanalyze tsfile.ts -p 0x10
```

---

## Output formats

Use `-o` to choose how the decoded tables are written out. When the target is a
real output file (anything but `stdout`), the result is stored next to the input
as `<input>.<ext>` (e.g. `stream.ts` + `-o html` produces `stream.ts.html`).

### Plain text (`-o stdout`, default)

Human readable tree of every recognized table and descriptor.

```sh
./tsanalyze tsfile.ts -o stdout
```

### JSON (`-o json`)

A strict, valid JSON tree of all tables — handy for piping into `jq`, `python`, etc.

```sh
./tsanalyze tsfile.ts -o json        # -> tsfile.ts.json
```

### YAML (`-o yaml`)

A YAML tree. Duplicate mapping keys (e.g. several `CA_descriptor`s in one
program, or multiple versions of the same table) are made unique automatically,
so the output always conforms to the YAML spec.

```sh
./tsanalyze tsfile.ts -o yaml        # -> tsfile.ts.yaml
```

### HTML (`-o html`)

Embeds the parsed result in a self-contained, interactive single-file viewer (no
external dependencies) — open it directly in any browser:

```sh
./tsanalyze tsfile.ts -o html        # -> tsfile.ts.html
```

The HTML viewer offers:

- **Table cards** — each table/table-group is rendered as a card with a colored header.
- **Expand / collapse** — click the `-` / `+` toggle beside any section to fold or unfold its children.
- **Version pagination** — when a stream carries many versions of the same table, use the `◀` / `▶` arrows to page through the different versions (e.g. `1 / 14`).

---

## Tests

Test TS streams are generated at build time and exercised through **CTest**:

```sh
cd build
ctest
```

The suite validates robustness against truncated / garbage streams, DVB / ISDB /
ATSC PSIP table decoding, descriptor parsing, and JSON / YAML well-formedness.

---

## Descriptors

Not every descriptor is implemented yet. See `doc/descriptor.md` to learn how to
add new descriptors.

[pybind11]: https://github.com/pybind/pybind11