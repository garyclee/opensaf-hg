/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *
 */
/*****************************************************************************
..............................................................................

  MODULE NAME: PSR_RFMT.H

..............................................................................

  DESCRIPTION:

  This file contains the version related macros for PSSv  

 ****************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef PSR_RFMT_H
#define PSR_RFMT_H

typedef enum pss_reformat_type {
	/* 1:    represents the persistent store with out PSS_TABLE_DETAILS_HEADER info.
	   1EXT: represents the persistent store with PSS_TABLE_DETAILS_HEADER info in a <tbl_id>_tbl_details file.
	   2:    represents the persistent store with PSS_TABLE_DETAILS_HEADER in <tbl_id> file itself.
	   Changing persistent store between any of these formats - Store Reformatting.
	   Modifying the persistent file according to the MIB changes - MIB Reformatting */
	PSS_REFORMAT_TYPE_STORE_1_TO_1EXT,
	PSS_REFORMAT_TYPE_STORE_HIGHER_TO_1EXT,
	PSS_REFORMAT_TYPE_MIB,
	PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER,
	PSS_REFORMAT_TYPE_STORE_HIGHER_TO_2,
	PSS_REFORMAT_TYPE_MAX
} PSS_REFORMAT_TYPE;

typedef struct pss_table_path_record {
	uns32 ps_format_version;
	uns8 profile[NCS_PSS_MAX_PROFILE_NAME];
	PW_ENV_ID pwe_id;
	int8 pcn[NCSMIB_PCN_LENGTH_MAX];
	uns32 tbl_id;
} PSS_TABLE_PATH_RECORD;

/* Data Structure is written into the persistent store for each of the table.
   This is useful for MIB Reformatting the persistent store */
typedef struct table_details_header {
	uns16 header_len;	/* holds the length of PSS_TABLE_DETAILS_HEADER- With this backward compatibility is achieved */
	uns8 ps_format_version;	/* Persistent Store format version */
	uns8 table_version;
	uns32 max_row_length;
	uns16 max_key_length;
	uns16 bitmap_length;
} PSS_TABLE_DETAILS_HEADER;

typedef enum return_codes {
	PSSRC_DESTFILEOPEN_FAILURE,
	PSSRC_DESTFILECLOSE_FAILURE,
	PSSRC_SRCFILEOPEN_FAILURE,
	PSSRC_SRCFILECLOSE_FAILURE,
	PSSRC_SRCFILESEEK_FAILURE,
	PSSRC_SRCFILESIZE_FAILURE,
	PSSRC_SRCFILEREAD_FAILURE,
	PSSRC_DESTFILEWRITE_FAILURE,
	PSSRC_SUCCESS,
} PSS_RETURN_CODES;

#define PSS_TABLE_DETAILS_HEADER_LEN (sizeof(PSS_TABLE_DETAILS_HEADER))

#define PSS_MIN_TABLE_DETAILS_HEADER_LEN 12

/************************************************************************
  PSS Reformatting routines
*************************************************************************/
EXTERN_C uns32 pss_check_n_reformat(PSS_CB *inst, uns32 ps_format_version);
EXTERN_C uns32 pss_tbl_details_header_read(PSS_CB *inst, uns32 ts_hdl, long tfile_hdl, PSS_TABLE_PATH_RECORD *rec,
					   PSS_TABLE_DETAILS_HEADER *hdr);
EXTERN_C uns32 pss_tbl_details_header_write(PSS_CB *inst, uns32 ts_hdl, long tfile_hdl, PSS_TABLE_PATH_RECORD *rec);
EXTERN_C int persistent_file_filter(const struct dirent *ps_file);

#endif
