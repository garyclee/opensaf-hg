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

  $Header: mds_demo.c$

..............................................................................

  DESCRIPTION:

******************************************************************************
*/

#include "mds_papi.h"
#include "ncsencdec_pub.h" /*  This is for encode and decode utilities */
#include "ncs_mda_papi.h"
#include "ncs_main_papi.h"

/********************************************************\
** Sample application's MAIN ROUTINE and relevant data  **
\********************************************************/
int     process_no=0;  /* Sample application runs two processes called "1=SENDER_MDS_PID" and "2=RCVR_MDS_PID" */
int     state_event=0; /* State of the Sender or receiver defined as follows */   

/* Variable to record sample application's state */
enum {
    /* Sender side states */
    AWAITING_RCVR = 0,
    DISCOVERED_RCVR,
    NO_ACTIVE_RCVR,
    NEW_ACTIVE_RCVR,
    DEAD_RCVR,

    /* Receiver side states */
    AWAITING_SENDER = 0,
    DISCOVERED_SENDER,
    DEAD_SENDER,
}STATE_EVENT;

/* An (arbitrary) structure used for sending/receiving messages  */
#define MSG_SIZE_MAX  50
typedef struct mds_demo_msg{
   uns32 send_len;
   char send_data[MSG_SIZE_MAX];

   uns32 recvd_len;
   char recvd_data[MSG_SIZE_MAX];
}MDS_DEMO_MSG;
static MDS_DEMO_MSG  gl_sample_msg;

/* The main routine */
static void  mds_sample(int process_no);

/*****************************************************************************\
** SUBROUTINES, VARIABLES, etc. that are typical of MDS User code.           **
\*****************************************************************************/
#define   MDS_DEMO_VDEST1      1002         /* To demonstrate use of FIXED VDESTs */
#define   SENDER_MDS_SVCID     94
#define   RCVR_MDS_SVCID       95

#define   SENDER_MDS_PID       1
#define   RCVR_MDS_PID         2

MDS_HDL         mds_hdl=0;        /* A handle that goes in all MDS's user API calls */
NCS_SEL_OBJ     sel_obj;             /* Used to poll for MDS events           */
MDS_DEST        peer_dest=0;         /* Peer's MDS address(discovered)        */
MDS_SVC_ID      peer_svcid=0;        /* Peer's service-id (known apriori)     */

/* Callback given by User to MDS*/
static uns32 mds_demo_callback(struct ncsmds_callback_info *info);

/* Creates a subscription */
static uns32 mds_create_subscription(int fr_svc, int to_svc);

/* Uninstalls a service*/
static uns32 mds_demo_svc_uninstall(uns32 svc);

/* Generates a Sample MSG for sending*/
static uns32 mds_demo_generate_sample_msg(MDS_DEMO_MSG  *gl_sample_msg);

/* Sends a message from one SVC to other SVC*/
static uns32 mds_demo_send_sample_mesg(MDS_DEST dest, MDS_SVC_ID svc);

/* Used to wait on the select object till the time provided as input parameter to this function*/
static void  mdm_demo_process_time_wait(uns32 time);


/***************************************************************************\
 *
 * PROCEDURE   : main
 *
 * DESCRIPTION : Sets process configuration and start the process.
 *
\***************************************************************************/
int main(int argc, char *argv[])
{

   if (argc < 2)
   {
      printf("Usage:mds_demo <process_num(1 or 2)>\n");
      return 1;
   }
   else
      process_no = atoi(argv[1]);

   /* Initialized OpenSAF agents (including MDS). Pass on command-line arguments
      to ncs_agents_startup(). It will only parse arguments that it finds
      relevant.
   */
   ncs_agents_startup(argc, argv);

   /* Start the sample process*/
   mds_sample(process_no);
   return 1;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_sample
 *
 * DESCRIPTION : starts the respective processes
 *
\***************************************************************************/
static void mds_sample(int process_no)
{
    NCSMDS_INFO           svc_info;
    NCSVDA_INFO           vda_info;
    NCSADA_INFO           ada_info;
    memset(&ada_info, 0, sizeof(ada_info));
    memset(&vda_info, 0, sizeof(vda_info));
    memset(&svc_info, 0, sizeof(svc_info));

    if(process_no==SENDER_MDS_PID)
    {
        /* Install the service on ADEST */
    
        /* Get MDS ADEST hdl using ncsada_api() */
        ada_info.req = NCSADA_GET_HDLS;

        if(ncsada_api(&ada_info) != NCSCC_RC_SUCCESS)
        {
            printf("mds_sample: Getting handles of ADEST -FAIL\n");
        }
        else
        {
           printf("mds_sample: Got MDS handles For ADEST \n");
        }

        /* Save PWE1 hdl got via ncsada_api() for further use */
        mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;

        /* Verify that it is possibles to do an MDS_INSTALL with
        this service-id. */

        svc_info.i_mds_hdl = mds_hdl;

        /* Application specific svc-info setup follows */
        svc_info.i_op = MDS_INSTALL;
        svc_info.i_svc_id = SENDER_MDS_SVCID;
        svc_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
        svc_info.info.svc_install.i_mds_q_ownership = TRUE;
        svc_info.info.svc_install.i_svc_cb = mds_demo_callback;
        svc_info.info.svc_install.i_yr_svc_hdl = 0; /* Some junk value merely for testing sakes */
        svc_info.info.svc_install.i_mds_svc_pvt_ver = 1;
        /* Finally, start using MDS API */
        if(ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS)
        {
            printf("mds_sample: Service Install on Adest PWE1 FAIL\n");
        }
        else
        {
            printf("mds_sample: SUCCESS Service Install on ADEST PWE1\n");
        }
        sel_obj=svc_info.info.svc_install.o_sel_obj;

        /* Now create the subscription to RCVR_MDS_SVCID to send messages */
        mds_create_subscription(SENDER_MDS_SVCID, RCVR_MDS_SVCID);

        while(1)
        {
           /* This while loop waits for the Process 2(Receiver Process) to come up,
                 once this detects that process 2 has come up, this starts sending the messages to the 
                 SVC on process 2 until the process 2 dies down and receives the DOWN event. Subsequently
                 SVC installed on this process also gets uninstalled*/

           uns32 count=0;
           NCS_SEL_OBJ numfds;
           NCS_SEL_OBJ_SET readfd;
           uns32 time=10;

           numfds.raise_obj=0;
           numfds.rmv_obj=0;

           m_NCS_SEL_OBJ_ZERO(&readfd);

           m_NCS_SEL_OBJ_SET(sel_obj,&readfd);
           numfds=m_GET_HIGHER_SEL_OBJ(sel_obj,numfds);

           /* Wait for the timeout or any event to come */   
           if(state_event==AWAITING_RCVR)
              count=m_NCS_SEL_OBJ_SELECT(numfds,&readfd,NULL,NULL,NULL);
           else
              count=m_NCS_SEL_OBJ_SELECT(numfds,&readfd,NULL,NULL,&time);

           /* Check after timeout that any event has been recd*/ 
           if(m_NCS_SEL_OBJ_ISSET(sel_obj,&readfd))
           {
               /* Now call the MDS retreive function to get the events or data. This is done
                       as MDS-Q Ownership is maintained by MDS */
               NCSMDS_INFO mds_info;
               memset(&mds_info, 0, sizeof(mds_info));
               mds_info.i_mds_hdl = mds_hdl;
               mds_info.i_svc_id = SENDER_MDS_SVCID;
               mds_info.i_op = MDS_RETRIEVE;

               mds_info.info.retrieve_msg.i_dispatchFlags = SA_DISPATCH_ALL;

               if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
               {
                   printf("mds_sample:MDS Retrieve Failed\n");
               }
           }
           
           if(state_event!=AWAITING_RCVR)
           {
              /* Send message to RCVR_MDS_SVCID once its comes up*/
              mds_demo_send_sample_mesg(peer_dest,peer_svcid);
           }
        }
    }
    else if (process_no==RCVR_MDS_PID)
    {
        /* Install a service on vdest*/
        vda_info.req = NCSVDA_VDEST_CREATE;
        vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;    
        vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
        vda_info.info.vdest_create.info.specified.i_vdest = MDS_DEMO_VDEST1;

        /* Finally, invoke ncsvda_api() to get the handle */
        if(ncsvda_api(&vda_info)!= NCSCC_RC_SUCCESS)
        {
            printf("mds_sample: PWE1 Creation on Vdest FAIL\n");
        }
        else
        {
            printf("mds_sample: PWE1 Creation on Vdest Success\n");
        }
        /* Save PWE1 hdl got via ncsvda_api() for further use */
        mds_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl; 

        svc_info.i_mds_hdl = mds_hdl;

        /* Application specific svc-info setup follows */
        svc_info.i_op = MDS_INSTALL;
        svc_info.i_svc_id = RCVR_MDS_SVCID;
        svc_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
        svc_info.info.svc_install.i_mds_q_ownership = TRUE;
        svc_info.info.svc_install.i_svc_cb = mds_demo_callback;
        svc_info.info.svc_install.i_yr_svc_hdl = 0;
        svc_info.info.svc_install.i_mds_svc_pvt_ver = 1;

        /* Finally, start using MDS API */
        if(ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS)
        {
            printf("mds_sample: SVC install on VDEST PWE1 Fail\n");
        }
        else
        {
            printf("mds_sample: SVC install on VDEST PWE1 SUCCESS\n");
        }
        sel_obj=svc_info.info.svc_install.o_sel_obj;

        /* Now create subscription */
        mds_create_subscription(RCVR_MDS_SVCID, SENDER_MDS_SVCID);


        while(1)
        {
           /* This while loop waits for the PROCESS 1<SENDER MDS> to come UP,
              once it comes up, changes the role of VDEST to active, then after 5 seconds to quiesced,
              after 2 seconds to standby and again makes to active after 1 sec. Waits for 6 seconds and uninstalls the SVC*/
            /* Now wait for the messages to come */
             uns32 count=0;
             NCS_SEL_OBJ numfds;
             NCS_SEL_OBJ_SET readfd;

             numfds.raise_obj=0;
             numfds.rmv_obj=0;

             m_NCS_SEL_OBJ_ZERO(&readfd);

             m_NCS_SEL_OBJ_SET(sel_obj,&readfd);
             numfds=m_GET_HIGHER_SEL_OBJ(sel_obj,numfds);

             if(state_event==AWAITING_SENDER)
                count=m_NCS_SEL_OBJ_SELECT(numfds,&readfd,NULL,NULL,NULL);
             else
             {
                /* Sender has come up*/
                mdm_demo_process_time_wait(500);

                /* Now make to quiesced */
                memset(&vda_info, 0, sizeof(vda_info));
                vda_info.req = NCSVDA_VDEST_CHG_ROLE;
                vda_info.info.vdest_chg_role.i_vdest =svc_info.info.svc_install.o_dest;
                vda_info.info.vdest_chg_role.i_new_role = V_DEST_RL_QUIESCED;
                
                if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
                {
                    printf("mds_sample: Vdest Role change to quiesced Fail\n");
                }
                else
                {
                    printf("mds_sample: Vdest Role changed to quiesced\n");
                }
                
                /* Wait and receive the messages in between if recd*/ 
                mdm_demo_process_time_wait(200);

                /* Now make to standby */
                memset(&vda_info, 0, sizeof(vda_info));
                vda_info.req = NCSVDA_VDEST_CHG_ROLE;
                vda_info.info.vdest_chg_role.i_vdest =svc_info.info.svc_install.o_dest;
                vda_info.info.vdest_chg_role.i_new_role = V_DEST_RL_STANDBY;
                
                if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
                {
                    printf("mds_sample: Vdest Role change to standby Fail\n");
                }
                else
                {
                    printf("mds_sample: Vdest Role changed to standby\n");
                }

                /* Wait and receive the messages in between if recd*/ 
                mdm_demo_process_time_wait(100);

                /* Now make to Active and wait for 4 sec and exit */
                memset(&vda_info, 0, sizeof(vda_info));
                vda_info.req = NCSVDA_VDEST_CHG_ROLE;
                vda_info.info.vdest_chg_role.i_vdest =svc_info.info.svc_install.o_dest;
                vda_info.info.vdest_chg_role.i_new_role = V_DEST_RL_ACTIVE;
                
                if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
                {
                    printf("mds_sample: Vdest Role change to Active Fail\n");
                }
                else
                {
                    printf("mds_sample: Vdest Role changed to Active\n");
                }

                /* Wait and receive the messages in between if recd*/ 
                mdm_demo_process_time_wait(600);

                /* Uninstall the SVC and Quit*/
                mds_demo_svc_uninstall(RCVR_MDS_SVCID);
             }

             if(m_NCS_SEL_OBJ_ISSET(sel_obj,&readfd))
             {
                 /* Now call the MDS retreive function to get the events or data */
                 NCSMDS_INFO mds_info;
                 memset(&mds_info, 0, sizeof(mds_info));
                 mds_info.i_mds_hdl = mds_hdl;
                 mds_info.i_svc_id = RCVR_MDS_SVCID;
                 mds_info.i_op = MDS_RETRIEVE;

                 mds_info.info.retrieve_msg.i_dispatchFlags = SA_DISPATCH_ALL;

                 if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
                 {
                     printf("mds_sample: Retrieve Failed\n");
                 }
                 
                 while(1)
                 {
                    if(state_event==AWAITING_SENDER)
                    {
                       /* Do nothing just continue, if the process 1 has come up, then make the role of vdest as active*/
                    }
                    else
                    {
                       /*Process 1 has come up Now need to make role of vdest as active */
                       memset(&vda_info, 0, sizeof(vda_info));
                       vda_info.req = NCSVDA_VDEST_CHG_ROLE;
                       vda_info.info.vdest_chg_role.i_vdest =svc_info.info.svc_install.o_dest;
                       vda_info.info.vdest_chg_role.i_new_role = V_DEST_RL_ACTIVE;
                       if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
                       {
                          printf("mds_sample: Vdest Role change to active Fail\n");
                       }
                       else
                       {
                          printf("mds_sample: Vdest Role changed to active\n");
                       }
                       break;
                    }
                }

             }
        }
    }
}

/***************************************************************************\
 *
 * PROCEDURE   : mdm_demo_process_time_wait
 *
 * DESCRIPTION : waits till the time provided as input and process the messages in between
 *
\***************************************************************************/
static void mdm_demo_process_time_wait(uns32 time)
{
    uns32           select_rc = 0;  /* Select return code */
    NCS_SEL_OBJ     maxfd;
    NCS_SEL_OBJ_SET orig_readfds, readfds;
    NCSMDS_INFO mds_info;


    memset(&maxfd, 0, sizeof(maxfd));
    m_NCS_SEL_OBJ_ZERO(&orig_readfds);
    memset(&mds_info, 0, sizeof(mds_info));

    mds_info.i_mds_hdl = mds_hdl;
    mds_info.i_svc_id = RCVR_MDS_SVCID;
    mds_info.i_op = MDS_RETRIEVE;
    mds_info.info.retrieve_msg.i_dispatchFlags = SA_DISPATCH_ALL;

    m_NCS_SEL_OBJ_SET(sel_obj,&orig_readfds);
    maxfd=m_GET_HIGHER_SEL_OBJ(sel_obj,maxfd);

    while(1)
    {
        readfds = orig_readfds;
        select_rc = m_NCS_SEL_OBJ_SELECT(maxfd,&readfds,NULL,NULL,&time);

        if(select_rc == 0)
           return ; /* Required timeout has happened */

        if (select_rc < 0)
        {
            printf("mds_sample: UNKNOWN ERROR: Select returns failure.\n");
            return;
        }

        if(m_NCS_SEL_OBJ_ISSET(sel_obj,&readfds))
        {
          /* Now call the MDS retreive function to get the events or data */
          if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
          {
             printf("mds_sample: UNKNOWN ERROR: Retrieve Failed\n");
             return;
          }
        }
        else
        {
             printf("UNKNOWN ERROR: Unknown FD set in fdset returned by select.\n");
             return;
        }
    }
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_create_subscription
 *
 * DESCRIPTION : creates a subscription
 *
\***************************************************************************/
static uns32 mds_create_subscription(int fr_svc, int to_svc)
{
    MDS_SVC_ID              svc_array[1];

    NCSMDS_INFO             mds_info;
    memset(&mds_info, 0, sizeof(mds_info));

    svc_array[0] = to_svc;
    
    /* mds_info.i_op = MDS_SUBSCRIBE; Is filled below based on view-type */
    mds_info.i_mds_hdl = mds_hdl;
    mds_info.i_svc_id = fr_svc;
    mds_info.info.svc_subscribe.i_num_svcs = 1;
    mds_info.info.svc_subscribe.i_svc_ids = svc_array;

    mds_info.i_op = MDS_SUBSCRIBE;

    mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;

    if (ncsmds_api(&mds_info) != NCSCC_RC_SUCCESS)
    {
       printf("mds_sample: Error: Service=%d failed to subscribe to Service=%d\n",fr_svc,to_svc);
       return NCSCC_RC_FAILURE;
    }
    else
    {
       printf("mds_sample: Service=%d subscribed to Service=%d successfully\n",fr_svc,to_svc);
       return NCSCC_RC_SUCCESS;
    }
}


static uns32 mds_demo_cb_enc      (struct ncsmds_callback_info *info);
static uns32 mds_demo_cb_dec      (struct ncsmds_callback_info *info);
static uns32 mds_demo_cb_enc_flat (struct ncsmds_callback_info *info);
static uns32 mds_demo_cb_dec_flat (struct ncsmds_callback_info *info);
static uns32 mds_demo_cb_rcv      (struct ncsmds_callback_info *info);
static uns32 mds_demo_svc_event(struct ncsmds_callback_info *info);
static uns32 mds_demo_quiesced_event(struct ncsmds_callback_info *info);

static uns32 dummy (struct ncsmds_callback_info *info);
/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_callback
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_callback(struct ncsmds_callback_info *info)
{
   static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
         dummy,      /* MDS_CALLBACK_COPY      */
         mds_demo_cb_enc,      /* MDS_CALLBACK_ENC       */
         mds_demo_cb_dec,      /* MDS_CALLBACK_DEC       */
         mds_demo_cb_enc_flat, /* MDS_CALLBACK_ENC_FLAT  */
         mds_demo_cb_dec_flat, /* MDS_CALLBACK_DEC_FLAT  */
         mds_demo_cb_rcv,      /* MDS_CALLBACK_RECEIVE   */
         mds_demo_svc_event, /* MDS_CALLBACK_SVC_EVENT */
         dummy, /* MDS_CALLBACK_SYS_EVENT */
         mds_demo_quiesced_event, /* MDS_CALLBACK_QUIESCED_ACK */
         dummy,      /* MDS_CALLBACK_DIRECT_RECEIVE   */
   };

   return (*cb_set[info->i_op])(info);
}
/***************************************************************************\
 *
 * PROCEDURE   : dummy
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 dummy (struct ncsmds_callback_info *info)
{
    return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_quiesced_event
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_quiesced_event(struct ncsmds_callback_info *info)
{
   printf("mds_sample: Received Quiesced Ack Event\n");
   return NCSCC_RC_SUCCESS;
}
/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_cb_rcv
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_cb_rcv      (struct ncsmds_callback_info *info)
{
   printf("mds_sample: Svc-id=%d receives (MDS-CALLBACK model) msg.\n",((process_no==1)?SENDER_MDS_SVCID:RCVR_MDS_SVCID));
   printf("           and len = %d,message = %s\n",
            ((MDS_DEMO_MSG*)info->info.receive.i_msg)->recvd_len,
            ((MDS_DEMO_MSG*)info->info.receive.i_msg)->recvd_data);

   /* IR 82292 Fix */
   free(info->info.receive.i_msg);

   return NCSCC_RC_SUCCESS;
}


/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_svc_event
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_svc_event(struct ncsmds_callback_info *info)
{

    if (info->info.svc_evt.i_change == NCSMDS_UP)
    {
        state_event=((process_no==SENDER_MDS_PID)?DISCOVERED_RCVR:DISCOVERED_SENDER);
        printf("mds_sample: Svc-id = %d gets an UP for svc-id=%d @ MDS_DEST=<%llx>\n",
          ((process_no==SENDER_MDS_PID)?SENDER_MDS_SVCID:RCVR_MDS_SVCID),
          info->info.svc_evt.i_svc_id,
          info->info.svc_evt.i_dest);
        peer_dest=info->info.svc_evt.i_dest;
        peer_svcid=info->info.svc_evt.i_svc_id;

        /* Now send the message to the destination*/
        if(process_no==SENDER_MDS_PID)
            mds_demo_send_sample_mesg(peer_dest, peer_svcid);
    }
    else if (info->info.svc_evt.i_change == NCSMDS_DOWN)
    {
        state_event=((process_no==SENDER_MDS_PID)?DEAD_RCVR:DEAD_SENDER);

        printf("mds_sample: Recd DOWN Event\n");
        mds_demo_svc_uninstall(((process_no==SENDER_MDS_PID)?SENDER_MDS_SVCID:RCVR_MDS_SVCID));
    }
    else if (info->info.svc_evt.i_change == NCSMDS_NO_ACTIVE)
    {
        printf(" mds_sample: Recd NO ACTIVE Event\n");
    }
    else if (info->info.svc_evt.i_change == NCSMDS_NEW_ACTIVE)
    {
        state_event=NEW_ACTIVE_RCVR;
        printf(" mds_sample: Recd NEW_ACTIVE Event\n");
    }
    else if (info->info.svc_evt.i_change == NCSMDS_RED_UP)
    {
        printf(" mds_sample: Recd RED_UP Event\n");
    }
    else if (info->info.svc_evt.i_change == NCSMDS_RED_DOWN)
    {
        printf(" mds_sample: Recd RED_DOWN Event\n");
    }
    else if (info->info.svc_evt.i_change == NCSMDS_CHG_ROLE)
    {
        printf(" mds_sample: Recd CHG_ROLE Event\n");
    }
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_send_sample_mesg
 *
 * DESCRIPTION : Prepares and sends a sample msg
 *
\***************************************************************************/
static uns32 mds_demo_send_sample_mesg(MDS_DEST dest, MDS_SVC_ID svc)
{
    NCSMDS_INFO             mds_info;
    memset(&mds_info, 0, sizeof(mds_info));

    mds_info.i_mds_hdl = mds_hdl;
    mds_info.i_svc_id =((process_no==1)?SENDER_MDS_SVCID:RCVR_MDS_SVCID);
    mds_info.i_op = MDS_SEND;

    mds_demo_generate_sample_msg(&gl_sample_msg);
    mds_info.info.svc_send.i_msg = &gl_sample_msg;
    mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
    mds_info.info.svc_send.i_to_svc = svc;

    mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
    mds_info.info.svc_send.info.snd.i_to_dest = peer_dest;

    if (ncsmds_api(&mds_info) == NCSCC_RC_SUCCESS)
    {
        printf("mds_sample: Message sent from Svc-id=%d to svc_id=%d\n",((process_no==1)?SENDER_MDS_SVCID:RCVR_MDS_SVCID),
             svc);
    }
    else
    {
        printf("mds_sample: Message sent from Svc-id=%d to svc_id=%d failed\n",((process_no==1)?SENDER_MDS_SVCID:RCVR_MDS_SVCID),
            svc);
    }
    return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_generate_sample_msg
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_generate_sample_msg(MDS_DEMO_MSG  *msg)
{
   static uns32 msg_seq_no;

   msg_seq_no++;
   /* sprintf: Ensure message size is less than MSG_SIZE_MAX - 1  */
   sprintf(msg->send_data, "Message-no = %d", msg_seq_no);
   msg->send_len = MSG_SIZE_MAX - 1;
   msg->send_data[msg->send_len-2]='P';
   msg->send_data[msg->send_len-1]=0;
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_cb_enc
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_cb_enc      (struct ncsmds_callback_info *info)
{
   uns8 *p8;
   MDS_DEMO_MSG *msg;
   NCS_UBAID       *uba;

   msg = (MDS_DEMO_MSG*)info->info.enc.i_msg;
   uba = info->info.enc.io_uba;
   info->info.enc.o_msg_fmt_ver = 1;

   /* ENCODE length */
   p8 = ncs_enc_reserve_space(uba, 2);
   ncs_encode_16bit(&p8, msg->send_len);
   ncs_enc_claim_space(uba, 2);

   /* ENCODE data */
   ncs_encode_n_octets_in_uba(uba, (uns8*)msg->send_data, msg->send_len);

   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_cb_dec
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_cb_dec      (struct ncsmds_callback_info *info)
{
   uns8 *p8;
   MDS_DEMO_MSG *msg;
   NCS_UBAID       *uba;


   if (info->info.dec.o_msg != NULL)
   {
      /* We are receiving a synchronous reply! */
      msg = info->info.dec.o_msg;
   }
   else
   {
      /* We are receiving an asynchronous message! */
      msg = (MDS_DEMO_MSG*)malloc(sizeof(MDS_DEMO_MSG));
      info->info.dec.o_msg = (uns8*)msg;
   }
   uba = info->info.dec.io_uba;

   /* DECODE length */
   p8 = ncs_dec_flatten_space(uba, (uns8*)&msg->recvd_len, 2);
   msg->recvd_len = ncs_decode_16bit(&p8);
   ncs_dec_skip_space(uba, 2);

   /* DECODE data */
   if (msg->recvd_len > (sizeof(msg->recvd_data) - 1))
   {
      /* response too long */
      return NCSCC_RC_FAILURE;
   }
   ncs_decode_n_octets_from_uba(uba, (uns8*)msg->recvd_data, msg->recvd_len);
   msg->recvd_data[msg->recvd_len] = 0; /* NULL termination for string */


   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_cb_enc_flat
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_cb_enc_flat (struct ncsmds_callback_info *info)
{
   uns8 *p8;
   MDS_DEMO_MSG *msg;
   NCS_UBAID       *uba;

   msg = (MDS_DEMO_MSG*)info->info.enc_flat.i_msg;
   uba = info->info.enc_flat.io_uba;
   info->info.enc_flat.o_msg_fmt_ver = 1;

   /* ENCODE length */
   p8 = ncs_enc_reserve_space(uba, sizeof(msg->send_len));
   m_NCS_OS_MEMCPY(p8, &msg->send_len, sizeof(msg->send_len)); /* htons not required */
   ncs_enc_claim_space(uba, sizeof(msg->send_len));
   ncs_encode_n_octets_in_uba(uba, (uns8*)msg->send_data, msg->send_len);
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_cb_dec_flat
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_cb_dec_flat (struct ncsmds_callback_info *info)
{
   uns8 *p8;
   MDS_DEMO_MSG *msg;
   NCS_UBAID       *uba;

   if (info->info.dec_flat.o_msg != NULL)
   {
      /* We are receiving a synchronous reply! */
      msg = info->info.dec_flat.o_msg;
   }
   else
   {
      /* We are receiving an asynchronous message! */
      msg = (MDS_DEMO_MSG*)malloc(sizeof(MDS_DEMO_MSG));
      info->info.dec_flat.o_msg = (uns8*)msg;
   }
   uba = info->info.dec_flat.io_uba;

   p8 = ncs_dec_flatten_space(uba, (uns8*)&msg->recvd_len, sizeof(msg->send_len));

   m_NCS_OS_MEMCPY(&msg->recvd_len, p8, sizeof(msg->send_len)); /* send_len and recvd_len should be same size */
   ncs_dec_skip_space(uba, sizeof(msg->send_len));
   /* DECODE data */
   ncs_decode_n_octets_from_uba(uba, (uns8*)msg->recvd_data, msg->recvd_len);
   return NCSCC_RC_SUCCESS;
}

/***************************************************************************\
 *
 * PROCEDURE   : mds_demo_svc_uninstall
 *
 * DESCRIPTION :
 *
\***************************************************************************/
static uns32 mds_demo_svc_uninstall(uns32 svc)
{
    NCSMDS_INFO           svc_info;
    memset(&svc_info, 0, sizeof(svc_info));

    svc_info.i_mds_hdl = mds_hdl;

    /* Application specific svc-info setup follows */
    svc_info.i_op = MDS_UNINSTALL;
    svc_info.i_svc_id = svc;
    svc_info.info.svc_uninstall.i_msg_free_cb = NULL; /* Since it is not required */

    /* Finally, finish using MDS API */
    if(ncsmds_api(&svc_info) != NCSCC_RC_SUCCESS)
    {
        printf("mds_sample: Service Uninstall FAIL\n");
    }
    else
    {
        printf("mds_sample: Service Uninstall Success\n");
    }
   exit(0);
   return NCSCC_RC_SUCCESS;
}
