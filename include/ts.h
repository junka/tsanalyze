#ifndef _BITS_H_
#define _BITS_H_


/* define ts structure ,see ISO/IEC13818-1 */



#ifdef __cplusplus
extern "C"{
#endif

#define TS_PACKET_SIZE 188
#define TS_DVHS_PACKET_SIZE 192
#define TS_FEC_PACKET_SIZE 204
#define TS_MAX_PACKET_SIZE 204

#define TS_SYNC_BYTE	(0x47)

typedef struct{
	uint8_t sync_byte;	/*0x47*/
	uint16_t transport_error_indicator:1;
	uint16_t payload_unit_start_indicator:1;
	uint16_t transport_priority:1;
	uint16_t PID:13;
	uint8_t transport_scrambling_control:2;
	uint8_t adaptation_field_control:2;
	uint8_t continuity_counter:4;
}ts_header;

typedef struct{
	uint8_t adaptation_field_length;
	uint8_t discontinuity_indicator:1;
	uint8_t random_access_indicator:1;
	uint8_t elementary_stream_priority_indicator:1;
	uint8_t PCR_flag:1;
	uint8_t OPCR_flag:1;
	uint8_t splicing_point_flag:1;
	uint8_t transport_private_data_flag:1;
	uint8_t adaptation_field_extension_flag:1;
}ts_adaptation_field;

enum PID_e{
	PAT_PID		= 0x0000,
	CAT_PID		= 0x0001,
	TSDT_PID	= 0x0002,
	IPMP_PID	= 0x0003,

	/* 4 - 15 reserved for future use */
	NIT_PID		= 0x0010,
	SDT_PID		= 0x0011,
	BAT_PID		= 0x0011,
	EIT_PID		= 0x0012,
	RST_PID		= 0x0013,
	TDT_PID		= 0x0014,
	TOT_PID		= 0x0014,
	RNT_PID		= 0x0016,
	DIT_PID		= 0x001E,
	SIT_PID		= 0x001F,

	/* 32-8186 for PMT */
	
	NULL_PID	= 0x1FFF,
};


/*define helper for reading bits set*/
#define TS_READ8(buff,offset,len) (buff[0] &(1<<(7-offset)))

#define TS_READ16(buff,offset,len) \
	buff[0] & ()


#ifdef __cplusplus
}
#endif

#endif
