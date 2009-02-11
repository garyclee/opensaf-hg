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

  MODULE NAME: PSR_RFMT.C

..............................................................................

  DESCRIPTION: This file includes implementation for Store and MIB Reformatting.
*****************************************************************************/ 
#if (NCS_MAB == 1)
#if (NCS_PSR == 1)

#include "psr.h"
#include "psr_rfmt.h"

static uns32 pss_store_reformatting_1ext_to_higher(PSS_CB *inst, uns32 ts_hdl, PSS_TABLE_PATH_RECORD *rec);
static uns32 pss_store_reformatting_1_to_1ext(PSS_CB *inst, PSS_TABLE_PATH_RECORD *rec);
static uns32 pss_store_reformatting_higher_to_2(PSS_CB *inst, uns32 ts_hdl,
                                         PSS_TABLE_PATH_RECORD *rec);
static uns32 pss_store_reformatting_higher_to_1ext(PSS_CB *inst, uns32 ts_hdl, PSS_TABLE_PATH_RECORD *rec);
static uns32 pss_verify_n_mib_reformat(PSS_CB *inst, uns32 ts_hdl, PSS_TABLE_PATH_RECORD *rec);
static uns32 pss_mib_reformat(PSS_CB *inst, uns32 ts_hdl, long file_hdl, uns32 file_size,
                              PSS_TABLE_PATH_RECORD *rec, PSS_TABLE_DETAILS_HEADER *hdr);

static uns32 pss_reformat(PSS_CB *inst, PSS_REFORMAT_TYPE reformat_type, uns32 ts_hdl, uns32 cur_ps_format);

static uns32 pss_fappend(char *dest_file, char *source_file, uns32 start_offset);
static uns32 pss_frame_current_bitmap(PSS_CB *inst, uns32 tbl_id, uns8 *new_bitmap);
static uns32 pss_log_pss_return_codes(PSS_RETURN_CODES pss_retval, char* src_file_name, char* dest_file_name);

/*******************************************************************************\
*  Name:          pss_frame_current_bitmap                                      *
*                                                                               *
*  Description:   This routine frames the bitmap of the loaded MIB.             *
*                 It sets the bit to one if it is not obsolete                  *
*                                                                               *
*  Arguments:                                                                   *
*                 tbl_id: table_id for which the bitmap need to be framed.      *
*                 *new_bitmap: pointer where the bitmap need to be stored.      *
*                                                                               *
*  Returns:       NCSCC_RC_SUCCESS                                              *
*                                                                               *
\*******************************************************************************/

static uns32 pss_frame_current_bitmap(PSS_CB *inst, uns32 tbl_id, uns8 *new_bitmap)
{
   PSS_VAR_INFO         *pfields = inst->mib_tbl_desc[tbl_id]->pfields;
   uns32                param_id;

   for(param_id = 0; param_id < inst->mib_tbl_desc[tbl_id]->ptbl_info->num_objects; param_id++)
   {
      if(pfields[param_id].var_info.status != NCSMIB_OBJ_OBSOLETE)
      {
         *(new_bitmap + (param_id/8)) |= (uns8) (1 << (param_id%8));
      }
   }
   return NCSCC_RC_SUCCESS;
}

/*******************************************************************************\
*  Name:          pss_fappend                                                   *
*                                                                               *
*  Description:   This routine appends the contents of source file, from        *
*                 offset specified till the end, to the dest file.              *
*                                                                               *
*  Arguments:                                                                   *
*                 *dest_file: dest file name.                                   *
*                 *source_file: source file name.                               *
*                 start_offset: Offset of the source file from where to append  *
*                               into dest file.                                 *
*                                                                               *
*  Returns:       PSS_RETURN_CODES                                              *
*                                                                               *
\*******************************************************************************/

PSS_RETURN_CODES pss_fappend(char *dest_file, char *source_file, uns32 start_offset)
{
   NCS_OS_FILE                inst_file;
   uns32                      rem_file_size;
   long                      src_file_handle = 0, dest_file_handle = 0;
   uns8                       buff[NCS_PSS_MAX_CHUNK_SIZE];
   uns32                      bytes_read = 0; /* represents no. of bytes read */
   uns32                      read_bytes = 0; /* represents no. of bytes to be read */
   PSS_RETURN_CODES           pss_retval = PSSRC_SUCCESS;
   uns32                      retval = NCSCC_RC_SUCCESS;
   NCS_OS_FILE                src_inst_file;
   NCS_OS_FILE                dest_inst_file;


   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));

   m_NCS_CONS_PRINTF("\nDest_file: %s", dest_file);
   m_NCS_CONS_PRINTF("\nSource_file: %s", source_file);
   m_NCS_CONS_PRINTF("\nStart_offset: %d", start_offset);
   /* Get the file size to determine how many times to read */
   inst_file.info.size.i_file_name = source_file;
   if(NCSCC_RC_SUCCESS != m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_SIZE))
   {
       /* Log retrieving file size failed */
      return PSSRC_SRCFILESIZE_FAILURE;
   }

   rem_file_size = (inst_file.info.size.o_file_size - start_offset);

   m_NCS_CONS_PRINTF("\nfilesize: %d\t rem_file_size: %d", inst_file.info.size.o_file_size, rem_file_size);
   /* Now open source file for reading */
   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   inst_file.info.open.i_file_name = source_file;
   inst_file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

   if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_OPEN))
       return PSSRC_SRCFILEOPEN_FAILURE;

   src_file_handle = (long) inst_file.info.open.o_file_handle;

   /* Now open dest file for appending */
   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   inst_file.info.open.i_file_name = dest_file;
   inst_file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_APPEND;
    
   if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_OPEN))
   {
      if (src_file_handle != 0)
      {
          m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
          inst_file.info.close.i_file_handle = (void *) src_file_handle;
          if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_CLOSE))
              return PSSRC_SRCFILECLOSE_FAILURE;
      }
      return PSSRC_DESTFILEOPEN_FAILURE;
   }

   dest_file_handle = (long) inst_file.info.open.o_file_handle;
   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   /* Moving till the offset location in the source file */
   inst_file.info.seek.i_file_handle = (void *) src_file_handle;
   inst_file.info.seek.i_offset = start_offset;
   if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_SEEK))
   {
       pss_retval = PSSRC_SRCFILESEEK_FAILURE;
       goto closeallfiles;
   }

   m_NCS_CONS_PRINTF("\nfile size: %d", rem_file_size);
   m_NCS_MEMSET(&src_inst_file, '\0', sizeof(NCS_OS_FILE));
   m_NCS_MEMSET(&dest_inst_file, '\0', sizeof(NCS_OS_FILE));
   src_inst_file.info.read.i_file_handle = (void *) src_file_handle;
   dest_inst_file.info.write.i_file_handle = (void *) dest_file_handle;

   while(rem_file_size > 0)
   {
      if(rem_file_size < NCS_PSS_MAX_CHUNK_SIZE)
          read_bytes = rem_file_size;
      else
         read_bytes = NCS_PSS_MAX_CHUNK_SIZE;

      /* Reading the "read_bytes" from source file */
      src_inst_file.info.read.i_buf_size    = read_bytes;
      src_inst_file.info.read.i_buffer      = buff;

      retval = m_NCS_FILE_OP (&src_inst_file, NCS_OS_FILE_READ);
  
      bytes_read = src_inst_file.info.read.o_bytes_read;
      if(NCSCC_RC_SUCCESS != retval || bytes_read != read_bytes)
      {
         m_NCS_CONS_PRINTF("\nRead on file %s failed", source_file);
         pss_retval = PSSRC_SRCFILEREAD_FAILURE;
         goto closeallfiles;
      }

      dest_inst_file.info.write.i_buf_size    = bytes_read;
      dest_inst_file.info.write.i_buffer      = buff;
    
      if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&dest_inst_file, NCS_OS_FILE_WRITE))
      {
         m_NCS_CONS_PRINTF("\nWrite on file %s failed", dest_file);
         pss_retval = PSSRC_DESTFILEWRITE_FAILURE;
         goto closeallfiles;
      }

      rem_file_size -= bytes_read;
   }

   m_NCS_CONS_PRINTF("\nEnd of while: rem_file_size %d", rem_file_size);

   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   /* Get the file size to determine how many times to read */
   inst_file.info.size.i_file_name = dest_file;
   if(NCSCC_RC_SUCCESS != m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_SIZE))
   {
       /* Log retrieving file size failed */
      return PSSRC_SRCFILESIZE_FAILURE;
   }

   m_NCS_CONS_PRINTF("\nReached End file_size: %d", inst_file.info.size.o_file_size);

closeallfiles:
   if (src_file_handle != 0)
   {
      inst_file.info.close.i_file_handle = (void *) src_file_handle;
      if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_CLOSE))
         pss_retval = PSSRC_SRCFILECLOSE_FAILURE;
   }
   if (dest_file_handle != 0)
   {
      inst_file.info.close.i_file_handle = (void *) dest_file_handle;
      if(NCSCC_RC_SUCCESS != m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_CLOSE))
         pss_retval = PSSRC_DESTFILECLOSE_FAILURE;
   }

   if(PSSRC_SUCCESS != pss_retval)
   {
      m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
      inst_file.info.remove.i_file_name = dest_file;
      m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_REMOVE);
   }
   return pss_retval;
}

/*******************************************************************************\
*  Name:          persistent_file_filter                                        *
*                                                                               *
*  Description:   This routine is a filter function that can be specified in    *
*                 scandir(). This filters the ".", ".." and other files which   *
*                 are not having the file names as digits and registered table. *
*                                                                               *
*  Arguments:                                                                   *
*                 *ps_file: dirent pointer which scandir provides.              *
*                                                                               *
*  Returns:       1/0                                                           *
*                                                                               *
\*******************************************************************************/

int persistent_file_filter(const struct dirent *ps_file)
{
 int i;
 for(i = 0; ps_file->d_name[i] != '\0'; i++)
    if(!isdigit(ps_file->d_name[i]))
       return 0;

 i = atoi(ps_file->d_name);
 if(i < MIB_UD_TBL_ID_END && i > 0)
    return 1;

 return 0;
}

/********************************************************************************\
*  Name:          pss_tbl_details_header_read                                    *
*                                                                                *
*  Description:   This routine reads a file for PSS_TABLE_DETAILS_HEADER.        *
*                                                                                *
*  Arguments:                                                                    *
*                 ts_hdl: Target service handle.                                 *
*                 tfile_hdl: file handle from which the header needs to be read. *
*                 *rec: Path details of the file.                                *
*                 *hdr: Output pointer of PSS_TABLE_DETAILS_HEADER.              *
*                                                                                *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                              *
*                                                                                *
\********************************************************************************/

uns32 pss_tbl_details_header_read(PSS_CB *inst, uns32 ts_hdl, long tfile_hdl, PSS_TABLE_PATH_RECORD *rec,
                                  PSS_TABLE_DETAILS_HEADER *hdr)
{
   uns32  retval = NCSCC_RC_SUCCESS;
   uns32  bytes_read = 0;
   char               src_file_id[NCS_PSSTS_MAX_PATH_LEN];

   m_NCS_PSSTS_FILE_READ(inst->pssts_api, ts_hdl, retval, tfile_hdl, PSS_TABLE_DETAILS_HEADER_LEN,
                         0, (char*)hdr, bytes_read);
   if(NCSCC_RC_SUCCESS != retval || PSS_TABLE_DETAILS_HEADER_LEN != bytes_read)
   {
       m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
       snprintf(src_file_id, sizeof(src_file_id)-1, "%d", rec->tbl_id);
       m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_TABLE_DETAILS_READ_FAIL, src_file_id);
       return retval;
   }
   m_NCS_CONS_PRINTF("\nRead Header details: \n\theader_len: %d \n\tPS_FORMAT: %d \n\tTableVersion: %d \n\tmax_row_length: %d\n\tmax_key_length: %d \n\tbitmap_length: %d", hdr->header_len, hdr->ps_format_version, hdr->table_version, hdr->max_row_length, hdr->max_key_length, hdr->bitmap_length);

   if(hdr->header_len < PSS_MIN_TABLE_DETAILS_HEADER_LEN)
   {
      /* Log that there is INCONSISTENTCY in header details */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_TABLE_DETAILS_HEADER_LEN, hdr->header_len);
      m_NCS_MEMSET(hdr, '\0', PSS_TABLE_DETAILS_HEADER_LEN);
      return NCSCC_RC_FAILURE;
   }

   return retval;
}

/*********************************************************************************\
*  Name:          pss_tbl_details_header_write                                    *
*                                                                                 *
*  Description:                                                                   *
*                 It writes the header details read from the loaded MIBs          *
*                 to the file provided.                                           *
*                                                                                 *
*  Arguments:                                                                     *
*                 ts_hdl: Target service handle.                                  *
*                 tfile_hdl: file handle to which the header needs to be written. *
*                 *rec: Path details of the file.                                 *
*                                                                                 *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                               *
*                                                                                 *
\*********************************************************************************/

uns32 pss_tbl_details_header_write(PSS_CB *inst, uns32 ts_hdl, long tfile_hdl,
                                   PSS_TABLE_PATH_RECORD *rec)
{
   uns32                    retval = NCSCC_RC_SUCCESS;
   uns32                    tbl_id = rec->tbl_id;
   PSS_TABLE_DETAILS_HEADER hdr;

   hdr.header_len = PSS_TABLE_DETAILS_HEADER_LEN;
   hdr.ps_format_version = PSS_PS_FORMAT_VERSION;
   hdr.table_version = inst->mib_tbl_desc[tbl_id]->i_tbl_version;
   hdr.max_row_length = inst->mib_tbl_desc[tbl_id]->max_row_length;
   hdr.max_key_length = inst->mib_tbl_desc[tbl_id]->max_key_length;
   hdr.bitmap_length = inst->mib_tbl_desc[tbl_id]->bitmap_length;
   m_NCS_CONS_PRINTF("\nFilling Header details: \n\theader_len: %d \n\tPS_FORMAT: %d \n\tTableVersion: %d \n\tmax_row_length: %d\n\tmax_key_length: %d \n\tbitmap_length: %d\n", hdr.header_len, hdr.ps_format_version, hdr.table_version, hdr.max_row_length, hdr.max_key_length, hdr.bitmap_length);

   m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, ts_hdl, retval,
                         tfile_hdl, PSS_TABLE_DETAILS_HEADER_LEN, (char*)&hdr);
   if (NCSCC_RC_SUCCESS != retval)
   {
       m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
       m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_WRITE_FAIL,
           rec->pwe_id, rec->pcn, rec->tbl_id);
       return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

/*********************************************************************************\
*  Name:          pss_store_reformatting_1_to_1ext                                *
*                                                                                 *
*  Description:   This routine formats the persistent store from                  *
*                  1 format to 1Ext format.                                       *
*                 1 format: does not contains table details in any form.          *
*                 1Ext format: contains table details in a                        *
*                              separate file, <table_id>_tbl_details.             *
*                                                                                 *
*  Arguments:                                                                     *
*                 *rec: Path details of the file to be reformatted.               *
*                                                                                 *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                               *
*                                                                                 *
\*********************************************************************************/

uns32 pss_store_reformatting_1_to_1ext(PSS_CB *inst, PSS_TABLE_PATH_RECORD *rec)
{
   uns32         retval = NCSCC_RC_SUCCESS;
   uns32         final_retval = NCSCC_RC_SUCCESS;
   long         tfile_hdl = 0;

   m_NCS_CONS_PRINTF("\nEntered case PSS_REFORMAT_TYPE_STORE_1_TO_1EXT");

   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_1_TO_1EXT_START,
                    rec->tbl_id);
 /* Create <tbl_id>_tbl_details file */

   /* Open the temporary file for writing */
   m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF("\nOpening Temporary file success");
 
 /* Write the table details header into <tbl_id>_tbl_details file and save it in proper location */
   final_retval = pss_tbl_details_header_write(inst, inst->pssts_hdl, tfile_hdl, rec);
 
   if (tfile_hdl != 0)
   {
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, tfile_hdl);
   }

   if(NCSCC_RC_SUCCESS != final_retval)
   {
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, inst->pssts_hdl, retval);
      return final_retval;
   }
   m_NCS_CONS_PRINTF("Profile: %s, PWE: %d, PCN: %s, TBL: %d", rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
   m_NCS_PSSTS_MOVE_TBL_DETAILS_FILE(inst->pssts_api, inst->pssts_hdl, retval,
                                     rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
   if(NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MOVE_TABLE_DETAILS_FILE_FAIL);
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, inst->pssts_hdl, retval);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF("\nDone ...%d... PSS_REFORMAT_TYPE_STORE_1_TO_1EXT", rec->tbl_id);

   return NCSCC_RC_SUCCESS;
}

/******************************************************************************\
*  Name:          pss_log_pss_return_codes                                     *
*                                                                              *
*  Description:   This routine logs the appropriate log messages               *
*                 for PSS_RETURN_CODES.                                        *
*                                                                              *
*  Arguments:                                                                  *
*                 pss_retval: error code returned by fappedn()                 *
*                 *src_file_name: Source file name which was been appended.    *
*                 *dest_file_name: Dest file name for which the src file       *
*                                  is been appended.                           *
*                                                                              *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                            *
*                                                                              *
\******************************************************************************/

uns32 pss_log_pss_return_codes(PSS_RETURN_CODES pss_retval, char* src_file_name,
                               char* dest_file_name)
{
   switch(pss_retval)
   {
      case PSSRC_SRCFILESIZE_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_FILE_SIZE_OP_FAIL, src_file_name);
           break;

      case PSSRC_SRCFILEOPEN_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, src_file_name);
           break;

      case PSSRC_SRCFILECLOSE_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_CLOSE_FAIL, src_file_name);
           break;

      case PSSRC_DESTFILEOPEN_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, dest_file_name);
           break;

      case PSSRC_SRCFILESEEK_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_SEEK_FAIL, src_file_name);
           break;

      case PSSRC_SRCFILEREAD_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, src_file_name);
           break;

      case PSSRC_DESTFILEWRITE_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_WRITE_FAIL, dest_file_name);
           break;

      case PSSRC_DESTFILECLOSE_FAILURE:
           m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_CLOSE_FAIL, dest_file_name);
           break;

      default:
           break;
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
*  Name:          pss_store_reformatting_1ext_to_higher                       *
*                                                                             *
*  Description:   This routine formats the persistent store from 1Ext format  *
*                 to any compatible higher version format.                    *
*                 1Ext format: contains table details in a separate           *
*                              file, <table_id>_tbl_details.                  *
*                 Higher format: contains table details as a prefix           *
*                                in persistent file of the table.             *
*                                                                             *
*  Arguments:                                                                 *
*                 ts_hdl: target service handle.                              *
*                 *rec: Path details of the file to be reformatted.           *
*                                                                             *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                           *
*                                                                             *
\*****************************************************************************/

uns32 pss_store_reformatting_1ext_to_higher(PSS_CB *inst, uns32 ts_hdl, PSS_TABLE_PATH_RECORD *rec)
{
   NCS_OS_FILE        inst_file;
   char               source_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   char               dest_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   PSS_RETURN_CODES   pss_retval = PSSRC_SUCCESS;
   char               dest_file_id[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   char               src_file_id[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   uns32              retval = NCSCC_RC_SUCCESS;

   m_NCS_CONS_PRINTF("\nEntered case PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER");
   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_1_TO_1EXT_START, rec->tbl_id);

   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   snprintf(dest_file_path, sizeof(dest_file_path)-1, "%s%d/%s/%d/%s/%d_tbl_details",
                          NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
   m_NCS_CONS_PRINTF("\n dest_file_path : %s", dest_file_path);
   inst_file.info.file_exists.i_file_name = dest_file_path;
   m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_EXISTS);
   if(inst_file.info.file_exists.o_file_exists == TRUE)
   {
      /* Prefixing table details to the persistent file of the table */
      snprintf(source_file_path, sizeof(source_file_path)-1, "%s%d/%s/%d/%s/%d",
                     NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
      m_NCS_CONS_PRINTF("\n source_file_path : %s", source_file_path);

      /* Now Source_file is persistent of the table i.e., <tbl_id>
             Dest_file is <tbl_id>_tbl_details file */
      pss_retval = pss_fappend(dest_file_path, source_file_path, 0);
      if(PSSRC_SUCCESS == pss_retval)
      {
/*       m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id); */
         m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
         inst_file.info.rename.i_file_name     = dest_file_path;
         inst_file.info.rename.i_new_file_name = source_file_path;
         if(m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_RENAME) != NCSCC_RC_SUCCESS)
         {
            /* What action should be taken */
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
            m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_RENAME_FILE_FAILED);
            return NCSCC_RC_FAILURE;
         }
      }
      else
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
         snprintf(dest_file_id, sizeof(dest_file_id)-1, "%d_tbl_details", rec->tbl_id);
         snprintf(src_file_id, sizeof(src_file_id)-1, "%d", rec->tbl_id);
         /* Log that appending the files failed */
         pss_log_pss_return_codes(pss_retval, src_file_id, dest_file_id);
         m_NCS_CONS_PRINTF("\nFailed to append the file");
      }
   }
   else
   {
      /* Log Persistent store is inconsistent */
      /* Delete ps_file */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_TABLE_DETAILS_EXISTS_ON_PERSISTENT_STORE, rec->tbl_id);
      m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
      m_NCS_CONS_PRINTF("\nINCONSISTENT ...%d... PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER", rec->tbl_id);
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
*  Name:          pss_store_reformatting_higher_to_2                          *
*                                                                             *
*  Description:   This routine formats the persistent store from any          *
*                 compatible higher format to format 2.                       *
*                 to any compatible higher version format.                    *
*                 format 2: contains table details as a prefix in             *
*                           persistent file of the table and                  *
*                           its table details header is of size 12.           *
*                 Higher format: Here higher format refers to any compatible  *
*                           format. Here the table details header could be    *
*                           any greater size than 12.                         *
*                                                                             *
*  Arguments:                                                                 *
*                 ts_hdl: target service handle.                              *
*                 *rec: Path details of the file to be reformatted.           *
*                                                                             *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                           *
*                                                                             *
\*****************************************************************************/

uns32 pss_store_reformatting_higher_to_2(PSS_CB *inst, uns32 ts_hdl,
                                         PSS_TABLE_PATH_RECORD *rec)
{
   uns32         retval = NCSCC_RC_SUCCESS;
   uns32         final_retval = NCSCC_RC_SUCCESS;
   PSS_RETURN_CODES           pss_retval = PSSRC_SUCCESS;
   PSS_TABLE_DETAILS_HEADER   hdr;
   long          tfile_hdl = 0, file_hdl = 0;
   char          source_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   char          temp_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
   char          dest_file[]="temp_file";
   char          src_file_id[NCS_PSSTS_MAX_PATH_LEN] = "\0";

   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_2_START,
                    rec->tbl_id);
   m_NCS_CONS_PRINTF("\nEntered case PSS_REFORMAT_TYPE_STORE_HIGHER_TO_2");
   /* Write table details header of format 2 */
   /* Open the temporary file for writing table details */
   m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, ts_hdl, retval, tfile_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF("\nOpening temp file success");

   /* Write the table details header into temporary file
                                   and save it in proper location */
   final_retval = pss_tbl_details_header_write(inst, ts_hdl, tfile_hdl, rec);

   if (tfile_hdl != 0)
   {
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
   }

   if(NCSCC_RC_SUCCESS != final_retval)
   {
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
      return final_retval;
   }

   /* Open the persistent file of the table for reading Table details header */
   m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, ts_hdl, retval, rec->profile,
                         rec->pwe_id, rec->pcn, rec->tbl_id,
                         NCS_PSSTS_FILE_PERM_READ, file_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_OPEN_FAIL,
                                   rec->pwe_id, rec->pcn, rec->tbl_id);
      return NCSCC_RC_FAILURE;
   }

   /* Read the persistent file of the table for table details */
   retval = pss_tbl_details_header_read(inst, ts_hdl, file_hdl, rec, &hdr);

   if(NCSCC_RC_SUCCESS != retval)
   {
      if (file_hdl != 0)
         m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
      return NCSCC_RC_FAILURE;
   }

   snprintf(temp_file_path, sizeof(temp_file_path)-1, "%s%d/%s",
            NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, NCS_PSSTS_TEMP_FILE_NAME);

   snprintf(source_file_path, sizeof(source_file_path)-1, "%s%d/%s/%d/%s/%d",
            NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, rec->profile,
            rec->pwe_id, rec->pcn, rec->tbl_id);

   m_NCS_CONS_PRINTF("\n temp_file_path : %s", temp_file_path);
   m_NCS_CONS_PRINTF("\n source_file_path : %s", source_file_path);
   if(PSSRC_SUCCESS == pss_fappend(temp_file_path, source_file_path, hdr.header_len))
   {
      /* Log store reformatting is done for this table */
      m_NCS_CONS_PRINTF("\nDone ...%d... PSS_REFORMAT_TYPE_STORE_2_TO_1EXT", rec->tbl_id);
      m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, ts_hdl, retval, rec->profile,
                                 rec->pwe_id, rec->pcn, rec->tbl_id);
      if (NCSCC_RC_SUCCESS != retval)
      {
          m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_FAIL);
          m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
          return NCSCC_RC_FAILURE;
      }
   }
   else
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      /* Log that appending the files failed */
      snprintf(src_file_id, sizeof(src_file_id)-1, "%d", rec->tbl_id);
      pss_log_pss_return_codes(pss_retval, src_file_id, dest_file);
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************\
*  Name:          pss_store_reformatting_higher_to_1ext                       *
*                                                                             *
*  Description:   This routine formats the persistent store from 1Ext format  *
*                 to any compatible higher version format.                    *
*                 1Ext format: contains table details in a separate           *
*                              file, <table_id>_tbl_details.                  *
*                 Higher format: contains table details as a prefix           *
*                                in persistent file of the table.             *
*                                                                             *
*  Arguments:                                                                 *
*                 ts_hdl: target service handle.                              *
*                 *rec: Path details of the file to be reformatted.           *
*                                                                             *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                           *
*                                                                             *
\*****************************************************************************/

uns32 pss_store_reformatting_higher_to_1ext(PSS_CB *inst, uns32 ts_hdl,
                                            PSS_TABLE_PATH_RECORD *rec)
{
   uns32         retval = NCSCC_RC_SUCCESS;
   uns32         final_retval = NCSCC_RC_SUCCESS;
   PSS_RETURN_CODES           pss_retval = PSSRC_SUCCESS;
   long          tfile_hdl = 0;
   char          source_file_path[NCS_PSSTS_MAX_PATH_LEN];
   char          temp_file_path[NCS_PSSTS_MAX_PATH_LEN];
   char          dest_file[]="temp_file";
   char          src_file_id[NCS_PSSTS_MAX_PATH_LEN];

   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_1EXT_START,
                    rec->tbl_id);
   m_NCS_CONS_PRINTF("\nEntered case PSS_REFORMAT_TYPE_STORE_2_TO_1EXT");
   /* Create <tbl_id>_tbl_details file */
   /* Open the temporary file for writing table details */
   m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, ts_hdl, retval, tfile_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_CONS_PRINTF("\nOpening temp file success");

   /* Write the table details header into <tbl_id>_tbl_details file
                                   and save it in proper location */
   final_retval = pss_tbl_details_header_write(inst, ts_hdl, tfile_hdl, rec);

   if (tfile_hdl != 0)
   {
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
   }

   if(NCSCC_RC_SUCCESS != final_retval)
   {
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
      return final_retval;
   }

   m_NCS_PSSTS_MOVE_TBL_DETAILS_FILE(inst->pssts_api, ts_hdl, retval,
                                     rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
   if(NCSCC_RC_SUCCESS != retval)
   {
      /* Log moving the table details file failed */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_MOVE_TABLE_DETAILS_FILE_FAIL);
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
      return NCSCC_RC_FAILURE;
   }

   /* Open the temporary file for writing persistent file records
                              (excluding table details header) */
   m_NCS_PSSTS_FILE_OPEN_TEMP(inst->pssts_api, ts_hdl, retval, tfile_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_OPEN_TEMP_FILE_FAIL);
      return NCSCC_RC_FAILURE;
   }
   snprintf(temp_file_path, sizeof(temp_file_path)-1, "%s%d/%s",
            NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, NCS_PSSTS_TEMP_FILE_NAME);

   snprintf(source_file_path, sizeof(source_file_path)-1, "%s%d/%s/%d/%s/%d",
            NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, rec->profile,
            rec->pwe_id, rec->pcn, rec->tbl_id);

   m_NCS_CONS_PRINTF("\n temp_file_path : %s", temp_file_path);
   m_NCS_CONS_PRINTF("\n source_file_path : %s", source_file_path);
   if(PSSRC_SUCCESS == pss_fappend(temp_file_path, source_file_path, PSS_TABLE_DETAILS_HEADER_LEN))
   {
      /* Log store reformatting is done for this table */
      m_NCS_CONS_PRINTF("\nDone ...%d... PSS_REFORMAT_TYPE_STORE_2_TO_1EXT", rec->tbl_id);
      m_NCS_PSSTS_FILE_COPY_TEMP(inst->pssts_api, ts_hdl, retval, rec->profile,
                                 rec->pwe_id, rec->pcn, rec->tbl_id);
      if (NCSCC_RC_SUCCESS != retval)
      {
          m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN2_SET_COPY_TEMP_FILE_FAIL);
          m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
          return NCSCC_RC_FAILURE;
      }
   }
   else
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      /* Log that appending the files failed */
      snprintf(src_file_id, sizeof(src_file_id)-1, "%d", rec->tbl_id);
      pss_log_pss_return_codes(pss_retval, src_file_id, dest_file);
      m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, ts_hdl, retval);
   }
   return NCSCC_RC_SUCCESS;
}

/**********************************************************************\
*  Name:          pss_verify_n_mib_reformat                            *
*                                                                      *
*  Description:   This routine checks whether the table is required    *
*                 to be MIB reformatted.                               *
*                                                                      *
*  Arguments:                                                          *
*                 ts_hdl: target service handle.                       *
*                 *rec: Path details of the file to be reformatted.    *
*                                                                      *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                    *
*                                                                      *
\**********************************************************************/

uns32 pss_verify_n_mib_reformat(PSS_CB *inst, uns32 ts_hdl,
                                PSS_TABLE_PATH_RECORD *rec)
{
   uns32                      retval = NCSCC_RC_SUCCESS;
   uns32                      final_retval = NCSCC_RC_SUCCESS;
   long                       file_hdl = 0;
   NCS_OS_FILE                inst_file;
   char                       dest_file_path[NCS_PSSTS_MAX_PATH_LEN];
   PSS_TABLE_DETAILS_HEADER   hdr;
   uns32                      file_size = 0, temp_file_size = 0;
   NCS_BOOL                   do_MIB_reformatting = FALSE;
   PSS_MIB_TBL_INFO           *tbl_info = inst->mib_tbl_desc[rec->tbl_id];

   m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_1EXT_START,
                    rec->tbl_id);
   m_NCS_CONS_PRINTF("\nEntered case PSS_REFORMAT_TYPE_MIB_IS_READY");
   m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
   /* The persistent store format version registered with targsvcs could be
      different with the version in which the table details file is being opened.
      Thus the targsvcs macros cannot be used.
      It demands much more effort to change the macros accordingly. */
   snprintf(dest_file_path, sizeof(dest_file_path)-1, "%s%d/%s/%d/%s/%d_tbl_details", 
            NCS_PSS_DEF_PSSV_ROOT_PATH, rec->ps_format_version, rec->profile,
            rec->pwe_id, rec->pcn, rec->tbl_id);

   m_NCS_CONS_PRINTF("\ndest_file_path: %s", dest_file_path);
   inst_file.info.file_exists.i_file_name = dest_file_path;
   m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_EXISTS);

   if(inst_file.info.file_exists.o_file_exists == FALSE)
   {
      m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
      /* Open the persistent file of the table for reading */
      m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, ts_hdl, retval, rec->profile,
                            rec->pwe_id, rec->pcn, rec->tbl_id,
                            NCS_PSSTS_FILE_PERM_READ, file_hdl);
      if (NCSCC_RC_SUCCESS != retval)
      {
         m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_OPEN_FAIL,
                                      rec->pwe_id, rec->pcn, rec->tbl_id);
         return NCSCC_RC_FAILURE;
      }

      /* Read the persistent file of the table for table details */
      retval = pss_tbl_details_header_read(inst, ts_hdl, file_hdl, rec, &hdr);

      if(NCSCC_RC_SUCCESS != retval)
      {
         if (file_hdl != 0)
            m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
         return NCSCC_RC_FAILURE;
      }

      /* Get the file size for consistent check */
      m_NCS_PSSTS_FILE_SIZE(inst->pssts_api, ts_hdl, retval, rec->profile, rec->pwe_id,
                            rec->pcn, rec->tbl_id, file_size);
      if (NCSCC_RC_SUCCESS != retval)
      {
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
         m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_FILE_SIZE_OP_FAIL,
                         rec->pwe_id, rec->pcn, rec->tbl_id);
         if (file_hdl != 0)
            m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
 
         return NCSCC_RC_FAILURE;
      }

      m_NCS_CONS_PRINTF("\nfile_size : %d, hdr.max_row_length: %d", file_size, hdr.max_row_length);

      if(file_size == 0)
      {

        /* Log that the ps_file is not consistent */
        m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_FILE_SIZE, file_size);

        if (file_hdl != 0)
           m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
        /* Delete the ps_file */
        m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, rec->profile,
                                           rec->pwe_id, rec->pcn, rec->tbl_id);
        return NCSCC_RC_FAILURE;
      }
      else
      {
         temp_file_size = file_size - hdr.header_len;
         if(temp_file_size == 0)
         {
            /* No rows to reformat */
            if (file_hdl != 0)
               m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
            return NCSCC_RC_SUCCESS;
         }
         if(temp_file_size % hdr.max_row_length != 0)
         {
            /* Log that the ps_file is not consistent */
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_FILE_SIZE, file_size);
            if (file_hdl != 0)
               m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
            /* Delete the ps_file */
            m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, rec->profile,
                                               rec->pwe_id, rec->pcn, rec->tbl_id);
            return NCSCC_RC_FAILURE;
         }
      }

      if(hdr.table_version == tbl_info->i_tbl_version)
      {
         if((hdr.max_row_length == tbl_info->max_row_length) &&
         (hdr.max_key_length == tbl_info->max_key_length) &&
         (hdr.bitmap_length == tbl_info->bitmap_length))
         {
            m_NCS_CONS_PRINTF("\n No change in the table_version and the table details are consistent: %d", rec->tbl_id);
            if (file_hdl != 0)
               m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
            return NCSCC_RC_SUCCESS;
         }
         else
         {
            /* Log that Table details are not consistent */
            m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
            m_LOG_PSS_TBL_DTLS(NCSFL_SEV_ERROR, PSS_HDLN_PROPERTIES_CHANGED_BUT_NOT_TABLE_VERSION,
                               hdr.table_version, hdr.max_row_length, hdr.max_key_length, hdr.bitmap_length,
                               tbl_info->i_tbl_version, tbl_info->max_row_length,
                               tbl_info->max_key_length, tbl_info->bitmap_length);
            m_NCS_CONS_PRINTF("\nINCONSISTENT: Table_version is not changed but table details are changed: %d", rec->tbl_id);
            if (file_hdl != 0)
               m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);
            return NCSCC_RC_FAILURE;
         }
      }
      do_MIB_reformatting = TRUE;
   }
   else /* table details file exist */
   {
      /* Log that table details file exists and this is inconsistent */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_TABLE_DETAILS_EXISTS_ON_PERSISTENT_STORE,
                                                                             rec->tbl_id);
 
      /* Delete ps_file and tbl_details file */
      m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, rec->profile,
                                         rec->pwe_id, rec->pcn, rec->tbl_id);
      m_NCS_PSSTS_DELETE_TBL_DETAILS_FILE(inst->pssts_api, ts_hdl, retval,
                                          rec->profile, rec->pwe_id, rec->pcn, rec->tbl_id);
 
      return NCSCC_RC_FAILURE;
   }
 
   if(do_MIB_reformatting == TRUE)
   {
      final_retval = pss_mib_reformat(inst, ts_hdl, file_hdl, file_size, rec, &hdr);

      if(file_hdl != 0)
         m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, file_hdl);

      if (NCSCC_RC_SUCCESS != final_retval)
         return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/**********************************************************************************\
*  Name:          pss_mib_reformat                                                 *
*                                                                                  *
*  Description:   This routine formats the persistent file compatible              *
*                 to the loaded MIBs.                                              *
*                                                                                  *
*  Arguments:                                                                      *
*                 ts_hdl: target service handle.                                   *
*                 file_hdl: file handle of the table that need to be reformatted.  *
*                 file_size: size of the file that need to be reformatted.         *
*                 *rec: Path details of the file to be reformatted.                *
*                 hdr: table details header that is read from                      *
*                      the persistent file of the table.                           *
*                                                                                  *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                                *
*                                                                                  *
\**********************************************************************************/

uns32 pss_mib_reformat(PSS_CB *inst, uns32 ts_hdl, long file_hdl, uns32 file_size,
                         PSS_TABLE_PATH_RECORD *rec, PSS_TABLE_DETAILS_HEADER *hdr)
{
   uns32                      retval = NCSCC_RC_SUCCESS;
   long                       tfile_hdl = 0;
   NCS_OS_FILE                inst_file;
   PSS_MIB_TBL_INFO           *tbl_info = inst->mib_tbl_desc[rec->tbl_id];
   uns32                      hdr_data_length = 0, cb_data_length = 0;
   uns32                      min_data_length = 0;
   uns32                      max_bitmap_bytes = 0, bitmap_byte = 0;
   uns32                      max_rows_in_chunk = 0;
   uns32                      read_row_size = 0, write_row_size = 0;
   uns32                      read_buf_size = 0, write_buf_size = 0;
   uns32                      rem_num_rows = 0, read_rows = 0;
   uns8                       *read_buff = NULL, *write_buff = NULL;
   uns8                       *read_buff_ptr = NULL, *write_buff_ptr = NULL;
   uns8                       *new_bitmap = NULL, *bitmap_ptr = NULL;
   char                       src_file[NCS_PSSTS_MAX_PATH_LEN];
   uns32                      row = 0;
   uns32                      read_offset = 0, bytes_read = 0;
   uns32                      read_size = 0, write_size = 0;

   /* Open the persistent file of the table for writing */
   m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, ts_hdl, retval, "_ISU",
                         rec->pwe_id, rec->pcn, rec->tbl_id,
                         NCS_PSSTS_FILE_PERM_WRITE, tfile_hdl);
   if (NCSCC_RC_SUCCESS != retval)
   {
      m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_OPEN_FAIL,
                                   rec->pwe_id, rec->pcn, rec->tbl_id);
      return NCSCC_RC_FAILURE;
   }

/* Modify the header details */
   retval = pss_tbl_details_header_write(inst, ts_hdl, tfile_hdl, rec);
   
   if(NCSCC_RC_SUCCESS != retval)
   {
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
      return NCSCC_RC_FAILURE;
   }

/* Framing the new bitmap */
   /* Getting the max of bitmap */
   max_bitmap_bytes = (hdr->bitmap_length > tbl_info->bitmap_length)
                          ? hdr->bitmap_length : tbl_info->bitmap_length;

   new_bitmap = m_MMGR_ALLOC_PSS_OCT(max_bitmap_bytes);
   if(NULL == new_bitmap)
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL,
                                             "pss_mib_reformat(): new_bitmap");
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
      return NCSCC_RC_FAILURE;
   }

   bitmap_ptr = new_bitmap;
   m_NCS_MEMSET(bitmap_ptr, '\0', max_bitmap_bytes);

   /* Framing bitmap for the changed MIB row */
   pss_frame_current_bitmap(inst, rec->tbl_id, new_bitmap);

/* Required for being compatible with higher versions 
   where there could be extensions in Table details header */
   if(hdr->header_len > PSS_TABLE_DETAILS_HEADER_LEN)
   {
      m_NCS_CONS_PRINTF("\nhdr->header_len: %d > PSS_TABLE_DETAILS_HEADER_LEN: %d ", hdr->header_len, PSS_TABLE_DETAILS_HEADER_LEN);
      /* Moving the file position indicator to point the first record */
      m_NCS_MEMSET(&inst_file, '\0', sizeof(NCS_OS_FILE));
      inst_file.info.seek.i_file_handle = (void *)(long) file_hdl;
      inst_file.info.seek.i_offset = hdr->header_len - PSS_TABLE_DETAILS_HEADER_LEN;
      retval = m_NCS_FILE_OP (&inst_file, NCS_OS_FILE_SEEK);
      if (NCSCC_RC_SUCCESS != retval)
      {
         /* Log that file seek failed */
         snprintf(src_file, sizeof(src_file)-1, "%d", rec->tbl_id);
         m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_TABLE_DETAILS_HEADER_LEN,
                                                                     hdr->header_len);
         m_LOG_PSS_STORE_C(NCSFL_SEV_ERROR, PSS_MIB_SEEK_FAIL, src_file);
         m_MMGR_FREE_PSS_OCT(new_bitmap);
         m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
         return NCSCC_RC_FAILURE;
      }
   }

   /* Calculating READ CHUNK size, number of chunks, WRITE CHUNK size */
   read_row_size = hdr->max_row_length;
   max_rows_in_chunk = (NCS_PSS_MAX_CHUNK_SIZE < read_row_size)
                                ? 1 : (NCS_PSS_MAX_CHUNK_SIZE / read_row_size); 

   read_buf_size = max_rows_in_chunk * read_row_size;
   rem_num_rows = ((file_size - hdr->header_len) / read_row_size);

   write_row_size = tbl_info->max_row_length;
   write_buf_size = max_rows_in_chunk * write_row_size;
  
   /* Allocating memory for read and write buffer chunks */
   if(NULL == (read_buff = m_MMGR_ALLOC_PSS_OCT(read_buf_size)))
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL,
                                              "pss_mib_reformat(): read_buff");
      m_MMGR_FREE_PSS_OCT(new_bitmap);
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
      return NCSCC_RC_FAILURE;
   }
   if(NULL == (write_buff = m_MMGR_ALLOC_PSS_OCT(write_buf_size)))
   {
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_ERR_TABLE_FOUND, rec->tbl_id);
      m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL,
                                             "pss_mib_reformat(): write_buff");
      m_MMGR_FREE_PSS_OCT(new_bitmap);
      m_MMGR_FREE_PSS_OCT(read_buff);
      m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
      return NCSCC_RC_FAILURE;
   }

   read_offset = hdr->header_len;

   while(rem_num_rows != 0)
   {
      uns8    temp[max_bitmap_bytes];

      m_NCS_MEMSET(read_buff, '\0', read_buf_size);
      read_buff_ptr = read_buff;

      read_rows = (rem_num_rows < max_rows_in_chunk) ? rem_num_rows : max_rows_in_chunk;
      read_size = read_rows * read_row_size;
      write_size = read_rows * write_row_size;
      rem_num_rows -= read_rows;

      m_NCS_PSSTS_FILE_READ(inst->pssts_api, ts_hdl, retval, file_hdl,
                                read_size, read_offset, read_buff, bytes_read);
      if(NCSCC_RC_SUCCESS != retval || bytes_read != read_size)
      {
         m_MMGR_FREE_PSS_OCT(read_buff);
         m_MMGR_FREE_PSS_OCT(write_buff);
         m_MMGR_FREE_PSS_OCT(new_bitmap);
         m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
         m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_READ_FAIL,
                         rec->pwe_id, rec->pcn, rec->tbl_id);
         return NCSCC_RC_FAILURE;
      }

      m_NCS_MEMSET(write_buff, '\0', write_buf_size);
      write_buff_ptr = write_buff;
      for(row = read_rows; row != 0; row--)
      {
         /* Modifying the bitmap as per the current MIB */
         m_NCS_MEMSET(temp, '\0', max_bitmap_bytes);
         m_NCS_MEMCPY(temp, read_buff_ptr, hdr->bitmap_length);
         read_buff_ptr  += hdr->bitmap_length;

         for(bitmap_byte = max_bitmap_bytes; bitmap_byte != 0; bitmap_byte--)
            temp[bitmap_byte-1] = temp[bitmap_byte-1] & new_bitmap[bitmap_byte-1];

         /* Copying the bitmap */
         m_NCS_MEMCPY(write_buff_ptr, temp, tbl_info->bitmap_length);
         write_buff_ptr += tbl_info->bitmap_length;

         /* Copying the index */
         m_NCS_MEMCPY(write_buff_ptr, read_buff_ptr, hdr->max_key_length);
         write_buff_ptr += tbl_info->max_key_length;
         read_buff_ptr  += hdr->max_key_length;

         /* Copying the data */
         hdr_data_length = hdr->max_row_length - hdr->max_key_length - hdr->bitmap_length;
         cb_data_length  = tbl_info->max_row_length - tbl_info->max_key_length - tbl_info->bitmap_length;
         min_data_length = (hdr_data_length < cb_data_length) ? hdr_data_length : cb_data_length;

         m_NCS_MEMCPY(write_buff_ptr, read_buff_ptr, min_data_length);
         write_buff_ptr += cb_data_length;
         read_buff_ptr += hdr_data_length;
      }
      m_NCS_PSSTS_FILE_WRITE(inst->pssts_api, ts_hdl, retval,
                            tfile_hdl, write_size, (char*)write_buff);
      if (NCSCC_RC_SUCCESS != retval)
      {
         m_MMGR_FREE_PSS_OCT(read_buff);
         m_MMGR_FREE_PSS_OCT(write_buff);
         m_MMGR_FREE_PSS_OCT(new_bitmap);
         m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);
         m_LOG_PSS_STORE(NCSFL_SEV_ERROR,PSS_MIB_WRITE_FAIL, rec->pwe_id, rec->pcn, rec->tbl_id);
         return NCSCC_RC_FAILURE;
      }
      read_offset += read_size;
   }
   m_MMGR_FREE_PSS_OCT(read_buff);
   m_MMGR_FREE_PSS_OCT(write_buff);
   m_MMGR_FREE_PSS_OCT(new_bitmap);
   m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, ts_hdl, retval, tfile_hdl);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          pss_reformat                                               * 
*                                                                            *
*  Description:   This function processes the reformatting requests.         *
*                                                                            *
*  Arguments:     reformat_type - type of reformatting requested.            *
*                 ts_hdl        - target service handle on which             * 
*                                 reformatting should be performed.          *
*                 cur_ps_format - Existing Persistent Store format.          *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *
*                                                                            *
\****************************************************************************/
uns32 
pss_reformat(PSS_CB *inst, PSS_REFORMAT_TYPE reformat_type, uns32 ts_hdl,
                                                     uns32 cur_ps_format)
{
    uns32                      get_profile_retval = NCSCC_RC_SUCCESS;
    uns32                      retval = NCSCC_RC_SUCCESS;
    uns32                      final_retval = NCSCC_RC_SUCCESS;
    uns8                       profile_name[NCS_PSS_MAX_PROFILE_NAME]="\0";
    uns8                       profile_next[NCS_PSS_MAX_PROFILE_NAME]="\0";
    char                       pcn_path[256]="\0";
    char                       root_dir[256] = NCS_PSS_DEF_PSSV_ROOT_PATH;
    char                       source_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
    char                       dest_file_path[NCS_PSSTS_MAX_PATH_LEN] = "\0";
    PSS_TABLE_PATH_RECORD      ps_file_record;
    NCS_OS_FILE                inst_dir;
    int32                      table_cnt = 0;
    struct dirent              **tbl_list;
    NCS_UBAID                  lcl_uba;

    m_NCS_CONS_PRINTF("Entered pss_reformat, reformat_type: %d\n", reformat_type);
    m_NCS_CONS_PRINTF("cur_ps_format: %d\n", cur_ps_format);
    m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_REFORMAT_REQUEST, reformat_type);

    do /* Loop for all profiles in the cur_ps_format */
    {
       /* Do the clean up */
       m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, ts_hdl, retval, "_ISU");
       m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, inst->pssts_hdl, retval);

       /* Get the next profile */
       m_NCS_PSSTS_GET_NEXT_PROFILE(inst->pssts_api, ts_hdl, get_profile_retval,
                                    profile_name, sizeof(profile_next), profile_next);

       if(NCSCC_RC_SUCCESS == get_profile_retval)
       {
          USRBUF               *buff = NULL;
          uns32                buff_len = 0;
          uns8                 *buff_ptr = NULL;
          uns16                pwe_cnt = 0, pcn_cnt = 0, str_len = 0, j = 0, k = 0;
          PW_ENV_ID            pwe_id = 0;
          uns32                tbl_id = 0;
          PSS_MIB_TBL_INFO     *tbl_info = NULL;
          char                 pcn[NCSMIB_PCN_LENGTH_MAX];

          m_NCS_CONS_PRINTF("\nProfile found: %s", profile_next);
          m_LOG_PSS_HDLN_STR(NCSFL_SEV_NOTICE, PSS_HDLN_PROFILE_FOUND, profile_next);

          /* Profile exists */
          /* Get Clients fills in all PCNs in all the PWEs */
          m_NCS_PSSTS_GET_CLIENTS(inst->pssts_api, ts_hdl, retval, profile_next, &buff);
          if(NCSCC_RC_SUCCESS != retval)
          {
             m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_GET_PROF_CLIENTS_OP_FAILED);
             if(buff != NULL)
                m_MMGR_FREE_BUFR_LIST(buff);
             return NCSCC_RC_FAILURE;
          }

          /* Decode the USRBUF for clients information */
          buff_len = m_MMGR_LINK_DATA_LEN(buff);
          m_NCS_CONS_PRINTF("\nbuff_len: %d", buff_len);
          if(buff_len == 0)
          {
             /* No Clients in the profile */
             m_LOG_PSS_INFO_C(NCSFL_SEV_NOTICE, PSS_INFO_NO_CLIENTS_FOUND_IN_PROFILE,
                                                                        profile_next);
             break;
          }
          m_NCS_MEMSET(&lcl_uba, '\0', sizeof(lcl_uba));
          ncs_dec_init_space(&lcl_uba, buff);

          buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pwe_cnt, sizeof(uns16));
          if(buff_ptr == NULL)
          {
             m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
                                                               "Decoding PWE count");
             final_retval = NCSCC_RC_FAILURE;
             goto clean_uba;
          }
          pwe_cnt = ncs_decode_16bit(&buff_ptr);
          ncs_dec_skip_space(&lcl_uba, sizeof(uns16));
          m_NCS_CONS_PRINTF("\npwe_cnt: %d", pwe_cnt);
          m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_PWE_COUNT, pwe_cnt);

          /* Creating _ISU profile */
          if(PSS_REFORMAT_TYPE_MIB == reformat_type)
          {
             m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, ts_hdl, retval, "_ISU", profile_next);
             if(NCSCC_RC_SUCCESS != retval)
             {
                /* Log Copying "profile_next" to _ISU profile failed */
                m_LOG_PSS_HDLN_STR(NCSFL_SEV_ERROR, PSS_HDLN_PROFILE_COPY_FAIL, "To _ISU");
                final_retval = NCSCC_RC_FAILURE;
                goto clean_uba; 
             }
          }

          for(j = 0; j < pwe_cnt; j++) /* Loop for all PWEs */
          {
            buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pwe_id, sizeof(PW_ENV_ID));
             if(buff_ptr == NULL)
             {
                m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
                                                                     "Decoding PWE_ID");
                final_retval = NCSCC_RC_FAILURE;
                goto clean_stale_ps_n_uba;
             }
             pwe_id = ncs_decode_16bit(&buff_ptr);
             ncs_dec_skip_space(&lcl_uba, sizeof(PW_ENV_ID));

             m_NCS_CONS_PRINTF("\npwe_id: %d", pwe_id);
             m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_PWE_FOUND, pwe_id);
             buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pcn_cnt, sizeof(uns16));
             if(buff_ptr == NULL)
             {
                m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
                                                                   "Decoding PCN count");
                final_retval = NCSCC_RC_FAILURE;
                goto clean_stale_ps_n_uba;
             }
             pcn_cnt = ncs_decode_16bit(&buff_ptr);
             ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

             m_NCS_CONS_PRINTF("\npcn_cnt: %d", pcn_cnt);
             m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_PCN_COUNT, pcn_cnt);
             for(k = 0; k < pcn_cnt; k++) /* Loop for all PCNs */
             {
                buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&str_len, sizeof(uns16));
                if(buff_ptr == NULL)
                {
                   m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_UBA_DEC_FLATTEN_SPACE_FAIL,
                                                             "Decoding length of PCN name");
                   final_retval = NCSCC_RC_FAILURE;
                   goto clean_stale_ps_n_uba;
                }
                str_len = ncs_decode_16bit(&buff_ptr);
                ncs_dec_skip_space(&lcl_uba, sizeof(uns16));
                m_NCS_CONS_PRINTF("\nstr_len: %d", str_len);
                m_NCS_MEMSET(pcn, '\0', sizeof(NCSMIB_PCN_LENGTH_MAX));
                if(NCSCC_RC_SUCCESS != ncs_decode_n_octets_from_uba(&lcl_uba, (char*)&pcn, str_len))
                {
                   m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_UBA_DEC_OCTETS_FAIL,
                                                                "Decoding PCN name");
                   final_retval = NCSCC_RC_FAILURE;
                   goto clean_stale_ps_n_uba;
                }

                m_LOG_PSS_HDLN_STR(NCSFL_SEV_NOTICE, PSS_HDLN_PCN_FOUND, &pcn);
                /* Using snprintf directly, because the leap routine does not
                   accept variable arguments, which leads to multiple function calls */
                snprintf(pcn_path, sizeof(pcn_path)-1, "%s%d/%s/%d/%s",
                         root_dir, cur_ps_format, profile_next, pwe_id, pcn);
                /* None of the Leap implementations accept filter routines,
                   they always pass NULL- Thats the reason scandir is used directly */
                m_NCS_CONS_PRINTF("\nPCN_PATH: %s", pcn_path);
                table_cnt = scandir(pcn_path, &tbl_list, NULL, NULL);
                m_NCS_CONS_PRINTF("\ntable_cnt: %d", table_cnt);
                if (table_cnt < 0)
                {
                   m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_SCAN_DIR_FAILED);
                   final_retval = NCSCC_RC_FAILURE;
                   goto clean_stale_ps_n_uba;
                }

                m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_TABLE_COUNT, table_cnt);
                if(table_cnt > 0)
                {
                    while(table_cnt--)
                    {
                       char *endptr = NULL;

                       m_NCS_CONS_PRINTF("%s\n", tbl_list[table_cnt]->d_name);
                       tbl_id = strtol(tbl_list[table_cnt]->d_name, &endptr, 0);
                       if(*endptr)
                       {
                          /* Found a file whose name is not digits in complete */
                          free(tbl_list[table_cnt]);
                          continue;
                       }
                       m_NCS_CONS_PRINTF("%d\n", tbl_id);
                       if(NULL != (tbl_info = inst->mib_tbl_desc[tbl_id]))
                       {
                          tbl_info = inst->mib_tbl_desc[tbl_id];
                          /* Filling the current persistent format version, profile,
                                       pwe_id, pcn and table_id in ps_file_record */
                          ps_file_record.ps_format_version = cur_ps_format;
                          m_NCS_MEMCPY(&ps_file_record.profile, &profile_next, sizeof(profile_next));
                          ps_file_record.pwe_id = pwe_id;
                          m_NCS_MEMCPY(&ps_file_record.pcn, &pcn, sizeof(pcn));
                          ps_file_record.tbl_id = tbl_id;

                          switch(reformat_type)
                          {
                             case PSS_REFORMAT_TYPE_STORE_1_TO_1EXT:
                                if(NCSCC_RC_SUCCESS != pss_store_reformatting_1_to_1ext(inst, &ps_file_record))
                                {
                                   final_retval = NCSCC_RC_FAILURE;
                                   goto clean_stale_ps_n_uba_n_tbl_lst;
                                   /* Log Store reformatting failed for this table */
                                   /* What action need to be taken */
                                }
                                m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_1_TO_1EXT_SUCCESS,
                                                                                                              tbl_id);
                                break;

                             case PSS_REFORMAT_TYPE_STORE_HIGHER_TO_1EXT:
                                  if(NCSCC_RC_SUCCESS != pss_store_reformatting_higher_to_1ext(inst, ts_hdl,
                                                                                               &ps_file_record))
                                  {
                                     final_retval = NCSCC_RC_FAILURE;
                                     goto clean_stale_ps_n_uba_n_tbl_lst;
                                     /* Log Store reformatting failed for this table */
                                     /* What action need to be taken */
                                  }
                                  m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_HIGHER_TO_1EXT_SUCCESS,
                                                                                                                    tbl_id);
                                  break;
                             case PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER:
                                  if(NCSCC_RC_SUCCESS != pss_store_reformatting_1ext_to_higher(inst, ts_hdl,
                                                                                               &ps_file_record))
                                  {
                                     final_retval = NCSCC_RC_FAILURE;
                                     goto clean_stale_ps_n_uba_n_tbl_lst;
                                  }
                                  m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_1EXT_TO_HIGHER_SUCCESS,
                                                                                                                    tbl_id);
                                  break;
                             case PSS_REFORMAT_TYPE_STORE_HIGHER_TO_2:
                                  if(NCSCC_RC_SUCCESS != pss_store_reformatting_higher_to_2(inst, ts_hdl,
                                                                                               &ps_file_record))
                                  {
                                     final_retval = NCSCC_RC_FAILURE;
                                     goto clean_stale_ps_n_uba_n_tbl_lst;
                                  }
                                  m_LOG_PSS_HDLN_I(NCSFL_SEV_DEBUG, PSS_HDLN_STORE_REFORMATTING_FROM_1EXT_TO_HIGHER_SUCCESS,
                                                                                                                    tbl_id);
                                  break;
                             case PSS_REFORMAT_TYPE_MIB:
                                  if(NCSCC_RC_SUCCESS != pss_verify_n_mib_reformat(inst, ts_hdl, &ps_file_record))
                                  {
                                     m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, "_ISU",
                                                                                 pwe_id, pcn, tbl_id);
                                     final_retval = NCSCC_RC_FAILURE;
                                     goto clean_stale_ps_n_uba_n_tbl_lst;
                                     /* Log Store reformatting failed for this table */
                                     /* What action need to be taken */
                                  }
                                  break;
                             default:
                                  return NCSCC_RC_FAILURE;
                          }
                       }
                       else
                       {
                          /* This table is not loaded:
                                  the loaded libraries does not have this table or
                                  the library containing this table is not registered to PSSv */
                          m_NCS_PSSTS_FILE_DELETE(inst->pssts_api, ts_hdl, retval, profile_next,
                                                                            pwe_id, pcn, tbl_id);
                       }

                       m_NCS_CONS_PRINTF("\nDone with table %d\n", tbl_id);
                       free(tbl_list[table_cnt]);
                    } /* End of while (table_cnt--) */
                    free(tbl_list);
                }
             } /* End of Loop for all PCNs */
          } /* End of Loop for all PWEs */
          if(lcl_uba.start != NULL)
          {               
             m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
          }
          else if(lcl_uba.ub != NULL)
          {
             m_MMGR_FREE_BUFR_LIST(lcl_uba.ub);
          }
          if(PSS_REFORMAT_TYPE_MIB == reformat_type)
          {
             m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, ts_hdl, retval, profile_next);
             if (NCSCC_RC_SUCCESS != retval)
                 return NCSCC_RC_FAILURE;
             
             m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, ts_hdl, retval, profile_next, "_ISU");
             if (NCSCC_RC_SUCCESS != retval)
                 return NCSCC_RC_FAILURE;
          }
       } /* if(NCSCC_RC_SUCCESS == get_profile_retval) */
       else if(NCSCC_RC_FAILURE == get_profile_retval)
       {
          /* LOG FAILURE - new headline required */
          m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_GET_NEXT_PROFILE_OP_FAILED);
          return NCSCC_RC_FAILURE;
       }
       m_NCS_STRCPY((char*)&profile_name, (char*)&profile_next);
       printf("Profile_name :%s", profile_name);
    }while(NCSCC_RC_SUCCESS == get_profile_retval);
                     /* End of Loop do for all profiles in the cur_ps_format */

    m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, inst->pssts_hdl, retval);

    m_NCS_CONS_PRINTF("\n Done with loop COMPLETED WITH ALL PROFILES");
    if(NCSCC_RC_NO_INSTANCE == get_profile_retval)
    {
       if(PSS_REFORMAT_TYPE_STORE_HIGHER_TO_1EXT == reformat_type)
       {
          snprintf(dest_file_path, sizeof(dest_file_path)-1, "%s1", root_dir);
       }
       else if(PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER == reformat_type)
       {
          snprintf(dest_file_path, sizeof(dest_file_path)-1, "%s%d", root_dir, PSS_PS_FORMAT_VERSION);
       }
       else if(PSS_REFORMAT_TYPE_STORE_HIGHER_TO_2 == reformat_type)
       {
          snprintf(dest_file_path, sizeof(dest_file_path)-1, "%s%d", root_dir, PSS_PS_FORMAT_VERSION);
       }

       if(PSS_REFORMAT_TYPE_STORE_1_TO_1EXT != reformat_type && PSS_REFORMAT_TYPE_MIB != reformat_type)
       {
          m_NCS_CONS_PRINTF("\n dest_file_path: %s", dest_file_path);
          /* Deleting the old persistent store */
          inst_dir.info.copy_dir.i_dir_name = dest_file_path;
          m_NCS_FILE_OP (&inst_dir, NCS_OS_FILE_DELETE_DIR);
   
          snprintf(source_file_path, sizeof(source_file_path)-1, "%s%d",
                                                root_dir, cur_ps_format);
          m_NCS_CONS_PRINTF("\n source_file_path: %s", source_file_path);
          /* coyping the persistent store */
          inst_dir.info.copy_dir.i_dir_name = source_file_path;
          inst_dir.info.copy_dir.i_new_dir_name = dest_file_path;
          m_NCS_FILE_OP (&inst_dir, NCS_OS_FILE_COPY_DIR);
   
          /* Deleting the old persistent store */
          inst_dir.info.copy_dir.i_dir_name = source_file_path;
          m_NCS_FILE_OP (&inst_dir, NCS_OS_FILE_DELETE_DIR);
       }
    }

    return NCSCC_RC_SUCCESS;

clean_stale_ps_n_uba_n_tbl_lst:
   while(table_cnt--)
      free(tbl_list[table_cnt]);
   free(tbl_list);

clean_stale_ps_n_uba:
   m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, ts_hdl, retval, "_ISU");
   m_NCS_PSSTS_DELETE_TEMP_FILE(inst->pssts_api, inst->pssts_hdl, retval);

clean_uba:

   if(lcl_uba.start != NULL)
   {               
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
   }
   else if(lcl_uba.ub != NULL)
   {
      m_MMGR_FREE_BUFR_LIST(lcl_uba.ub);
   }

   return final_retval;
}

/*******************************************************************************\
*  Name:          pss_check_n_reformat                                          *
*                                                                               *
*  Description:   This function checks the validity of persistent store format  *
*                 and invokes appropriate type of reformatting required.        *
*                                                                               *
*  Arguments:                                                                   *
*                 ps_format_version: Existing Persistent Store format.          *
*                                                                               *
*  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                             *
*                                                                               *
\*******************************************************************************/

uns32 
pss_check_n_reformat(PSS_CB *inst, uns32 ps_format_version)
{
   /*
     For 3.0.b:
         If persistent store format is less than 1
            Log that it is Invalid persistent store format.
         If the persistent store format is 1
            Store reformatting from 1Ext to higher(2) is to be invoked. 
            MIB reformatting is to be invoked.
         If the persistent store format is 2
            MIB reformatting is to be invoked. This is required 
            while upgrading without any change in the PS format.
         If the persistent store format is none of these (greater than 2)
           -> Store reformatting from Higher to 3.0.b(2) is to be invoked.
           -> MIB reformatting is to be invoked.
   */

   NCSPSS_CREATE   pwe;
   uns32           rc = NCSCC_RC_SUCCESS;

   if(ps_format_version < 1)
   {
      /* Log Invalid persistent store format */
      m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PERSISTENT_STORE_FORMAT, ps_format_version);
      return NCSCC_RC_FAILURE;
   }
   if(ps_format_version == 1)
   {
      /* Create PSS Target Service (File System/FLASH/SQA DB?) */
      rc = pss_ts_create(&pwe, ps_format_version);
      if (rc != NCSCC_RC_SUCCESS)
      {
         /* log the error */
         m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR,
               PSS_ERR_TGT_SVSC_CREATE_FAILED, rc);

         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      if(NCSCC_RC_SUCCESS != pss_reformat(inst, PSS_REFORMAT_TYPE_STORE_1EXT_TO_HIGHER, pwe.i_pssts_hdl, ps_format_version))
         return NCSCC_RC_FAILURE;
      return (pss_reformat(inst, PSS_REFORMAT_TYPE_MIB, inst->pssts_hdl, PSS_PS_FORMAT_VERSION));
   }
   else if(ps_format_version == 2)
   {
      return (pss_reformat(inst, PSS_REFORMAT_TYPE_MIB, inst->pssts_hdl, ps_format_version));
   }
   else
   {
      /* Create PSS Target Service (File System/FLASH/SQA DB?) */
      rc = pss_ts_create(&pwe, ps_format_version);
      if (rc != NCSCC_RC_SUCCESS)
      {
         /* log the error */
         m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR,
               PSS_ERR_TGT_SVSC_CREATE_FAILED, rc);

         return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      if(NCSCC_RC_SUCCESS != pss_reformat(inst, PSS_REFORMAT_TYPE_MIB, inst->pssts_hdl, PSS_PS_FORMAT_VERSION))
         return NCSCC_RC_FAILURE;
      return (pss_reformat(inst, PSS_REFORMAT_TYPE_STORE_HIGHER_TO_2, pwe.i_pssts_hdl, ps_format_version));
   }
   return NCSCC_RC_SUCCESS;
}

#endif /* (NCS_PSR == 1) */

#endif /* (NCS_MAB == 1) */
