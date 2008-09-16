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
 */

/*****************************************************************************
..............................................................................



..............................................................................

  DESCRIPTION:

  This file contains the routines for the AvSv application. It is a simple 
  counter that counts in active state.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  avsv_app_init.....................Creates & starts AvSv application task.
  avsv_app_process..................Entry point for the AvSv application task.


******************************************************************************
*/


/* Common header files */
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"

#include "saAis.h"
#include "saAmf.h"


/*############################################################################
                            Global Variables
############################################################################*/

/* AvSv application task handle */
NCSCONTEXT gl_app_task_hdl = 0;

/* Counter Value */
uns32 gl_count1 = 0;
uns32 gl_count2 = 0;

/*############################################################################
                            Macro Definitions
############################################################################*/

/* AvSv application task related parameters  */
#define AVSV_APP_TASK_PRIORITY   (5)
#define AVSV_APP_STACKSIZE       NCS_STACKSIZE_HUGE

/* Time duration (in ms) after which counter value is incremented */
#define AVSV_APP_COUNTER_UPDATE_TIME (1000)

/*############################################################################
                           Extern Decalarations
############################################################################*/

/* Top level routine to start application task */
extern uns32 avsv_app_init(void);

extern SaAmfHAStateT gl_ha_state;

extern void avsv_ckpt_data_write(uns32 *,uns32 *);

/*############################################################################
                       Static Function Decalarations
############################################################################*/

/* Entry level routine application task */
static void avsv_app_process(void);


/****************************************************************************
  Name          : avsv_app_init
 
  Description   : This routine creates & starts the AvSv application task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avsv_app_init(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Create AvSv application task */
   rc = m_NCS_TASK_CREATE ( (NCS_OS_CB)avsv_app_process, (void *)0, "AVSV-APP",
                            AVSV_APP_TASK_PRIORITY, AVSV_APP_STACKSIZE, 
                            &gl_app_task_hdl );
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   /* Start the task */
   rc = m_NCS_TASK_START (gl_app_task_hdl);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      goto err;
   }

   m_NCS_CONS_PRINTF("\n\n AVSV-APP TASK CREATION SUCCESS !!! \n\n");

   return rc;

err:
   /* destroy the task */
   if (gl_app_task_hdl) m_NCS_TASK_RELEASE(gl_app_task_hdl);
   m_NCS_CONS_PRINTF("\n\n AVSV-APP TASK CREATION FAILED !!! \n\n");

   return rc;
}


/****************************************************************************
  Name          : avsv_app_process
 
  Description   : This routine is an entry point for the AvSv application task.
                  It counts in active state.
 
  Arguments     : None.
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void avsv_app_process (void)
{
   while (1)
   {
      if ( SA_AMF_HA_ACTIVE == gl_ha_state )
      {
         gl_count1++;
	 gl_count2 = gl_count1 +500;
       //  m_NCS_CONS_PRINTF("\n\n COUNTER VALUE: %d \n\n", gl_count1);

         /* Update the standby with this value */
         avsv_ckpt_data_write(&gl_count1,&gl_count2);
      }

      m_NCS_TASK_SLEEP(AVSV_APP_COUNTER_UPDATE_TIME);
   }

   return;
}

