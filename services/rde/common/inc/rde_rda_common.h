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

  MODULE NAME: rde_rda_common.h 

..............................................................................

  DESCRIPTION:  This file declares the common definitions for RDE and RDA

******************************************************************************
*/

#ifndef RDE_RDA_COMMON_H
#define RDE_RDA_COMMON_H

/*
** includes
*/


/*
** Return/error codes
*/

/*
**    Name of this particular RDA socket implementation
**
*/
#define  RDE_RDA_SOCK_NAME    "/tmp/rde_rda_socket_interface"

/*
** Protocal message primitives for RDE-RDA interaction
*/
typedef enum
{
    RDE_RDA_UNKNOWN,
    RDE_RDA_GET_ROLE_REQ,
    RDE_RDA_GET_ROLE_RES,
    RDE_RDA_SET_ROLE_REQ,
    RDE_RDA_SET_ROLE_ACK,
    RDE_RDA_SET_ROLE_NACK,
    RDE_RDA_AVD_HB_ERR_REQ,
    RDE_RDA_AVD_HB_ERR_NACK,
    RDE_RDA_AVD_HB_ERR_ACK,
    RDE_RDA_AVND_HB_ERR_REQ,
    RDE_RDA_AVND_HB_ERR_NACK,
    RDE_RDA_AVND_HB_ERR_ACK,
    RDE_RDA_REG_CB_REQ,
    RDE_RDA_REG_CB_ACK,
    RDE_RDA_REG_CB_NACK,
    RDE_RDA_DISCONNECT_REQ,
    RDE_RDA_HA_ROLE,
    RDE_RDA_SCB_SWOVER,
    RDE_RDA_NODE_RESET_CMD
    
}RDE_RDA_CMD_TYPE;


/*
** PDU (Protocol Data Units) for the above 
*/


#endif /* RDE_RDA_COMMON_H */

