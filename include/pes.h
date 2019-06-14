#ifndef _PES_H_
#define _PES_H_

#ifdef __cplusplus
extern "C"{
#endif

enum stream_id {
	program_stream_map = 0xBC,
	private_stream_1 = 0xBD,
	padding_stream = 0xBE,
	private_stream_2 = 0xBF,
	/*0x110x xxxx is ISO/IEC 13818-3 or ISO/IEC 11172-3 or 
		ISO/IEC 13818-7 or ISO/IEC 14496-3 audio stream number x xxxx*/
	/* 0x1110 xxxx is ITU-T Rec. H.262 | ISO/IEC 13818-2 or 
		ISO/IEC 11172-2 or ISO/IEC 14496-2 video stream number xxxx*/
	ECM_stream = 0xF0,
	EMM_stream = 0xF1,
	DSMCC_stream = 0xF2,
	ISO13522_stream = 0xF3,
	H222_1_A = 0xF4,
	H222_1_B = 0xF5,
	H222_1_C = 0xF6,
	H222_1_D = 0xF7,
	H222_1_E = 0xF8,
	ancillary_stream = 0xF9,
	SL_packetized_stream = 0xFA,
	FlexMux_stream = 0xFB,
	/* reserved 0xFC, 0xFD, 0xFE */
	program_stream_directory = 0xFF,
};


typedef struct {
	uint32_t packet_start_code_prefix:24;
	uint32_t stream_id:8;
	uint16_t PES_packet_length;
	
} pes_t;

#ifdef __cplusplus
}
#endif

#endif /*_PES_H_*/
