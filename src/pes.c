#include <stdint.h>
#include <stdio.h>

#include "ts.h"
#include "pes.h"

int parse_pes(uint8_t*pkt,uint16_t len)
{
	pes_t pes;
	uint8_t * buf = pkt;
	pes.packet_start_code_prefix = ((uint32_t)buf[0]<<16|(uint32_t)buf[1]<<8|buf[2]);
	pes.stream_id = buf[3];
	buf += 4;
	pes.PES_packet_length = TS_READ16(buf); 
	buf += 2;
	if((pes.stream_id!= program_stream_map)&&(pes.stream_id!=padding_stream)&&(pes.stream_id!=private_stream_2)
		&&(pes.stream_id!=ECM_stream)&&(pes.stream_id!=EMM_stream)&&(pes.stream_id!=program_stream_directory)
		&&(pes.stream_id!=DSMCC_stream)&&(pes.stream_id!=H222_1_E))
	{
		pes.packet_data.PES_scrambling_control = (buf[0] >>4)&0x3;
	}else if((pes.stream_id== program_stream_map)||(pes.stream_id==private_stream_2)
		&&(pes.stream_id==ECM_stream)&&(pes.stream_id==EMM_stream)&&(pes.stream_id==program_stream_directory)
		&&(pes.stream_id==DSMCC_stream)&&(pes.stream_id==H222_1_E))
	{
		pes.PES_packet_data_byte = buf;
	}
	else if(pes.stream_id==padding_stream)
	{
		pes.padding_byte = buf;
	}
	return 0;
}




