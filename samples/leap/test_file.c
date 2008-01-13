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

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"
#include "leaptest.h"

int
lt_file_ops(int argc, char **argv)
{
    NCS_OS_FILE  file_handle;
    NCS_OS_FILE  file_handle2;
    uns32       retval;

    char *      root_dir = "tmp_dir123";
    char *      subdir1  = "subdir1";
    char *      subdir2  = "subdir2";
    char *      subdir3  = "subdir3";

    char *      file1    = "file1.txt";
    char *      file2    = "file2.txt";
    char *      file3    = "file3.txt";
    char *      file4    = "file4.txt";

    char        data[512];
    char        buf1[512];
    char        buf2[1024];
    char        buf3[512];
    char        buf4[512];
    char        buf5[512];

    /* TEST CASES */
    /* Create the root directory */
    file_handle.info.create_dir.i_dir_name = (uns8 *)root_dir;
    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_CREATE_DIR);
    m_NCS_CONS_PRINTF("Test1: ");
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to create root directory %s\n", root_dir);
    else
        m_NCS_CONS_PRINTF("Created root directory successfully\n");

    /* Create a new file, write some data and close it */
    file_handle.info.dir_path.i_buf_size = sizeof(buf1);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf1;
    file_handle.info.dir_path.i_main_dir = (uns8 *) root_dir;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) file1;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get dir path for dir %s and file %s\n", root_dir, file1);
    else
        m_NCS_CONS_PRINTF("Dir path for file is %s\n", buf1);

    file_handle2.info.create.i_file_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE(&file_handle2, NCS_OS_FILE_CREATE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to create file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Created file %s\n", buf1);

    file_handle.info.write.i_file_handle = file_handle2.info.create.o_file_handle;
    file_handle.info.write.i_buf_size    = sizeof(data);
    file_handle.info.write.i_buffer      = (uns8 *) data;
    memset(data, 'a', sizeof(data) - 1);
    data[511] = '\0';
    m_NCS_OS_SNPRINTF(data, sizeof(data) - 1, "Test string, testing file io operations\n");

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_WRITE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to write to file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Wrote to file %s\n", buf1);

    file_handle.info.close.i_file_handle = file_handle2.info.create.o_file_handle;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_CLOSE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to close file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Closed file %s\n", buf1);


    /* Get the size of the existing file */
    file_handle.info.size.i_file_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE (&file_handle, NCS_OS_FILE_SIZE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get size of file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Size of file %s is %d\n", buf1,file_handle.info.size.o_file_size);

    /* Open the existing file and read data from it and close it */
    file_handle2.info.open.i_file_name = (uns8 *) buf1;
    file_handle2.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

    retval = m_NCS_OS_FILE(&file_handle2, NCS_OS_FILE_OPEN);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to open file %s for reading\n", buf1);
    else
        m_NCS_CONS_PRINTF("Opened file %s for reading\n", buf1);

    file_handle.info.read.i_file_handle = file_handle2.info.open.o_file_handle;
    file_handle.info.read.i_buffer      = (uns8 *)buf2;
    file_handle.info.read.i_buf_size    = sizeof(buf2);

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_READ);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Error reading from file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Read %d bytes from file %s\n",
                            file_handle.info.read.o_bytes_read, buf1);

    file_handle.info.close.i_file_handle = file_handle2.info.open.o_file_handle;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_CLOSE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to close file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Closed file %s\n", buf1);

    /* Copy the existing file */
    file_handle.info.dir_path.i_main_dir = (uns8 *) root_dir;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) file2;
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf3;
    file_handle.info.dir_path.i_buf_size = sizeof(buf3);

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path of file %s\n", file2);
    else
        m_NCS_CONS_PRINTF("The 2nd file name is %s\n", buf3);

    file_handle.info.copy.i_file_name = (uns8 *) buf1;
    file_handle.info.copy.i_new_file_name = (uns8 *) buf3;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_COPY);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to copy file %s to %s\n", buf1, buf3);
    else
        m_NCS_CONS_PRINTF("Copied file %s to %s\n", buf1, buf3);


    /* Rename an existing file */

    file_handle.info.dir_path.i_buf_size = sizeof(buf4);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf4;
    file_handle.info.dir_path.i_main_dir = (uns8 *) root_dir;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) file3;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path for file %s\n", file3);
    else
        m_NCS_CONS_PRINTF("Full path is %s\n", buf4);

    file_handle.info.rename.i_file_name = (uns8 *) buf3;
    file_handle.info.rename.i_new_file_name = (uns8 *) buf4;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_RENAME);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to rename file %s\n", file3);
    else
        m_NCS_CONS_PRINTF("Renamed file %s to %s\n", buf3, buf4);

    /* Check if the file exists */

    file_handle.info.file_exists.i_file_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE (&file_handle, NCS_OS_FILE_EXISTS);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get file info %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("File %s is present?: %d\n",
            buf1, file_handle.info.file_exists.o_file_exists);

    /* Delete an existing file */

    file_handle.info.remove.i_file_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_REMOVE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to remove file %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Removed file %s\n", buf1);

    /* Check again if the file exists */

    file_handle.info.file_exists.i_file_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE (&file_handle, NCS_OS_FILE_EXISTS);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get file info %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("File %s is present?: %d\n",
            buf1, file_handle.info.file_exists.o_file_exists);

    /* Create a new directory */

    file_handle.info.dir_path.i_buf_size = sizeof(buf1);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf1;
    file_handle.info.dir_path.i_main_dir = (uns8 *) root_dir;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) subdir1;

    retval = m_NCS_OS_FILE (&file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path %s\n", subdir1);
    else
        m_NCS_CONS_PRINTF("Full path is %s\n", buf1);

    file_handle.info.create_dir.i_dir_name = (uns8 *)buf1;

    retval = m_NCS_OS_FILE (&file_handle, NCS_OS_FILE_CREATE_DIR);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to create directory %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Created directory %s\n", buf1);

    /* Now create a file inside this directory */

    file_handle.info.dir_path.i_buf_size = sizeof(buf3);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf3;
    file_handle.info.dir_path.i_main_dir = (uns8 *) buf1;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) file4;

    retval = m_NCS_OS_FILE( &file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path %s\n", file4);
    else
        m_NCS_CONS_PRINTF("Full path is %s\n", buf3);

    file_handle2.info.create.i_file_name = (uns8 *) buf3;

    retval = m_NCS_OS_FILE(&file_handle2, NCS_OS_FILE_CREATE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to create file %s\n", buf3);
    else
        m_NCS_CONS_PRINTF("Created file %s\n", buf3);

    file_handle.info.write.i_file_handle = file_handle2.info.create.o_file_handle;
    file_handle.info.write.i_buf_size    = sizeof(data);
    file_handle.info.write.i_buffer      = (uns8 *) data;
    memset(data, 'a', sizeof(data) - 1);
    data[511] = '\0';
    m_NCS_OS_SNPRINTF(data, sizeof(data) - 1, "Test string, testing file io operations\n");

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_WRITE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to write to file %s\n", buf3);
    else
        m_NCS_CONS_PRINTF("Wrote to file %s\n", buf3);

    file_handle.info.close.i_file_handle = file_handle2.info.create.o_file_handle;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_CLOSE);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to close file %s\n", buf3);
    else
        m_NCS_CONS_PRINTF("Closed file %s\n", buf3);

    /* Create a new directory within the first subdirectory */

    file_handle.info.dir_path.i_buf_size = sizeof(buf5);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf5;
    file_handle.info.dir_path.i_main_dir = (uns8 *) buf1;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) subdir3;

    retval = m_NCS_OS_FILE( &file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path %s\n", subdir3);
    else
        m_NCS_CONS_PRINTF("Full path is %s\n", buf5);

    file_handle.info.create_dir.i_dir_name = (uns8 *) buf5;

    retval = m_NCS_OS_FILE( &file_handle, NCS_OS_FILE_CREATE_DIR);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get create dir %s\n", buf5);
    else
        m_NCS_CONS_PRINTF("Created dir %s\n", buf5);


    /* Copy a directory */

    file_handle.info.dir_path.i_buf_size = sizeof(buf4);
    file_handle.info.dir_path.io_buffer  = (uns8 *) buf4;
    file_handle.info.dir_path.i_main_dir = (uns8 *) root_dir;
    file_handle.info.dir_path.i_sub_dir  = (uns8 *) subdir2;

    retval = m_NCS_OS_FILE( &file_handle, NCS_OS_FILE_DIR_PATH);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to get full path %s\n", subdir2);
    else
        m_NCS_CONS_PRINTF("Full path is %s\n", buf4);

    file_handle.info.copy_dir.i_dir_name = (uns8 *) buf1;
    file_handle.info.copy_dir.i_new_dir_name = (uns8 *) buf4;

    retval = m_NCS_OS_FILE( &file_handle, NCS_OS_FILE_COPY_DIR);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to copy dir %s to %s\n", buf1, buf4);
    else
        m_NCS_CONS_PRINTF("Copied dir %s to %s\n", buf1, buf4);


    /* Now list all the directories under the root directory */
    {
        file_handle.info.get_next.i_dir_name = (uns8 *)root_dir;
        file_handle.info.get_next.i_file_name = NULL;
        file_handle.info.get_next.i_buf_size = sizeof(buf5);
        file_handle.info.get_next.io_next_file = (uns8 *)buf5;

        do
        {
            retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_GET_NEXT);
            if(retval != NCSCC_RC_SUCCESS)
            {
                m_NCS_CONS_PRINTF("Unable to get the next file\n");
                break;
            }

            m_NCS_CONS_PRINTF("Next file: %s\n", buf5);
            m_NCS_OS_STRCPY(buf4, buf5);
            file_handle.info.get_next.i_file_name = (uns8 *) buf4;
        } while(buf5[0] != '\0');
    }
    /* Delete a directory */
    /* First check that the directory is present */
    file_handle.info.dir_exists.i_dir_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DIR_EXISTS);
    if (TRUE == file_handle.info.dir_exists.o_exists)
        m_NCS_CONS_PRINTF("The directory %s exists\n", buf1);
    else
        m_NCS_CONS_PRINTF("The directory %s does not exist\n", buf1);


    file_handle.info.delete_dir.i_dir_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DELETE_DIR);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to delete dir %s\n", buf1);
    else
        m_NCS_CONS_PRINTF("Deleted dir %s\n", buf1);


    file_handle.info.dir_exists.i_dir_name = (uns8 *) buf1;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DIR_EXISTS);
    if (TRUE == file_handle.info.dir_exists.o_exists)
        m_NCS_CONS_PRINTF("The directory %s exists\n", buf1);
    else
        m_NCS_CONS_PRINTF("The directory %s does not exist\n", buf1);

    /* Delete the root directory */
    file_handle.info.delete_dir.i_dir_name = (uns8 *) root_dir;

    retval = m_NCS_OS_FILE(&file_handle, NCS_OS_FILE_DELETE_DIR);
    if (NCSCC_RC_SUCCESS != retval)
        m_NCS_CONS_PRINTF("Unable to delete dir %s\n", root_dir);
    else
        m_NCS_CONS_PRINTF("Deleted dir %s\n", root_dir);


    return NCSCC_RC_SUCCESS;
}
