#include <stdint.h>

#include "ts.h"

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
		return 100 + score - CHECK_COUNT;
	else if(dvhs_score > score && dvhs_score > fec_score && dvhs_score > 6) 
		return 100 + dvhs_score - CHECK_COUNT;
	else if(fec_score > 6) 
		return 100 + fec_score - CHECK_COUNT;
	else 
		return -1;
}


#define MAX_TS_PID_NUM 8192

static uint16_t tspid[MAX_TS_PID_NUM];

static 
