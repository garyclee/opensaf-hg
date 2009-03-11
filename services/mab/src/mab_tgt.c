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



*/

/** H&J Includes...
 **/

#include "mab.h"

#if (NCS_MAB == 1)
static void pssts_destroy_temp_sort_db(NCS_PATRICIA_TREE *pTree);

/*****************************************************************************

  PROCEDURE NAME:    mab_dbg_sink

  DESCRIPTION:

   MAB is entirely instrumented to fall into this debug sink function
   if it hits any if-conditional that fails, but should not fail. This
   is the default implementation of that SINK macro.
 
   The customer is invited to redefine the macro (in rms_tgt.h) or 
   re-populate the function here.  This sample function is invoked by
   the macro m_MAB_DBG_SINK.
   
  ARGUMENTS:

  uns32   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE

  RETURNS:

  code    - just echo'ed back 

*****************************************************************************/

#if (MAB_DEBUG == 1)

uns32 mab_dbg_sink(uns32 l, char* f, uns32 code)
  {
  m_NCS_CONS_PRINTF ("IN MAB_DBG_SINK: line %d, file %s\n",l,f);
  return code;
  }

#endif

/*****************************************************************************

  PROCEDURE NAME:    sysf_<mab-component>-validate

  DESCRIPTION:
  
  return the correct MAB subcomponent handle, which was handed over to the 
  MAB client at sub-component CREATE time in the form of the o_<mab>_hdl 
  return value.

  NOTES: This is sample code. The customer can choose any strategy that suits
  thier design.

*****************************************************************************/

/* For MAC validation */

void* sysf_mac_validate(uns32 k)
  {
  return (void *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, k);
  }

/* For MAS validation */

void* sysf_mas_validate(uns32 k)
  {
  return (void *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, k);
  }

/* For OAC validation */

void* sysf_oac_validate(uns32 k)
  {
  return (void *)ncshm_take_hdl(NCS_SERVICE_ID_MAB, k);
  }

void* sysf_pss_validate(uns32 k)
  {
  return (void *)ncshm_take_hdl(NCS_SERVICE_ID_PSS, k);
  }

#if (MAB_TRACE2 == 1)


void mab_dbg_dump_fltr_op(MAS_TBL*  inst,
                          MAS_FLTR* fltr,
                          uns32     tbl_id,
                          MAB_FMODE mode)
{
  uns8* role_name = NULL;

  m_NCS_CONS_PRINTF("\n\n===============================================");
  m_NCS_CONS_PRINTF("\n [MAS_FLTR] MODE:%s.",mode == MFM_CREATE ? "CREATE" :
                                          mode == MFM_MODIFY ? "MODIFY" : "DESTROY");
  m_NCS_CONS_PRINTF("\n================================================");

#if (NCS_RMS == 1)    
    role_name = (uns8*) (inst->re.role == NCSFT_ROLE_PRIMARY ? "PRIMARY" : inst->re.role == NCSFT_ROLE_BACKUP ? "BACKUP" : "OTHER");
#else
    role_name = (uns8*) ("NONE");
#endif
  m_NCS_CONS_PRINTF("\n MAS_TBL:%p | role:%s.",inst,role_name);

  m_NCS_CONS_PRINTF("\n================================================");
  m_NCS_CONS_PRINTF("\ntbl_id:%d / vcard:%d / fltr_id[0]:%d / fltr_id[1]:%d.",
                      tbl_id,fltr->vcard,fltr->fltr_ids[0],fltr->fltr_ids[1]);
  m_NCS_CONS_PRINTF("\nfltr type: %d.",fltr->fltr.type);
  switch(fltr->fltr.type)
  {
   case NCSMAB_FLTR_RANGE:
     m_NCS_CONS_PRINTF("\nrange fltr:%d",fltr->fltr.fltr.range.i_min_idx_fltr[0]);
     break;
   case NCSMAB_FLTR_ANY:
     m_NCS_CONS_PRINTF("\nany fltr.");
     break;
   case NCSMAB_FLTR_SAME_AS:
     m_NCS_CONS_PRINTF("\nsame as fltr:%d",fltr->fltr.fltr.same_as.i_table_id);
     break;
   case NCSMAB_FLTR_SCALAR:
     m_NCS_CONS_PRINTF("\nscalar...");
     break;
   case NCSMAB_FLTR_DEFAULT:
     m_NCS_CONS_PRINTF("\ndefault...");
     break;
   default:
     break;
  }

  m_NCS_CONS_PRINTF("\n===============================================\n\n");

}

#endif


/* PSS Target Service Code goes here */
MABCOM_API uns32 ncs_pssts_api (NCS_PSSTS_ARG * parg);

/* This function takes in all file parameters
 * and returns the full path of the file
 * in the caller provided buffer.
 */
static uns32 get_full_path_of_file(uns8 * root, uns8 * profile, char *n_pcn,
                            uns16 pwe, uns32 tbl,
                            uns8 *buf, uns32 buf_len)
{
    uns8  path1[NCS_PSSTS_MAX_PATH_LEN];
    uns8  path2[NCS_PSSTS_MAX_PATH_LEN];
    uns8  buf2[32 + NCSMIB_PCN_LENGTH_MAX];
    uns32 retval;
    NCS_OS_FILE file;

    /* First, get the path of the profile directory */
    file.info.dir_path.i_main_dir = root;
    file.info.dir_path.i_sub_dir  = profile;
    file.info.dir_path.i_buf_size = sizeof(path1);
    file.info.dir_path.io_buffer  = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Get the path of the pwe directory */
    m_NCS_OS_SNPRINTF((char *)buf2, sizeof(buf2), "%d", pwe);
    file.info.dir_path.i_main_dir = path1;
    file.info.dir_path.i_sub_dir  = buf2;
    file.info.dir_path.i_buf_size = sizeof(path2);
    file.info.dir_path.io_buffer  = path2;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Get the path of the n_pcn directory */
    m_NCS_OS_SNPRINTF((char *)buf2, sizeof(buf2), "%s", n_pcn);
    file.info.dir_path.i_main_dir = path2;
    file.info.dir_path.i_sub_dir  = buf2;
    file.info.dir_path.i_buf_size = sizeof(path1);
    file.info.dir_path.io_buffer  = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Get the path of the table file */
    m_NCS_OS_SNPRINTF((char *)buf2, sizeof(buf2), "%d", tbl);
    file.info.dir_path.i_main_dir = path1;
    file.info.dir_path.i_sub_dir  = buf2;
    file.info.dir_path.i_buf_size = buf_len;
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


static uns32 create_directories(uns8 * root, uns8 * profile, char *n_pcn, uns16 pwe)
{
    uns8   path1[NCS_PSSTS_MAX_PATH_LEN];
    uns8   path2[NCS_PSSTS_MAX_PATH_LEN];
    uns8   buf2[32 + NCSMIB_PCN_LENGTH_MAX];
    uns32  retval;
    NCS_OS_FILE file;

    /* Get the full path of the profile */
    file.info.dir_path.i_main_dir = root;
    file.info.dir_path.i_sub_dir  = profile;
    file.info.dir_path.i_buf_size = sizeof(path1);
    file.info.dir_path.io_buffer  = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the profile directory exists */
    file.info.dir_exists.i_dir_name = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (file.info.dir_exists.o_exists == FALSE) /* Create the directory */
    {
        file.info.create_dir.i_dir_name = path1;

        retval = m_NCS_FILE_OP ( &file, NCS_OS_FILE_CREATE_DIR);
        if (retval != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;
    }

    /* Get the full path for pwe directory */
    memset((char*)&buf2, '\0', sizeof(buf2));
    memset((char*)&path2, '\0', sizeof(path2));
    m_NCS_OS_SNPRINTF((char *)buf2, sizeof(buf2), "%d", pwe);
    file.info.dir_path.i_main_dir = path1;
    file.info.dir_path.i_sub_dir  = buf2;
    file.info.dir_path.i_buf_size = sizeof(path2);
    file.info.dir_path.io_buffer  = path2;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the n_pcn directory exists */
    file.info.dir_exists.i_dir_name = path2;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (file.info.dir_exists.o_exists == FALSE) /* Create the directory */
    {
        file.info.create_dir.i_dir_name = path2;

        retval = m_NCS_FILE_OP ( &file, NCS_OS_FILE_CREATE_DIR);
        if (retval != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;
    }

    /* Get the full path for n_pcn directory */
    m_NCS_OS_SNPRINTF((char *)buf2, m_NCS_OS_STRLEN(n_pcn) + 1, "%s", n_pcn);
    file.info.dir_path.i_main_dir = path2;
    file.info.dir_path.i_sub_dir  = buf2;
    file.info.dir_path.i_buf_size = sizeof(path1);
    file.info.dir_path.io_buffer  = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the pwe directory exists */
    file.info.dir_exists.i_dir_name = path1;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (file.info.dir_exists.o_exists == FALSE) /* Create the directory */
    {
        file.info.create_dir.i_dir_name = path1;

        retval = m_NCS_FILE_OP ( &file, NCS_OS_FILE_CREATE_DIR);
        if (retval != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_open_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_OPEN_FILE *open)
{
    uns32 retval;
    NCS_OS_FILE  file;
    uns8  buf[NCS_PSSTS_MAX_PATH_LEN];

    retval = create_directories(inst->root_dir, open->i_profile_name,
                                open->i_pcn, open->i_pwe_id);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    switch (open->i_mode)
    {
    case NCS_PSSTS_FILE_PERM_READ: /* Open the existing file for reading */
        {
            retval = get_full_path_of_file(inst->root_dir,
                open->i_profile_name,
                open->i_pcn,
                open->i_pwe_id,
                open->i_tbl_id,
                buf, sizeof(buf));
            if (retval != NCSCC_RC_SUCCESS)
                return NCSCC_RC_FAILURE;

            file.info.open.i_file_name = buf;
            file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

            retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_OPEN);
            if (retval != NCSCC_RC_SUCCESS)
                return NCSCC_RC_FAILURE;

            open->o_handle = (long)(file.info.open.o_file_handle);

            return NCSCC_RC_SUCCESS;
        }
        break;

    case NCS_PSSTS_FILE_PERM_WRITE:/* Open a file for writing. Create if necessary */
        {
            /* Create directory structure if it does not exist */
            retval = create_directories(inst->root_dir,
                open->i_profile_name,
                open->i_pcn,
                open->i_pwe_id);
            if (retval != NCSCC_RC_SUCCESS)
                return NCSCC_RC_FAILURE;

            retval = get_full_path_of_file(inst->root_dir,
                open->i_profile_name,
                open->i_pcn,
                open->i_pwe_id,
                open->i_tbl_id,
                buf, sizeof(buf));
            if (retval != NCSCC_RC_SUCCESS)
                return NCSCC_RC_FAILURE;

            file.info.open.i_file_name = buf;
            file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_WRITE;

            retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_OPEN);
            if (retval != NCSCC_RC_SUCCESS)
                return NCSCC_RC_FAILURE;

            open->o_handle = (long)(file.info.open.o_file_handle);

            return NCSCC_RC_SUCCESS;
        }
        break;

    default:
        return NCSCC_RC_FAILURE;
    }
}


static uns32 pssts_read_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_READ_FILE * read)
{
    uns32 retval;
    NCS_OS_FILE  file;

    /* Seek the particular offset and then read from it */
    file.info.seek.i_file_handle = (void *)(long)(read->i_handle);
    file.info.seek.i_offset      = read->i_offset;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_SEEK);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.read.i_file_handle = (void *)(long)(read->i_handle);
    file.info.read.i_buf_size    = read->i_bytes_to_read;
    file.info.read.i_buffer      = read->io_buffer;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_READ);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    read->o_bytes_read = file.info.read.o_bytes_read;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_write_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_WRITE_FILE * write)
{
    NCS_OS_FILE file;

    file.info.write.i_file_handle = (void *)(long)(write->i_handle);
    file.info.write.i_buf_size    = write->i_buf_size;
    file.info.write.i_buffer      = write->i_buffer;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_WRITE);
}


static uns32 pssts_close_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_CLOSE_FILE * close)
{
    NCS_OS_FILE file;

    file.info.close.i_file_handle = (void *)(long)(close->i_handle);

    return m_NCS_FILE_OP ( &file, NCS_OS_FILE_CLOSE);
}


static uns32 pssts_file_exists (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_FILE_EXISTS * file_exists)
{
    NCS_OS_FILE file;
    uns32 retval;
    uns8 buf[NCS_PSSTS_MAX_PATH_LEN];

    /* Get the full file path */
    retval = get_full_path_of_file(inst->root_dir,
        file_exists->i_profile_name,
        file_exists->i_pcn,
        file_exists->i_pwe_id,
        file_exists->i_tbl_id,
        buf, sizeof(buf));
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the file exists */
    file.info.file_exists.i_file_name = buf;

    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_EXISTS);
    if (retval != NCSCC_RC_SUCCESS) /* Some error */
        return NCSCC_RC_FAILURE;

    file_exists->o_exists = file.info.file_exists.o_file_exists;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_file_size (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_FILE_SIZE * file_size)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    /* Get the full file path */
    retval = get_full_path_of_file(inst->root_dir,
        file_size->i_profile_name,
        file_size->i_pcn,
        file_size->i_pwe_id,
        file_size->i_tbl_id,
        buf, sizeof(buf));
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.size.i_file_name = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_SIZE);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file_size->o_file_size = file.info.size.o_file_size;
    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_create_profile (NCS_PSSTS_CB * inst,
                                   NCS_PSSTS_ARG_CREATE_PROFILE * create_profile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    /* First get the full paths of the profile */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = create_profile->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Now create the directory */
    file.info.create_dir.i_dir_name = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_CREATE_DIR);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_move_profile (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_MOVE_PROFILE * move_profile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];

    /* First get the full paths of the 2 profiles */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = move_profile->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = move_profile->i_new_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer  = buf2;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Now move the directory */
    file.info.rename.i_file_name = buf;
    file.info.rename.i_new_file_name = buf2;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_RENAME);
}


static uns32 pssts_copy_profile (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_COPY_PROFILE * copy_profile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];

    /* First get the full paths of the 2 profiles */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = copy_profile->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = copy_profile->i_new_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer  = buf2;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.copy_dir.i_dir_name = buf;
    file.info.copy_dir.i_new_dir_name = buf2;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_COPY_DIR);
}


static uns32 pssts_delete_profile (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_DELETE_PROFILE * delete_profile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    /* First get the full path of the profile */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = delete_profile->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.delete_dir.i_dir_name = buf;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_DELETE_DIR);
}


static uns32 pssts_get_profile_desc (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_GET_DESC * get_desc)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];
    void *     file_handle;

    /* Get the full path of the description file and open it for reading */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = get_desc->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_path.i_main_dir = buf;
    file.info.dir_path.i_sub_dir  = (uns8 *) NCS_PSSTS_PROFILE_DESC_FILE;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer  = buf2;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the file exists */
    file.info.file_exists.i_file_name = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_EXISTS);
    if (retval != NCSCC_RC_SUCCESS) /* Some error */
        return NCSCC_RC_FAILURE;
    if(file.info.file_exists.o_file_exists == FALSE)
    {
        /* File doesn't exist. Return normally. */
        get_desc->o_exists = FALSE;
        return NCSCC_RC_SUCCESS;
    }

    /* Now open the file for reading */
    file.info.open.i_file_name = buf2;
    file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_OPEN);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file_handle = file.info.open.o_file_handle;

    file.info.read.i_file_handle = file_handle;
    file.info.read.i_buf_size    = get_desc->i_buf_length;
    file.info.read.i_buffer      = get_desc->io_buffer;

    /* Read the data and then close the file */
    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_READ);

    file.info.close.i_file_handle = file_handle;
    m_NCS_FILE_OP (&file, NCS_OS_FILE_CLOSE);

    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    get_desc->o_exists = TRUE;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_set_profile_desc (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_SET_DESC * set_desc)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];
    void *     file_handle;

    /* Get the full path of the description file and open it for writing */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = set_desc->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the profile directory exists, else create it */
    file.info.dir_exists.i_dir_name = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (file.info.dir_exists.o_exists == FALSE)
    {
        file.info.create_dir.i_dir_name = buf;

        retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_CREATE_DIR);
        if (retval != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;
    }

    file.info.dir_path.i_main_dir = buf;
    file.info.dir_path.i_sub_dir  = (uns8 *) NCS_PSSTS_PROFILE_DESC_FILE;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer  = buf2;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Now open the file for writing */
    file.info.open.i_file_name = buf2;
    file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_WRITE;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_OPEN);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file_handle = file.info.open.o_file_handle;

    file.info.write.i_file_handle = file_handle;
    file.info.write.i_buf_size    = strlen((const char *)(set_desc->i_buffer)) + 1;
    file.info.write.i_buffer      = set_desc->i_buffer;

    /* Write the data and then close the file */
    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_WRITE);

    file.info.close.i_file_handle = file_handle;
    m_NCS_FILE_OP (&file, NCS_OS_FILE_CLOSE);

    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_pss_config (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_PSS_CONFIG * pss_config)
{
    /* Populate the fields of the PSS_CONFIG structure */
    pss_config->current_profile_name = inst->current_profile;
    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_set_config (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_SET_CONFIG * set_config)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    strcpy(inst->current_profile, set_config->i_current_profile_name);

    /* Now create the Root directory */
    file.info.dir_exists.i_dir_name = inst->root_dir;
    retval = m_NCS_FILE_OP (&file,NCS_OS_FILE_DIR_EXISTS);
    if(retval != NCSCC_RC_SUCCESS)
        return retval;

    if(file.info.dir_exists.o_exists == FALSE)
    {
       file.info.create_dir.i_dir_name = inst->root_dir;
       retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_CREATE_DIR);
       if (retval != NCSCC_RC_SUCCESS)
          return NCSCC_RC_FAILURE;
    }

    /* First get the full paths of the profile */
    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = inst->current_profile;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Now create the directory */
    file.info.dir_exists.i_dir_name = buf;
    retval = m_NCS_FILE_OP (&file,NCS_OS_FILE_DIR_EXISTS);
    if((retval == NCSCC_RC_SUCCESS) && (file.info.dir_exists.o_exists == TRUE))
        return NCSCC_RC_SUCCESS;

    file.info.create_dir.i_dir_name = buf;
    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_CREATE_DIR);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_open_temp_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_OPEN_TEMP_FILE * open_tfile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = (uns8 *)NCS_PSSTS_TEMP_FILE_NAME;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.open.i_file_name = buf;
    file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_WRITE;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_OPEN);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    open_tfile->o_handle = (long)(file.info.open.o_file_handle);
    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_copy_temp_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_COPY_TEMP_FILE * copy_tfile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];

    retval = create_directories(inst->root_dir, copy_tfile->i_profile_name,
                                copy_tfile->i_pcn, copy_tfile->i_pwe_id);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = (uns8 *)NCS_PSSTS_TEMP_FILE_NAME;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    retval = get_full_path_of_file(inst->root_dir,
        copy_tfile->i_profile_name,
        copy_tfile->i_pcn,
        copy_tfile->i_pwe_id,
        copy_tfile->i_tbl_id,
        buf2, sizeof(buf2));
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.copy.i_file_name = buf;
    file.info.copy.i_new_file_name = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_RENAME);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_get_next_profile (NCS_PSSTS_CB * inst,
                                     NCS_PSSTS_ARG_GET_NEXT_PROFILE * get_next)
{
    NCS_OS_FILE file;
    uns32      retval, i = 0, j = 0, len = 0;
    NCS_PSSTS_SORT_DB lcl_db;
    NCS_PSSTS_SORT_NODE *node = NULL;
    NCS_PATRICIA_NODE *pNode = NULL;
    NCS_PSSTS_SORT_KEY key;

    memset(get_next->io_buffer, '\0', get_next->i_buf_length);

    memset(&lcl_db, '\0', sizeof(lcl_db));
    lcl_db.params.key_size = sizeof(NCS_PSSTS_SORT_KEY);
    if (ncs_patricia_tree_init(&lcl_db.tree, &lcl_db.params) != NCSCC_RC_SUCCESS)
    {
       return NCSCC_RC_FAILURE;
    }
    m_NCS_MEM_DBG_LOC(lcl_db.tree.root_node.key_info);

    memset(&file, '\0', sizeof(file));
    file.info.get_list.i_dir_name   = inst->root_dir;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_LIST);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    for(i = 0; i < file.info.get_list.o_list_count; i++)
    {
       if((m_NCS_STRCMP(file.info.get_list.o_namelist[i], ".") == 0) ||
          (m_NCS_STRCMP(file.info.get_list.o_namelist[i], "..") == 0))
       {
          continue;
       }
       /* Add each entry to the patricia tree. */
       if((node = m_NCS_OS_MEMALLOC(sizeof(NCS_PSSTS_SORT_NODE), NULL)) == NULL)
       {
          pssts_destroy_temp_sort_db(&lcl_db.tree);
          for(j = i; j < file.info.get_list.o_list_count; j++)
             m_NCS_OS_MEMFREE(file.info.get_list.o_namelist[i], NULL);
          m_NCS_OS_MEMFREE(file.info.get_list.o_namelist, NULL);
          return NCSCC_RC_FAILURE;
       }
       memset(node, '\0', sizeof(NCS_PSSTS_SORT_NODE));
       node->key.len = m_NCS_STRLEN(file.info.get_list.o_namelist[i]);
       strcpy((char*)&node->key.name, file.info.get_list.o_namelist[i]);
       m_NCS_OS_MEMFREE(file.info.get_list.o_namelist[i], NULL);

       pNode = (NCS_PATRICIA_NODE *)node;
       pNode->key_info = (uns8*)&node->key;

       retval = ncs_patricia_tree_add(&lcl_db.tree, pNode);
       if(retval != NCSCC_RC_SUCCESS)
       {
          pssts_destroy_temp_sort_db(&lcl_db.tree);
          for(j = i; j < file.info.get_list.o_list_count; j++)
             m_NCS_OS_MEMFREE(file.info.get_list.o_namelist[i], NULL);
          m_NCS_OS_MEMFREE(file.info.get_list.o_namelist, NULL);
          return NCSCC_RC_FAILURE;
       }
    }
    m_NCS_OS_MEMFREE(file.info.get_list.o_namelist, NULL);

    /* Now, look for the next match. */
    memset(&key, '\0', sizeof(key));
    if(get_next->i_profile_name != NULL)
    {
       key.len = m_NCS_STRLEN(get_next->i_profile_name);
       len = m_NCS_STRLEN(get_next->i_profile_name);
       if(len != 0)
       {
          strcpy(&key.name, get_next->i_profile_name);
       }
    }
    if(len == 0)
    {
       pNode = ncs_patricia_tree_getnext(&lcl_db.tree, NULL);
    }
    else
    {
       pNode = ncs_patricia_tree_getnext(&lcl_db.tree, (const uns8 *)&key);
    }
    if(pNode == NULL)
    {
       /* No more entries. */
       pssts_destroy_temp_sort_db(&lcl_db.tree);
       return NCSCC_RC_NO_INSTANCE;
    }

    /* Got the required node. Now, copy the data to the returnable-location. */
    strcpy(get_next->io_buffer, ((NCS_PSSTS_SORT_NODE*)pNode)->key.name);

    /* Cleanup the temporary sort database */
    pssts_destroy_temp_sort_db(&lcl_db.tree);

    return NCSCC_RC_SUCCESS;
}

static void pssts_destroy_temp_sort_db(NCS_PATRICIA_TREE *pTree)
{
   NCS_PATRICIA_NODE *pNode = NULL;

   while((pNode = ncs_patricia_tree_getnext(pTree, NULL)) != NULL)
   {
      ncs_patricia_tree_del(pTree, pNode);
      m_NCS_OS_MEMFREE(pNode, NULL);
      pNode = NULL;
   }
   ncs_patricia_tree_destroy(pTree);

   return;
}

static uns32 pssts_profile_exists (NCS_PSSTS_CB * inst,
                            NCS_PSSTS_ARG_PROFILE_EXISTS * profile_exists)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = profile_exists->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_exists.i_dir_name = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    profile_exists->o_exists = file.info.dir_exists.o_exists;
    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_pwe_exists (NCS_PSSTS_CB * inst,
                        NCS_PSSTS_ARG_PWE_EXISTS * pwe_exists)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf3[NCS_PSSTS_MAX_PATH_LEN];

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = pwe_exists->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_SNPRINTF((char *)buf3, sizeof(buf3), "%d", pwe_exists->i_pwe);
    file.info.dir_path.i_main_dir = buf;
    file.info.dir_path.i_sub_dir = buf3;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_exists.i_dir_name = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    pwe_exists->o_exists = file.info.dir_exists.o_exists;
    return NCSCC_RC_SUCCESS;
}


static uns32 pssts_pcn_exists (NCS_PSSTS_CB * inst,
                          NCS_PSSTS_ARG_PCN_EXISTS * pcn_exists)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf3[NCS_PSSTS_MAX_PATH_LEN];

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = pcn_exists->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_SNPRINTF((char *)buf3, sizeof(buf3), "%d", pcn_exists->i_pwe);
    file.info.dir_path.i_main_dir = buf;
    file.info.dir_path.i_sub_dir = buf3;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_SNPRINTF((char *)buf3, m_NCS_OS_STRLEN(buf3), "%s", pcn_exists->i_pcn);
    file.info.dir_path.i_main_dir = buf2;
    file.info.dir_path.i_sub_dir = buf3;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_exists.i_dir_name = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    pcn_exists->o_exists = file.info.dir_exists.o_exists;
    return NCSCC_RC_SUCCESS;
}

static uns32 pssts_delete_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_DELETE_FILE *delete_file)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    retval = get_full_path_of_file(inst->root_dir,
                   delete_file->i_profile_name,
                   delete_file->i_pcn,
                   delete_file->i_pwe,
                   delete_file->i_tbl_id,
                   buf, sizeof(buf));
    if (retval != NCSCC_RC_SUCCESS)
         return NCSCC_RC_FAILURE;

    file.info.remove.i_file_name = (char*)&buf;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_REMOVE);
}

static uns32 pssts_delete_pcn(NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_DELETE_PCN *delete_pcn)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf3[NCS_PSSTS_MAX_PATH_LEN];
    uns8       buf4[NCS_PSSTS_MAX_PATH_LEN];

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = delete_pcn->i_profile_name;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_SNPRINTF((char *)buf3, sizeof(buf3), "%d", delete_pcn->i_pwe);
    file.info.dir_path.i_main_dir = buf;
    file.info.dir_path.i_sub_dir = buf3;
    file.info.dir_path.i_buf_size = sizeof(buf2);
    file.info.dir_path.io_buffer = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_SNPRINTF((char *)buf3, sizeof(buf3), "%s", delete_pcn->i_pcn);
    file.info.dir_path.i_main_dir = buf2;
    file.info.dir_path.i_sub_dir = buf3;
    file.info.dir_path.i_buf_size = sizeof(buf4);
    file.info.dir_path.io_buffer = buf4;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    /* Check if the PCN exists */
    file.info.dir_exists.i_dir_name = buf4;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_EXISTS);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    if (file.info.dir_exists.o_exists == FALSE)
        return NCSCC_RC_SUCCESS; /* Objective was to delete, if existing. */

    file.info.delete_dir.i_dir_name = buf4;
    return m_NCS_FILE_OP (&file, NCS_OS_FILE_DELETE_DIR);
}


static uns32 pssts_get_clients(NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_GET_CLIENTS *get_clients)
{
    NCS_OS_FILE file;
    NCS_UBAID   uba;
    uns32      retval, pwe_id = 0;
    uns16      pwe_cnt = 0, pcn_cnt = 0;
    char       cur_pwe[64], next_pwe[64]; /* Assuming the max length of each pwe/mib file name */
    char       cur_pcn[NCS_PSSTS_MAX_PATH_LEN], next_pcn[NCS_PSSTS_MAX_PATH_LEN];
    char       prof_path[NCS_PSSTS_MAX_PATH_LEN], pwe_path[NCS_PSSTS_MAX_PATH_LEN];
    uns8       *enc_pwe_cnt_loc = NULL, *enc_pcn_cnt_loc = NULL, *p8;
    uns16      len = 0;

    memset(&uba, 0, sizeof(uba));
    memset(&next_pwe, '\0', sizeof(next_pwe));
    memset(&cur_pwe, '\0', sizeof(cur_pwe));
    memset(&next_pcn, '\0', sizeof(next_pcn));
    memset(&cur_pcn, '\0', sizeof(cur_pcn));
    file.info.dir_path.i_main_dir   = inst->root_dir;
    file.info.dir_path.i_sub_dir    = get_clients->i_profile_name;
    file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
    file.info.dir_path.io_buffer    = (uns8*)&prof_path; /* Profile path */
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.get_next.i_dir_name   = (char*)&prof_path;
    file.info.get_next.i_file_name  = NULL;
    file.info.get_next.i_buf_size   = 64;
    file.info.get_next.io_next_file = (char*)&next_pwe;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
    if (retval != NCSCC_RC_SUCCESS)
       return NCSCC_RC_FAILURE;

    while((retval == NCSCC_RC_SUCCESS) && (next_pwe[0] != '\0'))
    {
       if((m_NCS_STRCMP((char *)&next_pwe, ".") == 0) ||
          (m_NCS_STRCMP((char *)&next_pwe, "..") == 0) ||
          (m_NCS_STRCMP((char *)&next_pwe, "ProDesc.txt") == 0))
       {
          strcpy((char *)&cur_pwe, (char *)&next_pwe);
          file.info.get_next.i_dir_name = (char*)&prof_path;
          file.info.get_next.i_file_name  = (char*)&cur_pwe;
          file.info.get_next.i_buf_size   = 64;
          file.info.get_next.io_next_file = (char*)&next_pwe;
          retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
          continue;
       }
       if(enc_pwe_cnt_loc == NULL)
       {
           ncs_enc_init_space(&uba);
           enc_pwe_cnt_loc = ncs_enc_reserve_space(&uba, 2);
           if(enc_pwe_cnt_loc == NULL)
           {
              retval = NCSCC_RC_FAILURE;
              continue;
           }
           ncs_enc_claim_space(&uba, 2);
           get_clients->o_usrbuf = uba.start;
       }
       strcpy((char *)&cur_pwe, (char *)&next_pwe);
       /* encode "cur_pwe" into o_uba */
       p8 = ncs_enc_reserve_space(&uba, 2);
       if(p8 == NULL)
       {
          retval = NCSCC_RC_FAILURE;
          continue;
       }
       sscanf((char*)&cur_pwe, "%d", (int*)&pwe_id);
       ncs_encode_16bit(&p8, (uns16)pwe_id);
       ncs_enc_claim_space(&uba, 2);

       ++pwe_cnt;
       pcn_cnt = 0;
       enc_pcn_cnt_loc = NULL;

       file.info.dir_path.i_main_dir   = (char*)&prof_path;
       file.info.dir_path.i_sub_dir    = (char*)&cur_pwe;
       file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
       file.info.dir_path.io_buffer    = (char*)&pwe_path; /* PWE path */
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
       if (retval != NCSCC_RC_SUCCESS)
          continue;

       file.info.get_next.i_dir_name = (char*)&pwe_path;
       file.info.get_next.i_file_name  = NULL;
       file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
       file.info.get_next.io_next_file = (char*)&next_pcn;
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       if (retval != NCSCC_RC_SUCCESS)
          continue;

       while((retval == NCSCC_RC_SUCCESS) && (next_pcn[0] != '\0'))
       {
          if((m_NCS_STRCMP((char *)&next_pcn, ".") == 0) ||
             (m_NCS_STRCMP((char *)&next_pcn, "..") == 0))
          {
             strcpy((char *)&cur_pcn, (char *)&next_pcn);
             file.info.get_next.i_dir_name = (char*)&pwe_path;
             file.info.get_next.i_file_name  = (char*)&cur_pcn;
             file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
             file.info.get_next.io_next_file = (char*)&next_pcn;
             retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
             continue;
          }
          if(enc_pcn_cnt_loc == NULL)
          {
             enc_pcn_cnt_loc = ncs_enc_reserve_space(&uba, 2);
             if(enc_pcn_cnt_loc == NULL)
             {
                retval = NCSCC_RC_FAILURE;
                continue;
             }
             ncs_enc_claim_space(&uba, 2);
          }
          strcpy((char *)&cur_pcn, (char *)&next_pcn);
          ++pcn_cnt;
          /* encode "cur_pcn" into o_uba */
          len = m_NCS_STRLEN((char*)&cur_pcn) + 1;
          p8 = ncs_enc_reserve_space(&uba, 2);
          if(p8 == NULL)
          {
             retval = NCSCC_RC_FAILURE;
             continue;
          }
          ncs_encode_16bit(&p8, len);
          ncs_enc_claim_space(&uba, 2);
          retval = ncs_encode_n_octets_in_uba(&uba, (char*)&cur_pcn, len);
          if(retval != NCSCC_RC_SUCCESS)
             continue;

          file.info.get_next.i_dir_name = (char*)&pwe_path;
          file.info.get_next.i_file_name  = (char*)&cur_pcn;
          file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
          file.info.get_next.io_next_file = (char*)&next_pcn;
          retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       }  /* while on next_pcn */ 

       if((retval == NCSCC_RC_SUCCESS) && (enc_pcn_cnt_loc != NULL))
       {
          /* encode "pcn_cnt" into o_uba */ 
          ncs_encode_16bit(&enc_pcn_cnt_loc, pcn_cnt);
          enc_pcn_cnt_loc = NULL;
       }

       file.info.get_next.i_dir_name   = (char*)&prof_path;
       file.info.get_next.i_file_name  = (char*)&cur_pwe;
       file.info.get_next.i_buf_size   = 64;
       file.info.get_next.io_next_file = (char*)&next_pwe;
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       if (retval != NCSCC_RC_SUCCESS)
       {
          if(next_pwe[0] == '\0')
             retval = NCSCC_RC_SUCCESS; /* End of the files in the directory */
          continue;
       }
    }  /* while on next_pwe */
    if((retval == NCSCC_RC_SUCCESS) && (enc_pwe_cnt_loc != NULL))
    {
       /* encode "pwe_cnt" into o_uba */ 
       ncs_encode_16bit(&enc_pwe_cnt_loc, pwe_cnt);
    }

    return retval;
}

static uns32 pssts_get_mib_list_per_pcn(NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_GET_MIBLIST_PER_PCN *get_mlist)
{
    NCS_OS_FILE file;
    NCS_UBAID   uba;
    uns32      retval, pwe_id = 0, pwe_cnt = 0, tbl_cnt = 0, tbl_id = 0, i = 0;
    char       cur_pwe[64], next_pwe[64]; /* Assuming the max length of each pwe/mib file name */
    char       cur_pcn[NCS_PSSTS_MAX_PATH_LEN], next_pcn[NCS_PSSTS_MAX_PATH_LEN];
    char       prof_path[NCS_PSSTS_MAX_PATH_LEN], pwe_path[NCS_PSSTS_MAX_PATH_LEN];
    char       pcn_path[NCS_PSSTS_MAX_PATH_LEN], cur_tbl[NCS_PSSTS_MAX_PATH_LEN];
    char       next_tbl[NCS_PSSTS_MAX_PATH_LEN], tbl_path[NCS_PSSTS_MAX_PATH_LEN];
    uns8       *enc_pwe_cnt_loc = NULL, *enc_tbl_cnt_loc = NULL, *p8 = NULL;
    uns8       *enc_pwe_id_loc = NULL;
    NCS_BOOL   valid_tbl_id = FALSE;

    memset(&uba, 0, sizeof(uba));
    memset(&next_pwe, '\0', sizeof(next_pwe));
    memset(&cur_pwe, '\0', sizeof(cur_pwe));
    memset(&next_pcn, '\0', sizeof(next_pcn));
    memset(&cur_pcn, '\0', sizeof(cur_pcn));
    file.info.dir_path.i_main_dir   = inst->root_dir;
    file.info.dir_path.i_sub_dir    = get_mlist->i_profile_name;
    file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
    file.info.dir_path.io_buffer    = (uns8*)&prof_path; /* Profile path */
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.get_next.i_dir_name   = (char*)&prof_path;
    file.info.get_next.i_file_name  = NULL;
    file.info.get_next.i_buf_size   = 64;
    file.info.get_next.io_next_file = (char*)&next_pwe;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
    if (retval != NCSCC_RC_SUCCESS)
       return NCSCC_RC_FAILURE;

    while((retval == NCSCC_RC_SUCCESS) && (next_pwe[0] != '\0'))
    {
       if((m_NCS_STRCMP((char *)&next_pwe, ".") == 0) ||
          (m_NCS_STRCMP((char *)&next_pwe, "..") == 0) ||
          (m_NCS_STRCMP((char *)&next_pwe, "ProDesc.txt") == 0))
       {
          strcpy((char *)&cur_pwe, (char *)&next_pwe);
          file.info.get_next.i_dir_name = (char*)&prof_path;
          file.info.get_next.i_file_name  = (char*)&cur_pwe;
          file.info.get_next.i_buf_size   = 64;
          file.info.get_next.io_next_file = (char*)&next_pwe;
          retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
          continue;
       }
       if(enc_pwe_cnt_loc == NULL)
       {
           ncs_enc_init_space(&uba);
           enc_pwe_cnt_loc = ncs_enc_reserve_space(&uba, 2);
           if(enc_pwe_cnt_loc == NULL)
           {
              retval = NCSCC_RC_FAILURE;
              m_MMGR_FREE_BUFR_LIST(uba.ub);
              get_mlist->o_usrbuf = NULL;
              return retval;
           }
           get_mlist->o_usrbuf = uba.start;
           ncs_enc_claim_space(&uba, 2);
       }
       strcpy((char *)&cur_pwe, (char *)&next_pwe);

       sscanf((char*)&cur_pwe, "%d", (int*)&pwe_id);
       enc_pwe_id_loc = NULL;

       file.info.dir_path.i_main_dir   = (char*)&prof_path;
       file.info.dir_path.i_sub_dir    = (char*)&cur_pwe;
       file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
       file.info.dir_path.io_buffer    = (char*)&pwe_path; /* PWE path */
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
       if (retval != NCSCC_RC_SUCCESS)
          continue;

       file.info.get_next.i_dir_name = (char*)&pwe_path;
       file.info.get_next.i_file_name  = NULL;
       file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
       file.info.get_next.io_next_file = (char*)&next_pcn;
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       if (retval != NCSCC_RC_SUCCESS)
          continue;

       while((retval == NCSCC_RC_SUCCESS) && (next_pcn[0] != '\0'))
       {
          if((m_NCS_STRCMP((char *)&next_pcn, ".") == 0) ||
             (m_NCS_STRCMP((char *)&next_pcn, "..") == 0))
          {
             strcpy((char *)&cur_pcn, (char *)&next_pcn);
             file.info.get_next.i_dir_name = (char*)&pwe_path;
             file.info.get_next.i_file_name  = (char*)&cur_pcn;
             file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
             file.info.get_next.io_next_file = (char*)&next_pcn;
             retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
             continue;
          }
          else if(m_NCS_STRCMP((char*)&next_pcn, get_mlist->i_pcn) == 0)
          {
             /* Now, get the table-list */
             strcpy((char *)&cur_pcn, (char *)&next_pcn);
             file.info.dir_path.i_main_dir   = (char*)&pwe_path;
             file.info.dir_path.i_sub_dir    = (char*)&cur_pcn;
             file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
             file.info.dir_path.io_buffer    = (char*)&pcn_path; /* PCN path */
             retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
             if (retval != NCSCC_RC_SUCCESS)
                return retval;

             tbl_cnt = 0;  /* Initialize counter */
             enc_tbl_cnt_loc = NULL; /* Initialize counter pointer */

             file.info.get_next.i_dir_name = (char*)&pcn_path;
             file.info.get_next.i_file_name  = NULL;
             file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
             file.info.get_next.io_next_file = (char*)&next_tbl;
             retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
             if (retval != NCSCC_RC_SUCCESS)
                return retval;

             while((retval == NCSCC_RC_SUCCESS) && (next_tbl[0] != '\0'))
             {
                valid_tbl_id = TRUE;
                m_NCS_CONS_PRINTF("\nnext_tbl : %s", next_tbl);
                if((m_NCS_STRCMP((char *)&next_tbl, ".") == 0) ||
                   (m_NCS_STRCMP((char *)&next_tbl, "..") == 0))
                {
                   valid_tbl_id = FALSE;
                }
                strcpy((char *)&cur_tbl, (char *)&next_tbl);

                for(i = 0; next_tbl[i] != '\0'; i++)
                {
                   if(!isdigit(next_tbl[i]))
                   {
                      valid_tbl_id = FALSE;
                      break;
                   }
                }
               
                if(!(atoi(next_tbl) < MIB_UD_TBL_ID_END))
                   valid_tbl_id = FALSE;

                if(FALSE == valid_tbl_id)
                {
                   strcpy((char *)&cur_tbl, (char *)&next_tbl);
                   file.info.get_next.i_dir_name = (char*)&pcn_path;
                   file.info.get_next.i_file_name  = (char*)&cur_tbl;
                   file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
                   file.info.get_next.io_next_file = (char*)&next_tbl;
                   retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
                   continue;
                }

                /* Get file path, and lookup the file size */
                file.info.dir_path.i_main_dir   = (char*)&pcn_path;
                file.info.dir_path.i_sub_dir    = (char*)&cur_tbl;
                file.info.dir_path.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
                file.info.dir_path.io_buffer    = (char*)&tbl_path; /* TBL path */
                retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
                if (retval != NCSCC_RC_SUCCESS)
                   return retval;

                file.info.size.i_file_name = tbl_path;
                file.info.size.o_file_size = 0;
                retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_SIZE);
                if (retval != NCSCC_RC_SUCCESS)
                   return retval;
                if(file.info.size.o_file_size == 0)
                {
                   continue; /* Show only non-empty MIB files */
                }

                if(enc_pwe_id_loc == NULL)
                {
                   p8 = ncs_enc_reserve_space(&uba, 2);
                   if(p8 == NULL)
                   {
                      retval = NCSCC_RC_FAILURE;
                      m_MMGR_FREE_BUFR_LIST(uba.ub);
                      get_mlist->o_usrbuf = NULL;
                      return retval;
                   }
                   enc_pwe_id_loc = p8; /* Store it until we are ready to add 
                                           tbl_cnt into the buffer. */
                   ncs_encode_16bit(&p8, (uns16)pwe_id);
                   ncs_enc_claim_space(&uba, 2);
                   ++pwe_cnt;

                   enc_tbl_cnt_loc = ncs_enc_reserve_space(&uba, 2);
                   if(enc_tbl_cnt_loc == NULL)
                   {
                      retval = NCSCC_RC_FAILURE;
                      m_MMGR_FREE_BUFR_LIST(uba.ub);
                      get_mlist->o_usrbuf = NULL;
                      return retval;
                   }
                   ncs_enc_claim_space(&uba, 2);
                }
                /* Got the table now. Now, encode "cur_tbl" into o_uba  */
                p8 = ncs_enc_reserve_space(&uba, 4);
                if(p8 == NULL)
                {
                   retval = NCSCC_RC_FAILURE;
                   m_MMGR_FREE_BUFR_LIST(uba.ub);
                   get_mlist->o_usrbuf = NULL;
                   return retval;
                }
                sscanf((char*)&cur_tbl, "%d", (int*)&tbl_id);
                ncs_encode_32bit(&p8, tbl_id);
                ncs_enc_claim_space(&uba, 4);
                ++ tbl_cnt; /* Increment the count */

                strcpy((char *)&cur_tbl, (char *)&next_tbl);
                file.info.get_next.i_dir_name = (char*)&pcn_path;
                file.info.get_next.i_file_name  = (char*)&cur_tbl;
                file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
                file.info.get_next.io_next_file = (char*)&next_tbl;
                retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
                if(retval != NCSCC_RC_SUCCESS)
                {
                   return NCSCC_RC_FAILURE;
                }
             }

             /* All tables of this PCN done. Encode "tbl_cnt" into o_uba, and move to next PWE. */ 
             if(enc_tbl_cnt_loc != NULL)
             {
                ncs_encode_16bit(&enc_tbl_cnt_loc, tbl_cnt);
             }
             break; /* Go to the next PWE for the same PCN */
          }
          strcpy((char *)&cur_pcn, (char *)&next_pcn);
          file.info.get_next.i_dir_name = (char*)&pwe_path;
          file.info.get_next.i_file_name  = (char*)&cur_pcn;
          file.info.get_next.i_buf_size   = NCS_PSSTS_MAX_PATH_LEN;
          file.info.get_next.io_next_file = (char*)&next_pcn;
          retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       }  /* while on next_pcn */

       file.info.get_next.i_dir_name   = (char*)&prof_path;
       file.info.get_next.i_file_name  = (char*)&cur_pwe;
       file.info.get_next.i_buf_size   = 64;
       file.info.get_next.io_next_file = (char*)&next_pwe;
       retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_GET_NEXT);
       if (retval != NCSCC_RC_SUCCESS)
          continue;
    }  /* while on next_pwe */
    if((retval == NCSCC_RC_SUCCESS) && (enc_pwe_cnt_loc != NULL))
    {
       /* encode "pwe_cnt" into o_uba */ 
       ncs_encode_16bit(&enc_pwe_cnt_loc, pwe_cnt);
    }
    get_mlist->o_usrbuf = uba.start;

    return retval;
}

static uns32 pssts_delete_temp_file (NCS_PSSTS_CB * inst)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN] = "\0";

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = (uns8 *)NCS_PSSTS_TEMP_FILE_NAME;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.remove.i_file_name = (char*)&buf;
    
    return m_NCS_FILE_OP (&file, NCS_OS_FILE_REMOVE);
}

static uns32 pssts_move_tbl_details_file(NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_MOVE_TBL_DETAILS_FILE *move_detailsfile)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN] = "\0";
    uns8       buf2[NCS_PSSTS_MAX_PATH_LEN] = "\0";

    retval = create_directories(inst->root_dir, move_detailsfile->i_profile_name,
                                move_detailsfile->i_pcn, move_detailsfile->i_pwe_id);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.dir_path.i_main_dir = inst->root_dir;
    file.info.dir_path.i_sub_dir  = (uns8 *)NCS_PSSTS_TEMP_FILE_NAME;
    file.info.dir_path.i_buf_size = sizeof(buf);
    file.info.dir_path.io_buffer  = buf;

    retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_DIR_PATH);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    retval = get_full_path_of_file(inst->root_dir,
        move_detailsfile->i_profile_name,
        move_detailsfile->i_pcn,
        move_detailsfile->i_pwe_id,
        move_detailsfile->i_tbl_id,
        buf2, sizeof(buf2));
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    m_NCS_STRCAT(buf2, "_tbl_details");

    printf("\nTemp file path: %s", buf);
    printf("\nfull_path: %s", buf2);

    file.info.copy.i_file_name = buf;
    file.info.copy.i_new_file_name = buf2;
    retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_COPY);
    if (retval != NCSCC_RC_SUCCESS)
        return NCSCC_RC_FAILURE;

    file.info.remove.i_file_name = (char*)&buf;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_REMOVE);
}

static uns32 pssts_delete_tbl_details_file (NCS_PSSTS_CB * inst, NCS_PSSTS_ARG_DELETE_FILE *delete_file)
{
    NCS_OS_FILE file;
    uns32      retval;
    uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    retval = get_full_path_of_file(inst->root_dir,
                   delete_file->i_profile_name,
                   delete_file->i_pcn,
                   delete_file->i_pwe,
                   delete_file->i_tbl_id,
                   buf, sizeof(buf));
    if (retval != NCSCC_RC_SUCCESS)
         return NCSCC_RC_FAILURE;

    m_NCS_STRCAT(buf, "_tbl_details");

    printf("\nTable details file path: %s", buf);

    file.info.remove.i_file_name = (char*)&buf;

    return m_NCS_FILE_OP (&file, NCS_OS_FILE_REMOVE);
}

uns32 ncs_pssts_api (NCS_PSSTS_ARG * parg)
{
    uns32  retval;
    NCS_PSSTS_CB * inst = ncshm_take_hdl(NCS_SERVICE_ID_PSSTS, parg->ncs_hdl);
    if (inst == NULL)
        return NCSCC_RC_FAILURE;

    switch (parg->i_op)
    {
    case NCS_PSSTS_OP_OPEN_FILE: /* Open a file for reading/writing */
        retval = pssts_open_file (inst, &parg->info.open_file);
        break;

    case NCS_PSSTS_OP_READ_FILE: /* Read in data from a particular offset */
        retval = pssts_read_file (inst, &parg->info.read_file);
        break;

    case NCS_PSSTS_OP_WRITE_FILE: /* Write the data to the file */
        retval = pssts_write_file (inst, &parg->info.write_file);
        break;

    case NCS_PSSTS_OP_CLOSE_FILE: /* Close the opened file */
        retval = pssts_close_file (inst, &parg->info.close_file);
        break;

    case NCS_PSSTS_OP_FILE_EXISTS: /* Check if the file exists */
        retval = pssts_file_exists (inst, &parg->info.file_exists);
        break;

    case NCS_PSSTS_OP_FILE_SIZE: /* Get the file size */
        retval = pssts_file_size (inst, &parg->info.file_size);
        break;

    case NCS_PSSTS_OP_CREATE_PROFILE: /* Create a new profile */
        retval = pssts_create_profile (inst, &parg->info.create_profile);
        break;

    case NCS_PSSTS_OP_MOVE_PROFILE: /* Rename an existing profile */
        retval = pssts_move_profile (inst, &parg->info.move_profile);
        break;

    case NCS_PSSTS_OP_COPY_PROFILE: /* Copy an existing profile */
        retval = pssts_copy_profile (inst, &parg->info.copy_profile);
        break;

    case NCS_PSSTS_OP_DELETE_PROFILE: /* Delete an existing profile */
        retval = pssts_delete_profile (inst, &parg->info.delete_profile);
        break;

    case NCS_PSSTS_OP_GET_PROFILE_DESC: /* Get the profile description */
        retval = pssts_get_profile_desc (inst, &parg->info.get_desc);
        break;

    case NCS_PSSTS_OP_SET_PROFILE_DESC: /* Set the description text for a profile */
        retval = pssts_set_profile_desc (inst, &parg->info.set_desc);
        break;

    case NCS_PSSTS_OP_GET_PSS_CONFIG: /* Get the Config information */
        retval = pssts_pss_config (inst, &parg->info.pss_config);
        break;

    case NCS_PSSTS_OP_SET_PSS_CONFIG: /* Set the Config information */
        retval = pssts_set_config (inst, &parg->info.set_config);
        break;

    case NCS_PSSTS_OP_OPEN_TEMP_FILE:
        retval = pssts_open_temp_file (inst, &parg->info.open_tfile);
        break;

    case NCS_PSSTS_OP_COPY_TEMP_FILE:
        retval = pssts_copy_temp_file (inst, &parg->info.copy_tfile);
        break;

    case NCS_PSSTS_OP_GET_NEXT_PROFILE:
        retval = pssts_get_next_profile (inst, &parg->info.get_next);
        break;

    case NCS_PSSTS_OP_PROFILE_EXISTS:
        retval = pssts_profile_exists (inst, &parg->info.profile_exists);
        break;

    case NCS_PSSTS_OP_PWE_EXISTS:
        retval = pssts_pwe_exists (inst, &parg->info.pwe_exists);
        break;

    case NCS_PSSTS_OP_PCN_EXISTS:
        retval = pssts_pcn_exists (inst, &parg->info.pcn_exists);
        break;

    case NCS_PSSTS_OP_FILE_DELETE:
        retval = pssts_delete_file(inst, &parg->info.delete_file);
        break;

    case NCS_PSSTS_OP_PCN_DELETE:
        retval = pssts_delete_pcn(inst, &parg->info.delete_pcn);
        break;

    case NCS_PSSTS_OP_GET_CLIENTS:
        retval = pssts_get_clients(inst, &parg->info.get_clients);
        break;

    case NCS_PSSTS_OP_GET_MIB_LIST_PER_PCN:
        retval = pssts_get_mib_list_per_pcn(inst, &parg->info.get_miblist_per_pcn);
        break;

    case NCS_PSSTS_OP_DELETE_TEMP_FILE:
        retval = pssts_delete_temp_file(inst);
        break;

    case NCS_PSSTS_OP_MOVE_TBL_DETAILS_FILE:
        retval = pssts_move_tbl_details_file(inst, &parg->info.move_detailsfile);
        break;

    case NCS_PSSTS_OP_DELETE_TBL_DETAILS_FILE:
        retval = pssts_delete_tbl_details_file(inst, &parg->info.delete_file);
        break;

    default:
        retval = NCSCC_RC_FAILURE;
    }

    ncshm_give_hdl(parg->ncs_hdl);
    return retval;
}

static uns32 ncspssts_lm_create(NCS_PSSTS_LM_CREATE * create)
{
    NCS_PSSTS_CB * inst;

    inst = (NCS_PSSTS_CB *)m_NCS_MEM_ALLOC(sizeof(NCS_PSSTS_CB),
                                   NCS_MEM_REGION_TRANSIENT,
                                   NCS_SERVICE_ID_PSSTS, 0);
    if (inst == NULL)
        return NCSCC_RC_FAILURE;

    strcpy(inst->root_dir, (char *)create->i_root_dir);
    strcpy(inst->current_profile, NCS_PSSTS_DEFAULT_PROFILE);
    inst->my_key = create->i_usr_key;

    inst->hmpool_id = create->i_hmpool_id;
    inst->hm_hdl = ncshm_create_hdl(inst->hmpool_id, NCS_SERVICE_ID_PSSTS,
                                   (NCSCONTEXT)inst);

    create->o_handle = inst->hm_hdl;

    return NCSCC_RC_SUCCESS;
}


static uns32 ncspssts_lm_destroy(NCS_PSSTS_LM_DESTROY * destroy)
{
    NCS_PSSTS_CB * inst = ncshm_take_hdl(NCS_SERVICE_ID_PSSTS, destroy->i_handle);

    if (inst == NULL)
        return NCSCC_RC_FAILURE;

    ncshm_give_hdl(destroy->i_handle);
    ncshm_destroy_hdl(NCS_SERVICE_ID_PSSTS, destroy->i_handle);
    m_NCS_MEM_FREE(inst, NCS_MEM_REGION_TRANSIENT,
                   NCS_SERVICE_ID_PSSTS, 0);

    return NCSCC_RC_SUCCESS;
}


uns32 ncspssts_lm(NCS_PSSTS_LM_ARG* arg )
{
    switch (arg->i_op)
    {
    case NCS_PSSTS_LM_OP_CREATE:
        return ncspssts_lm_create(&arg->info.create);

    case NCS_PSSTS_LM_OP_DESTROY:
        return ncspssts_lm_destroy(&arg->info.destroy);

    default:
        return NCSCC_RC_FAILURE;
    }
}

#endif
