#ifndef _SCTE_H_
#define _SCTE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum splice_command_type {
	SPLICE_NULL = 0,
	/* reserved */
	SPLICE_SCHEDULE = 4,
	SPLICE_INSERT = 5,
	SPLICE_TIME_SIGNAL = 6,
	SPLICE_BANDWIDTH_RESERVATION = 7,
	/* reserved */
	SPLICE_PRIVATE = 0xff,
};


struct splice_time {
	uint8_t time_specified_flag :1;
	uint8_t reserved: 6;
	uint8_t pts_time_h:1;
	/* only when time_specified_flag == 1*/
	uint32_t pts_time;
};

struct break_duration {
    uint8_t auto_return:1;
    uint8_t reserved:6;
    uint8_t duration_h:1;
    uint32_t duration;
} __attribute__((packed));


struct splice_event {
    uint32_t splice_event_id;
    uint8_t splice_event_cancel_indicator: 1;
    uint8_t reserved:7;

    uint8_t out_of_network_indicator:1;
    uint8_t program_splice_flag:1;
    uint8_t duration_flag:1;
    uint8_t splice_immediate_flag:1;
    uint8_t reserved2:4;

	union {
		uint32_t utc_splice_time;
		struct splice_time time;
	};

    uint8_t component_count;
    struct event_component {
        uint8_t component_tag;
		union {
        	uint32_t utc_splice_time;
			struct splice_time time;
		};
    } *components;

    struct break_duration duration;

    uint16_t unique_program_id;
    uint8_t avail_num;
    uint8_t avails_expected;
};

struct splice_schedule {
    uint8_t splice_count;
    struct splice_event* splices;
};

enum sap_type {
	SAP_TYPE_1 = 0,
	SAP_TYPE_2 = 1,
	SAP_TYPE_3 = 2,
	SAP_TYPE_NOT_SPECIFIED = 3
};

enum encryption_algorithm {
    NO_ENCRYPTION = 0,
    DES_ECB = 1,
    DES_CBC = 2,
    TRIPLE_DES_EDE3 = 3,
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
    union {
        struct splice_schedule schedule;
        struct splice_event evt;
        struct splice_time t;
    };
	uint16_t descriptor_loop_length;
	struct list_head list; /* splice descriptor list */
	/* alignment_stuffing */
	uint32_t e_crc32;
	uint32_t crc32;
} scte_t;

#define foreach_enum_scte_splice_descriptor \
    _(avail, 0x0)   \
    _(DTMF, 0x01) \
    _(segmentation, 0x2) \
    _(time, 0x3) \
    _(audio, 0x4) 

struct avail_splice_descriptor {
    uint8_t splice_desciptor_tag;
    uint8_t descriptor_length;
	struct list_node n;
    uint32_t identifier;
    uint32_t provider_avail_id;
};

struct DTMF_splice_descriptor {
    uint8_t splice_desciptor_tag;
    uint8_t descriptor_length;
    uint32_t identifier;
    uint8_t preroll;
    uint8_t dtmf_count:3;
    uint8_t reserved:5;
    uint8_t *DTMF_char;
};

struct segmentation_splice_descriptor {
    uint8_t splice_desciptor_tag;
    uint8_t descriptor_length;
    uint32_t identifier;
    uint32_t segmentation_event_id;
    uint8_t segmentation_event_cancel_indicator:1;
    uint8_t reserved:7;

    uint8_t program_segmentation_flag:1;
    uint8_t segmentation_duration_flag:1;
    uint8_t delivery_not_restricted_flag:1;
    uint8_t web_delivery_allowed_flag:1;
    uint8_t no_regional_blackout_flag:1;
    uint8_t archive_allowed_flag:1;
    uint8_t device_restrictions:2;

    uint8_t component_count;
    struct segmentation_component {
        uint8_t component_tag;
        uint8_t reserved:7;
        uint8_t pts_offset_h:1;
        uint32_t pts_offset;
    } * components;

    uint40_t segmentation_duration;

    uint8_t segmentation_upid_type;
    uint8_t segmentation_upid_length;

    uint8_t segmentation_type_id;
    uint8_t segment_num;
    uint8_t segments_expected;
    uint8_t sub_segment_num;
    uint8_t sub_segments_expected;

};

void dump_scte_info(void);

void register_scte_ops(uint16_t pid);

void unregister_scte_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* _SCTE_H_ */