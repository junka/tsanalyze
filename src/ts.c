#include <stdint.h>
#include <stdio.h>
#include "ts.h"
#include "table.h"

/*
* port from ffmpeg for judging TS packet length
*/

static int analyze(const uint8_t *buf, int size, int packet_size, int *index){
	int stat[TS_MAX_PACKET_SIZE];
	int i;
	int x=0;
	int best_score=0;

	memset(stat, 0, packet_size*sizeof(int));

	for (x=i=0; i < size-3; i++)
	{
		if ((buf[i] == 0x47) && !(buf[i+1] & 0x80) && (buf[i+3] & 0x30))
		{
			stat[x]++;

		if (stat[x] > best_score)
		{
			best_score= stat[x];
			if (index) 
				*index= x;
		}
	}

	x++;
	if (x == packet_size) 
		x= 0; 
	}

	return best_score;
}

static int mpegts_probe(unsigned char *buf, int buf_size)
{
#define CHECK_COUNT 10

	const int size= buf_size;
	int score, fec_score, dvhs_score;
	int check_count= size / TS_FEC_PACKET_SIZE;

	if (check_count < CHECK_COUNT)
		return -1;

	score = analyze(buf, TS_PACKET_SIZE *check_count, TS_PACKET_SIZE , NULL) 
		* CHECK_COUNT / check_count;
	dvhs_score= analyze(buf, TS_DVHS_PACKET_SIZE*check_count, TS_DVHS_PACKET_SIZE, NULL)
		* CHECK_COUNT / check_count;
	fec_score = analyze(buf, TS_FEC_PACKET_SIZE *check_count, TS_FEC_PACKET_SIZE , NULL)
		* CHECK_COUNT / check_count;

	/* 
	* we need a clear definition for the returned score ,
	* otherwise things will become messy sooner or later
	*/
	if (score > fec_score && score > dvhs_score && score > 6) 
		return 0;//100 + score - CHECK_COUNT;
	else if(dvhs_score > score && dvhs_score > fec_score && dvhs_score > 6) 
		return 1;//100 + dvhs_score - CHECK_COUNT;
	else if(fec_score > 6) 
		return 2;//100 + fec_score - CHECK_COUNT;
	else 
		return -1;
}


struct pid_ops{
	uint16_t pid;
	uint64_t pkts_in;
	uint64_t error_in;
	uint64_t bits_in;
	table_ops *tops;
};


#define MAX_TS_PID_NUM 8192
struct pid_ops pid_dev[MAX_TS_PID_NUM];
extern table_ops drop_ops;
extern table_ops pat_ops;
extern table_ops cat_ops;
extern table_ops pmt_ops;
extern table_ops sdt_ops;

int ts_proc(uint8_t *data,uint8_t len)
{
	ts_header head;
	uint8_t *ptr = data;
	if(ptr[0]!=TS_SYNC_BYTE)
		return -1;
	ptr+=1;
	head.PID = TS_READ16(ptr) & 0x1FFF;
	head.transport_error_indicator = TS_READ8(ptr) >>7;
	ptr += 2;
	head.adaptation_field_control = (TS_READ8(ptr)>>4)&0x3;
	//memcpy(&head,data,sizeof(ts_header));
	//printf("receive PID 0x%x\n",head.PID);
	ptr+=1;
	len-=4;
	if(head.adaptation_field_control ==ADAPT_ONLY||head.adaptation_field_control ==ADAPT_BOTH)
	{
		ts_adaptation_field adapt;
		adapt.adaptation_field_length = TS_READ8(ptr);
		ptr += 1;
		ptr += adapt.adaptation_field_length;
		len -=1;
		len -= adapt.adaptation_field_length;
		//TODO
	}
	
	pid_dev[head.PID].pkts_in++;
	if(head.transport_error_indicator == 1 )
	{
		pid_dev[head.PID].error_in++;
	}
	
	//pointer_field
	ptr+= 1;
	len -=1;
	pid_dev[head.PID].tops->table_proc(head.PID,ptr ,len);//sizeof(ts_header)
	return 0;
}


void register_pmt_ops(uint16_t pid)
{
	pid_dev[pid].tops = &pmt_ops;
}
void unregister_pmt_ops(uint16_t pid)
{
	pid_dev[pid].tops = &drop_ops;
}

void dump_TS_info()
{
	uint16_t pid=0;
	printf("\n");
	printf("TS bits statistics:\n");
	for(pid = 0; pid <=NULL_PID ; pid++){
		if(pid_dev[pid].pkts_in)
			printf("  PID %13d(0x%x)  \t:  %d\t%10d\n",pid,pid,pid_dev[pid].pkts_in,pid_dev[pid].error_in );
	}
}

int init_pid_ops(void)
{
	int i =0;
	for(i = 0; i < 8192; i++)
	{
		pid_dev[i].pid = i;
		pid_dev[i].tops = &drop_ops;
	}
	
	pid_dev[PAT_PID].tops = &pat_ops;
	pid_dev[CAT_PID].tops = &cat_ops;
	pid_dev[SDT_PID].tops = &sdt_ops;

	return 0;
}


extern struct ts_ana_configuration tsaconf;
extern struct io_ops file_ops;

int init_pid_processor()
{
	file_ops.open(tsaconf.name);
	void * ptr = NULL;
	size_t len;
	int ts_pktlen = 0;
	int start_index;
	int typ;
	uint8_t pkt_con[TS_FEC_PACKET_SIZE];
	int pkt_con_len = 0;
	
	file_ops.read(&ptr,&len);

	typ = mpegts_probe((uint8_t *)ptr,len);
	if(typ==0)
	{
		ts_pktlen = TS_PACKET_SIZE;
	}else if(typ == 1)
	{
		ts_pktlen = TS_DVHS_PACKET_SIZE;
	}else if(typ == 2)
	{
		ts_pktlen = TS_FEC_PACKET_SIZE;
	}else
	{
		printf("not a valid TS format file\n");
		return -1;
	}

	//hexdump(ptr, 188);
	analyze(ptr, ts_pktlen*2, ts_pktlen , &start_index);
	printf("valid ts starting at %d\n",start_index);
	//hexdump(ptr+start_index, ts_pktlen);

	init_pid_ops();
	init_descriptor_parsers();

	ptr += start_index;
	len -= start_index;
	
	while(file_ops.end()){
		if(pkt_con_len == ts_pktlen){
			ts_proc(pkt_con,ts_pktlen);
			pkt_con_len = 0;
		}
		while(len>ts_pktlen)
		{
			ts_proc(ptr,ts_pktlen);
			len -= ts_pktlen;
			ptr += ts_pktlen;
		}
		memcpy(pkt_con,ptr,len);
		pkt_con_len = len;
		if(file_ops.read(&ptr,&len) < 0)
			break;
		memcpy(pkt_con + pkt_con_len, ptr , ts_pktlen - pkt_con_len);
		ptr += ts_pktlen - pkt_con_len;
		len -= (ts_pktlen - pkt_con_len);
		pkt_con_len = ts_pkt_len;
	}
	file_ops.close();

	return 0;
}
