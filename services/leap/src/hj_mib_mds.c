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

  DESCRIPTION:

  This module contains procedures related to Universal MIB operations

..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncsmib_init                   - initial creator puts an NCSMIB_ARG into start state

  ncsmib_sync_request           - PUBLIC :model a MIB request as synchronous; set timer
  mib_sync_response            - PRIVATE:callback function to complete sync paradigm

  ncsmib_timed_request          - PUBLIC :associate timer with asynchronous MIB request
  mib_timed_store              - PRIVATE:store a timer block waiting for response
  mib_timed_find               - PRIVATE:find stored timer block
  mib_timed_remove             - PRIVATE:remove store timer block
  mib_timed_expiry             - PRIVATE:timout function when we waited long enough
  mib_timed_response           - PRIVATE:if here, the answer beat the clock usually.

  ncsmib_make_req_looklike_rsp  - PUBLIC:put rsp data on a copy of original request.
  ncs_key_memcopy               - PUBLIC:make a HEAP copy of a NCS_KEY
  ncsmib_inst_memcopy           - PUBLIC:make a HEAP copy of a MIB instance array.
  ncsmib_oct_memcopy            - PUBLIC:make a HEAP copy of a MIB OctetString
  ncsmib_memcopy                - PUBLIC:make a HEAP copy of a NCSMIB_ARG 
  ncsmib_memfree                - PUBLIC:free a pure HEAP instanse of an NCSMIB_ARG

  ncsmib_encode                 - PUBLIC: put an NCSMIB_ARG in a USRBUF
  ncsmib_decode                 - PUBLIC: get an NCSMIB_ARG out of a USRBUF

******************************************************************************/

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_def.h"
#include "ncssysf_mem.h"

/** Get the key abstractions for this thing...**/

#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncsmib_mem.h"
#include "ncs_mib_pub.h"

/** Changed from ncsencdec.h to below: **/
#include "ncsencdec_pub.h"
#include "ncs_edu.h"

#include "ncsgl_defs.h"

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_encode

  DESCRIPTION:
    Encode in the passed USRBUF the passed in NCSMIB_ARG data.

  RETURNS:
    SUCCESS     - Successful encode
    FAILURE     - something went wrong

*****************************************************************************/

uns32 ncsmib_encode(NCSMIB_ARG*    arg, NCS_UBAID*     uba, uns16  msg_fmt_ver)
{
   uns8* stream;

   if (uba == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (arg == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_16bit(&stream,arg->i_op);
   ncs_enc_claim_space(uba,sizeof(uns16));

   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream,arg->i_tbl_id);
   ncs_enc_claim_space(uba,sizeof(uns32));

   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream,arg->i_xch_id);
   ncs_enc_claim_space(uba,sizeof(uns32));

#if 0
   stream = ncs_enc_reserve_space(uba,sizeof(uns64));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_64bit(&stream,(uns64)arg->i_rsp_fnc);
   ncs_enc_claim_space(uba,sizeof(uns64));
#endif

   if(msg_fmt_ver == 1)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns32));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_32bit(&stream,NCS_PTR_TO_UNS32_CAST(arg->i_rsp_fnc));
       ncs_enc_claim_space(uba,sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns64));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_64bit(&stream,(long)arg->i_rsp_fnc);
       ncs_enc_claim_space(uba,sizeof(uns64));
   }

   if(msg_fmt_ver == 1)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns32));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_32bit(&stream,arg->i_mib_key);
       ncs_enc_claim_space(uba, sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns64));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_64bit(&stream,arg->i_mib_key);
       ncs_enc_claim_space(uba, sizeof(uns64));
   }

#if 0
   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream,arg->i_usr_key);
   ncs_enc_claim_space(uba, sizeof(uns32));
#endif

   if(msg_fmt_ver == 1)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns32));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_32bit(&stream,arg->i_usr_key);
       ncs_enc_claim_space(uba, sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns64));
       if (stream == NULL)
          return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_64bit(&stream,arg->i_usr_key);
       ncs_enc_claim_space(uba, sizeof(uns64));
   }

   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream,arg->i_idx.i_inst_len);
   ncs_enc_claim_space(uba,sizeof(uns32));

   if (ncsmib_inst_encode(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len,uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

    /* added from here -- Mahesh */
   stream = ncs_enc_reserve_space(uba, sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream, arg->i_policy);
   ncs_enc_claim_space(uba,sizeof(uns32));
   /* End -- Mahesh */
      
   if (ncsstack_encode(&(arg->stack),uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
#if 0
   stream = ncs_enc_reserve_space(uba, sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream, arg->i_policy);
   ncs_enc_claim_space(uba,sizeof(uns32));
#endif

   if (m_NCSMIB_ISIT_A_REQ(arg->i_op)) /* if request op. */
   {
      if (NCSCC_RC_SUCCESS != ncsmib_req_encode(arg->i_op, &arg->req, uba))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   else if (m_NCSMIB_ISIT_A_RSP(arg->i_op)) /* if response op. */
   {
      if (NCSCC_RC_SUCCESS != ncsmib_rsp_encode(arg->i_op, &arg->rsp, uba, msg_fmt_ver))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   else
   {
        return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_req_encode

  DESCRIPTION:
    Encode in the passed USRBUF the passed in NCSMIB_REQ data.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful encode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/

uns32   ncsmib_req_encode( NCSMIB_OP     op,
                          NCSMIB_REQ*   req,
                          NCS_UBAID*    uba)
{
   uns8* stream = NULL;

   if ((NULL == req) || (NULL == uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (m_NCSMIB_ISIT_A_RSP(op))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   switch (op)
   {
      case NCSMIB_OP_REQ_GET  :
      case NCSMIB_OP_REQ_GET_INFO:
      case NCSMIB_OP_REQ_NEXT :

         stream = ncs_enc_reserve_space(uba,sizeof(uns32));
         if (stream == NULL)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         ncs_encode_32bit(&stream,req->info.get_req.i_param_id);
         ncs_enc_claim_space(uba,sizeof(uns32));

         break;

      case NCSMIB_OP_REQ_SET  :
      case NCSMIB_OP_REQ_TEST :

         if (ncsmib_param_val_encode(&(req->info.set_req.i_param_val), uba) != NCSCC_RC_SUCCESS)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

         break;

      case NCSMIB_OP_REQ_GETROW  :
      case NCSMIB_OP_REQ_NEXTROW :

         break;

      case NCSMIB_OP_REQ_TESTROW :
      case NCSMIB_OP_REQ_SETROW  :
      case NCSMIB_OP_REQ_SETALLROWS:
      case NCSMIB_OP_REQ_REMOVEROWS:
         {
            NCSMIB_SETROW_REQ* sr_req = &req->info.setrow_req;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(sr_req->i_usrbuf);

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(sr_req->i_usrbuf));
         }

         break;

      case NCSMIB_OP_REQ_MOVEROW :
        {
            NCSMIB_MOVEROW_REQ* mr_req = &req->info.moverow_req;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(mr_req->i_usrbuf);

            stream = ncs_enc_reserve_space(uba,sizeof(MDS_DEST));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            mds_st_encode_mds_dest(&stream,&mr_req->i_move_to);
            ncs_enc_claim_space(uba,sizeof(MDS_DEST));

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(mr_req->i_usrbuf));
        }

        break;

      case NCSMIB_OP_REQ_CLI :
         {
            NCSMIB_CLI_REQ *cli_req = &req->info.cli_req;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(cli_req->i_usrbuf);

           /* Encode i_cmnd_id */
            stream = ncs_enc_reserve_space(uba,sizeof(uns16));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_16bit(&stream,cli_req->i_cmnd_id);
            ncs_enc_claim_space(uba,sizeof(uns16));

           /* Encode i_cmnd_id */
            stream = ncs_enc_reserve_space(uba,sizeof(uns16));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_16bit(&stream,cli_req->i_wild_card);
            ncs_enc_claim_space(uba,sizeof(uns16));

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(cli_req->i_usrbuf));
         }
         break;

      default:
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_req_decode

  DESCRIPTION:
    Decode the passed USRBUF into a NCSMIB_REQ.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful encode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/

uns32 ncsmib_req_decode( NCSMIB_OP     op,
                        NCSMIB_REQ*   req,
                        NCS_UBAID*    uba)
{
   uns8*      stream;
   uns8       space[20];

   if ((NULL == req) || (NULL == uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (m_NCSMIB_ISIT_A_RSP(op))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   switch (op)
   {
      case NCSMIB_OP_REQ_GET  :
      case NCSMIB_OP_REQ_GET_INFO  :
      case NCSMIB_OP_REQ_NEXT :
         {
            NCSMIB_GET_REQ* get_req = &req->info.get_req;

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            get_req->i_param_id = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            break;
         }

      case NCSMIB_OP_REQ_SET  :
      case NCSMIB_OP_REQ_TEST :
         {
            NCSMIB_SET_REQ* set_req = &req->info.set_req;

            if (ncsmib_param_val_decode(&(set_req->i_param_val),NULL,uba) != NCSCC_RC_SUCCESS)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            break;
         }

      case NCSMIB_OP_REQ_GETROW  :
      case NCSMIB_OP_REQ_NEXTROW :
         /* nothing ... */
         break;

      case NCSMIB_OP_REQ_TESTROW:
      case NCSMIB_OP_REQ_SETROW:
      case NCSMIB_OP_REQ_SETALLROWS:
      case NCSMIB_OP_REQ_REMOVEROWS:
         {
            uns32 ubsize;
            uns32 cpy_size;
            NCSMIB_SETROW_REQ* sr_req = &req->info.setrow_req;

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            ubsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            /* KCQ: because the usrbuf is the last thing in the uba,
                    we just copy whatever's left...
            */
            sr_req->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);
            cpy_size = m_MMGR_LINK_DATA_LEN(sr_req->i_usrbuf);
            if (cpy_size != ubsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_dec_skip_space(uba,cpy_size);

            break;
         }

      case NCSMIB_OP_REQ_MOVEROW:
         {
           uns32 ubsize;
           uns32 cpy_size;
           NCSMIB_MOVEROW_REQ* mr_req = &req->info.moverow_req;

           stream = ncs_dec_flatten_space(uba,space,sizeof(MDS_DEST));
           mds_st_decode_mds_dest(&stream, &mr_req->i_move_to);
           ncs_dec_skip_space(uba,sizeof(MDS_DEST));

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            ubsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            /* KCQ: because the usrbuf is the last thing in the uba,
                    we just copy whatever's left...
            */
            mr_req->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);
            cpy_size = m_MMGR_LINK_DATA_LEN(mr_req->i_usrbuf);
            if (cpy_size != ubsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_dec_skip_space(uba,cpy_size);

            break;
         }

      case NCSMIB_OP_REQ_CLI:
         {
            uns32 ubsize;
            uns32 cpy_size;
            NCSMIB_CLI_REQ* cli_req = &req->info.cli_req;

            /* Get i_cmnd_id */
            stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
            cli_req->i_cmnd_id = ncs_decode_16bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns16));

            /* Get i_wild_card */
            stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
            cli_req->i_wild_card = ncs_decode_16bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns16));

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            ubsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            /* KCQ: because the usrbuf is the last thing in the uba,
                    we just copy whatever's left...
            */
            cli_req->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);
            cpy_size = m_MMGR_LINK_DATA_LEN(cli_req->i_usrbuf);
            if (cpy_size != ubsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_dec_skip_space(uba,cpy_size);

            break;
         }

      default:
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   };

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_decode

  DESCRIPTION:
    Decode the passed USRBUF into a NCSMIB_ARG.

  RETURNS:
    NULL        - The decode did not go so well
    value       - pointer to a HEAP version of NCSMIB_ARG.

*****************************************************************************/

NCSMIB_ARG* ncsmib_decode( NCS_UBAID*    uba, uns16 msg_fmt_ver)
{
   NCSMIB_ARG* new_arg;
   uns8*      stream;
   uns8       space[sizeof(NCS_KEY)]; 
   uns32      status = NCSCC_RC_SUCCESS;

   if (uba == NULL)
   {
      m_LEAP_DBG_SINK(0);
      return NULL;
   }


   if ((new_arg = m_MMGR_ALLOC_NCSMIB_ARG) == NULL)
   {
      m_LEAP_DBG_SINK(0);
      return NULL;
   }

   m_NCS_MEMSET(new_arg,0,sizeof(NCSMIB_ARG));

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   new_arg->i_op = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_tbl_id = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_xch_id = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));

 #if 0
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns64));
   new_arg->i_rsp_fnc = (NCSMIB_RSP_FNC)ncs_decode_64bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns64));
 #endif

   if(msg_fmt_ver == 1)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
       new_arg->i_rsp_fnc = (NCSMIB_RSP_FNC)NCS_INT32_TO_PTR_CAST(ncs_decode_32bit(&stream));
       ncs_dec_skip_space(uba,sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns64));
       new_arg->i_rsp_fnc = (NCSMIB_RSP_FNC)(long)ncs_decode_64bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns64));
   }

 #if 0
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_mib_key = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));
 #endif

   if(msg_fmt_ver == 1)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
       new_arg->i_mib_key = ncs_decode_32bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns64));
       new_arg->i_mib_key = ncs_decode_64bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns64));
   }

 #if 0
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_usr_key = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));
 #endif

   if(msg_fmt_ver == 1)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
       new_arg->i_usr_key = ncs_decode_32bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns32));
   }
   else if(msg_fmt_ver == 2)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns64));
       new_arg->i_usr_key = ncs_decode_64bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns64));
   }

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_idx.i_inst_len = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));

   if (ncsmib_inst_decode((uns32**)&new_arg->i_idx.i_inst_ids,new_arg->i_idx.i_inst_len,uba) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_NCSMIB_ARG(new_arg);
      m_LEAP_DBG_SINK(0);
      return NULL;
   }

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   new_arg->i_policy = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));

   if (ncsstack_decode(&(new_arg->stack),uba) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_NCSMIB_ARG(new_arg);
      m_LEAP_DBG_SINK(0);
      return NULL;
   }

   /* Request and Response Ops. */
   if (m_NCSMIB_ISIT_A_REQ(new_arg->i_op))
      status = ncsmib_req_decode(new_arg->i_op, &new_arg->req, uba);
   else if (m_NCSMIB_ISIT_A_RSP(new_arg->i_op))
      status = ncsmib_rsp_decode(new_arg->i_op, &new_arg->rsp, uba, msg_fmt_ver);
   else
      status = NCSCC_RC_FAILURE;

   if (NCSCC_RC_SUCCESS != status)
   {
      m_MMGR_FREE_NCSMIB_ARG(new_arg);
      m_LEAP_DBG_SINK(0);
      return NULL;
   }

   return new_arg;
}
