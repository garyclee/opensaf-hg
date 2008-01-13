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

  Data Link Interface Definitions

******************************************************************************
*/


#ifndef NCS_DL_H
#define NCS_DL_H

#include "ncsgl_defs.h"
#include "ncsusrbuf.h"
#include "ncs_scktprm.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct ncs_dl_indication_info_tag;
struct ncs_dl_request_info_tag;

#define SYSF_DL_MAX_LAYER_HANDLE_LEN (sizeof(void *))
#define SYSF_DL_MAX_CLIENT_HANDLE_LEN (sizeof(void *))

struct ncs_dl_request_info_tag;
EXTERN_C uns32 sysf_dl_request (struct ncs_dl_request_info_tag *dlr);

typedef enum 
{
   NCS_DL_TYPE_ETHERNET,
   NCS_DL_TYPE_MAX = NCS_DL_TYPE_ETHERNET

} NCS_DL_TYPE;


typedef struct ncs_l2_addr_tag
{
   NCS_DL_TYPE   dl_type;          /* NCS_L2_TYPE */
   union
   {
      uns8 eth[6];                /* Ethernet address */
      uns32  ppp_unit_num;        /* PPP unit number */
                                  /* Need entries here for supported DL types */
      void* generic;              /* For all other dl types */
   } data;
} NCS_L2_ADDR;



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                  Data Link Interface Definition

 Note this interface defines the following:
 Indications go from the DL Protocol Stack to the DL User.
 Requests go from the DL User to the DL Protocol Stack.

  (1) NCS_DL_TYPE        This enum defines the type of DL connections 

  (2) NCS_DL_INDICATIONS This enum defines the indications that 
                        the DL Stack can send to the DL User.

  (3) NCS_DL_INDICATION  Function prototype for received DL data and 
                        control indications.  The DL USER must register 
                        a function matching this prototype with the DL 
                        protocol stack in the NCS_DL_CTRL_REQUEST OPEN 
                        primitive. 

  (4) NCS_DL_INDICATION_INFO This structure defines the information passed 
                            across the interface, for each inication type.


  (5) NCS_DL_REQUESTS    This enum defines the requests that an DL User can make.
  
  (6) NCS_DL_REQUEST     Function prototype for DL Control requests and 
                        for sending DL data.  The target system must 
                        register a function matching this prototype with 
                        the DL User prior to the DL connections being open.

  (7) NCS_DL_REQUEST_INFO  This structure defines the information passed 
                          across the interface, for request type.
 

  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/



/*****************************************************************************

  ENUM NAME:            NCS_DL_INDICATIONS

  DESCRIPTION:          This enum defines the types of DL Indications from the
                  DL protocol stack to the DL user.


  VALUES:
   NCS_DL_CTRL_INDICATION_CONNECT:  The specified DL connection has become 
                                   operational.  

   NCS_DL_CTRL_INDICATION_DISCONNECT:    The specified DL connection has
   been shutdown, or disabled.  

   NCS_DL_CTRL_INDICATION_ERROR:     The specified DL connection had
   an error and been shutdown.  No further DL messages can be sent or 
   received over this connection.  

   NCS_DL_DATA_INDICATION_RECV_DATA: The provided data has been received
   over this connection.


  NOTES:


*****************************************************************************/

typedef enum
{

   NCS_DL_INDICATION_MIN,

   NCS_DL_CTRL_INDICATION_MIN         = NCS_DL_INDICATION_MIN,
   NCS_DL_CTRL_INDICATION_CONNECT     = NCS_DL_CTRL_INDICATION_MIN,
   NCS_DL_CTRL_INDICATION_DISCONNECT,
   NCS_DL_CTRL_INDICATION_ERROR,
   NCS_DL_CTRL_INDICATION_MAX         = NCS_DL_CTRL_INDICATION_ERROR,

   NCS_DL_DATA_INDICATION_MIN,
   NCS_DL_DATA_INDICATION_RECV_DATA   = NCS_DL_DATA_INDICATION_MIN,
   NCS_DL_DATA_INDICATION_MAX         = NCS_DL_DATA_INDICATION_RECV_DATA,

   NCS_DL_INDICATION_MAX              = NCS_DL_DATA_INDICATION_MAX

} NCS_DL_INDICATIONS;



/*****************************************************************************

  ENUM NAME:            NCS_DL_REQUESTS

  DESCRIPTION:          This enum defines the type of requests that the DL 
                  User can make of the DL protocol stack.


  VALUES:
   NCS_DL_CTRL_REQUEST_BIND:         Bind client software to the DL 
   protocol stack.  This request provides the client's data and control
   indication handlers, and returns a handle the client can use in future
   open/close connection and send data requests.

   NCS_DL_CTRL_REQUEST_UNBIND:           Unbind client software from the DL
   protocol stack.  This client will nolonger use the DL stack.

   NCS_DL_CTRL_REQUEST_OPEN:         Open a new connection to a destination.
   
   NCS_DL_CTRL_REQUEST_CLOSE:            Close an existing Raw DL connection.
   No further DL messages can be sent or received over this connection.  

   NCS_DL_DATA_REQUEST_SEND_DATA:       Send Raw Ethernet data over an
   existing connection.


  NOTES:


*****************************************************************************/

typedef enum
{

   NCS_DL_REQUEST_MIN,

   NCS_DL_CTRL_REQUEST_MIN       = NCS_DL_REQUEST_MIN,
   NCS_DL_CTRL_REQUEST_BIND      = NCS_DL_CTRL_REQUEST_MIN,
   NCS_DL_CTRL_REQUEST_UNBIND,
   NCS_DL_CTRL_REQUEST_OPEN,
   NCS_DL_CTRL_REQUEST_MULTICAST_JOIN,
   NCS_DL_CTRL_REQUEST_MULTICAST_LEAVE,
   /* Fix for IR10315 */
   NCS_DL_CTRL_REQUEST_GET_ETH_ADDR,
   NCS_DL_CTRL_REQUEST_CLOSE,
   NCS_DL_CTRL_REQUEST_MAX       = NCS_DL_CTRL_REQUEST_CLOSE,

   NCS_DL_DATA_REQUEST_MIN,
   NCS_DL_DATA_REQUEST_SEND_DATA = NCS_DL_DATA_REQUEST_MIN,
   NCS_DL_REQUEST_MAX            = NCS_DL_DATA_REQUEST_SEND_DATA
} NCS_DL_REQUESTS;



/*****************************************************************************

  ENUM NAME:            NCS_DL_TYPE

  DESCRIPTION:    This enum defines the type of DL connections available.
                  NCS_DL_TYPE_DGRAM and NCS_DL_TYPE_RAW
                  are the only required types. It is provided in the 
                  NCS_DL_CTRL_REQUEST_OPEN to specify the type of DL connection.

  VALUES:
   
   NCS_DL_TYPE_DGRAM:    This connection is same as RAW, but only received
                        specified protocol (example IP, ARP)
   NCS_DL_TYPE_RAW:      This connection will use another (specified L3 protocol).


  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_L2_SOCK_TYPE_DGRAM,
   NCS_L2_SOCK_TYPE_RAW,
   NCS_L2_SOCK_TYPE_MAX = NCS_L2_SOCK_TYPE_RAW
} NCS_L2_SOCK_TYPE;


/*****************************************************************************

  ENUM NAME:            NCSSOCK_ERROR_TYPES

  DESCRIPTION:          This enum defines the type of common socket error
                        indications.  Used both by the DataLink Layer and 
                        the IP Layer.

  VALUES:

   NCSSOCK_ERROR_TYPES_NO_ERROR:        No error occurred. 

   NCSSOCK_ERROR_TYPES_DL_DOWN:         The Layer 2 interface is not operating, 
                                        unavailable, or not yet initialized.

   NCSSOCK_ERROR_TYPES_IP_DOWN:         The IP Protocol Stack is not operating, 
                                        unavailable, or not yet initialized.

   NCSSOCK_ERROR_TYPES_NO_MEM:          Memory resources were not available to
                                        fulfill the request.

   NCSSOCK_ERROR_TYPES_FRAME_TOO_LARGE: The frame was too large to send
                                        (bigger than MTU)

   NCSSOCK_ERROR_TYPES_CONN_DOWN:       The specified connection is not 
                                        ready - it has been aborted, reset 
                                        or is not yet connected.

   NCSSOCK_ERROR_TYPES_CONN_UNKNOWN:    The specified connection is unknown.

   NCSSOCK_ERROR_TYPES_CONN_UNSUPPORTED:A connection orientated request was 
                                        made of a UDP or Raw connection, or
                                        a datagram oriented request was made
                                        of a stream (tcp) connection. 

   NCSSOCK_ERROR_TYPES_UNKNOWN:         Unspecified error condition occurred.

   NCSSOCK_ERROR_TYPES_SND_GIVEUP:      Send operation failed because the 
                                        max retry count for blocking sends is
                                        reached or the send request can't be
                                        queued for queueing sends.

  NOTES:

*****************************************************************************/
typedef enum
{
   NCSSOCK_ERROR_TYPES_NO_ERROR,
   NCSSOCK_ERROR_TYPES_DL_DOWN,
   NCSSOCK_ERROR_TYPES_IP_DOWN,
   NCSSOCK_ERROR_TYPES_NO_MEM,
   NCSSOCK_ERROR_TYPES_FRAME_TOO_LARGE,
   NCSSOCK_ERROR_TYPES_CONN_DOWN,
   NCSSOCK_ERROR_TYPES_CONN_UNKNOWN,
   NCSSOCK_ERROR_TYPES_CONN_UNSUPPORTED,
   NCSSOCK_ERROR_TYPES_UNKNOWN,           
   NCSSOCK_ERROR_TYPES_SND_GIVEUP,
   NCSSOCK_ERROR_TYPES_WOULDBLOCK,
   NCSSOCK_ERROR_TYPES_MAX = NCSSOCK_ERROR_TYPES_WOULDBLOCK
} NCSSOCK_ERROR_TYPES;


/*****************************************************************************

  ENUM NAME:            NCS_DL_ERROR

  DESCRIPTION:          This enum defines the type of DL error indications.

  VALUES:

   NCS_DL_ERROR_NO_ERROR:            No error occurred. 

   NCS_DL_ERROR_DL_DOWN:         The Layer 2 interface is not operating, 
   unavailable, or not yet initialized.

   NCS_DL_ERROR_NO_MEM:              Memory resources were not available to
   fulfill the request.

   NCS_DL_ERROR_FRAME_TOO_LARGE: the frame was too large to send (bigger than MTU)

   NCS_DL_ERROR_CONN_DOWN:           The specified connection is not ready - it
   has been aborted, reset or is not yet connection.

   NCS_DL_ERROR_CONN_UNKNOWN:        The specified connection is unknown.

   NCS_DL_ERROR_UNKNOWN:         Unspecified error condition occurred.


  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_DL_ERROR_NO_ERROR = NCSSOCK_ERROR_TYPES_NO_ERROR,
   NCS_DL_ERROR_DL_DOWN = NCSSOCK_ERROR_TYPES_DL_DOWN,
   NCS_DL_ERROR_NO_MEM = NCSSOCK_ERROR_TYPES_NO_MEM,
   NCS_DL_ERROR_FRAME_TOO_LARGE = NCSSOCK_ERROR_TYPES_FRAME_TOO_LARGE,
   NCS_DL_ERROR_CONN_DOWN = NCSSOCK_ERROR_TYPES_CONN_DOWN,
   NCS_DL_ERROR_CONN_UNKNOWN = NCSSOCK_ERROR_TYPES_CONN_UNKNOWN,
   NCS_DL_ERROR_UNKNOWN = NCSSOCK_ERROR_TYPES_UNKNOWN,
   NCS_DL_ERROR_MAX = NCS_DL_ERROR_UNKNOWN
} NCS_DL_ERROR;



/*****************************************************************************
 * STRUCTURE NAME:     NCS_DL_LAYER_HANDLE
 *
 * DESCRIPTION:        This structure represents the handle given to 
 *                     the User by the target system at genesis to identify 
 *                     the DL protocol stack.
 *
 * NOTES:
 *   A 'len' field of zero indicates 'no handle'
 *
 *   Sizes of the 'handle' array and use of the 'len' field is a target-system
 *   design issue.
 *
 *   The symbol SYSF_DL_MAX_LAYER_HANDLE_SIZE is defined in a customer-
 *   modifiable file.  Typically it might be defined to be sizeof(void*)
 *   if the actual handle value is a pointer.
 *
 ****************************************************************************/
typedef struct ncs_dl_layer_handle_tag
{
   unsigned int len;     /* actual size of handle.  '0' means none. */
   uns8         data[SYSF_DL_MAX_LAYER_HANDLE_LEN];
} NCS_DL_LAYER_HANDLE;



/*****************************************************************************
 * STRUCTURE NAME:     NCS_DL_CLIENT_HANDLE
 *
 * DESCRIPTION:        This structure represents the handle returned to 
 *                     the User by the target system by the  
 *                     NCS_DL_CTRL_REQUEST_BIND request.
 *
 * NOTES:
 *   A 'len' field of zero indicates 'no handle'
 *
 *   Sizes of the 'handle' array and use of the 'len' field is a target-system
 *   design issue.
 *
 *   The symbol SYSF_DL_MAX_CLIENT_HANDLE_SIZE is defined in a customer-
 *   modifiable file.  Typically it might be defined to be sizeof(void*)
 *   if the actual handle value is a pointer.
 *
 ****************************************************************************/
typedef struct ncs_dl_client_handle_tag
{
   unsigned int len;     /* actual size of handle.  '0' means none. */
   uns8         data[SYSF_DL_MAX_CLIENT_HANDLE_LEN];
} NCS_DL_CLIENT_HANDLE;


/*****************************************************************************

  Macro NAME:           NCS_DL_INDICATION

  DESCRIPTION:          A function matching this prototype must be provided 
                  when a DL user opens a new connection with the
                  NCS_DL_CTRL_REQUEST OPEN primitve.  This function 
                  will be used by the DL protocol stack deliver received 
                  DL data and control indications to this DL User.

  ARGUMENTS:
   ip_indication:       Pointer to a NCS_DL_INDICATION_INFO information block.

  RETURNS: 
   NCSCC_RC_SUCCESS: Success.  
   NCSCC_RC_FAILURE: Failure. 
  NOTES:

*****************************************************************************/

typedef uns32 (*NCS_DL_INDICATION) (struct ncs_dl_indication_info_tag *dl_indication);



/*****************************************************************************

  Macro NAME:           NCS_DL_REQUEST

  DESCRIPTION:          A function matching this prototype must be provided 
                  by the DL protocol stack prior to an DL user opening
                  any connections.  This function will be used by the DL
                  user to send DL control and data requests to the DL
                  stack.

  ARGUMENTS:
   dl_request:          Pointer to a NCS_DL_REQUEST_INFO information block.

  RETURNS: 
   NCSCC_RC_SUCCESS: Success.  
   NCSCC_RC_FAILURE: Failure. 
  NOTES:

*****************************************************************************/

typedef uns32 (*NCS_DL_REQUEST) (struct ncs_dl_request_info_tag *dl_request);



/*****************************************************************************

  ENUM NAME:            NCS_DL_INDICATION_INFO

  DESCRIPTION:          This structure defines control block used in the
                  H&J DL interface.

  NOTES:
  This structure contains a union with a separate element for each primitive
  request type.  In this way, each primitive clearly defines what fields are
  required inputs and outputs.  The fields that exist outside of the union
  are required for all request types.

  Fields prefixed with "i_" are inputs to the request.
  Fields prefixed with "o_" are output from successfully completed requests.


*****************************************************************************/

typedef struct ncs_dl_indication_info_tag
{

   NCS_DL_INDICATIONS       i_indication;

   /* DL protocol stack's opaque handle for the connection */
   NCSCONTEXT               i_dl_handle;

   /* The LMS handle */
   NCSCONTEXT               i_user_handle;

   /* DL User's opaque handle for the connection */
   NCSCONTEXT               i_user_connection_handle;

   /* This is a union of each indication type. */
   union
   {
      /* This is a union of each control plane indication type */
      union
      {

         /* This DL Connection is now Active */
         struct
         {
            /* The DL User's DL Address */
            NCS_L2_ADDR      i_local_addr;

            /* The destination's DL Address */
            NCS_L2_ADDR      i_remote_addr;

            /* Interface index to use for connection. */
            uns32          i_if_index;

         } connect;


         /* This DL Connection is shutdown and disabled */
         struct
         {
            /* The DL User's DL Address */
            NCS_L2_ADDR      i_local_addr;

            /* The destination's DL Address */
            NCS_L2_ADDR      i_remote_addr;

            /* Interface index to use for connection. */
            uns32          i_if_index;

         } disconnect;

         struct
         {
            /* The DL User's DL Address */
            NCS_L2_ADDR      i_local_addr;

            /* The destination's DL Address */
            NCS_L2_ADDR      i_remote_addr;

            /* Interface index to use for connection. */
            uns32          i_if_index;

            /* The error code being reported, one of NCS_DL_ERROR enum */
            NCS_DL_ERROR       i_error;

         } error;

      } ctrl;

      /* This is a union of each data plane indication type */
      union
      {
         struct
         {
            /* The DL User's DL Address */
            NCS_L2_ADDR      i_local_addr;

            /* The destination's DL Address */
            NCS_L2_ADDR      i_remote_addr;

            /* Interface index to use for connection. */
            uns32          i_if_index;

            /* The received data frame, as a pointer to a USRBUF chain */
            USRBUF            *i_usrbuf;

         } recv_data;
      } data;
   } info;

} NCS_DL_INDICATION_INFO;





/*****************************************************************************

  ENUM NAME:            NCS_DL_REQUEST_INFO

  DESCRIPTION:          This structure defines control block used in the
                        DL interface.

  FIELDS:
   i_dl_layer_handle:   Handle to DL protocol stack handle, if available.
   i_dl_type:           Type of DL User, see NCS_DL_TYPE, either DGRAM or RAW. 
   i_dl_handle:     DL protocol stack's opaque handle for the DL user instance 
                  (as returned by the NCS_DL_CTRL_REQUEST OPEN primitive.
   i_user_handle:       Opaque handle for this DL user, as provided by DL user in
                  the NCS_DL_CTRL_REQUEST OPEN primitive invocation.

  NOTES:


*****************************************************************************/

typedef struct ncs_dl_request_info_tag
{

   /* One of the NCS_DL_REQUESTS enum specify the operation type. */
   NCS_DL_REQUESTS                i_request;

   NCSCONTEXT                     i_user_handle;

   /* Extended error information  */
   NCS_DL_ERROR                   o_xerror;

   /* This is a union of each request type. */
   union
   {

      /* This is a union of each control plane request type */
      union
      {
         /* Bind DL Client to DL protocol stack. */
         struct
         {
            /* Handle to DL protocol stack instance. */
            NCS_DL_LAYER_HANDLE      i_dl_layer_handle;

            /* Return DL stack handle for this DL Client */
            NCS_DL_CLIENT_HANDLE     o_dl_client_handle;

         } bind;

         /* Unbind DL Client to DL protocol stack. */
         struct
         {
            /* Handle to DL protocol stack instance. */
            NCS_DL_LAYER_HANDLE      i_dl_layer_handle;

            /* DL stack handle for this DL Client (returned by bind request) */
            NCS_DL_CLIENT_HANDLE     i_dl_client_handle;
         } unbind;

         struct
         {
            /* DL stack handle for this DL Client (returned by bind request) */
            NCS_DL_CLIENT_HANDLE  i_dl_client_handle;

            /* Type of DL connection, one of NCS_DL_TYPE enum. */
            NCS_L2_SOCK_TYPE      i_dl_type;

            /* DL Protocol, if NCS_DL_TYPE_RAW */
            uns16                 i_dl_protocol;

            /* DL User's opaque handle for this connection */
            NCSCONTEXT            i_user_connection_handle;
            
            /* The DL User's L2 Address */
            NCS_L2_ADDR           i_local_addr;

            /* The destination's L2 Address. 
               If this is set to all zeroes, then packets from any
               L2 address is allowed to be received over this
               "connection". 
               If this is non-zero, then packets only from the
               L2 address specified below will be received */
             NCS_L2_ADDR           i_remote_addr;

            /* Interface index to use for connection. 
               If this is zero, this field is ignored. Instead
               "i_if_name" is used as the interface */
             uns32                 i_if_index;

            /* Fix for IR10318 */
            /* Interface name to use for connection. Used if 
               "i_if_index is zero */
            uns8                  i_if_name[NCS_IF_NAMESIZE];

            /* Client Data indication handler, matching NCS_DL_INDICATION prototype */
            NCS_DL_INDICATION     i_dl_data_indication;

            /* Client Control indication handler, matching NCS_DL_INDICATION prototype */
            NCS_DL_INDICATION     i_dl_ctrl_indication;

            /* If 1, use BIND to Device option - Raw Only */
            NCS_BOOL              i_bindtodevice;
                  
            /* Returned DL protocol stack's opaque handle for this DL connection */
            NCSCONTEXT            o_dl_handle;

         } open;

         struct
         {

            NCSCONTEXT            i_dl_handle;

         } close;

         struct
         {
            /* DL protocol stack's opaque handle for this DL connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_dl_handle;

            /* The Multicast Group Address */
            NCS_L2_ADDR         i_multicast_addr;

         } multicastjoin;

         struct
         {
            /* DL protocol stack's opaque handle for this DL connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_dl_handle;

            /* The Multicast Group Address */
            NCS_L2_ADDR         i_multicast_addr;

        } multicastleave;
        /* Fix for IR10315 */ 
        struct
        {
             /* Input/Output ifindex - Used only if it is
                non-NULL and non-zero. If non-NULL but
                zero, it is treated as an "out" 
                variable and the if_index is returned
                based on the if_name - see below 
             */
             uns32 *io_if_index;

             /* Input/Output ifname - Used if "io_if_index" above
               is NULL or zero. If "io_if_index" is non-zero, then 
               (a non-NULL) "io_if_name" returns the "io_if_name" 
               - see above 
             */
             char *io_if_name;

             /* Output MAC address pointer */
             uns8      o_mac_addr[6];

        } get_eth_addr;

      } ctrl;


      /* This is a union of each data plane request type */
      union
      {
         struct
         {

            /* DL protocol stack's opaque handle for this DL connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_dl_handle;

            /* The destination's Layer 2 Address */
            NCS_L2_ADDR         i_remote_addr;

            /* The frame to send, as a pointer to a USRBUF chain. */
            USRBUF              *i_usrbuf;

            /* Outgoing if_index */
            uns32                i_if_index;
            /* ifindex can be ignored as it is no longer used internally,
               Actually we are using if-index stored in socket entry
               */

          } send_data;
      } data;

   } info;

} NCS_DL_REQUEST_INFO;



/* External functions */
EXTERN_C uns32 ncs_dl_bind_l2_layer (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_unbind_l2_layer (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_open_l2_layer (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_close_l2_layer (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_send_data_to_l2_layer (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_mcast_join (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_mcast_leave (struct ncs_dl_request_info_tag *dlr);
EXTERN_C uns32 ncs_dl_getl2_eth_addr(uns32 *if_index, char if_name[NCS_IF_NAMESIZE], uns8 mac_addr[6]);

#ifdef  __cplusplus
}
#endif

#endif

