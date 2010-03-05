#ifndef PLMS_HPI_H
#define PLMS_HPI_H

typedef struct
{
	SaUint8T    chassis_area_offset;
	SaUint8T    board_area_offset;
	SaUint8T    product_area_offset;
} PLMS_IDR_HEADER; 

typedef struct
{
	SaUint8T  format_version;
	SaHpiTextBufferT manufacturer_name;
	SaHpiTextBufferT  product_name;
	SaHpiTextBufferT serial_no;
	SaHpiTextBufferT part_no;
	SaHpiTextBufferT product_version;
	SaHpiTextBufferT asset_tag;
	SaHpiTextBufferT fru_field_id;

} PLMS_PRODUCT_INFO_AREA ;

typedef struct
{
	SaUint8T  version;
	SaHpiTextBufferT manufacturer_name;
	SaHpiTextBufferT  product_name;
	SaHpiTextBufferT  serial_no;
	SaHpiTextBufferT  part_no;
	SaHpiTextBufferT fru_field_id;
} PLMS_BOARD_INFO_AREA;	


typedef struct
{
	SaUint8T  format_version;
	SaUint8T  chassis_type;
	SaHpiTextBufferT  serial_no;
	SaHpiTextBufferT  part_no;
} PLMS_CHASSIS_INFO_AREA;	
 
typedef struct {
	PLMS_IDR_HEADER            hdr;
	PLMS_CHASSIS_INFO_AREA     chassis_area;
	PLMS_BOARD_INFO_AREA       board_area;
	PLMS_PRODUCT_INFO_AREA     product_area;
}PLMS_INV_DATA;

#endif
