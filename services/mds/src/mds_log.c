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
  DESCRIPTION:  MDS LOG APIs

******************************************************************************
*/

#include "mds_log.h"
#include "mds_dt2c.h" /* Include for arch-word definitions */


static char log_line_prefix[40];
char *lf=NULL;

uns32 mds_log_init(char *log_file_name, char *line_prefix);
static void log_mds(const char *str);




/*******************************************************************************
* Funtion Name   :    mds_log_init
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
#define MAX_MDS_FNAME_LEN  256
static char mds_log_fname[MAX_MDS_FNAME_LEN];

uns32 mds_log_init (char *log_file_name, char *line_prefix)
{
    FILE    *fh;
    time_t   tod;
    char     asc_tod[40];
    uns32    dummy_rc,process_id=0;

    process_id=(uns32)getpid();

    /* Copy the log-line-prefix */
    if (m_NCS_OS_STRLEN(line_prefix)+1 > sizeof(log_line_prefix))
    {
        m_NCS_SYSLOG(NCS_LOG_INFO,"MDS_LOG:Passed line prefix len is greater than the line prefix\n");
    }
    
    m_NCS_OS_STRNCPY(log_line_prefix, line_prefix, sizeof(log_line_prefix) - 1);
    log_line_prefix[sizeof(log_line_prefix) - 1] = 0;/* Terminate string */

    if (lf != NULL)
        return NCSCC_RC_FAILURE;

    if (m_NCS_OS_STRLEN(log_file_name) >= MAX_MDS_FNAME_LEN)
        return NCSCC_RC_FAILURE;

    m_NCS_OS_STRCPY(mds_log_fname,log_file_name);
    
    lf = mds_log_fname;

    if ((fh = sysf_fopen(lf, "a+")) != NULL)
    {
    asc_tod[0] = '\0';
    m_NCS_OS_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod);
    sysf_fprintf(fh,"NCSMDS|%s|%s|BEGIN MDS LOGGING| PROCESS_ID=%d|ARCH=%d|64bit=%d\n", log_line_prefix, asc_tod, process_id, MDS_ARCH_TYPE,MDS_WORD_SIZE_TYPE);
    sysf_fclose(fh);
    }
    else
        m_NCS_SYSLOG(NCS_LOG_ERR,"MDS_LOG:0");
        dummy_rc = NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}

/*******************************************************************************
* Funtion Name   :    log_mds_critical
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
void log_mds_critical(char *fmt, ...)
{
    char str[200];
    char *write_ptr = str;
    va_list ap;

    write_ptr += sysf_sprintf(write_ptr, "CRITICAL    |");
    va_start(ap, fmt);
    vsprintf(write_ptr, fmt, ap);
    va_end(ap);
    log_mds(str);
    /* Print Critical Logs on console */
    m_NCS_CONS_PRINTF("MDS:%s\n", str);

}

/*******************************************************************************
* Funtion Name   :    log_mds_err
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
void log_mds_err(char *fmt, ...)
{
    char str[200];
    char *write_ptr = str;
    va_list ap;

    write_ptr += sysf_sprintf(write_ptr, "ERR    |");
    va_start(ap, fmt);
    vsprintf(write_ptr, fmt, ap);
    va_end(ap);
    log_mds(str);
    /* Print Error Logs on console */
    m_NCS_CONS_PRINTF("MDS:%s\n", str);
}
/*******************************************************************************
* Funtion Name   :    log_mds_notify
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
void log_mds_notify(char *fmt, ...)
{
    char str[200];
    char *write_ptr = str;
    va_list ap;

    write_ptr += sysf_sprintf(write_ptr, "NOTIFY |");
    va_start(ap, fmt);
    vsprintf(write_ptr, fmt, ap);
    va_end(ap);
    log_mds(str);
}

/*******************************************************************************
* Funtion Name   :    log_mds_info
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
void log_mds_info(char *fmt, ...)
{
    char str[200];
    char *write_ptr = str;
    va_list ap;

    write_ptr += sysf_sprintf(write_ptr, "INFO   |");
    va_start(ap, fmt);
    vsprintf(write_ptr, fmt, ap);
    va_end(ap);
    log_mds(str);
}

/*******************************************************************************
* Funtion Name   :    log_mds_dbg
*
* Purpose        :
*
* Return Value   :    NCSCC_RC_SUCCESS
*                     NCSCC_RC_FAILURE
*
*******************************************************************************/
void log_mds_dbg(char *fmt, ...)
{ 
    char str[200];
    char *write_ptr = str;
    va_list ap;

    write_ptr += sysf_sprintf(write_ptr, "DBG    |");
    va_start(ap, fmt);
    vsprintf(write_ptr, fmt, ap);
    va_end(ap);
    log_mds(str);
}
/*******************************************************************************
* Funtion Name   :    log_mds
*
* Purpose        :
*
* Return Value   :    None
*
*******************************************************************************/
static void log_mds(const char *str)
{
  FILE *fp;
  time_t tod;
  char asc_tod[40];

  if (lf != NULL && ((fp = sysf_fopen(lf, "a+")) != NULL))
  {
    asc_tod[0] = '\0';
    m_NCS_OS_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod);
    sysf_fprintf(fp,"NCSMDS|%s|%s|%s\n", log_line_prefix, asc_tod, str);
    sysf_fclose(fp);
  }
  else
  {
     printf(" Unable to log into log file\n");
  }
}
