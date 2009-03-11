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

..............................................................................

  DESCRIPTION:  This file contains the utility routines to access the 
                statistics of resources (ie. from /proc file system in LINUX)

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************
*/

#include "srmsv.h"

/****************************************************************************
  Name          :  srmst_get_data_from_file
 
  Description   :  This routine gets the respective item from the said file.

  Arguments     :  in_file - File to parse
                   line_num - Line to parse
                   position - Position of the item
                   in_split_pattern - split pattern
                   result - Output result of an item
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_get_data_from_file(char  *in_file,
                               uns16 line_num,
                               uns16 position,
                               char  *in_split_pattern,
                               uns32 *o_result)
{
   char line_buffer[SRMST_MAX_BUF_SIZE];
   char item_buffer[SRMST_ITEM_BUF_SIZE];
  
   memset(line_buffer, 0, SRMST_MAX_BUF_SIZE);
   memset(item_buffer, 0, SRMST_ITEM_BUF_SIZE);

   /* get a line from the file */
   if (srmst_file_parser(in_file, line_buffer, line_num) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;  
    
   /* get the data from the line */
   if (srmst_str_parser(line_buffer, in_split_pattern, item_buffer, position)
       != NCSCC_RC_SUCCESS)
   {
      /* LOG appropriate message */
      return NCSCC_RC_FAILURE;
   }

   *o_result = atol(item_buffer);

   return NCSCC_RC_SUCCESS; 
}


/****************************************************************************
  Name          :  srmst_file_parser
 
  Description   :  This routine reads the said line (complete string) from
                   the file into out_string buffer.

  Arguments     :  in_file - Input file to parse
                   out_string - Output string
                   line - line number to read
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmst_file_parser(char *in_file, char *out_string, uns16 line)
{
   uns16 num;
   FILE  *fd;

   /* Open the file in read mode */
   if ((fd = fopen(in_file, "r")) == NULL)
   {
      /* LOG appropriate message */
      return NCSCC_RC_FAILURE;
   }

   for (num = 1; num<=line; num++)
   {
      memset(out_string, 0, SRMST_MAX_BUF_SIZE);
      if ((fgets(out_string, SRMST_MAX_BUF_SIZE, fd)) == NULL) 
      {
         /* LOG appropriate message */
         fclose(fd);
         return NCSCC_RC_FAILURE;
      }
   }

   /* Close the file */
   fclose(fd);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  srmst_str_parser
 
  Description   :  This routine gets the appropriate string from input string

  Arguments     :  in_string - Input string for parsing
                   in_split_pattern - Split pattern
                   out_string - Output string
                   num - String position
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32  srmst_str_parser(char *in_string,
                        char *in_split_pattern,
                        char *out_string,
                        uns16 num)
{
   uns16 i;
   char  *out_data;
   char  buffer[SRMST_MAX_BUF_SIZE];

   memset(&buffer, 0, SRMST_MAX_BUF_SIZE);   
   m_NCS_STRCPY(buffer, in_string);

   if ((out_data = strtok(buffer, in_split_pattern)) == NULL)
      return NCSCC_RC_FAILURE;

   for (i = 1; i<=num; i++)
   {
      out_data = strtok(NULL, in_split_pattern);
      if (out_data == NULL)
         return NCSCC_RC_FAILURE;           
   }

   memset(out_string, 0, SRMST_ITEM_BUF_SIZE);
   m_NCS_STRCPY(out_string, out_data);

   return NCSCC_RC_SUCCESS;
}





