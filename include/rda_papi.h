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

  $Header:  $

  MODULE NAME: pcs_rda_papi.h

..............................................................................

  DESCRIPTION:  This file declares the interface API for RDF. 

******************************************************************************
*/

#ifndef PCS_RDA_PAPI_H
#define PCS_RDA_PAPI_H

/*
** Includes
*/
#include <stdio.h>
#include "ncsgl_defs.h"  /* uns32 */

/*
** Error strings (defined in pcs_rda_test.c)
*/
extern char *pcsrda_err_str[];
extern char *pcsrda_role_str[];

/*
** Return/error codes
*/
typedef enum
{
    PCSRDA_RC_SUCCESS,
    PCSRDA_RC_TIMEOUT,
    PCSRDA_RC_INVALID_PARAMETER,
    PCSRDA_RC_LIB_LOAD_FAILED,
    PCSRDA_RC_LIB_NOT_INITIALIZED,
    PCSRDA_RC_LIB_NOT_FOUND,
    PCSRDA_RC_LIB_SYM_FAILED,
    PCSRDA_RC_IPC_CREATE_FAILED,
    PCSRDA_RC_IPC_CONNECT_FAILED,
    PCSRDA_RC_IPC_SEND_FAILED,
    PCSRDA_RC_IPC_RECV_FAILED,
    PCSRDA_RC_TASK_SPAWN_FAILED,
    PCSRDA_RC_MEM_ALLOC_FAILED,
    PCSRDA_RC_CALLBACK_REG_FAILED,
    PCSRDA_RC_CALLBACK_ALREADY_REGD,
    PCSRDA_RC_ROLE_GET_FAILED,
    PCSRDA_RC_ROLE_SET_FAILED,
    PCSRDA_RC_AVD_HB_ERR_FAILED,
    PCSRDA_RC_AVND_HB_ERR_FAILED,
    PCSRDA_RC_LEAP_INIT_FAILED,
    PCSRDA_RC_FATAL_IPC_CONNECTION_LOST

}PCSRDA_RETURN_CODE;


/*
** Structure declarations
*/
typedef enum
{ 
    PCS_RDA_LIB_INIT,
    PCS_RDA_LIB_DESTROY,
    PCS_RDA_REGISTER_CALLBACK,
    PCS_RDA_UNREGISTER_CALLBACK,
    PCS_RDA_SET_ROLE,
    PCS_RDA_GET_ROLE,
    PCS_RDA_AVD_HB_ERR,
    PCS_RDA_AVND_HB_ERR

} PCS_RDA_REQ_TYPE;

typedef enum
{ 
    PCS_RDA_ACTIVE,
    PCS_RDA_STANDBY,
    PCS_RDA_QUIESCED,
    PCS_RDA_ASSERTING,
    PCS_RDA_YIELDING,
    PCS_RDA_UNDEFINED

} PCS_RDA_ROLE;

typedef enum
{ 
    PCS_RDA_NODE_RESET_CMD = 1,
    PCS_RDA_CMD_TYPE_MAX
}PCS_RDA_CMD_TYPE;

typedef struct  
{ 
    uns32 shelf_id;
    uns32 slot_id;
}PCS_RDA_NODE_RESET_INFO;

typedef struct  
{ 
    PCS_RDA_CMD_TYPE req_type;
    union
    {
       PCS_RDA_NODE_RESET_INFO node_reset_info;
    }info;
}PCS_RDA_CMD;

typedef void (* PCS_RDA_CB_PTR)(uns32 callback_handle, PCS_RDA_ROLE role, PCSRDA_RETURN_CODE error_code, PCS_RDA_CMD cmd);

typedef struct  
{ 
    PCS_RDA_REQ_TYPE req_type;
    uns32            callback_handle;
    union
    {
        PCS_RDA_CB_PTR  call_back;  
        PCS_RDA_ROLE    io_role; 
        uns32           phy_slot_id;

    } info;

}PCS_RDA_REQ;


/*
** API declarations
*/
int pcs_rda_request(PCS_RDA_REQ *pcs_rda_req);


#endif /* PCS_RDA_PAPI_H */

