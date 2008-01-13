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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/base/products/mab/mac/mac_pvt.c 22    3/28/01 2:12p Questk $


..............................................................................

  DESCRIPTION:

  This has the private function of the MIB Access Client, a subcomponent of the
  MIB Access Broker (MAB) subystem. This includes:

  mac_do_evts      Infinite loop services the passed SYSF_MBX
  mac_do_evt       Master Dispatch function and services off MAC work queue
  mac_mib_response MAC NCSMIB_ARG callback function, which in turn calls back
                    the original invoking client.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/


#if (NCS_MAB == 1)
#include "mac.h"

/*****************************************************************************

  PROCEDURE NAME: mac_do_evts

  DESCRIPTION:    The loop function that services MAB messages arriving
                  at the MAC.

*****************************************************************************/

void
mac_do_evts( SYSF_MBX*  mbx)
{
  MAB_MSG   *msg;
  
  while ((msg = m_MAC_RCV_MSG(mbx, msg)) != NULL)
  {
    m_MAB_DBG_TRACE("\nmac_do_evt():entered.");
    if (mac_do_evt(msg) == NCSCC_RC_DISABLED)
    {
      m_MAB_DBG_TRACE("\nmac_do_evt():left.");
      return;
    }
    m_MAB_DBG_TRACE("\nmac_do_evt():left.");
  }
}

/*****************************************************************************

  PROCEDURE NAME: mac_do_evt

  DESCRIPTION:    Do the dispatch deed.
                      
*****************************************************************************/

uns32
mac_do_evt( MAB_MSG* msg)
{  
  switch (msg->op)
  {
  case MAB_MAC_RSP_HDLR:
    return mac_mib_response(msg);
    
  case MAB_MAC_TRAP_HDLR:
    break; /* SMM need a plan!!! */

  case MAB_MAC_PLAYBACK_START:
      return mac_playback_start(msg);

  case MAB_MAC_PLAYBACK_END:
      return mac_playback_end(msg);

  case MAB_MAC_DESTROY:
      m_MMGR_FREE_MAB_MSG(msg);
      return maclib_mac_destroy(NULL);

  default:
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_RCVD_INVALID_OP);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    break;
  }
  return NCSCC_RC_SUCCESS; /* SMM revisit this return.. */
}

/*****************************************************************************

  PROCEDURE NAME: mac_mib_response

  DESCRIPTION:    An answer has arrived back from another part of MAB (MAS or
                  OAC). Now we have to get this back to the initiating client.
                  Pop the NCSMIB_ARG stack and re-plant invokers values. Find
                  client's callback and invoke it.

*****************************************************************************/

uns32 mac_mib_response(MAB_MSG* msg)
{
#if 0
  MAC_INST*    inst;
#endif
  NCSMIB_ARG*   rsp;

#if 0
  m_MAC_LK_INIT;
#endif

  m_MAB_DBG_TRACE("\nmac_mib_response():entered.");

#if 0
  inst = (MAC_INST*)msg->yr_hdl;

  m_MAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_MAC_LOCKED,&inst->lock);
#endif

  rsp = msg->data.data.snmp;

  if (rsp == NULL)
    {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAC_NULL_MIB_RSP_RCVD);
#if 0
    m_MAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
#endif
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

  if(! m_NCSMIB_ISIT_A_RSP(rsp->i_op))
    {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAC_INVALID_MIB_RSP_RCVD);
#if 0
    m_MAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
#endif
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

  /* put good values for usr_key, xchg_id and rsp_fnc in the response */

  if(rsp->i_usr_key != 0)
    rsp->i_usr_key = 0;

  /* recover the information about the original MIB request sender */
    {
    NCS_SE*        se;
    uns8*         stream;

    if((se = ncsstack_pop(&rsp->stack)) == NULL)
    {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_POP_ORIG_FAILED);
#if 0
      m_MAC_UNLK(&inst->lock);
#endif
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }   

    stream = m_NCSSTACK_SPACE(se);

    if(se->type != NCS_SE_TYPE_MIB_ORIG)
    {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAC_POP_NOT_MIB_ORIG);
#if 0
      m_MAC_UNLK(&inst->lock);
#endif
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    rsp->i_usr_key = ncs_decode_64bit(&stream);
    if(4 == sizeof(void *))
       rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_32bit(&stream);
    else if(8 == sizeof(void *))
       rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_64bit(&stream);
    }


#if 0
  m_MAC_UNLK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
#endif

  m_MMGR_FREE_MAB_MSG(msg);

  rsp->i_rsp_fnc(rsp);
  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAC_USR_RSP_FNC_RET);

  rsp->i_usr_key = 0;
  if(ncsmib_arg_free_resources(rsp,FALSE) != NCSCC_RC_SUCCESS)
  {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAC_MIBARG_FREE_FAILED);
        m_MAB_DBG_TRACE("\nmac_mib_response():left.");
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  m_MAB_DBG_TRACE("\nmac_mib_response():left.");
  
  return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 mac_playback_start
 **********************************************************************/
uns32 mac_playback_start(MAB_MSG * msg)
{
  MAC_INST*    inst;

  m_MAC_LK_INIT;

  m_MAB_DBG_TRACE("\nmac_playback_start():entered.");

  inst = (MAC_INST*)msg->yr_hdl;

  if (inst == NULL)
  {
      m_LOG_MAB_NO_CB("mac_playback_start()"); 
      m_MAB_DBG_TRACE("\nmac_playback_start():left.");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  m_MAC_LK(&inst->lock, NCS_LOCK_WRITE);
  m_LOG_MAB_LOCK(MAB_LK_MAC_LOCKED,&inst->lock);

  inst->playback_session = TRUE;
  if(inst->plbck_tbl_list != NULL)
  {
     m_MMGR_FREE_MAC_TBL_BUFR(inst->plbck_tbl_list);
  }
  inst->plbck_tbl_list = msg->data.data.plbck_start.o_tbl_list;

  m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
  m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);

  m_MMGR_FREE_MAB_MSG(msg);
  m_MAB_DBG_TRACE("\nmac_playback_start():left.");

  return NCSCC_RC_SUCCESS;
}


/***********************************************************************
 mac_playback_end
 **********************************************************************/
uns32 mac_playback_end(MAB_MSG * msg)
{
  MAC_INST*    inst;

  m_MAC_LK_INIT;

  m_MAB_DBG_TRACE("\nmac_playback_end():entered.");

  inst = (MAC_INST*)msg->yr_hdl;

  if (inst == NULL)
  {
      m_LOG_MAB_NO_CB("mac_playback_end()"); 
      m_MAB_DBG_TRACE("\nmac_playback_end():left.");
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  m_MAC_LK(&inst->lock, NCS_LOCK_WRITE);
  m_LOG_MAB_LOCK(MAB_LK_MAC_LOCKED,&inst->lock);

  inst->playback_session = FALSE;
  if(inst->plbck_tbl_list != NULL)
  {
     m_MMGR_FREE_MAC_TBL_BUFR(inst->plbck_tbl_list);
     inst->plbck_tbl_list = NULL;
  }

  m_MAC_UNLK(&inst->lock, NCS_LOCK_WRITE);
  m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);

  m_MMGR_FREE_MAB_MSG(msg);
  m_MAB_DBG_TRACE("\nmac_playback_end():left.");

  return NCSCC_RC_SUCCESS;
}

#endif /* (NCS_MAB == 1) */
