#ifndef _SCTE_H_
#define _SCTE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum splice_command_type {
	SPLICE_NULL = 0,
	/* reserved */
	SPLICE_SCEDULE = 4,
	SPLICE_INSERT = 5,
	SPLICE_TIME_SIGNAL = 6,
	SPLICE_BANDWIDTH_RESERVATION = 7,
	/* reserved */
	SPLICE_PRIVATE = 0xff,
};

struct splice_scedule {
    uint8_t splice_count;
    struct {
        uint32_t splice_event_id;
        uint8_t splice_event_cacel_indicator : 1;
        uint8_t reserved : 7;
        /*if splice_event_cacel_indicator == 0*/
        uint8_t out_of_network_indicator : 1;
        uint8_t program_splice_flag : 1;
        uint8_t duration_flag : 1;
        uint8_t reserved1 : 5;
        /* program_splice_flag == 1 */
        uint32_t utc_splice_time;
        /* program_splice_flag == 0 */
        uint8_t component_count;
        struct {
            uint8_t component_tag;
            uint32_t utc_splice_time;
        } *components;
        /* duration_flag > 0*/
        //break_duration;

        uint16_t unique_program_id;
        uint8_t avail_num;
        uint8_t avails_expected;
    }* splices;
};

struct splice_insert {
    uint32_t splice_event_id;
    uint8_t splice_event_cancel_indicator:1;
    uint8_t reserved:7;

};

struct splice_time {
	uint8_t time_specified_flag :1;
	uint8_t reserved: 6;
	uint8_t pts_time_h:1;
	/* only when time_specified_flag == 1*/
	uint32_t pts_time;
};

struct break_duration {
    uint8_t auto_return :1;
    uint8_t reserved : 6;
    uint8_t duration_h :1;
    uint32_t duration;
};

enum sap_type {
	SAP_TYPE_1 = 0,
	SAP_TYPE_2 = 1,
	SAP_TYPE_3 = 2,
	SAP_TYPE_NOT_SPECIFIED = 3
};

typedef struct splice_info {
	uint16_t pid;

	uint8_t table_id;
	uint16_t section_syntax_indicator : 1;
	uint16_t private_indicator : 1;
	uint16_t sap_type : 2;
	uint16_t section_length : 12;
	uint8_t protocol_version;
	uint8_t encrypted_packet : 1;
	uint8_t encryption_algorithm : 6;
	uint8_t pts_adjustment_h : 1;
	uint32_t pts_adjustment;
	uint8_t cw_index;
	uint32_t tier : 12;
	uint32_t splice_command_length : 12;
	uint32_t splice_command_type : 8;

	uint16_t descriptor_loop_length;
	struct list_head list; /* splice descriptor list */
	/* alignment_stuffing */
	uint32_t e_crc32;
	uint32_t crc32;
} scte_t;


void dump_scte_info(void);

void register_scte_ops(uint16_t pid);

void unregister_scte_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* _SCTE_H_ */