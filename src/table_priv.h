#ifndef _TABLE_PRIV_H_
#define _TABLE_PRIV_H_

/*
 * Internal interface shared only between the table modules
 * (src/table.c, src/dvb_si.c, src/atsc_psip.c).  These symbols are private
 * to this group and must NOT be relied upon by other subsystems.
 */

#include "table.h"

/* ---- section reassembly helpers (defined in src/table.c) ---- */
int check_section_header_version(uint8_t *pbuf, uint16_t bufsize, uint8_t cur_version);
int parse_section_header(uint8_t *pbuf, uint16_t buf_size, struct table_header *ptable);
void dump_section_header(const char *table_name, struct table_header *hdr);

/* ---- DVB SI (ETSI EN 300 468) parsers / dumpers (src/dvb_si.c) ---- */
void dump_tdt(tdt_t *p_tdt);
void dump_tot(tot_t *p_tot);
void dump_sdt(sdt_t *p_sdt);
void dump_bat(bat_t *p_bat);
void dump_nit(nit_t *p_nit);
void dump_eit(eit_t *p_eit);

int parse_nit(uint8_t *pbuf, uint16_t buf_size, nit_t *p_nit);
int parse_bat(uint8_t *pbuf, uint16_t buf_size, bat_t *p_bat);
int parse_sdt(uint8_t *pbuf, uint16_t buf_size, sdt_t *p_sdt);
int parse_eit(uint8_t *pbuf, uint16_t buf_size, eit_t *p_eit);
int parse_tdt(uint8_t *pbuf, uint16_t buf_size, tdt_t *p_tdt);
int parse_tot(uint8_t *pbuf, uint16_t buf_size, tot_t *p_tot);

/* ---- ATSC PSIP (A/65) (src/atsc_psip.c) ---- */
void atsc_init_tables(void);
bool atsc_tables_seen(void);
void dump_atsc_tables(void);
int atsc_psip_proc(uint16_t pid, uint8_t *pkt, uint16_t len);

#endif /* _TABLE_PRIV_H_ */