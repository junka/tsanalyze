cmake_minimum_required(VERSION 3.18.0)
project(Tsanalyze)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# common warnings (GCC/Clang only; MSVC treats -W/-Wall as /W//Wall)
if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wfatal-errors -Wstrict-prototypes -Wshadow -Wno-unused-parameter")
endif()
if(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
    add_compile_options(-fno-omit-frame-pointer -fmacro-backtrace-limit=0)
    add_compile_options(-fsanitize=address -fsanitize-recover=all)
    if(CMAKE_HOST_APPLE)
        add_link_options(-fsanitize=address -fsanitize-recover=all)
    else()
        add_compile_options(-fsanitize=leak)
    endif()
else()
    if(WIN32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Od")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    endif()
endif()

# core library shared by the CLI and the optional python binding.
# main.c is excluded because it owns main(), which must not be linked into the pybinding.
file(GLOB SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")

add_library(tsanalyze_core STATIC ${SRC_LIST})
target_include_directories(tsanalyze_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(tsanalyze "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
target_link_libraries(tsanalyze PRIVATE tsanalyze_core)

# ---------------------------------------------------------------------------
# tests (ctest)
# ---------------------------------------------------------------------------
enable_testing()

set(TESTDATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/testdata")

# generator that builds all test streams (robustness + DVB / ISDB / ATSC PSIP)
add_executable(gen_full "${CMAKE_CURRENT_SOURCE_DIR}/tests/gen_full.c")
add_custom_command(TARGET gen_full POST_BUILD
	COMMAND gen_full "${TESTDATA_DIR}"
	COMMENT "Generating test TS streams into ${TESTDATA_DIR}")

set(TSANA "$<TARGET_FILE:tsanalyze>")

# ultra-short / malformed streams must terminate cleanly (no crash, exit 0)
foreach(name IN ITEMS
		empty short_1 short_10 short_100 short_1000
		garbage_2040 truncated valid_pat_trailing)
	add_test(NAME "stream_${name}"
		COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/${name}.ts")
endforeach()

# a valid stream must be probed as 188-byte and print the PAT table
add_test(NAME "stream_valid_pat"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/valid_pat.ts")
set_tests_properties(stream_valid_pat PROPERTIES
	PASS_REGULAR_EXPRESSION "program_number @ PMT_PID")

# invalid file path must fail
add_test(NAME "invalid_filepath"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/does_not_exist.ts")
set_tests_properties(invalid_filepath PROPERTIES WILL_FAIL TRUE)

# ---------------------------------------------------------------------------
# comprehensive streams (gen_full.c): DVB / multi-PMT / ISDB / ATSC PSIP
# ---------------------------------------------------------------------------

# dvb_all.ts : every DVB SI table + special descriptors must be recognised
add_test(NAME "full_dvb_tables"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/dvb_all.ts")
set_tests_properties(full_dvb_tables PROPERTIES
	PASS_REGULAR_EXPRESSION
		"^PAT;^CAT;^TSDT;active PMT;^NIT ACTUAL;^SDT ACTUAL;^BAT;^EIT;^TDT;^TOT")

# dvb_all.ts : special descriptors (subtitling, teletext, AC3, CA, registration)
add_test(NAME "full_dvb_descriptors"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/dvb_all.ts")
set_tests_properties(full_dvb_descriptors PROPERTIES
	PASS_REGULAR_EXPRESSION
		"subtitling_descriptor;teletext_descriptor;AC3_descriptor;CA_descriptor;registration_descriptor;service_descriptor;short_event_descriptor;bouquet_name_descriptor;local_time_offset_descriptor")

# dvb_multi_pmt.ts : 3 programs, one PMT spanning multiple TS packets
add_test(NAME "full_multi_pmt"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/dvb_multi_pmt.ts")
set_tests_properties(full_multi_pmt PROPERTIES
	PASS_REGULAR_EXPRESSION
		"program_number @ PMT_PID;1 @ 0x20;2 @ 0x30;3 @ 0x40;@ 0xe0;@ 0x1f")

# isdb_stream.ts : DVB tables carrying ISDB (ARIB) descriptors
add_test(NAME "full_isdb"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/isdb_stream.ts")
set_tests_properties(full_isdb PROPERTIES
	PASS_REGULAR_EXPRESSION
		"AVC_video_descriptor;digital_copy_control_descriptor;network_identification_descriptor;isdb_terrestrial_delivery_system_descriptor;hierarchical_transmission_descriptor")

# atsc_psip.ts : all 9 PSIP tables on PID 0x1FFB
add_test(NAME "full_atsc_psip"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/atsc_psip.ts")
set_tests_properties(full_atsc_psip PROPERTIES
	PASS_REGULAR_EXPRESSION
		"MGT \\(Master Guide Table\\);TVCT \\(Terrestrial Virtual Channel Table\\);CVCT \\(Cable Virtual Channel Table\\);RRT \\(Rating Region Table\\);ATSC EIT \\(Event Information Table\\);ETT \\(Extended Text Table\\);STT \\(System Time Table\\);DCCT \\(Directed Channel Change Table\\);DCCSCT \\(Directed Channel Change Selection Table\\)")

# atsc_psip.ts : PSIP descriptors
add_test(NAME "full_atsc_descriptors"
	COMMAND "${TSANA}" -f file -o stdout "${TESTDATA_DIR}/atsc_psip.ts")
set_tests_properties(full_atsc_descriptors PROPERTIES
	PASS_REGULAR_EXPRESSION "caption_service_descriptor;extended_channel_name_descriptor")

# subtitle.ts : DVB subtitling PES (EN 300 743) must be parsed and dumped.
# -d enables the PES/elementary-stream dump path.  Expect the registered
# subtitle stream plus the parsed page_composition_segment tree.
add_test(NAME "full_subtitle"
	COMMAND "${TSANA}" -d -f file -o stdout "${TESTDATA_DIR}/subtitle.ts")
set_tests_properties(full_subtitle PROPERTIES
	PASS_REGULAR_EXPRESSION
		"subtitle;page_composition_segment;page_id;page_time_out;n_region")

# missing required arguments must fail
add_test(NAME "missing_args"
	COMMAND "${TSANA}")
set_tests_properties(missing_args PROPERTIES WILL_FAIL TRUE)

# optional python binding; the CLI still builds when pybind11 is unavailable
find_package(Python 3.8 COMPONENTS Interpreter Development QUIET)
find_package(pybind11 CONFIG QUIET)
if(pybind11_FOUND AND Python3_Development_FOUND)
    pybind11_add_module(tsana ./pybind/binding.cpp)
    target_link_libraries(tsana PRIVATE tsanalyze_core)
    install(FILES ./pybind/tsanalyze.py DESTINATION /usr/local/bin)
endif()