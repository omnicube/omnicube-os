#ifndef SPDK_NVME_SPEC_H
#define SPDK_NVME_SPEC_H

#include <stdint.h>
#include <stddef.h>

/**
 * \file
 *
 */

/**
 * PCI class code for NVMe devices.
 *
 * Base class code 01h: mass storage
 * Subclass code 08h: non-volatile memory
 * Programming interface 02h: NVM Express
 */
#define NVME_CLASS_CODE 0x10802

/**
 * Use to mark a command to apply to all namespaces, or to retrieve global
 *  log pages.
 */
#define NVME_GLOBAL_NAMESPACE_TAG	((uint32_t)0xFFFFFFFF)

#define NVME_MAX_IO_QUEUES		(1 << 16)

union nvme_cap_lo_register {
	uint32_t	raw;
	struct {
		/** maximum queue entries supported */
		uint32_t mqes		: 16;

		/** contiguous queues required */
		uint32_t cqr		: 1;

		/** arbitration mechanism supported */
		uint32_t ams		: 2;

		uint32_t reserved1	: 5;

		/** timeout */
		uint32_t to		: 8;
	} bits;
};
_Static_assert(sizeof(union nvme_cap_lo_register) == 4, "Incorrect size");

union nvme_cap_hi_register {
	uint32_t	raw;
	struct {
		/** doorbell stride */
		uint32_t dstrd		: 4;

		uint32_t reserved3	: 1;

		/** command sets supported */
		uint32_t css_nvm	: 1;

		uint32_t css_reserved	: 3;
		uint32_t reserved2	: 7;

		/** memory page size minimum */
		uint32_t mpsmin		: 4;

		/** memory page size maximum */
		uint32_t mpsmax		: 4;

		uint32_t reserved1	: 8;
	} bits;
};
_Static_assert(sizeof(union nvme_cap_hi_register) == 4, "Incorrect size");

union nvme_cc_register {
	uint32_t	raw;
	struct {
		/** enable */
		uint32_t en		: 1;

		uint32_t reserved1	: 3;

		/** i/o command set selected */
		uint32_t css		: 3;

		/** memory page size */
		uint32_t mps		: 4;

		/** arbitration mechanism selected */
		uint32_t ams		: 3;

		/** shutdown notification */
		uint32_t shn		: 2;

		/** i/o submission queue entry size */
		uint32_t iosqes		: 4;

		/** i/o completion queue entry size */
		uint32_t iocqes		: 4;

		uint32_t reserved2	: 8;
	} bits;
};
_Static_assert(sizeof(union nvme_cc_register) == 4, "Incorrect size");

enum nvme_shn_value {
	NVME_SHN_NORMAL		= 0x1,
	NVME_SHN_ABRUPT		= 0x2,
};

union nvme_csts_register {
	uint32_t	raw;
	struct {
		/** ready */
		uint32_t rdy		: 1;

		/** controller fatal status */
		uint32_t cfs		: 1;

		/** shutdown status */
		uint32_t shst		: 2;

		uint32_t reserved1	: 28;
	} bits;
};
_Static_assert(sizeof(union nvme_csts_register) == 4, "Incorrect size");

enum nvme_shst_value {
	NVME_SHST_NORMAL	= 0x0,
	NVME_SHST_OCCURRING	= 0x1,
	NVME_SHST_COMPLETE	= 0x2,
};

union nvme_aqa_register {
	uint32_t	raw;
	struct {
		/** admin submission queue size */
		uint32_t asqs		: 12;

		uint32_t reserved1	: 4;

		/** admin completion queue size */
		uint32_t acqs		: 12;

		uint32_t reserved2	: 4;
	} bits;
};
_Static_assert(sizeof(union nvme_aqa_register) == 4, "Incorrect size");

struct nvme_registers {
	/** controller capabilities */
	union nvme_cap_lo_register	cap_lo;
	union nvme_cap_hi_register	cap_hi;

	uint32_t	vs;		/* version */
	uint32_t	intms;		/* interrupt mask set */
	uint32_t	intmc;		/* interrupt mask clear */

	/** controller configuration */
	union nvme_cc_register	cc;

	uint32_t	reserved1;
	uint32_t	csts;		/* controller status */
	uint32_t	nssr;		/* NVM subsystem reset */

	/** admin queue attributes */
	union nvme_aqa_register	aqa;

	uint64_t	asq;		/* admin submission queue base addr */
	uint64_t	acq;		/* admin completion queue base addr */
	uint32_t	reserved3[0x3f2];

	struct {
		uint32_t	sq_tdbl;	/* submission queue tail doorbell */
		uint32_t	cq_hdbl;	/* completion queue head doorbell */
	} doorbell[1];
};

/* NVMe controller register space offsets */
_Static_assert(0x00 == offsetof(struct nvme_registers, cap_lo), "Incorrect register offset");
_Static_assert(0x08 == offsetof(struct nvme_registers, vs), "Incorrect register offset");
_Static_assert(0x0C == offsetof(struct nvme_registers, intms), "Incorrect register offset");
_Static_assert(0x10 == offsetof(struct nvme_registers, intmc), "Incorrect register offset");
_Static_assert(0x14 == offsetof(struct nvme_registers, cc), "Incorrect register offset");
_Static_assert(0x1C == offsetof(struct nvme_registers, csts), "Incorrect register offset");
_Static_assert(0x20 == offsetof(struct nvme_registers, nssr), "Incorrect register offset");
_Static_assert(0x24 == offsetof(struct nvme_registers, aqa), "Incorrect register offset");
_Static_assert(0x28 == offsetof(struct nvme_registers, asq), "Incorrect register offset");
_Static_assert(0x30 == offsetof(struct nvme_registers, acq), "Incorrect register offset");

enum nvme_sgl_descriptor_type {
	NVME_SGL_TYPE_DATA_BLOCK	= 0x0,
	NVME_SGL_TYPE_BIT_BUCKET	= 0x1,
	NVME_SGL_TYPE_SEGMENT		= 0x2,
	NVME_SGL_TYPE_LAST_SEGMENT	= 0x3,
	/* 0x4 - 0xe reserved */
	NVME_SGL_TYPE_VENDOR_SPECIFIC	= 0xf
};

struct __attribute__((packed)) nvme_sgl_descriptor {
	uint64_t address;
	uint32_t length;
	uint8_t reserved[3];

	/** SGL descriptor type */
	uint8_t type : 4;

	/** SGL descriptor type specific */
	uint8_t type_specific : 4;
};
_Static_assert(sizeof(struct nvme_sgl_descriptor) == 16, "Incorrect size");

enum nvme_psdt_value {
	NVME_PSDT_PRP			= 0x0,
	NVME_PSDT_SGL_MPTR_CONTIG	= 0x1,
	NVME_PSDT_SGL_MPTR_SGL		= 0x2,
	NVME_PSDT_RESERVED		= 0x3
};

struct nvme_command {
	/* dword 0 */
	uint16_t opc	:  8;	/* opcode */
	uint16_t fuse	:  2;	/* fused operation */
	uint16_t rsvd1	:  4;
	uint16_t psdt	:  2;
	uint16_t cid;		/* command identifier */

	/* dword 1 */
	uint32_t nsid;		/* namespace identifier */

	/* dword 2-3 */
	uint32_t rsvd2;
	uint32_t rsvd3;

	/* dword 4-5 */
	uint64_t mptr;		/* metadata pointer */

	/* dword 6-9: data pointer */
	union {
		struct {
			uint64_t prp1;		/* prp entry 1 */
			uint64_t prp2;		/* prp entry 2 */
		} prp;

		struct nvme_sgl_descriptor sgl1;
	} dptr;

	/* dword 10-15 */
	uint32_t cdw10;		/* command-specific */
	uint32_t cdw11;		/* command-specific */
	uint32_t cdw12;		/* command-specific */
	uint32_t cdw13;		/* command-specific */
	uint32_t cdw14;		/* command-specific */
	uint32_t cdw15;		/* command-specific */
};
_Static_assert(sizeof(struct nvme_command) == 64, "Incorrect size");

struct nvme_status {
	uint16_t p	:  1;	/* phase tag */
	uint16_t sc	:  8;	/* status code */
	uint16_t sct	:  3;	/* status code type */
	uint16_t rsvd2	:  2;
	uint16_t m	:  1;	/* more */
	uint16_t dnr	:  1;	/* do not retry */
};
_Static_assert(sizeof(struct nvme_status) == 2, "Incorrect size");

struct nvme_completion {
	/* dword 0 */
	uint32_t		cdw0;	/* command-specific */

	/* dword 1 */
	uint32_t		rsvd1;

	/* dword 2 */
	uint16_t		sqhd;	/* submission queue head pointer */
	uint16_t		sqid;	/* submission queue identifier */

	/* dword 3 */
	uint16_t		cid;	/* command identifier */
	struct nvme_status	status;
};
_Static_assert(sizeof(struct nvme_completion) == 16, "Incorrect size");

struct nvme_dsm_range {
	uint32_t attributes;
	uint32_t length;
	uint64_t starting_lba;
};
_Static_assert(sizeof(struct nvme_dsm_range) == 16, "Incorrect size");

/* status code types */
enum nvme_status_code_type {
	NVME_SCT_GENERIC		= 0x0,
	NVME_SCT_COMMAND_SPECIFIC	= 0x1,
	NVME_SCT_MEDIA_ERROR		= 0x2,
	/* 0x3-0x6 - reserved */
	NVME_SCT_VENDOR_SPECIFIC	= 0x7,
};

/* generic command status codes */
enum nvme_generic_command_status_code {
	NVME_SC_SUCCESS				= 0x00,
	NVME_SC_INVALID_OPCODE			= 0x01,
	NVME_SC_INVALID_FIELD			= 0x02,
	NVME_SC_COMMAND_ID_CONFLICT		= 0x03,
	NVME_SC_DATA_TRANSFER_ERROR		= 0x04,
	NVME_SC_ABORTED_POWER_LOSS		= 0x05,
	NVME_SC_INTERNAL_DEVICE_ERROR		= 0x06,
	NVME_SC_ABORTED_BY_REQUEST		= 0x07,
	NVME_SC_ABORTED_SQ_DELETION		= 0x08,
	NVME_SC_ABORTED_FAILED_FUSED		= 0x09,
	NVME_SC_ABORTED_MISSING_FUSED		= 0x0a,
	NVME_SC_INVALID_NAMESPACE_OR_FORMAT	= 0x0b,
	NVME_SC_COMMAND_SEQUENCE_ERROR		= 0x0c,

	NVME_SC_LBA_OUT_OF_RANGE		= 0x80,
	NVME_SC_CAPACITY_EXCEEDED		= 0x81,
	NVME_SC_NAMESPACE_NOT_READY		= 0x82,
};

/* command specific status codes */
enum nvme_command_specific_status_code {
	NVME_SC_COMPLETION_QUEUE_INVALID	= 0x00,
	NVME_SC_INVALID_QUEUE_IDENTIFIER	= 0x01,
	NVME_SC_MAXIMUM_QUEUE_SIZE_EXCEEDED	= 0x02,
	NVME_SC_ABORT_COMMAND_LIMIT_EXCEEDED	= 0x03,
	/* 0x04 - reserved */
	NVME_SC_ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED = 0x05,
	NVME_SC_INVALID_FIRMWARE_SLOT		= 0x06,
	NVME_SC_INVALID_FIRMWARE_IMAGE		= 0x07,
	NVME_SC_INVALID_INTERRUPT_VECTOR	= 0x08,
	NVME_SC_INVALID_LOG_PAGE		= 0x09,
	NVME_SC_INVALID_FORMAT			= 0x0a,
	NVME_SC_FIRMWARE_REQUIRES_RESET		= 0x0b,

	NVME_SC_CONFLICTING_ATTRIBUTES		= 0x80,
	NVME_SC_INVALID_PROTECTION_INFO		= 0x81,
	NVME_SC_ATTEMPTED_WRITE_TO_RO_PAGE	= 0x82,
};

/* media error status codes */
enum nvme_media_error_status_code {
	NVME_SC_WRITE_FAULTS			= 0x80,
	NVME_SC_UNRECOVERED_READ_ERROR		= 0x81,
	NVME_SC_GUARD_CHECK_ERROR		= 0x82,
	NVME_SC_APPLICATION_TAG_CHECK_ERROR	= 0x83,
	NVME_SC_REFERENCE_TAG_CHECK_ERROR	= 0x84,
	NVME_SC_COMPARE_FAILURE			= 0x85,
	NVME_SC_ACCESS_DENIED			= 0x86,
};

/* admin opcodes */
enum nvme_admin_opcode {
	NVME_OPC_DELETE_IO_SQ			= 0x00,
	NVME_OPC_CREATE_IO_SQ			= 0x01,
	NVME_OPC_GET_LOG_PAGE			= 0x02,
	/* 0x03 - reserved */
	NVME_OPC_DELETE_IO_CQ			= 0x04,
	NVME_OPC_CREATE_IO_CQ			= 0x05,
	NVME_OPC_IDENTIFY			= 0x06,
	/* 0x07 - reserved */
	NVME_OPC_ABORT				= 0x08,
	NVME_OPC_SET_FEATURES			= 0x09,
	NVME_OPC_GET_FEATURES			= 0x0a,
	/* 0x0b - reserved */
	NVME_OPC_ASYNC_EVENT_REQUEST		= 0x0c,
	NVME_OPC_NAMESPACE_MANAGEMENT		= 0x0d,
	/* 0x0e-0x0f - reserved */
	NVME_OPC_FIRMWARE_COMMIT		= 0x10,
	NVME_OPC_FIRMWARE_IMAGE_DOWNLOAD	= 0x11,

	NVME_OPC_NAMESPACE_ATTACHMENT		= 0x15,

	NVME_OPC_FORMAT_NVM			= 0x80,
	NVME_OPC_SECURITY_SEND			= 0x81,
	NVME_OPC_SECURITY_RECEIVE		= 0x82,
};

/* nvme nvm opcodes */
enum nvme_nvm_opcode {
	NVME_OPC_FLUSH				= 0x00,
	NVME_OPC_WRITE				= 0x01,
	NVME_OPC_READ				= 0x02,
	/* 0x03 - reserved */
	NVME_OPC_WRITE_UNCORRECTABLE		= 0x04,
	NVME_OPC_COMPARE			= 0x05,
	/* 0x06-0x07 - reserved */
	NVME_OPC_WRITE_ZEROES			= 0x08,
	NVME_OPC_DATASET_MANAGEMENT		= 0x09,

	NVME_OPC_RESERVATION_REGISTER		= 0x0d,
	NVME_OPC_RESERVATION_REPORT		= 0x0e,

	NVME_OPC_RESERVATION_ACQUIRE		= 0x11,
	NVME_OPC_RESERVATION_RELEASE		= 0x15,
};

enum nvme_feature {
	/* 0x00 - reserved */
	NVME_FEAT_ARBITRATION			= 0x01,
	NVME_FEAT_POWER_MANAGEMENT		= 0x02,
	NVME_FEAT_LBA_RANGE_TYPE		= 0x03,
	NVME_FEAT_TEMPERATURE_THRESHOLD		= 0x04,
	NVME_FEAT_ERROR_RECOVERY		= 0x05,
	NVME_FEAT_VOLATILE_WRITE_CACHE		= 0x06,
	NVME_FEAT_NUMBER_OF_QUEUES		= 0x07,
	NVME_FEAT_INTERRUPT_COALESCING		= 0x08,
	NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION = 0x09,
	NVME_FEAT_WRITE_ATOMICITY		= 0x0A,
	NVME_FEAT_ASYNC_EVENT_CONFIGURATION	= 0x0B,
	/* 0x0C-0x7F - reserved */
	NVME_FEAT_SOFTWARE_PROGRESS_MARKER	= 0x80,
	/* 0x81-0xBF - command set specific (reserved) */
	/* 0xC0-0xFF - vendor specific */
};

enum nvme_dsm_attribute {
	NVME_DSM_ATTR_INTEGRAL_READ		= 0x1,
	NVME_DSM_ATTR_INTEGRAL_WRITE		= 0x2,
	NVME_DSM_ATTR_DEALLOCATE		= 0x4,
};

struct nvme_power_state {
	uint16_t mp;				/* bits 15:00: maximum power */

	uint8_t reserved1;

	uint8_t mps		: 1;		/* bit 24: max power scale */
	uint8_t nops		: 1;		/* bit 25: non-operational state */
	uint8_t reserved2	: 6;

	uint32_t enlat;				/* bits 63:32: entry latency in microseconds */
	uint32_t exlat;				/* bits 95:64: exit latency in microseconds */

	uint8_t rrt		: 5;		/* bits 100:96: relative read throughput */
	uint8_t reserved3	: 3;

	uint8_t rrl		: 5;		/* bits 108:104: relative read latency */
	uint8_t reserved4	: 3;

	uint8_t rwt		: 5;		/* bits 116:112: relative write throughput */
	uint8_t reserved5	: 3;

	uint8_t rwl		: 5;		/* bits 124:120: relative write latency */
	uint8_t reserved6	: 3;

	uint8_t reserved7[16];
};
_Static_assert(sizeof(struct nvme_power_state) == 32, "Incorrect size");

struct __attribute__((packed)) nvme_controller_data {
	/* bytes 0-255: controller capabilities and features */

	/** pci vendor id */
	uint16_t		vid;

	/** pci subsystem vendor id */
	uint16_t		ssvid;

	/** serial number */
	int8_t			sn[20];

	/** model number */
	int8_t			mn[40];

	/** firmware revision */
	uint8_t			fr[8];

	/** recommended arbitration burst */
	uint8_t			rab;

	/** ieee oui identifier */
	uint8_t			ieee[3];

	/** multi-interface capabilities */
	uint8_t			mic;

	/** maximum data transfer size */
	uint8_t			mdts;

	/** controller id */
	uint16_t		cntlid;

	/** version */
	uint32_t		ver;

	/** RTD3 resume latency */
	uint32_t		rtd3r;

	/** RTD3 entry latency */
	uint32_t		rtd3e;

	/** optional asynchronous events supported */
	uint32_t		oaes;

	uint8_t			reserved1[160];

	/* bytes 256-511: admin command set attributes */

	/** optional admin command support */
	struct {
		/* supports security send/receive commands */
		uint16_t	security  : 1;

		/* supports format nvm command */
		uint16_t	format    : 1;

		/* supports firmware activate/download commands */
		uint16_t	firmware  : 1;

		uint16_t	oacs_rsvd : 13;
	} oacs;

	/** abort command limit */
	uint8_t			acl;

	/** asynchronous event request limit */
	uint8_t			aerl;

	/** firmware updates */
	struct {
		/* first slot is read-only */
		uint8_t		slot1_ro  : 1;

		/* number of firmware slots */
		uint8_t		num_slots : 3;

		uint8_t		frmw_rsvd : 4;
	} frmw;

	/** log page attributes */
	struct {
		/* per namespace smart/health log page */
		uint8_t		ns_smart : 1;

		uint8_t		lpa_rsvd : 7;
	} lpa;

	/** error log page entries */
	uint8_t			elpe;

	/** number of power states supported */
	uint8_t			npss;

	/** admin vendor specific command configuration */
	struct {
		/* admin vendor specific commands use disk format */
		uint8_t		spec_format : 1;

		uint8_t		avscc_rsvd  : 7;
	} avscc;

	/** autonomous power state transition attributes */
	struct {
		/** controller supports autonomous power state transitions */
		uint8_t		supported  : 1;

		uint8_t		apsta_rsvd : 7;
	} apsta;

	/** warning composite temperature threshold */
	uint16_t		wctemp;

	/** critical composite temperature threshold */
	uint16_t		cctemp;

	/** maximum time for firmware activation */
	uint16_t		mtfa;

	/** host memory buffer preferred size */
	uint32_t		hmpre;

	/** host memory buffer minimum size */
	uint32_t		hmmin;

	/** total NVM capacity */
	uint64_t		tnvmcap[2];

	/** unallocated NVM capacity */
	uint64_t		unvmcap[2];

	/** replay protected memory block support */
	struct {
		uint8_t		num_rpmb_units	: 3;
		uint8_t		auth_method	: 3;
		uint8_t		reserved1	: 2;

		uint8_t		reserved2;

		uint8_t		total_size;
		uint8_t		access_size;
	} rpmbs;

	uint8_t			reserved2[196];

	/* bytes 512-703: nvm command set attributes */

	/** submission queue entry size */
	struct {
		uint8_t		min : 4;
		uint8_t		max : 4;
	} sqes;

	/** completion queue entry size */
	struct {
		uint8_t		min : 4;
		uint8_t		max : 4;
	} cqes;

	uint8_t			reserved3[2];

	/** number of namespaces */
	uint32_t		nn;

	/** optional nvm command support */
	struct {
		uint16_t	compare : 1;
		uint16_t	write_unc : 1;
		uint16_t	dsm: 1;
		uint16_t	reserved: 13;
	} oncs;

	/** fused operation support */
	uint16_t		fuses;

	/** format nvm attributes */
	uint8_t			fna;

	/** volatile write cache */
	struct {
		uint8_t		present : 1;
		uint8_t		reserved : 7;
	} vwc;

	/** atomic write unit normal */
	uint16_t		awun;

	/** atomic write unit power fail */
	uint16_t		awupf;

	/** NVM vendor specific command configuration */
	uint8_t			nvscc;

	uint8_t			reserved531;

	/** atomic compare & write unit */
	uint16_t		acwu;

	uint16_t		reserved534;

	/** SGL support */
	struct {
		uint32_t	supported : 1;
		uint32_t	reserved : 15;
		uint32_t	bit_bucket_descriptor_supported : 1;
		uint32_t	metadata_pointer_supported : 1;
		uint32_t	oversized_sgl_supported : 1;
	} sgls;

	uint8_t			reserved4[164];

	/* bytes 704-2047: i/o command set attributes */
	uint8_t			reserved5[1344];

	/* bytes 2048-3071: power state descriptors */
	struct nvme_power_state	psd[32];

	/* bytes 3072-4095: vendor specific */
	uint8_t			vs[1024];
};
_Static_assert(sizeof(struct nvme_controller_data) == 4096, "Incorrect size");

struct nvme_namespace_data {
	/** namespace size */
	uint64_t		nsze;

	/** namespace capacity */
	uint64_t		ncap;

	/** namespace utilization */
	uint64_t		nuse;

	/** namespace features */
	struct {
		/** thin provisioning */
		uint8_t		thin_prov : 1;
		uint8_t		reserved1 : 7;
	} nsfeat;

	/** number of lba formats */
	uint8_t			nlbaf;

	/** formatted lba size */
	struct {
		uint8_t		format    : 4;
		uint8_t		extended  : 1;
		uint8_t		reserved2 : 3;
	} flbas;

	/** metadata capabilities */
	struct {
		/** metadata can be transferred as part of data prp list */
		uint8_t		extended  : 1;

		/** metadata can be transferred with separate metadata pointer */
		uint8_t		pointer   : 1;

		/** reserved */
		uint8_t		reserved3 : 6;
	} mc;

	/** end-to-end data protection capabilities */
	struct {
		/** protection information type 1 */
		uint8_t		pit1     : 1;

		/** protection information type 2 */
		uint8_t		pit2     : 1;

		/** protection information type 3 */
		uint8_t		pit3     : 1;

		/** first eight bytes of metadata */
		uint8_t		md_start : 1;

		/** last eight bytes of metadata */
		uint8_t		md_end   : 1;
	} dpc;

	/** end-to-end data protection type settings */
	struct {
		/** protection information type */
		uint8_t		pit       : 3;

		/** 1 == protection info transferred at start of metadata */
		/** 0 == protection info transferred at end of metadata */
		uint8_t		md_start  : 1;

		uint8_t		reserved4 : 4;
	} dps;

	/** namespace multi-path I/O and namespace sharing capabilities */
	struct {
		uint8_t		can_share : 1;
		uint8_t		reserved : 7;
	} nmic;

	/** reservation capabilities */
	struct {
		/** supports persist through power loss */
		uint8_t		persist : 1;

		/** supports write exclusive */
		uint8_t		write_exclusive : 1;

		/** supports exclusive access */
		uint8_t		exclusive_access : 1;

		/** supports write exclusive - registrants only */
		uint8_t		write_exclusive_reg_only : 1;

		/** supports exclusive access - registrants only */
		uint8_t		exclusive_access_reg_only : 1;

		/** supports write exclusive - all registrants */
		uint8_t		write_exclusive_all_reg : 1;

		/** supports exclusive access - all registrants */
		uint8_t		exclusive_access_all_reg : 1;

		uint8_t		reserved : 1;
	} rescap;

	/** format progress indicator */
	uint8_t			fpi;

	uint8_t			reserved33;

	/** namespace atomic write unit normal */
	uint16_t		nawun;

	/** namespace atomic write unit power fail */
	uint16_t		nawupf;

	/** namespace atomic compare & write unit */
	uint16_t		nacwu;

	/** namespace atomic boundary size normal */
	uint16_t		nabsn;

	/** namespace atomic boundary offset */
	uint16_t		nabo;

	/** namespace atomic boundary size power fail */
	uint16_t		nabspf;

	uint16_t		reserved46;

	/** NVM capacity */
	uint64_t		nvmcap[2];

	uint8_t			reserved64[40];

	/** namespace globally unique identifier */
	uint8_t			nguid[16];

	/** IEEE extended unique identifier */
	uint64_t		eui64;

	/** lba format support */
	struct {
		/** metadata size */
		uint32_t	ms	  : 16;

		/** lba data size */
		uint32_t	lbads	  : 8;

		/** relative performance */
		uint32_t	rp	  : 2;

		uint32_t	reserved6 : 6;
	} lbaf[16];

	uint8_t			reserved6[192];

	uint8_t			vendor_specific[3712];
};
_Static_assert(sizeof(struct nvme_namespace_data) == 4096, "Incorrect size");

enum nvme_log_page {
	/* 0x00 - reserved */
	NVME_LOG_ERROR			= 0x01,
	NVME_LOG_HEALTH_INFORMATION	= 0x02,
	NVME_LOG_FIRMWARE_SLOT		= 0x03,
	/* 0x04-0x7F - reserved */
	/* 0x80-0xBF - I/O command set specific */
	/* 0xC0-0xFF - vendor specific */
};

struct nvme_error_information_entry {
	uint64_t		error_count;
	uint16_t		sqid;
	uint16_t		cid;
	struct nvme_status	status;
	uint16_t		error_location;
	uint64_t		lba;
	uint32_t		nsid;
	uint8_t			vendor_specific;
	uint8_t			reserved[35];
};
_Static_assert(sizeof(struct nvme_error_information_entry) == 64, "Incorrect size");

union nvme_critical_warning_state {
	uint8_t		raw;

	struct {
		uint8_t	available_spare		: 1;
		uint8_t	temperature		: 1;
		uint8_t	device_reliability	: 1;
		uint8_t	read_only		: 1;
		uint8_t	volatile_memory_backup	: 1;
		uint8_t	reserved		: 3;
	} bits;
};
_Static_assert(sizeof(union nvme_critical_warning_state) == 1, "Incorrect size");

struct __attribute__((packed)) nvme_health_information_page {
	union nvme_critical_warning_state	critical_warning;

	uint16_t		temperature;
	uint8_t			available_spare;
	uint8_t			available_spare_threshold;
	uint8_t			percentage_used;

	uint8_t			reserved[26];

	/*
	 * Note that the following are 128-bit values, but are
	 *  defined as an array of 2 64-bit values.
	 */
	/* Data Units Read is always in 512-byte units. */
	uint64_t		data_units_read[2];
	/* Data Units Written is always in 512-byte units. */
	uint64_t		data_units_written[2];
	/* For NVM command set, this includes Compare commands. */
	uint64_t		host_read_commands[2];
	uint64_t		host_write_commands[2];
	/* Controller Busy Time is reported in minutes. */
	uint64_t		controller_busy_time[2];
	uint64_t		power_cycles[2];
	uint64_t		power_on_hours[2];
	uint64_t		unsafe_shutdowns[2];
	uint64_t		media_errors[2];
	uint64_t		num_error_info_log_entries[2];

	uint8_t			reserved2[320];
};
_Static_assert(sizeof(struct nvme_health_information_page) == 512, "Incorrect size");

struct nvme_firmware_page {
	struct {
		uint8_t	slot		: 3; /* slot for current FW */
		uint8_t	reserved	: 5;
	} afi;

	uint8_t			reserved[7];
	uint64_t		revision[7]; /* revisions for 7 slots */
	uint8_t			reserved2[448];
};
_Static_assert(sizeof(struct nvme_firmware_page) == 512, "Incorrect size");

#define nvme_completion_is_error(cpl)					\
	((cpl)->status.sc != 0 || (cpl)->status.sct != 0)

#endif
