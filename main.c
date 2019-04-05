

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>

#include "bits.h"
#include "table.h"

//#include "type.h"
#if 0
#include "dvbpsi/dvbpsi.h"
#include "dvbpsi/descriptor.h"
#include "dvbpsi/demux.h"
#include "dvbpsi/psi.h"
#include "dvbpsi/pat.h"
#include "dvbpsi/cat.h"
#include "dvbpsi/pmt.h"
#include "dvbpsi/tot.h"
#include "dvbpsi/dr.h"
#endif

typedef int vlc_bool_t;
#define VLC_FALSE 0
#define VLC_TRUE  1

typedef int64_t mtime_t;


typedef struct{
	dvbpsi_t * handle;
	int i_pat_version;
	int i_ts_id;
} ts_pat_t;

typedef struct ts_pid_s{    
    int         i_pid;    
    vlc_bool_t  b_seen;    
    int         i_cc;   /* countinuity counter */    
    vlc_bool_t  b_pcr;  /* this PID is the PCR_PID */    
    mtime_t     i_pcr;  /* last know PCR value */
} ts_pid_t;

typedef struct ts_pmt_s{    
    dvbpsi_t * handle;   
    int         i_number; /* i_number = 0 is actually a NIT */    
    int         i_pmt_version;   
    ts_pid_t    *pid_pmt;  
    ts_pid_t    *pid_pcr;
} ts_pmt_t;

typedef struct ts_cat_s{    
    dvbpsi_t * handle;   
    int         i_number; /* i_number = 0 is actually a NIT */    
    int         i_cat_version;
    int	        system_id[4];
    ts_pid_t  emm_pid[4];
} ts_cat_t;

typedef struct ts_sdt_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_sdt_t;
typedef struct ts_tdt_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_tdt_t;


typedef struct{    
    ts_pat_t    pat;   
    ts_cat_t cat;
    ts_sdt_t    sdt;
    ts_tdt_t    tdt;
    int         i_pmt;    //total num of programs
    ts_pmt_t    pmt;    
    ts_pid_t    pid[8192];//0-0x1fff
} ts_stream_t;

#define SYSTEM_CLOCK_DR 0x0B
#define MAX_BITRATE_DR 0x0E
#define STREAM_IDENTIFIER_DR 0x52
#define SUBTITLING_DR 0x59

//static void DumpCAT(void* p_data, dvbpsi_cat_t* p_cat);

//static void DumpPMT(void* p_data, dvbpsi_pmt_t* p_pmt);

static void msg_callback(dvbpsi_t *handle, const dvbpsi_msg_level_t level, const char* msg)
{
    switch(level)    
    {        
        case DVBPSI_MSG_ERROR: 
            printf( "Error: "); 
            break;        
        case DVBPSI_MSG_WARN:  
            printf("Warning: "); 
            break;        
        case DVBPSI_MSG_DEBUG: 
            printf("Debug: "); 
            break;        
        default: /* do nothing */            
            return;    
    }    
    printf("%s\n", msg);
}

static int ReadPacket( int i_fd, uint8_t* p_dst )
{    
    int i = 187;    
    int i_rc = 1;    
    p_dst[0] = 0;    
    while((p_dst[0] != 0x47) && (i_rc > 0))    
    {       
        i_rc = read(i_fd, p_dst, 1);    
    }    
    while((i != 0) && (i_rc > 0))    
    {        
        i_rc = read(i_fd, p_dst + 188 - i, i);        
        if(i_rc >= 0)            
            i -= i_rc;    
    }    
    return (i_rc <= 0) ? i_rc : 188;
}





/***************************************************************************** * GetTypeName *****************************************************************************/
static char const* GetTypeName(uint8_t type)
{  
    switch (type)   
    {    
        case 0x00:      
            return "Reserved";    
        case 0x01:      
            return "ISO/IEC 11172 Video";    
        case 0x02:      
            return "ISO/IEC 13818-2 Video";    
        case 0x03:      
            return "ISO/IEC 11172 Audio";    
        case 0x04:      
            return "ISO/IEC 13818-3 Audio";    
        case 0x05:      
            return "ISO/IEC 13818-1 Private Section";    
        case 0x06:      
            return "ISO/IEC 13818-1 Private PES data packets";    
        case 0x07:      
            return "ISO/IEC 13522 MHEG";    
        case 0x08:      
            return "ISO/IEC 13818-1 Annex A DSM CC";    
        case 0x09:      
            return "H222.1";    
        case 0x0A:      
            return "ISO/IEC 13818-6 type A";    
        case 0x0B:      
            return "ISO/IEC 13818-6 type B";    
        case 0x0C:      
            return "ISO/IEC 13818-6 type C";    
        case 0x0D:      
            return "ISO/IEC 13818-6 type D";    
        case 0x0E:      
            return "ISO/IEC 13818-1 auxillary";    
        default:      
            if (type < 0x80)   
                return "ISO/IEC 13818-1 reserved";      
            else   
                return "User Private";   
        }
}

/***************************************************************************** * DumpMaxBitrateDescriptor *****************************************************************************/
static void DumpMaxBitrateDescriptor(dvbpsi_max_bitrate_dr_t* bitrate_descriptor)
{  
    printf( "Bitrate: %d\n", bitrate_descriptor->i_max_bitrate);
}


/***************************************************************************** * DumpSystemClockDescriptor *****************************************************************************/
static void DumpSystemClockDescriptor(dvbpsi_system_clock_dr_t* p_clock_descriptor)
{  
    printf( "External clock: %s, Accuracy: %E\n",     p_clock_descriptor->b_external_clock_ref ? "Yes" : "No",     p_clock_descriptor->i_clock_accuracy_integer *     pow(10.0, -(double)p_clock_descriptor->i_clock_accuracy_exponent));
}


/***************************************************************************** * DumpStreamIdentifierDescriptor *****************************************************************************/
static void DumpStreamIdentifierDescriptor(dvbpsi_stream_identifier_dr_t* p_si_descriptor)
{  
    printf( "Component tag: %d\n",     p_si_descriptor->i_component_tag);
}

/***************************************************************************** * DumpSubtitleDescriptor *****************************************************************************/
static void DumpSubtitleDescriptor(dvbpsi_subtitling_dr_t* p_subtitle_descriptor)
{  
    int a; 
    printf("%d subtitles,\n", p_subtitle_descriptor->i_subtitles_number);  
    for (a = 0; a < p_subtitle_descriptor->i_subtitles_number; ++a)    
    {      
        printf( "       | %d - lang: %c%c%c, type: %d, cpid: %d, apid: %d\n", a,         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[0],         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[1],         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[2],         p_subtitle_descriptor->p_subtitle[a].i_subtitling_type,         p_subtitle_descriptor->p_subtitle[a].i_composition_page_id,         p_subtitle_descriptor->p_subtitle[a].i_ancillary_page_id);    
    }
}


static void DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{    
    int i;    
    while(p_descriptor)    
    {        
        printf( "%s 0x%02x : ", str, p_descriptor->i_tag);        
        switch (p_descriptor->i_tag)        
        {        
            case SYSTEM_CLOCK_DR:            
                DumpSystemClockDescriptor(dvbpsi_DecodeSystemClockDr(p_descriptor));
                break;        
            case MAX_BITRATE_DR:            
                DumpMaxBitrateDescriptor(dvbpsi_DecodeMaxBitrateDr(p_descriptor));         
                break;       
            case STREAM_IDENTIFIER_DR:            
                DumpStreamIdentifierDescriptor(dvbpsi_DecodeStreamIdentifierDr(p_descriptor));     
                break;       
            case SUBTITLING_DR:      
                DumpSubtitleDescriptor(dvbpsi_DecodeSubtitlingDr(p_descriptor));      
                break;        
            default:        
                printf(  "\"");         
                for(i = 0; i < p_descriptor->i_length; i++)  
                    printf(  "%c", p_descriptor->p_data[i]);   
                printf( "\"\n");        
        }        
        p_descriptor = p_descriptor->p_next;    
    }
}

static void DumpCAT(void* p_data, dvbpsi_cat_t* p_cat)
{
    dvbpsi_descriptor_t* p_descriptor = p_cat->p_first_descriptor;  
    ts_stream_t* p_stream = (ts_stream_t*) p_data;
    p_stream->cat.i_cat_version=p_cat->i_version;
    printf("\n");
    printf("New active CAT\n");
    printf("  version number %d\n",p_cat->i_version);
    while(p_descriptor)  
    {
        uint16_t system_id = p_descriptor->p_data[0]<<8|p_descriptor->p_data[1];
        uint16_t emm_pid = p_descriptor->p_data[2]<<8|p_descriptor->p_data[3];
        p_stream->cat.i_number++;
        printf("  cat system id 0x%04x    emm pid 0x%04x\n",system_id,emm_pid);
        p_descriptor = p_descriptor->p_next;
    }
}


static void DumpPMT(void* p_data, dvbpsi_pmt_t* p_pmt)
{
    dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;   
    ts_stream_t* p_stream = (ts_stream_t*) p_data;   
    p_stream->pmt.i_pmt_version = p_pmt->i_version;   
    p_stream->pmt.pid_pcr = &p_stream->pid[p_pmt->i_pcr_pid];    
    p_stream->pid[p_pmt->i_pcr_pid].b_pcr = VLC_TRUE;    
    printf("\n" );
    printf( "New active PMT\n" ); 
    printf( "  program_number : %d\n", p_pmt->i_program_number );   
    printf( "  version_number : %d\n", p_pmt->i_version );    
    printf( "  PCR_PID        : 0x%x (%d)\n", p_pmt->i_pcr_pid, p_pmt->i_pcr_pid);
    DumpDescriptors("    ]", p_pmt->p_first_descriptor);   
    printf( "    | type @ elementary_PID\n");    
    while(p_es)
    {       
        printf( "    | 0x%02x (%s) @ 0x%x (%d)\n", p_es->i_type, GetTypeName(p_es->i_type),p_es->i_pid, p_es->i_pid);    
        DumpDescriptors("    |  ]", p_es->p_first_descriptor);        
        p_es = p_es->p_next;    
    }    
    dvbpsi_pmt_delete(p_pmt);
}

static void DumpPAT(void* p_data, dvbpsi_pat_t* p_pat)
{
    dvbpsi_pat_program_t* p_program = p_pat->p_first_program;  
    ts_stream_t* p_stream = (ts_stream_t*) p_data;
    if (p_stream->pmt.handle)   
    {        
        printf("freeing old PMT\n");        
        dvbpsi_pmt_detach(p_stream->pmt.handle);     
        dvbpsi_delete(p_stream->pmt.handle);     
        p_stream->pmt.handle = NULL;   
    }
    p_stream->pat.i_pat_version = p_pat->i_version;   
    p_stream->pat.i_ts_id = p_pat->i_ts_id;

    
    printf(  "\n");  
    printf(  "New PAT\n");  
    printf(  "  transport_stream_id : %d\n", p_pat->i_ts_id);  
    printf(  "  version_number      : %d\n", p_pat->i_version);  
    printf(  "    | program_number @ PMT_PID\n");
    while(p_program)  
    {   
        if (p_stream->pmt.handle)           
        {
            dvbpsi_pmt_detach(p_stream->pmt.handle);              
            dvbpsi_delete(p_stream->pmt.handle);
            p_stream->pmt.handle = NULL;
        }
        p_stream->i_pmt++;
        p_stream->pmt.i_number = p_program->i_number;
        p_stream->pmt.pid_pmt = &p_stream->pid[p_program->i_pid];
        p_stream->pmt.pid_pmt->i_pid = p_program->i_pid;
        p_stream->pmt.handle = dvbpsi_new(&msg_callback, DVBPSI_MSG_DEBUG);
        if (p_stream->pmt.handle == NULL)
        {
            printf( "could not allocate new dvbpsi_t handle\n");
            break;
        }  
        if (!dvbpsi_pmt_attach(p_stream->pmt.handle, p_program->i_number,DumpPMT, p_stream))   
        {           
            dvbpsi_delete(p_stream->pmt.handle);
            printf("could not attach PMT\n");      
            break;
        }
        printf("    | %14d @ 0x%x (%d)\n",p_program->i_number, p_program->i_pid, p_program->i_pid);
        p_program = p_program->p_next;  
    }  
    printf(  "  active              : %d\n", p_pat->b_current_next);  
    dvbpsi_pat_delete(p_pat);

}

static void handle_TOT(void* p_data, dvbpsi_tot_t* p_tot)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    uint8_t table_id = (p_tot->p_first_descriptor != NULL) ? 0x73 : 0x70;
    if (table_id == 0x70) /* TDT */
        printf("  TDT: Time and Date Table\n");
    else if (table_id == 0x73) /* TOT */
        printf("  TOT: Time Offset Table\n");

    printf("\tVersion number : %d\n", p_tot->i_version);
    printf("\tCurrent next   : %s\n", p_tot->b_current_next ? "yes" : "no");
    printf("\tUTC time       : %"PRId64"\n", p_tot->i_utc_time);

    DumpDescriptors("\t  |  ]", p_tot->p_first_descriptor);
    dvbpsi_tot_delete(p_tot);
}


static void handle_subtable(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                            void *p_data)
{
    switch (i_table_id)
    {
        case 0x70: /* TDT */
        case 0x73: /* TOT only */
            if (!dvbpsi_tot_attach(p_dvbpsi, i_table_id, i_extension, handle_TOT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach TOT subdecoder\n");
            break;
        default:
            break;

    }

}
int main(int argc,char *argv[])
{



    uint8_t buf[188] = {0};
    int      i_old_cc = -1;

  if(argc<2)
  {
    printf("Usage:tsanalyze <file>\n");
    return -1;
  }
  ts_stream_t *p_stream = NULL;
  //dvbpsi_t *p_dvbpsi;
  int ilen;
  uint32_t allbytes=0;
  int fd=open(argv[1],O_RDONLY);
  if(fd<0)
  {
    printf("File not found\n");
    return -1;
  }
  //buf=(uint8_t *) malloc( sizeof( uint8_t ) * 188 );
  p_stream = (ts_stream_t *) malloc( sizeof(ts_stream_t) );
  if( !p_stream )        
  {
    printf("out of memory\n");
    goto exit;
  }
  
  memset( p_stream, 0, sizeof(ts_stream_t) );
  //read first packet
  ilen =  ReadPacket(fd,buf);

  p_stream->pat.handle = dvbpsi_new(&msg_callback, DVBPSI_MSG_DEBUG);  
  if (p_stream->pat.handle == NULL)
  {
    printf("Context create fail\n");
    goto exit;
  }
  p_stream->cat.handle = dvbpsi_new(&msg_callback, DVBPSI_MSG_DEBUG);  
  if (p_stream->cat.handle == NULL)
  {
    printf("Context create fail\n");
    goto exit;
  }
  
  /* TDT demuxer */
  p_stream->tdt.handle = dvbpsi_new(&msg_callback, DVBPSI_MSG_DEBUG);
  if (p_stream->tdt.handle == NULL)
      goto exit;
  if (!dvbpsi_AttachDemux(p_stream->tdt.handle, handle_subtable, p_stream))
  {
      dvbpsi_delete(p_stream->tdt.handle);
      p_stream->tdt.handle = NULL;
      goto exit;
  }
    
  if(!dvbpsi_pat_attach(p_stream->pat.handle, DumpPAT, p_stream))
  {
    goto exit;
  }
  if(!dvbpsi_cat_attach(p_stream->cat.handle, DumpCAT, p_stream))
  {
    goto exit;
  }
  while(ilen > 0)
  {
        allbytes += ilen;
        int i = 0;
        for( i = 0; i < ilen; i += 188 )        
        {
            uint8_t   *p_tmp = &buf[i];
            uint16_t   i_pid = ((uint16_t)(p_tmp[1] & 0x1f) << 8) + p_tmp[2];            
            int i_cc = (p_tmp[3] & 0x0f);
            vlc_bool_t b_adaptation = (p_tmp[3] & 0x20); /* adaptation field */            
            vlc_bool_t b_discontinuity_seen = VLC_FALSE;
            if (buf[i] != 0x47) /* no sync skip this packet */            
            {
                printf("Missing TS sync word, skipping 188 bytes\n" );
                break;
            }
            if (i_pid == 0x1FFF) /* null packet - TS content undefined */                
                continue;
            if(i_pid == 0x0)
            {
            	dvbpsi_packet_push(p_stream->pat.handle, p_tmp);
            }
            else if(i_pid == 0x1)
            {            	
                dvbpsi_packet_push(p_stream->cat.handle, p_tmp);
            }else if (i_pid == 0x02) /* Transport Stream Description Table */
            {
                dvbpsi_packet_push(p_stream->tdt.handle, p_tmp);
             }
            else if(i_pid==0x11)
            {
	       //dvbpsi_packet_push(p_stream->sdt.handle, p_tmp);
            }
                    else if (i_pid == 0x14) /* TDT/TOT */
            {dvbpsi_packet_push(p_stream->tdt.handle, p_tmp);}
            else if( p_stream->pmt.pid_pmt && i_pid == p_stream->pmt.pid_pmt->i_pid )                
                dvbpsi_packet_push(p_stream->pmt.handle, p_tmp);

            /* Remember PID */            
            if( !p_stream->pid[i_pid].b_seen )            
            {                
                p_stream->pid[i_pid].b_seen = VLC_TRUE;                
                i_old_cc = i_cc;                
                p_stream->pid[i_pid].i_cc = i_cc;            
            }else{                
            /* Check continuity counter */                
                int i_diff = 0;                
                i_diff = i_cc - (p_stream->pid[i_pid].i_cc+1)%16;                
                b_discontinuity_seen = ( i_diff != 0 );                /* Update CC */                
                i_old_cc = p_stream->pid[i_pid].i_cc;                
                p_stream->pid[i_pid].i_cc = i_cc;            
            }

            if( b_adaptation )            
            {
                vlc_bool_t b_discontinuity_indicator = (p_tmp[5]&0x80);                
                vlc_bool_t b_random_access_indicator = (p_tmp[5]&0x40);              
                vlc_bool_t b_pcr = (p_tmp[5]&0x10);  /* PCR flag */              
                if( b_discontinuity_indicator )                    
                    printf( "Discontinuity indicator (pid %d)\n", i_pid );       
                if( b_random_access_indicator )                 
                    printf( "Random access indicator (pid %d)\n", i_pid );
                /* Dump PCR */               
                if( b_pcr && (p_tmp[4] >= 7) )           
                {                    
                    mtime_t i_pcr;  /* 33 bits */                  
                    i_pcr = ( ( (mtime_t)p_tmp[6] << 25 )|
                                    ( (mtime_t)p_tmp[7] << 17 ) |
                                    ( (mtime_t)p_tmp[8] << 9 ) |
                                    ( (mtime_t)p_tmp[9] << 1 ) |
                                    ( (mtime_t)p_tmp[10] >> 7 ) ) / 90;    
                    p_stream->pid[i_pid].i_pcr = i_pcr;
                    allbytes = 0; /* reset byte counter */
                    if( b_discontinuity_indicator )              
                    {
                        /* cc discontinuity is expected */                    
                        printf( "Server signalled the continuity counter discontinuity\n" );         
                        /* Discontinuity has been handled */                 
                        b_discontinuity_seen = VLC_FALSE;                  
                    }
                }
            }

            if( b_discontinuity_seen )       
            {                
                printf( "Continuity counter discontinuity (pid %d found %d expected %d)\n",i_pid, p_stream->pid[i_pid].i_cc, i_old_cc+1 );                /* Discontinuity has been handled */             
                b_discontinuity_seen = VLC_FALSE;           
            }  
            
        }
        
        ilen =  ReadPacket(fd,buf);
        
  }
  
exit:
    close(fd); 
    //if( buf )    free( buf );
    if( p_stream->pmt.handle )    
    {   
        dvbpsi_pmt_detach( p_stream->pmt.handle );        
        dvbpsi_delete( p_stream->pmt.handle );   
    }
   if (p_stream->pat.handle)  
    {    
        dvbpsi_pat_detach(p_stream->pat.handle);    
        dvbpsi_delete(p_stream->pat.handle);  
    }
   if (p_stream->cat.handle)  
    {    
        dvbpsi_cat_detach(p_stream->cat.handle);    
        dvbpsi_delete(p_stream->cat.handle);  
    }   
   if (dvbpsi_decoder_present(p_stream->sdt.handle))
       dvbpsi_DetachDemux(p_stream->sdt.handle);
   if( p_stream )  free( p_stream );
  return 0;
}
