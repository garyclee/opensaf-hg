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

/*
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_lib.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_main_papi.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncssysfpool.h>
#include <opensaf/ncsencdec_pub.h>
#include "mbcsv_demo_app.h"

#define CS_FAKE_NUM_ELEM     1
#define WS_FAKE_NUM_ELEM     1

#define CS_MAX_NUM_ELM       10
#define WS_MAX_NUM_ELM       10

#define FAKE_NUM_SVCS        1
#define FAKE_NUM_CKPTS       1

static uns32 mbcsv_initialize_svc(SS_SVC_ID   svc_id,
                                  uns16 version,
                                  uns32      *mbcsv_hdl);

static uns32 mbcsv_finalize_svc(uns32   mbcsv_hdl);

static uns32 mbcsv_open_ckpt(uns32    mbcsv_hdl,
                             NCS_MBCSV_CLIENT_HDL client_hdl,
                             uns32    pwe_hdl,
                             uns32    *ckpt_hdl);

static uns32 mbcsv_close_ckpt(uns32    mbcsv_hdl,
                              uns32    ckpt_hdl);

static uns32 mbcsv_fake_send_data(uns32    mbcsv_hdl,
                                  uns32    ckpt_hdl,
                                  uns32    action,
                                  uns32    reo_hdl,
                                  uns32    send_type);

static uns32 mbcsv_fake_send_data_req(uns32      mbcsv_hdl,
                                 uns32      ckpt_hdl);

static uns32 mbcsv_get_sel_obj(uns32                 mbcsv_hdl,
                               SaSelectionObjectT    *sel_obj);

static uns32 mbcsv_set_obj_svc(uns32  mbcsv_hdl, uns32 ckpt_hdl,
                               uns32  obj, uns32 val);

static uns32 mbcsv_fake_dispatch(uns32    mbcsv_hdl,
                                 uns32     flag);

static uns32 fake_encode_elem(uns32 num_elm, uns32 elem_num, 
                              NCS_UBAID* uba, 
                              FAKE_SS_CKPT  *sscb_ptr);

static uns32 fake_decode_elem(uns32 num_elm, NCS_UBAID* uba, 
                              FAKE_SS_CKPT  *sscb_ptr);

static uns32 fake_data_decode(NCS_UBAID* uba, uns32 *data_send);

static void fake_init_config(uns32 role, uns8 rcode);

static void mbcsv_dummy_testapp(void);

static uns32  mbcsv_fake_ss_cb(NCS_MBCSV_CB_ARG *arg);
static uns32  mbcsv_fake_process_enc_cb(NCS_MBCSV_CB_ARG *arg);
static uns32  mbcsv_fake_process_dec_cb(NCS_MBCSV_CB_ARG *arg, uns32 *wsfail);
static uns32  mbcsv_fake_process_peer_cb(NCS_MBCSV_CB_ARG *arg);
static void dispatch_all_blk(int *i);
static void mbcsv_start_dispatch_thread(int i);
static uns32  mbcsv_fake_process_notify(NCS_MBCSV_CB_ARG *arg);
static uns32  mbcsv_fake_process_err_ind(NCS_MBCSV_CB_ARG *arg);
static void create_fake_data(NCS_BOOL send_data);

static uns32 mds_demo_create_named_vdest(char * name, uns32 *pwe_hdl);

FAKE_SS_CB  ss_cb[FAKE_NUM_SVCS];
char *task_name[] = {"FAKE_TASK1", "FAKE_TASK2", "FAKE_TASK3"};
const uns32  role_arr[] = {3,1,2};
uns32   my_role;
NCS_BOOL    data_created;
uns32      peers_found;
MBCSV_FAKE_SS_CONFIG ss_config[9];

static void fake_init_config(uns32 role, uns8 rcode)
{

   ss_config[0].svc_id  = 102;
   ss_config[0].vdest_name = "MBCSV_FAKE_SS1";
   ss_config[0].role  = role;
   ss_config[0].version = DEMO_MBCSV_VERSION_ONE;
 

   ss_config[1].svc_id  = 102;
   ss_config[1].vdest_name = "MBCSV_FAKE_SS2";
   ss_config[1].role  = role;
   ss_config[1].version = DEMO_MBCSV_VERSION_ONE;

   ss_config[2].svc_id  = 102;
   ss_config[2].vdest_name = "MBCSV_FAKE_SS3";
   ss_config[2].role = role;
   ss_config[2].version = DEMO_MBCSV_VERSION_ONE;

   ss_config[3].svc_id  = 103;
   ss_config[3].vdest_name = "MBCSV_FAKE_SS1";
   ss_config[3].role  = role;
   ss_config[3].version = DEMO_MBCSV_VERSION_TWO;

   ss_config[4].svc_id  = 103;
   ss_config[4].vdest_name = "MBCSV_FAKE_SS2";
   ss_config[4].role  = role;
   ss_config[4].version = DEMO_MBCSV_VERSION_TWO;

   ss_config[5].svc_id  = 103;
   ss_config[5].vdest_name = "MBCSV_FAKE_SS3";
   ss_config[5].role  = role;
   ss_config[5].version = DEMO_MBCSV_VERSION_TWO;

   ss_config[6].svc_id  = 104;
   ss_config[6].vdest_name = "MBCSV_FAKE_SS1";
   ss_config[6].role  = role;
   ss_config[6].version = DEMO_MBCSV_VERSION_TWO;

   ss_config[7].svc_id  = 104;
   ss_config[7].vdest_name = "MBCSV_FAKE_SS2";
   ss_config[7].role  = role;
   ss_config[7].version = DEMO_MBCSV_VERSION_TWO;

   ss_config[8].svc_id  = 104;
   ss_config[8].vdest_name = "MBCSV_FAKE_SS3";
   ss_config[8].role  = role;
   ss_config[8].version = DEMO_MBCSV_VERSION_TWO;

}

/*****************************************************************************

  PROCEDURE NAME:    create_fake_ss

  DESCRIPTION:       Function used for creating the fake ss which will register
                     with the MBCSV and do the checkpointing operation as per
                     the role assigned to it.

*****************************************************************************/
void create_fake_ss(void * arg)
{
   MBCSV_FAKE_SS_CREATE_PARMS *c_parm = (MBCSV_FAKE_SS_CREATE_PARMS *)arg;
   uns32       i=0, j=0;

   fake_init_config(c_parm->role, c_parm->svc_id);
   my_role = c_parm->role;

   data_created = FALSE;
   peers_found  = 0;

   if (my_role == SA_AMF_HA_STANDBY)
      m_NCS_TASK_SLEEP(500);

   for (i=0; i < FAKE_NUM_SVCS; i++)
   {
      ss_cb[i].my_version = ss_config[i*3].version;

      if (NCSCC_RC_SUCCESS != mbcsv_initialize_svc(ss_config[i*3].svc_id, 
                                                   ss_cb[i].my_version,
                                                   &ss_cb[i].mbcsv_hdl))
      {
         printf(" MBCSV Initialization failed !!.\n");
         return;
      }

      ss_cb[i].svc_id = ss_config[i*3].svc_id;

      /*
       * Start MBCSv thread for each demo application services.
       */
      mbcsv_start_dispatch_thread(i);

      m_NCS_LOCK_INIT(&ss_cb[i].fake_ss_lock);

      for (j=0; j < FAKE_NUM_CKPTS; j++)
      {
         memset(ss_cb[i].my_ckpt[j].my_data, '\0',
                      10 * sizeof(FAKE_SS_DATA_STRUCT));

         if (NCSCC_RC_SUCCESS != mds_demo_create_named_vdest(ss_config[(i*3)+j].vdest_name,
                                                             &ss_cb[i].my_ckpt[j].pwe_hdl))
         {
            printf(" VDEST creation failed !!.\n");
            return;
         }
         
         if (NCSCC_RC_SUCCESS != mbcsv_open_ckpt(ss_cb[i].mbcsv_hdl,
                                                 NCS_PTR_TO_UNS64_CAST(&ss_cb[i].my_ckpt[j]),
                                                 ss_cb[i].my_ckpt[j].pwe_hdl,
                                                 &ss_cb[i].my_ckpt[j].ckpt_hdl))
         {
            printf(" MBCSV Open ckpt failed !!.\n");
            return;
         }
         
         ss_cb[i].my_ckpt[j].ha_state = ss_config[(i*3)+j].role;
         ss_cb[i].my_ckpt[j].cold_sync_elem_sent = 0;
         ss_cb[i].my_ckpt[j].warm_sync_elem_sent = 0;
         ss_cb[i].my_ckpt[j].cold_sync_cmplt_sent = FALSE;
         ss_cb[i].my_ckpt[j].cb_ptr              = &ss_cb[i];
         
         if (NCSCC_RC_SUCCESS != mbcsv_set_ckpt_role(ss_cb[i].mbcsv_hdl,
                                                     ss_cb[i].my_ckpt[j].ckpt_hdl,
                                                     ss_config[(i*3)+j].role))
         {
            printf(" MBCSV Role change failed. !!.\n");
            return;
         }
         
         if (NCSCC_RC_SUCCESS != mbcsv_get_sel_obj(ss_cb[i].mbcsv_hdl,
                               &ss_cb[i].sel_obj))
         {
            printf(" MBCSV get selection object failed !!.\n");
            return;
         }
         
         if (NCSCC_RC_SUCCESS != mbcsv_set_obj_svc(ss_cb[i].mbcsv_hdl,
            ss_cb[i].my_ckpt[j].ckpt_hdl, NCS_MBCSV_OBJ_TMR_WSYNC, 1000))
         {
            printf(" MBCSV set object (NCS_MBCSV_OBJ_TMR_WSYNC) failed !!.\n");
            return;
         }

         if (NCSCC_RC_SUCCESS != mbcsv_set_obj_svc(ss_cb[i].mbcsv_hdl,
            ss_cb[i].my_ckpt[j].ckpt_hdl, NCS_MBCSV_OBJ_WARM_SYNC_ON_OFF, FALSE))
         {
            printf(" MBCSV set object failed !!.\n");
         }

      }
   }

   create_fake_data(FALSE);
   mbcsv_dummy_testapp();

   m_NCS_TASK_SLEEP(1000); 
/*
   printf(" \n...CHANGE ROLE OF ACTIVE AND STANDBY...REPEAT TEST....!!.\n"); 

   for (i=0; i<FAKE_NUM_SVCS; i++)
   {
      for (j=0; j<FAKE_NUM_CKPTS; j++)
      {
         if (NCSCC_RC_SUCCESS != mbcsv_set_obj_svc(ss_cb[i].mbcsv_hdl,
            ss_cb[i].my_ckpt[j].ckpt_hdl, NCS_MBCSV_OBJ_WARM_SYNC_ON_OFF, FALSE))
         {
            printf(" MBCSV set object failed !!.\n");
         }

         if (ss_config[(i*3)+j].role == SA_AMF_HA_ACTIVE)
         {
         if (NCSCC_RC_SUCCESS != mbcsv_set_ckpt_role(ss_cb[i].mbcsv_hdl,
                                                     ss_cb[i].my_ckpt[j].ckpt_hdl,
                                                     SA_AMF_HA_QUIESCED))
         {
            printf(" MBCSV Role change failed. !!.\n");
            return;
         }

         m_NCS_TASK_SLEEP(100); 

         if (NCSCC_RC_SUCCESS != mbcsv_set_ckpt_role(ss_cb[i].mbcsv_hdl,
                                                     ss_cb[i].my_ckpt[j].ckpt_hdl,
                                                     SA_AMF_HA_STANDBY))
         {
            printf(" MBCSV Role change failed. !!.\n");
            return;
         }
         my_role = SA_AMF_HA_STANDBY;
         }
         else if (ss_config[(i*3)+j].role == SA_AMF_HA_STANDBY)
         {
         m_NCS_TASK_SLEEP(200); 
         if (NCSCC_RC_SUCCESS != mbcsv_set_ckpt_role(ss_cb[i].mbcsv_hdl,
                                                     ss_cb[i].my_ckpt[j].ckpt_hdl,
                                                     SA_AMF_HA_ACTIVE))
         {
            printf(" MBCSV Role change failed. !!.\n");
            return;
         }
         my_role = SA_AMF_HA_ACTIVE;
         }
      }
   }

   mbcsv_dummy_testapp();
*/
   m_NCS_TASK_SLEEP(5000); 

   /*
    * We are done with fun.
    * Cleanup everything.
    */
   for (i=0; i<FAKE_NUM_SVCS; i++)
   {
      for (j=0; j<FAKE_NUM_CKPTS; j++)
      {
         mbcsv_close_ckpt(ss_cb[i].mbcsv_hdl, ss_cb[i].my_ckpt[j].ckpt_hdl);
      }
      mbcsv_finalize_svc(ss_cb[i].mbcsv_hdl);
   }

   /* RETURNING FROM MAIN FUNCTION */
   return;

}

static void mbcsv_dummy_testapp()
{
   uns32   i, j;

   /*
    * Peer Discovery is done. So print Inventory.
    */
   printf("\n PRINTING MBCSV Inventory\n");
   m_NCS_TASK_SLEEP(10000);
   mbcsv_prt_inv();

   /*
    * First Application will sync using Cold Sync.
    */
   printf("\n  PRINTING DATA AFTER COLD SYNC\n");
   /*mbcsv_print_fake_ss_data();*/

   printf(" \n ...COLD SYNC TEST DONE......STARTING ASYNC UPDATE TEST.... \n");

   /*
    * Async update test.
    */
   m_NCS_TASK_SLEEP(2000);
   create_fake_data(TRUE);
   m_NCS_TASK_SLEEP(10000);
   printf(" PRINTING DATA AFTER ASYNC Update\n");
   mbcsv_print_fake_ss_data();

   m_NCS_TASK_SLEEP(10000);

   /*
    * Enable Warm Sync and then print data after warm sync.
    */
   for(i=0; i<FAKE_NUM_SVCS; i++)
   {
      for (j=0; j<FAKE_NUM_CKPTS; j++)
      {
         if (NCSCC_RC_SUCCESS != mbcsv_set_obj_svc(ss_cb[i].mbcsv_hdl,
            ss_cb[i].my_ckpt[j].ckpt_hdl, NCS_MBCSV_OBJ_WARM_SYNC_ON_OFF, TRUE))
         {
            printf(" MBCSV set object failed !!.\n");
            return;
         }
      }
   }

   printf(" \n ...COLD SYNC TEST DONE......STARTING WARM SYNC TEST.... \n");

   create_fake_data(FALSE);
   m_NCS_TASK_SLEEP(20000);
   printf(" PRINTING DATA AFTER WARM SYNC Update\n");
   mbcsv_print_fake_ss_data();

   return;
}


/*****************************************************************************

  PROCEDURE NAME:    mbcsv_print_fake_ss_data

  DESCRIPTION:       Function used for printing the data of fake ss.

*****************************************************************************/
void mbcsv_print_fake_ss_data()
{
   uns32   i, j, loop;
   time_t           tod;
   char             asc_tod[40];

   for(i=0; i<FAKE_NUM_SVCS; i++)
   {
      for (j=0; j<FAKE_NUM_CKPTS; j++)
      {
         m_NCS_LOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_READ);

         printf("\n---------------------------------------------------------------------");
         printf("\n|FAKE SS DATA SVC = %3d MBCSV = %X PWE = %8X STATE = %d |",
                       ss_cb[i].svc_id, ss_cb[i].mbcsv_hdl,
                       ss_cb[i].my_ckpt[j].pwe_hdl, ss_cb[i].my_ckpt[j].ha_state);
         printf("\n---------------------------------------------------------------------");
         printf("\n|  INDEX   |    EXIST    |     NUMBER   |           TIME             ");
         printf("\n---------------------------------------------------------------------");
         
         for (loop = 0; loop < 10; loop++)
         {
            if (ss_cb[i].my_ckpt[j].my_data[loop].time.seconds != 0) {
            tod = (time_t)ss_cb[i].my_ckpt[j].my_data[loop].time.seconds;
            strftime((char *)(asc_tod), 40,
               "%d%b%Y_%H.%M.%S", localtime(&tod));
            /*m_GET_ASCII_DATE_TIME_STAMP(tod, asc_tod);*/
            sprintf((asc_tod + strlen(asc_tod)) ,
               ".%.3d",ss_cb[i].my_ckpt[j].my_data[loop].time.millisecs);
            } else
              {
                 sprintf(asc_tod, "%s", "--------------------");
              }
            
            printf("\n|    %d     |    %d        |     %3d      |   %s   |",
               loop, ss_cb[i].my_ckpt[j].my_data[loop].exist, ss_cb[i].my_ckpt[j].my_data[loop].rand_value, asc_tod);
         }
         
         printf("\n---------------------------------------------------------------------");
         
         m_NCS_UNLOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_READ);
      }
   }

}


/*****************************************************************************

  PROCEDURE NAME:    mbcsv_fake_ss_cb

  DESCRIPTION:       Fake SS callback function.

*****************************************************************************/
static uns32  mbcsv_fake_ss_cb(NCS_MBCSV_CB_ARG *arg)
{
   uns32 status = NCSCC_RC_SUCCESS;
   uns32 warm_sync_cmpl = FALSE;

   FAKE_SS_CKPT  *sscb_ptr = (FAKE_SS_CKPT *)NCS_INT64_TO_PTR_CAST(arg->i_client_hdl);

   m_NCS_LOCK(&sscb_ptr->cb_ptr->fake_ss_lock, NCS_LOCK_WRITE);

   switch (arg->i_op)
   {
   case NCS_MBCSV_CBOP_ENC:
      if (data_created)
         status = mbcsv_fake_process_enc_cb(arg);
      break;

   case NCS_MBCSV_CBOP_DEC:
      status = mbcsv_fake_process_dec_cb(arg, &warm_sync_cmpl);
      break;

   case NCS_MBCSV_CBOP_PEER:
      status = mbcsv_fake_process_peer_cb(arg);
      break;

   case NCS_MBCSV_CBOP_NOTIFY:
      status = mbcsv_fake_process_notify(arg);
      break;

   case NCS_MBCSV_CBOP_ERR_IND:
      status = mbcsv_fake_process_err_ind(arg);
      break;

   default:
      status = NCSCC_RC_FAILURE;
      break;
   }

   m_NCS_UNLOCK(&sscb_ptr->cb_ptr->fake_ss_lock, NCS_LOCK_WRITE);

   if ((sscb_ptr->ha_state == SA_AMF_HA_STANDBY) &&
       (warm_sync_cmpl == TRUE))
   {
      /* uns32 temp = rand()%10;
      if((temp == 4) ||
         (temp == 5)) */
     mbcsv_fake_send_data_req(sscb_ptr->cb_ptr->mbcsv_hdl, sscb_ptr->ckpt_hdl);
   }

   return status;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_process_enc_cb
 *
 * DESCRIPTION : MBCSV process encode call back.
 *
\***************************************************************************/
static uns32  mbcsv_fake_process_enc_cb(NCS_MBCSV_CB_ARG *arg)
{
   FAKE_SS_CKPT  *sscb_ptr = (FAKE_SS_CKPT *)NCS_INT64_TO_PTR_CAST(arg->i_client_hdl);

   if(sscb_ptr == NULL)
   {
      printf("\n mbcsv_fake_process_enc_cb: Client handle is NULL");
      return NCSCC_RC_FAILURE;
   }

   /*printf("\n Received encode callback for message type = %d ",
      arg->info.encode.io_msg_type);*/

   switch (arg->info.encode.io_msg_type)
   {
   case NCS_MBCSV_MSG_ASYNC_UPDATE:
      {
         /*printf("\n Encode callback received for ver %c%c%d reo_hdl %d",
            arg->info.encode.i_peer_version->majorVersion, 
            arg->info.encode.i_peer_version->minorVersion ,
            arg->info.encode.i_peer_version->releaseCode,
            arg->info.encode.io_reo_hdl);*/

         fake_encode_elem(1, arg->info.encode.io_reo_hdl,
                          &arg->info.encode.io_uba,
                          sscb_ptr);
      }
      break;

   case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      sscb_ptr->cold_sync_elem_sent = 0;
      sscb_ptr->cold_sync_cmplt_sent = FALSE;
      break;

   case NCS_MBCSV_MSG_COLD_SYNC_RESP:
      {
         fake_encode_elem(CS_FAKE_NUM_ELEM, arg->info.encode.io_reo_hdl, 
                            &arg->info.encode.io_uba, sscb_ptr);

         /* printf("\n Number of cold sync elements sent = %d", arg->info.encode.io_reo_hdl);*/

         if (arg->info.encode.io_reo_hdl < CS_MAX_NUM_ELM)
         {
            sscb_ptr->in_cold_sync  = TRUE;
            arg->info.encode.io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP;
            arg->info.encode.io_reo_hdl += 1;
         }
         else
         {
            sscb_ptr->cold_sync_elem_sent = 0;
            sscb_ptr->cold_sync_cmplt_sent = TRUE;
            sscb_ptr->in_cold_sync  = FALSE;
            arg->info.encode.io_msg_type = NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE;
            arg->info.encode.io_reo_hdl = 0;
            mbcsv_print_fake_ss_data();
         }
      }
      break;

   case NCS_MBCSV_MSG_WARM_SYNC_REQ:
       sscb_ptr->warm_sync_elem_sent = 0;
      break;

   case NCS_MBCSV_MSG_WARM_SYNC_RESP:
      {
         fake_encode_elem(WS_FAKE_NUM_ELEM, arg->info.encode.io_reo_hdl, 
                           &arg->info.encode.io_uba, sscb_ptr);

         /*printf("\n Number of warm sync elements sent = %d", arg->info.encode.io_reo_hdl);*/

         if (arg->info.encode.io_reo_hdl < WS_MAX_NUM_ELM)
         {
            sscb_ptr->in_warm_sync  = TRUE;
            arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP;
            arg->info.encode.io_reo_hdl += 1;
         }
         else
         {
            sscb_ptr->warm_sync_elem_sent = 0;
            sscb_ptr->in_warm_sync  = FALSE;
            arg->info.encode.io_msg_type = NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE;
            arg->info.encode.io_reo_hdl = 0;
         }
      }
      break;

   case NCS_MBCSV_MSG_DATA_RESP:
       fake_encode_elem(1, sscb_ptr->data_to_send,
         &arg->info.encode.io_uba, sscb_ptr);

      arg->info.encode.io_msg_type = NCS_MBCSV_MSG_DATA_RESP;
      break;

   default:
      break;

   }
   return NCSCC_RC_SUCCESS;

}

static uns32 fake_encode_elem(uns32 num_elm, uns32 elem_num, 
                              NCS_UBAID* uba, 
                              FAKE_SS_CKPT  *sscb_ptr)
{
   uns8*    data;
   uns32    i;

   if(uba == NULL)
   {
      printf("\n fake_encode_elem: User buff is NULL");
      return NCSCC_RC_FAILURE;
   }

   if(elem_num > 10000)
   {
      printf("\n fake_encode_elem: Incorrect Element number");
      return NCSCC_RC_FAILURE;
   }

   for (i=0; i<num_elm; i++)
   {
   data = ncs_enc_reserve_space(uba, FAKE_SS_DATA_SIZE);

   if(data == NULL)
   {
      printf("\n fake_encode_elem: DATA NULL");
      return NCSCC_RC_FAILURE;
   }

   ncs_encode_32bit(&data, elem_num);
   ncs_encode_32bit(&data, sscb_ptr->my_data[elem_num].exist);
   ncs_encode_32bit(&data, sscb_ptr->my_data[elem_num].rand_value);
   ncs_encode_32bit(&data, sscb_ptr->my_data[elem_num].time.seconds);
   ncs_encode_32bit(&data, sscb_ptr->my_data[elem_num].time.millisecs);
   ncs_enc_claim_space(uba, FAKE_SS_DATA_SIZE);
   }

   return NCSCC_RC_SUCCESS;
}

static uns32 fake_data_decode(NCS_UBAID* uba, uns32 *data_send)
{
   uns8*    data;
   uns8     data_buff[1024];

   if(uba == NULL)
   {
      printf("\n fake_encode_elem: User buff is NULL");
      return NCSCC_RC_FAILURE;
   }

   data = ncs_dec_flatten_space(uba,data_buff, sizeof(uns32));

   if(data == NULL)
   {
      printf("\n fake_encode_elem: DATA NULL");
      return NCSCC_RC_FAILURE;
   }

   *data_send = ncs_decode_32bit(&data);

   ncs_dec_skip_space(uba, sizeof(uns32));

   return NCSCC_RC_SUCCESS;
}


static uns32 fake_decode_elem(uns32 num_elm, NCS_UBAID* uba, 
                              FAKE_SS_CKPT  *sscb_ptr)
{
   uns8*    data;
   uns32    elem_num;
   uns8     data_buff[1024];
   uns32    i;

   if(uba == NULL)
   {
      printf("\n fake_encode_elem: User buff is NULL");
      return NCSCC_RC_FAILURE;
   }

   /* ACTIVE can also send empty USRBUF */
   if (uba->ub == NULL)
      return NCSCC_RC_SUCCESS;

   for (i=0; i<num_elm; i++)
   {
   data = ncs_dec_flatten_space(uba,data_buff, FAKE_SS_DATA_SIZE);

   if(data == NULL)
   {
      printf("\n fake_decode_elem: DATA NULL");
      return NCSCC_RC_FAILURE;
   }

   elem_num = ncs_decode_32bit(&data);
   sscb_ptr->my_data[elem_num].exist = ncs_decode_32bit(&data);
   sscb_ptr->my_data[elem_num].rand_value = ncs_decode_32bit(&data);
   sscb_ptr->my_data[elem_num].time.seconds = ncs_decode_32bit(&data);
   sscb_ptr->my_data[elem_num].time.millisecs = ncs_decode_32bit(&data);
   ncs_dec_skip_space(uba, FAKE_SS_DATA_SIZE);
   }

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_process_dec_cb
 *
 * DESCRIPTION : MBCSV process decode call back.
 *
\***************************************************************************/
static uns32  mbcsv_fake_process_dec_cb(NCS_MBCSV_CB_ARG *arg, uns32 *wsfail)
{
   FAKE_SS_CKPT  *sscb_ptr = (FAKE_SS_CKPT *)NCS_INT64_TO_PTR_CAST(arg->i_client_hdl);
   uns32        elem;

   /*printf("\n Received Decode callback for message type = %d ",
      arg->info.decode.i_msg_type);*/

   switch (arg->info.decode.i_msg_type)
   {
   case NCS_MBCSV_MSG_ASYNC_UPDATE:
      {
         fake_decode_elem(1, &arg->info.decode.i_uba, sscb_ptr);
      }
      break;

   case NCS_MBCSV_MSG_COLD_SYNC_REQ:
      if (sscb_ptr->in_cold_sync == TRUE)
      {
         printf("\n MBCSv clinet is in cold sync.");
         /* return NCSCC_RC_FAILURE;*/
      }
      else
         sscb_ptr->in_cold_sync  = TRUE;

      break;

   case NCS_MBCSV_MSG_COLD_SYNC_RESP:
   case NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE:
      {
         for (elem=0; elem < 1; elem++)
            fake_decode_elem(CS_FAKE_NUM_ELEM, &arg->info.decode.i_uba, sscb_ptr);
      
         if (NCS_MBCSV_MSG_COLD_SYNC_RESP_COMPLETE == arg->info.decode.i_msg_type) 
            mbcsv_print_fake_ss_data();
      }
      break;

   case NCS_MBCSV_MSG_WARM_SYNC_REQ:
         sscb_ptr->in_warm_sync  = TRUE;
      break;

   case NCS_MBCSV_MSG_WARM_SYNC_RESP:
      {
         for (elem=0; elem < 1; elem++)
            fake_decode_elem(WS_FAKE_NUM_ELEM, &arg->info.decode.i_uba, sscb_ptr);
      }
      break;

   case NCS_MBCSV_MSG_WARM_SYNC_RESP_COMPLETE:
      {
         for (elem=0; elem < 1; elem++)
            fake_decode_elem(WS_FAKE_NUM_ELEM, &arg->info.decode.i_uba, sscb_ptr);
      }
      *wsfail = TRUE;
      return NCSCC_RC_FAILURE;
      break;

   case NCS_MBCSV_MSG_DATA_REQ:
      fake_data_decode(&arg->info.decode.i_uba, &sscb_ptr->data_to_send);
      break;

   case NCS_MBCSV_MSG_DATA_RESP:
   case NCS_MBCSV_MSG_DATA_RESP_COMPLETE:
      fake_decode_elem(1, &arg->info.decode.i_uba, sscb_ptr);
      break;

   }
   return NCSCC_RC_SUCCESS;

}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_process_peer_cb
 *
 * DESCRIPTION : MBCSV process peer info call back.
 *
\***************************************************************************/
static uns32  mbcsv_fake_process_peer_cb(NCS_MBCSV_CB_ARG *arg)
{
   /* Compare versions of the peer with self */
  /* printf("\n Received PEER callback");*/
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_process_notify
 *
 * DESCRIPTION : MBCSV process notify call back.
 *
\***************************************************************************/
static uns32  mbcsv_fake_process_notify(NCS_MBCSV_CB_ARG *arg)
{
   uns32 length;
   uns8* data;
   uns8  data_buff[1024];
   char  str[100];
   NCS_UBAID  *uba = &arg->info.notify.i_uba;
   
   data = ncs_dec_flatten_space(uba, data_buff, sizeof(uns32));
   length = ncs_decode_32bit(&data);
   ncs_dec_skip_space(uba, sizeof(uns32));
   
   ncs_decode_n_octets_from_uba(uba, (uns8 *)str, length);
   
   printf("\n NTFY MSG: %s \n", str);
   
   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_process_err_ind
 *
 * DESCRIPTION : MBCSV process error indication call back.
 *
\***************************************************************************/
static uns32  mbcsv_fake_process_err_ind(NCS_MBCSV_CB_ARG *arg)
{
   /*printf("\n Received Error Indication for Error type %d", 
      arg->info.error.i_code);*/

   switch(arg->info.error.i_code)
   {
   case NCS_MBCSV_COLD_SYNC_TMR_EXP:
      break;

   case NCS_MBCSV_WARM_SYNC_TMR_EXP:
      break;

   case NCS_MBCSV_DATA_RSP_CMPLT_TMR_EXP:
      break;

   case NCS_MBCSV_COLD_SYNC_CMPL_TMR_EXP:
      break;

   case NCS_MBCSV_WARM_SYNC_CMPL_TMR_EXP:
      break;

   case NCS_MBCSV_DATA_RESP_TERMINATED:
      /* printf("\n Received data responce terminated callback ");*/
      break;

   default:
      break;
   }

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\
 *
 * PROCEDURE   :mds_demo_create_named_vdest
 *
 * DESCRIPTION :Invoked as part of VDA API invocation to create Virtual 
 *              MDS-DEST using the "named" range.
 *
\***************************************************************************/
static uns32 mds_demo_create_named_vdest(char * name, uns32 *pwe_hdl)
{
   NCSVDA_INFO vda_info;

   memset(&vda_info, 0, sizeof(vda_info));

   vda_info.req = NCSVDA_VDEST_CREATE;
   vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_NAMED;
   vda_info.info.vdest_create.i_persistent = FALSE;
   vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
   vda_info.info.vdest_create.info.named.i_name.length = 
      (uns16)(strlen(name) + 1);
   memcpy(vda_info.info.vdest_create.info.named.i_name.value,
      name, vda_info.info.vdest_create.info.named.i_name.length);

   if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
   {
      printf(" VDEST creation failed !!.\n");

      return NCSCC_RC_FAILURE;
   }

   *pwe_hdl = (uns32)vda_info.info.vdest_create.o_mds_pwe1_hdl;

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_initialize_svc
 *
 * DESCRIPTION :Initialize service with MBCSv.
 *
\***************************************************************************/
static uns32 mbcsv_initialize_svc(SS_SVC_ID   svc_id,
                                  uns16 version,
                                  uns32      *mbcsv_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_INITIALIZE;
   mbcsv_arg.info.initialize.i_service  = svc_id;
   mbcsv_arg.info.initialize.i_mbcsv_cb = mbcsv_fake_ss_cb;
   mbcsv_arg.info.initialize.i_version    = version;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Initialize operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   *mbcsv_hdl = mbcsv_arg.info.initialize.o_mbcsv_hdl;

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_open_ckpt
 *
 * DESCRIPTION :Open checkpoint operation.
 *
\***************************************************************************/
static uns32 mbcsv_open_ckpt(uns32    mbcsv_hdl,
                             NCS_MBCSV_CLIENT_HDL    client_hdl,
                             uns32    pwe_hdl,
                             uns32    *ckpt_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_OPEN;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.open.i_pwe_hdl = pwe_hdl;
   mbcsv_arg.info.open.i_client_hdl = client_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Open operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   *ckpt_hdl = mbcsv_arg.info.open.o_ckpt_hdl;

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_set_ckpt_role
 *
 * DESCRIPTION :Set checkpoint role.
 *
\***************************************************************************/
uns32 mbcsv_set_ckpt_role(uns32    mbcsv_hdl,
                          uns32    ckpt_hdl,
                          uns32    role)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.chg_role.i_ckpt_hdl = ckpt_hdl;
   mbcsv_arg.info.chg_role.i_ha_state = role;

   /* printf("\n Setting role to %d", role);*/

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Role set operation failed!!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}
/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_get_sel_obj
 *
 * DESCRIPTION :Get selection object.
 *
\***************************************************************************/
static uns32 mbcsv_get_sel_obj(uns32                 mbcsv_hdl,
                               SaSelectionObjectT    *sel_obj)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_SEL_OBJ_GET;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV get selection object operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   *sel_obj = mbcsv_arg.info.sel_obj_get.o_select_obj;

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_dispatch
 *
 * DESCRIPTION :Dispatch all the messages from the mailbox.
 *
\***************************************************************************/
static uns32 mbcsv_fake_dispatch(uns32    mbcsv_hdl,
                                 uns32     flag)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.dispatch.i_disp_flags = flag;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV dispatch operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_send_data
 *
 * DESCRIPTION :Send ckpt data to peer standby.
 *
\***************************************************************************/
static uns32 mbcsv_fake_send_data(uns32    mbcsv_hdl,
                                  uns32    ckpt_hdl,
                                  uns32    action,
                                  uns32    reo_hdl,
                                  uns32    send_type)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_CKPT;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.send_ckpt.i_action = action;
   mbcsv_arg.info.send_ckpt.i_ckpt_hdl = ckpt_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_hdl  = reo_hdl;
   mbcsv_arg.info.send_ckpt.i_reo_type = 0;
   mbcsv_arg.info.send_ckpt.i_send_type = send_type;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV send data operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_close_ckpt
 *
 * DESCRIPTION :Close checkpoint operation.
 *
\***************************************************************************/
static uns32 mbcsv_close_ckpt(uns32    mbcsv_hdl,
                             uns32     ckpt_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_CLOSE;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.close.i_ckpt_hdl = ckpt_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Close operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_finalize_svc
 *
 * DESCRIPTION :Finalize service with MBCSv.
 *
\***************************************************************************/
static uns32 mbcsv_finalize_svc(uns32   mbcsv_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Finalize operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_set_obj_svc
 *
 * DESCRIPTION :Set service object with MBCSv.
 *
\***************************************************************************/
static uns32 mbcsv_set_obj_svc(uns32  mbcsv_hdl, uns32 ckpt_hdl,
                               uns32  obj, uns32 val)
{
   NCS_MBCSV_ARG     mbcsv_arg;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_OBJ_SET;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;
   mbcsv_arg.info.obj_set.i_ckpt_hdl = ckpt_hdl;
   mbcsv_arg.info.obj_set.i_obj = obj;
   mbcsv_arg.info.obj_set.i_val = val;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf(" MBCSV Set operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   :mbcsv_fake_send_data_req
 *
 * DESCRIPTION :Sends data request message.
 *
\***************************************************************************/
static uns32 mbcsv_fake_send_data_req(uns32      mbcsv_hdl,
                                 uns32      ckpt_hdl)
{
   NCS_MBCSV_ARG     mbcsv_arg;
   NCS_UBAID         *uba = NULL;
   uns8*             data;

   memset(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));

   mbcsv_arg.i_op = NCS_MBCSV_OP_SEND_DATA_REQ;
   mbcsv_arg.i_mbcsv_hdl = mbcsv_hdl;

   uba = &mbcsv_arg.info.send_data_req.i_uba;

   memset(uba, '\0', sizeof(NCS_UBAID));

   if (NCSCC_RC_SUCCESS != ncs_enc_init_space(uba))
   {
      printf("mbcsv_fake_send_data_req: Function ncs_enc_init_space returns failure.\n");
      return NCSCC_RC_FAILURE;
   }

   data = ncs_enc_reserve_space(uba, 1 * sizeof(uns32));
   ncs_encode_32bit(&data,rand()%10);
   ncs_enc_claim_space(uba, 1 * sizeof(uns32));

   mbcsv_arg.info.send_data_req.i_ckpt_hdl = ckpt_hdl;

   if (NCSCC_RC_SUCCESS != ncs_mbcsv_svc(&mbcsv_arg))
   {
      printf("mbcsv_fake_send_data_req: MBCSV send data req operation failed !!.\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:    mbcsv_start_dispatch_thread

  DESCRIPTION:       Start new thread for dispatch operation.

*****************************************************************************/
static void mbcsv_start_dispatch_thread(int i)
{
   int *arg;

   arg = (int *)m_NCS_MEM_ALLOC(sizeof(int), NCS_MEM_REGION_TRANSIENT, 102, 100);

   *arg = i;

   if(m_NCS_TASK_CREATE((NCS_OS_CB)dispatch_all_blk,
      (void *)arg, task_name[i],
      NCS_TASK_PRIORITY_7,
      NCS_STACKSIZE_HUGE,
      &ss_cb[i].task_hdl) !=  NCSCC_RC_SUCCESS)
   {
      printf("Task Creation failed \n");
      return;
   }

   if(m_NCS_TASK_START(ss_cb[i].task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(ss_cb[i].task_hdl);
      printf("Task Creation failed \n");
      return;
   }

}

/*****************************************************************************

  PROCEDURE NAME:    dispatch_all_blk

  DESCRIPTION:       Function used for dispatch all.

*****************************************************************************/
static void dispatch_all_blk(int *i)
{
   mbcsv_fake_dispatch(ss_cb[*i].mbcsv_hdl, SA_DISPATCH_BLOCKING);
}


/*****************************************************************************

  PROCEDURE NAME:    create_fake_data

  DESCRIPTION:       Function used for creating data of fake ss.

*****************************************************************************/
static void create_fake_data(NCS_BOOL send_data)
{
   uns32   i, j, add_del=TRUE, num_elem=0, loop = 0, elem;
   
   for (i=0; i<FAKE_NUM_SVCS; i++)
   {
   /*
   * first call dispatch to process the mbcsv mailbox.
      */
      for(j=0; j<FAKE_NUM_CKPTS; j++)
      {
         if (my_role == SA_AMF_HA_ACTIVE)
            memset(ss_cb[i].my_ckpt[j].my_data, '\0',
               10 * sizeof(FAKE_SS_DATA_STRUCT));
      }
   }

   while(++loop != 100)
   {
      for (i=0; i<FAKE_NUM_SVCS; i++)
      {
        /*
         * first call dispatch to process the mbcsv mailbox.
         */
         for(j=0; j<FAKE_NUM_CKPTS; j++)
         {
            elem = (rand() % 10);

            if (my_role == SA_AMF_HA_ACTIVE)
            {
               uns32 action;
               /*
               * Keep on adding new element in the array.till number of elements < 10.
               */
               if (add_del)
               {
                  m_NCS_LOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_WRITE);
                  if (ss_cb[i].my_ckpt[j].my_data[elem].exist == FALSE)
                  {
                     if(++num_elem == 8)
                        add_del = FALSE;
                     action = NCS_MBCSV_ACT_ADD;
                  }
                  else
                     action = NCS_MBCSV_ACT_UPDATE;

                  ss_cb[i].my_ckpt[j].my_data[elem].exist = TRUE;
                  ss_cb[i].my_ckpt[j].my_data[elem].rand_value = rand() % 1000;
                  m_GET_MSEC_TIME_STAMP(&ss_cb[i].my_ckpt[j].my_data[elem].time.seconds,
                     &ss_cb[i].my_ckpt[j].my_data[elem].time.millisecs);
                  m_NCS_UNLOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_WRITE);
               }
               else
               {
                  m_NCS_LOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_WRITE);
                  if (ss_cb[i].my_ckpt[j].my_data[elem].exist == FALSE)
                  {
                     action = NCS_MBCSV_ACT_UPDATE;;
                  }
                  else
                  {
                     ss_cb[i].my_ckpt[j].my_data[elem].exist = FALSE;

                     if(--num_elem == 3)
                        add_del = TRUE;

                     action = NCS_MBCSV_ACT_RMV;
                  }
                  m_NCS_UNLOCK(&ss_cb[i].fake_ss_lock, NCS_LOCK_WRITE);
               }

               if (send_data == TRUE)
                  mbcsv_fake_send_data(ss_cb[i].mbcsv_hdl, ss_cb[i].my_ckpt[j].ckpt_hdl, action,
                                    elem, NCS_MBCSV_SND_SYNC);
            }
         }
      }
   }

   data_created = TRUE;

   m_NCS_TASK_SLEEP(30);
}

