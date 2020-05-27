#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include "atsc/descriptor.h"
#include "dvb/descriptor.h"
#include "ts.h"
#include "types.h"

/* ISO/IEC 13818-1 */
/* 0x12 - 0x1A Defined in ISO/IEC 13818-6*/
/* 0x24 - 0x3F reserved, then EN 300 468*/


#define foreach_enum_descriptor                                                                                        \
	_(video_stream, 0x02)                                                                                              \
	_(audio_stream, 0x03)                                                                                              \ 
	_(hierarchy, 0x04)                                                                                                 \
	_(registration, 0x05)                                                                                              \
	_(data_stream_alignment, 0x06)                                                                                     \
	_(target_background_grid, 0x07)                                                                                    \
	_(video_window, 0x08)                                                                                              \
	_(CA, 0x09)                                                                                                        \
	_(ISO_639_language, 0x0A) _(system_clock, 0x0B) _(multiplex_buffer_utilization, 0x0C) _(copyright, 0x0D)       \
	_(maximum_bitrate, 0x0E) _(private_data_indicator, 0x0F) _(smoothing_buffer, 0x10) _(STD, 0x11) _(ibp, 0x12)   \
	_(MPEG4_video, 0x1B) _(MPEG4_audio, 0x1C) _(IOD, 0x1D) _(SL, 0x1E) _(FMC, 0x1F) _(external_ES_ID, 0x20)        \
	_(muxcode, 0x21) _(FmxBufferSize, 0x22) _(MultiplexBuffer, 0x23)

/*0x80 to 0xFE user defined */
/*0xFF forbidden */

enum descriptor_e {
#define _(a, b) dr_##a = b,
	foreach_enum_descriptor
#undef _
};

#define DUMP_MEMBER(str, dr, type, name)                                                                               \
	if (strncmp(#name, "reserved", sizeof("reserved")))                                                                \
	printf("  %s %s : 0x%x\n", str, #name, dr->name)

/* see ISO/IEC 13818-1 chapter 2.6*/
#define foreach_video_stream_member                                                                                    \
	__m(uint8_t, multiple_frame_rate_flag, 1, 2) /*'1' indicates that multiple frame rates may be present*/            \
		__m(uint8_t, frame_rate_code, 4, 2)                                                                            \
		__m(uint8_t, MPEG_1_only_flag, 1,                                                                              \
			2) /*'1' indicates that the video stream contains only ISO/IEC 11172-2 data*/                              \
		__m(uint8_t, constrained_parameter_flag, 1, 2)                                                                 \
		__m(uint8_t, still_picture_flag, 1,                                                                            \
			2) /*set to '1' indicates that the video stream contains only still pictures.*/ /*exist only when          \
																							   MPEG_1_only_flag == 0*/ \
		__m1(uint8_t, profile_and_level_indication, 3) __m(uint8_t, chroma_format, 2, 4)                               \
		__m(uint8_t, frame_rate_extension_flag, 1, 4) __m(uint8_t, reserved, 5, 4)

#define foreach_audio_stream_member                                                                                    \
	__m(uint8_t, free_format_flag, 1, 2) __m(uint8_t, ID, 1, 2) __m(uint8_t, layer, 2, 2)                              \
		__m(uint8_t, variable_rate_audio_indicator, 1, 2) __m(uint8_t, reserved, 3, 2)

enum hierarchy_type_e {
	spatial_scalability = 1,  /*ITU-T Rec. H.262 | ISO/IEC 13818-2 spatial_scalability*/
	SNR_scalability = 2,	  /*ITU-T Rec. H.262 | ISO/IEC 13818-2 SNR_scalability */
	temporal_scalability = 3, /* ITU-T Rec. H.262 | ISO/IEC 13818-2 temporal_scalability*/
	data_partitioning = 4,	/* ITU-T Rec. H.262 | ISO/IEC 13818-2 data_partitioning */
	extension_bitstream = 5,  /*ISO/IEC 13818-3 extension_bitstream*/
	private_stream = 6,		  /* ITU-T Rec.H222.0 | ISO/IEC 13818-1 Private Stream private_stream*/
	multiview_profile = 7,	/* ITU-T Rec. H.262 | ISO/IEC 13818-2 Multi-view Profile*/
	base_layer = 15,
};

#define foreach_hierarchy_member                                                                                       \
	__m(uint8_t, reserved, 4, 2) __m(uint8_t, hierarchy_type, 4, 2) /*see definition in @hierarchy_type_e*/            \
		__m(uint8_t, reserved1, 2, 3) __m(uint8_t, hierarchy_layer_index, 6, 3) __m(uint8_t, reserved2, 2, 4)          \
		__m(uint8_t, hierarchy_embedded_layer_index, 6, 4) __m(uint8_t, reserved3, 2, 5)                               \
		__m(uint8_t, hierarchy_channel, 6, 5)

#define foreach_registration_member                                                                                    \
	__m1(uint32_t, format_identifier, 2) __m1(uint8_t, additional_identification_info, 6)

/* format_identifier 0x41432D33 as AC-3 */

enum video_alignment_type_e {
	slice_or_video_access_unit = 1,
	video_access_unit = 2,
	GOP_or_SEQ = 3,
	SEQ = 4,
	/* 0x05 - 0xFF reserved*/
};

#define foreach_data_stream_alignment_member __m1(uint8_t, alignment_type, 2)
/* see definition in @video_alignment_type_e */

#define foreach_target_background_grid_member                                                                          \
	__m(uint32_t, horizontal_size, 14, 2) __m(uint32_t, vertical_size, 14, 2)                                          \
		__m(uint32_t, aspect_ratio_information, 4, 2)

#define foreach_video_window_member                                                                                    \
	__m(uint32_t, horizontal_offset, 14, 2) __m(uint32_t, vertical_offset, 14, 2) __m(uint32_t, window_priority, 4, 2)

#define foreach_CA_member                                                                                              \
	__m1(uint16_t, CA_system_ID, 2) __m(uint16_t, reserved, 3, 4) __m(uint16_t, CA_PID, 13, 4)                         \
		__mp(uint8_t, private_data_byte, 6)

enum audio_type_e {
	undefined = 0x0,
	clean_effects = 0x1,
	hearing_impaired = 0x2,
	visual_impaired_commentary = 0x3,
	/*0x04-0xFF reserved*/
};

struct language_node
{
	uint32_t ISO_639_language_code : 24;
	uint32_t audio_type : 8;
	/*see definition in @audio_type_e */
};

#define foreach_ISO_639_language_member __mp(uint32_t, languages, 2)

#define foreach_system_clock_member                                                                                    \
	__m(uint8_t, external_clock_reference_indicator, 1, 2) __m(uint8_t, reserved, 1, 2)                                \
		__m(uint8_t, clock_accuracy_integer, 6, 2) __m(uint8_t, clock_accuracy_exponent, 3, 3)                         \
		__m(uint8_t, reserved1, 5, 3)

#define foreach_multiplex_buffer_utilization_member                                                                    \
	__m(uint16_t, bound_valid_flag, 1, 2) __m(uint16_t, LTW_offset_lower_bound, 15, 2) __m(uint16_t, reserved, 1, 4)   \
		__m(uint16_t, LTW_offset_upper_bound, 15, 4)

#define foreach_copyright_member __m1(uint32_t, copyright_identifier, 2) __mp(uint8_t, additional_copyright_info, 6)

#define foreach_maximum_bitrate_member __m(uint32_t, reserved, 2, 2) __m(uint32_t, maximum_bitrate, 22, 2)
/* 22bit*/

#define foreach_private_data_indicator_member __m1(uint32_t, private_data_indicator, 2)

#define foreach_smoothing_buffer_member __m(uint32_t, sb_leak_rate, 24, 2) __m(uint32_t, sb_size, 24, 5)
// uint24_t reserved:2;
// uint24_t sb_leak_rate:22;
// uint24_t reserved1:2;
// uint24_t sb_size:22;

#define foreach_STD_member __m(uint8_t, reserved, 7, 2) __m(uint8_t, leak_valid_flag, 1, 2)

#define foreach_ibp_member                                                                                             \
	__m(uint16_t, closed_gop_flag, 1, 2) __m(uint16_t, identical_gop_flag, 1, 2) __m(uint16_t, max_gop_length, 14, 2)

#define foreach_MPEG4_video_member __m1(uint8_t, MPEG4_visual_profile_and_level, 2)

enum MPEG4_audio_profile_and_level_e {
	main_profile_lv1 = 0x10,
	main_profile_lv2 = 0x11,
	main_profile_lv3 = 0x12,
	main_profile_lv4 = 0x13,
	/*0x14-0x17 reserved */
};

#define foreach_MPEG4_audio_member __m1(uint8_t, MPEG4_audio_profile_and_level, 2)

#define foreach_IOD_member __m1(uint8_t, Scope_of_IOD_label, 2) __m1(uint8_t, IOD_label, 3)

#define foreach_SL_member __m1(uint16_t, ES_ID, 2)

struct FMC_node
{
	uint16_t ES_ID;
	uint8_t FlexMuxChannel;
} __attribute__((packed));

#define foreach_FMC_member __mp(struct FMC_node, FMCs, 2)
//	struct list_head list;
// struct FMC_info *FMC_info_list;

#define foreach_external_ES_ID_member __m1(uint16_t, external_ES_ID, 2)

/* see ISO/IEC 14496-1 */
struct MuxCodeSlot
{
	uint8_t m4MuxChannel;
	uint8_t numberOfBytes;
};

struct MuxCodeSubstrue
{
	uint8_t slotCount : 5;
	uint8_t repetitionCount : 3;
	struct MuxCodeSlot slots[7];
};

struct MuxCodeTableEntry
{
	uint8_t length;
	uint8_t MuxCode : 4;
	uint8_t version : 4;
	uint8_t substructureCount;
	struct MuxCodeSubstrue substructure;
};

#define foreach_muxcode_member
// __mp(struct MuxCodeTableEntry, entries, 2)

/* see ISO/IEC 14496-1 */
struct DefaultFlexMuxBufferDescriptor
{
	uint24_t FB_DefaultBufferSize;
};

struct FlexMuxBufferDescriptor
{
	uint32_t MuxChannel : 8;
	uint32_t FB_BufferSize : 24;
};

#define foreach_FmxBufferSize_member __m1(uint32_t, FB_DefaultBufferSize, 2) __mp(uint32_t, FlexMuxBufferDescriptor, 5)

#define foreach_MultiplexBuffer_member __m(uint32_t, MB_buffer_size, 24, 2) __m(uint32_t, TB_leak_rate, 24, 5)
/* in units of 400 bits per second the rate at which data is transferred */

struct descriptor_ops
{
	uint8_t tag;
	char tag_name[64];
	int (*descriptor_parse)(uint8_t *data, uint32_t len, void *ptr);
	void *(*descriptor_alloc)(void);
	void (*descriptor_free)(descriptor_t *ptr);
	void (*descriptor_dump)(const char *str, descriptor_t *ptr);
};

#define __m(type, name, bits, byte_off) type name : bits;
#define __m1(type, name, byte_off) type name;
#define __mp(type, name, byte_off) type *name;
#define _(desname, val)                                                                                                \
	typedef struct                                                                                                     \
	{                                                                                                                  \
		descriptor_t descriptor;                                                                                       \
		foreach_##desname##_member                                                                                     \
	} desname##_descriptor_t;

foreach_enum_descriptor
#undef _
#undef __m1
#undef __m

#define ALLOC(descriptor)                                                                                              \
	static inline void *alloc_##descriptor##_descriptor(void)                                                          \
	{                                                                                                                  \
		return malloc(sizeof(descriptor##_descriptor_t));                                                              \
	}

#define FREE(descriptor)                                                                                               \
	static inline void free_##descriptor##_descriptor(descriptor_t *ptr)                                               \
	{                                                                                                                  \
		free(ptr);                                                                                                     \
	}

#define _(a, b) ALLOC(a)
foreach_enum_descriptor
#undef _

#define _(a, b) FREE(a)
foreach_enum_descriptor
#undef _

#define INVALID_DR_RETURN(a, buf)                                                                                      \
	do {                                                                                                               \
		if (buf[0] != dr_##a) {                                                                                        \
			return -1;                                                                                                 \
		}                                                                                                              \
	} while (0)

#define DR_TAG(a, buf, ptr)                                                                                            \
	uint8_t bits_off = 0, bytes_off = 0;                                                                               \
	a##_descriptor_t *dr = (a##_descriptor_t *)ptr;                                                                    \
	dr->descriptor.tag = buf[0];                                                                                       \
	dr->descriptor.length = buf[1];                                                                                    \
	bytes_off += 2;

/* parse function macros */
#define __m(type, name, bits, byte_off)                                                                                \
	dr->name = TS_READ_BITS_##type(buf + byte_off, bits, bits_off);                                                    \
	bits_off += bits;                                                                                                  \
	if (bits_off == sizeof(type) * 8) {                                                                                \
		bits_off = 0;                                                                                                  \
		bytes_off += sizeof(type);                                                                                     \
	}

#define __m1(type, name, byte_off)                                                                                     \
	dr->name = TS_READ_##type(buf + byte_off);                                                                         \
	bytes_off += sizeof(type);

#define __mp(type, name, byte_off)                                                                                     \
	dr->name = (type *)malloc(sizeof(type));                                                                           \
	memcpy(dr->name, buf + byte_off, len - byte_off);

#define _(desname, val)                                                                                                \
	static inline int parse_##desname##_descriptor(uint8_t *buf, uint32_t len, void *ptr)                              \
	{                                                                                                                  \
		INVALID_DR_RETURN(desname, buf);                                                                               \
		DR_TAG(desname, buf, ptr) foreach_##desname##_member return 0;                                                 \
	}
foreach_enum_descriptor
#undef _
#undef __m1
#undef __m
	extern struct descriptor_ops des_ops[];

/* dump function macros*/
#define __m(type, name, bits, byte_off) DUMP_MEMBER(str, dr, type, name);

#define __m1(type, name, byte_off) DUMP_MEMBER(str, dr, type, name);

#ifdef __APPLE__
#define MALLOC_SIZE(x) malloc_size(x)
#else
#define MALLOC_SIZE(x) malloc_usable_size(x)
#endif
#define __mp(type, name, byte_off)                                                                                     \
	int i = 0, psize = MALLOC_SIZE(dr->name);                                                                          \
	printf("  %s %s :", str, #name);                                                                                   \
	while (i < psize) {                                                                                                \
		printf(" 0x%x", *(dr->name + i));                                                                              \
		i++;                                                                                                           \
	}                                                                                                                  \
	printf("\n");

#define _(desname, val)                                                                                                \
	static inline void dump_##desname##_descriptor(const char *str, descriptor_t *p_dr)                                \
	{                                                                                                                  \
		desname##_descriptor_t *dr = container_of(p_dr, desname##_descriptor_t, descriptor);                           \
		printf("%s 0x%02x (%s) : len %d\n", str, p_dr->tag, des_ops[p_dr->tag].tag_name, p_dr->length);                \
		foreach_##desname##_member                                                                                     \
	}
foreach_enum_descriptor
#undef _
#undef __m1
#undef __m

int parse_tlv(uint8_t *buf);

void init_descriptor_parsers(void);

void free_descriptors(struct list_head *list);

void dump_descriptors(const char *str, struct list_head *list);

void parse_descriptors(struct list_head *h, uint8_t *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* _DESCRIPTOR_H_ */
