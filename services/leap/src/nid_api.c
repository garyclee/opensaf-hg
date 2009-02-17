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
*                                                                            *
*  MODULE NAME:  nid_api.c                                                   *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module defines the API used by bladeinitd spawned services. From ser *
*  vices point of view, this API provides interface to communicate the       *
*  initialization status to bladeinitd. Bladeinitd spawned services are      *
*  HPM,RDF,XND,LHC,DRBD,NETWORKING,TIPC,MASV,PSSV,DTSV,PCAP,SCAP.           *
*                                                                            *
*****************************************************************************/

#include "nid_api.h"

/****************************************************************************
 * Name          : nid_notify                                               *
 *                                                                          *
 * Description   : Opens the FIFO to bladeinitd and write the service and   *
 *                 status code to FIFO.                                     *
 *                                                                          *
 * Arguments     : service  - input parameter providing service code        *
 *                 status   - input parameter providing status code         *
 *                 error    - output parameter to return error code if any  *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..                      *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32 nid_notify(char *service,uns32 status,uns32 *error)
{
   uns32 scode;
   uns8 msg[250];
   int32 fd = -1;
   uns32 retry=3;
   char strbuff[256];
                                                                                                         
   scode = status;
   
                                                       
   if((scode < 0)  ){
      if(error != NULL) *error = NID_INV_PARAM;
      return NCSCC_RC_FAILURE;
   }
                                                                                                         
   while(retry){
        if(nid_open_ipc(&fd,strbuff) != NCSCC_RC_SUCCESS){
          retry--;
        }else break;
   }
                                                                                                         
   if((fd < 0) && (retry == 0)){
     if(error != NULL) *error = NID_OFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                                                                                                         
   /************************************************************
   *    Prepare the message to be sent                         *
   ************************************************************/
   sprintf(msg,"%x:%s:%d",NID_MAGIC,service,scode);                                                                                                       
   /************************************************************
   *    Send the message                                       *
   ************************************************************/
   retry=3;
   while(retry){
        if(m_NCS_POSIX_WRITE(fd,msg,m_NCS_STRLEN(msg)) == m_NCS_STRLEN(msg)) break;
        else retry--;
   }
                                                                                                         
   if(retry == 0){
     if(error != NULL) *error = NID_WFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                 
   nid_close_ipc();
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : nis_notify                                               *
 *                                                                          *
 * Description   : Opens the FIFO to bladeinitd and write the service and   *
 *                 status code to FIFO.                                     *
 *                                                                          *
 * Arguments     : status   - input parameter providing status              *
 *                 error    - output parameter to return error code if any  *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..                      *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32 nis_notify(uns8 *status,uns32 *error)
{

   int32 fd = -1;
   uns32 retry=3;
   char strbuff[256];
                                                                                                         
   if( status == NULL ){ 
      if(error != NULL) *error = NID_INV_PARAM;
      return NCSCC_RC_FAILURE;
   }
                                                                                                         
   while(retry){
        if(nid_open_ipc(&fd,strbuff) != NCSCC_RC_SUCCESS){
          retry--;
        }else break;
   }
                                                                                                         
   if((fd < 0) && (retry == 0)){
     if(error != NULL) *error = NID_OFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                                                                                                         
   /************************************************************
   *    Send the message                                       *
   ************************************************************/
   retry=3;
   while(retry){
        if(m_NCS_POSIX_WRITE(fd,status,strlen(status)) == strlen(status)) break;
        else retry--;
   }
                                                                                                         
   if(retry == 0){
     if(error != NULL) *error = NID_WFIFO_ERR;
     return NCSCC_RC_FAILURE;
   }
                 
   nid_close_ipc();
   return NCSCC_RC_SUCCESS;
}







