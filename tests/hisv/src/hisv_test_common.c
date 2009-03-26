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
 *            Hewlett-Packard Company
 */

/*****************************************************************************

  DESCRIPTION:

  This file contains common HISv test functions.
  It includes
    void err_exit(int exitVal, char *fmt, ...)
    uns32 init_ncs_hisv(void)
    void hisv_cleanup(void)

******************************************************************************
*/

/* HISv Toolkit header file */
#include <hpl_api.h>
#include <hpl_msg.h>

#include <mds_papi.h>
#include <ncssysf_tmr.h>
#include <ncssysf_lck.h>
#include "../../../services/hisv/inc/hcd_mem.h"
#include "../../../services/hisv/inc/hcd_util.h"
#include "../../../services/hisv/inc/hisv_msg.h"
#include "../../../services/hisv/inc/ham_cb.h"
#include "../../../services/hisv/inc/hpl_cb.h"

#include "hisv_test.h"
#include <stdarg.h>


/*############################################################################
                            Global Variables
############################################################################*/
/* EVT Handle */
SaEvtHandleT          gl_evt_hdl = 0;

/*#############################################################################
                           End Of Declarations
###############################################################################*/

/****************************************************************************
 * Name          : err_exit
 *
 * Description   : This routine prints an error message to the console and
 *                 exits with the specified error code.
 *
 * Arguments     : int exitVal	-- exit code
 *		 		   fmt        	-- printf format string
 * 				   ...			-- varg exit value
 *
 * Return Values : void
 *                  
 *****************************************************************************/
void err_exit(int exitVal, char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   char msg[MAX_MSG_LEN];
   int maxMsgLen = sizeof(msg);
   memset(msg, 0, maxMsgLen);

    // Prepend "ERROR: " to every message
   char* mptr = msg;
   char error[] = "ERROR: ";
   int prependSize = strlen(error); 

    // Use strlen, since we've already zeroed out msg.
   strncpy(msg, error, prependSize);
   maxMsgLen -= prependSize;
   mptr += prependSize;
   int msglen = vsnprintf(mptr, maxMsgLen, fmt, ap);

    // Automatically add a new line too
   msg[msglen + prependSize] = '\n';
   printf(msg);
   fflush(stdout);
   va_end(ap);
   exit(exitVal);
}

/****************************************************************************
 * Name          : init_ncs_hisv
 * 
 * Description   : This routine initializes the hisv subsystem:
 *                 - Initializes the event subsystem to creae
 *                   a new channel and publish events to it,
 *                 - Start the client-side library to access the HPI
 *                   library routines.
 * 
 * Arguments     : None.
 * 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 * 
 * Notes         : Registers with the event-distribution service to propagate
 *                 events.
 *                 Intializes the client-side library for access to hpl routines.
 *****************************************************************************/
uns32 init_ncs_hisv(void)
{

   SaAisErrorT              rc;

   /* Initialize the event subsystem for communcation with HISv */
   printf("\tInitializing the event subsystem\n");
      // Variables related to event-system registration
   extern SaEvtHandleT		gl_evt_hdl;
   SaVersionT               ver;
   SaEvtCallbacksT          reg_callback_set; 
   ver.releaseCode = 'B';
   ver.majorVersion = 0x01;
   ver.minorVersion = 0x01;

     // Now perform the initialization
   rc = saEvtInitialize(&gl_evt_hdl, &reg_callback_set, &ver);
   if (rc != NCSCC_RC_SUCCESS) 
       err_exit(1, "Failed to register hisv events with evsv: ret code is %d (NCSCC_RC_FAILURE)\n", rc);

   /* Initialize the client-side library */
   printf("\tInitializing the client-side library\n");

   NCS_LIB_CREATE hisv_create_info;
   rc = hpl_initialize(&hisv_create_info);
   if (rc != NCSCC_RC_SUCCESS) 
       err_exit(1, "Failed to initialize the HPL library, ret code is %d (NCSCC_RC_FAILURE)\n", rc);


   /* Allow time for the client library callbacks to initialize */
   m_NCS_TASK_SLEEP(3000);

   return NCSCC_RC_SUCCESS;

}

/****************************************************************************
 * Name          : hisv_cleanup
 * 
 * Description   : Calls hpl_finalize to unregister the HPL from MDS and free
 *                 the associated control-block resources before destoying the
 *                 HPL control blok itself.
 *         
 *                 calls hpl_finalize()
 * 
 * Arguments     : None.
 * 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *                 (from the return value of hpl_finalize())
 * 
 * Notes         : Registers with the event-distribution service to propagate
 *                 events.
 *                 Intializes the client-side library for access to hpl routines.
 *****************************************************************************/
void hisv_cleanup(void)
{
   NCS_LIB_DESTROY destroy_info;
   printf("Unregistering HPI Private Library for this application\n");
   fflush(stdout);
   hpl_finalize(&destroy_info);
}

// vim: tabstop=4
