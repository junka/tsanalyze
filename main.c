

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

#include "ts.h"
#include "table.h"


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
