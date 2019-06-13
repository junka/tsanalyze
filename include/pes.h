#ifndef _PES_H_
#define _PES_H_

#ifdef __cplusplus
extern "C"{
#endif

enum stream_id {
	program_stream_map = 1,
	private_stream_1 = 2,
	private_stream_2 = 3,
	ECM_stream = 3,
	EMM_stream = 3,
	DSMCC_stream = 5,
	ISO13522_stream = 2,
	H222_1_A = 6,
	H222_1_B = 6,
	H222_1_C = 6,
	H222_1_D = 6,
	H222_1_E = 6,
	ancillary_stream = 7,
	program_stream_directory = 4,
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
