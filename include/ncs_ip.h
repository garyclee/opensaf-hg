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

  MODULE NAME: NCS_IP.H



..............................................................................

  DESCRIPTION:

  This defines the H&J TCP/UDP used by H&J Software for control and data
  plane access to the target system IP protocol stack.  It is used to send and 
  receive TCP, UDP and Raw IP messages.

  The target system must provide an implementation of this interface.
  H&J Provides a sample Winsock/Berkley Socket implementation.   

  NOTE:
  This interface definition is intentionally imcomplete.  Only those portions
  of the interface the H&J Software requires are defined.  The remaining
  necessary interfaces to a Label Processing Engine are viewed as outside the
  scope of H&J software do not need to be defined by H&J.

******************************************************************************
*/

#ifndef NCS_IP_H
#define NCS_IP_H

#define SYSF_IP_MAX_LAYER_HANDLE_LEN    (sizeof(void *))
#define SYSF_IP_MAX_CLIENT_HANDLE_LEN   (sizeof(void *))

/* Used to set IP Errors to Common Socket Errror as defined in file below */
#include "ncs_dl.h"
#include "ncs_iplib.h"
#include "ncsencdec_pub.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define IP_ALL_ROUTERS_ADDR     0xE0000002 /* 224.0.0.2  rfc1700 pg56 */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            Typedefs

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/



struct ncs_ip_indication_info_tag;
struct ncs_ip_request_info_tag;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                  TCP/UDP Interface Definition

 Note this interface defines the following:
 Indications go from the IP Protocol Stack to the IP User.
 Requests go from the IP User to the IP Protocol Stack.

  (1) NCS_IP_TYPE                This enum defines the type of IP connections 
                        that can be established - currently TCP, UDP 
                        or Raw IP.

  (2) NCS_IP_INDICATIONS         This enum defines the indications that 
                        the IP Stack can send to the IP User.

  (3) NCS_IP_INDICATION          Function prototype for received IP data and 
                        control indications.  The IP USER must register 
                        a function matching this prototype with the IP 
                        protocol stack in the NCS_IP_CTRL_REQUEST OPEN 
                        primitive. 
  (4) NCS_IP_INDICATION_INFO     This structure defines the information passed 
                        across the interface, for each inication type.


  (5) NCS_IP_REQUESTS            This enum defines the requests that an 
                        IP User can make.
  
  (6) NCS_IP_REQUEST             Function prototype for IP Control requests and 
                        for sending IP data.  The target system must 
                        register a function matching this prototype with 
                        the IP User prior to any IP connections being open.

  (7) NCS_IP_REQUEST_INFO        This structure defines the information passed 
                        across the interface, for request type.
 

  What is not defined is how a given IP User is bound the this IP interface.
  The assumption is made that we do not what a compile time binding between
  this generic IP interface and the IP protocol stack.  Instead the target
  system software must perform a runtime binding of the IP protocol stack
  to the IP user.  For example, here is a published API call into the LMS
  product:

   ncslms_bind_ip (NCSCONTEXT         lms_handle,
               NCS_IP_LAYER_HANDLE   ip_layer_handle,
               NCS_IP_REQUEST        ip_data_request,
               NCS_IP_REQUEST        ip_ctrl_request);


  By invoking this API, the target system binds the LMS's components to the
  specific IP protocol stack (by functions and by opaque handle).  



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/




/*****************************************************************************

  ENUM NAME:            NCS_IP_INDICATIONS

  DESCRIPTION:          This enum defines the types of IP Indications from the
                  IP protocol stack to the IP user.


  VALUES:
   NCS_IP_CTRL_INDICATION_CONNECT:       The specified IP connection has become 
   operational.  This is only valid for TCP connections, UDP and Raw IP 
   connections are either up or non-existant.

   NCS_IP_CTRL_INDICATION_DISCONNECT:    The specified IP connection has
   been shutdown, or disabled.  No further TCP messages can be sent or 
   received over this connection.  This is only valid for TCP connections,
   UDP and Raw IP connections are either up or non-existant.

   NCS_IP_CTRL_INDICATION_ERROR:     The specified IP connection had
   an error and been shutdown.  No further IP messages can be sent or 
   received over this connection.  

   NCS_IP_DATA_INDICATION_RECV_DATA: The provided data has been received
   over this connection.


  NOTES:


*****************************************************************************/

typedef enum
{

   NCS_IP_INDICATION_MIN,

   NCS_IP_CTRL_INDICATION_MIN         = NCS_IP_INDICATION_MIN,
   NCS_IP_CTRL_INDICATION_CONNECT     = NCS_IP_CTRL_INDICATION_MIN,
   NCS_IP_CTRL_INDICATION_DISCONNECT,
   NCS_IP_CTRL_INDICATION_ERROR,
   NCS_IP_CTRL_INDICATION_MAX         = NCS_IP_CTRL_INDICATION_ERROR,

   NCS_IP_DATA_INDICATION_MIN,
   NCS_IP_DATA_INDICATION_RECV_DATA   = NCS_IP_DATA_INDICATION_MIN,
   NCS_IP_DATA_INDICATION_MAX         = NCS_IP_DATA_INDICATION_RECV_DATA,

   NCS_IP_INDICATION_MAX              = NCS_IP_DATA_INDICATION_MAX

} NCS_IP_INDICATIONS;






/*****************************************************************************

  ENUM NAME:            NCS_IP_REQUESTS

  DESCRIPTION:          This enum defines the type of requests that the IP 
                  User can make of the IP protocol stack.


  VALUES:
   NCS_IP_CTRL_REQUEST_BIND:         Bind client software to the IP 
   protocol stack.  This request provides the client's data and control
   indication handlers, and returns a handle the client can use in future
   open/close connection and send data requests.

   NCS_IP_CTRL_REQUEST_UNBIND:           Unbind client software from the IP
   protocol stack.  This client will nolonger use the IP stack.

   NCS_IP_CTRL_REQUEST_OPEN:         Open a new connection to a destination.
   This request is used for all UDP and Raw IP opens, as well as for TCP 
   listens (not the initiator, i.e., the server side).  In the case of UDP
   it is not necessary to provide the destination IP address at this time.
   When listening for incoming TCP connections, once this request is made, 
   next event will be a control indication of "connect".

   NCS_IP_CTRL_REQUEST_OPEN_ESTABLISH:   Open a new connection to a destination.
   This request is used to open a TCP connection were this client is
   initiating the request (ie, sending the "tcp sin").  When initiating
   TCP connections, once this request is made,  next event will be a
   control indication of "connect".

   NCS_IP_CTRL_REQUEST_CLOSE:            Close an existing UDP, TCP or Raw
   IP connection.    No further IP messages can be sent or received over 
   this connection.  

   NCS_IP_DATA_REQUEST_SEND_DATA:        Send UDP, TCP or Raw IP data over an
   existing connection.


  NOTES:


*****************************************************************************/

typedef enum
{

   NCS_IP_REQUEST_MIN,

   NCS_IP_CTRL_REQUEST_MIN       = NCS_IP_REQUEST_MIN,
   NCS_IP_CTRL_REQUEST_BIND      = NCS_IP_CTRL_REQUEST_MIN,
   NCS_IP_CTRL_REQUEST_UNBIND,
   NCS_IPV6_CTRL_REQUEST_BIND,
   NCS_IPV6_CTRL_REQUEST_UNBIND,
   NCS_IP_CTRL_REQUEST_OPEN,
   NCS_IP_CTRL_REQUEST_OPEN_ESTABLISH,
   NCS_IP_CTRL_REQUEST_POPEN,
   NCS_IP_CTRL_REQUEST_POPEN_ESTABLISH,
   NCS_IP_CTRL_REQUEST_MULTICAST_JOIN,
   NCS_IP_CTRL_REQUEST_MULTICAST_LEAVE,
   NCS_IP_CTRL_REQUEST_SET_IPTTL,
   NCS_IP_CTRL_REQUEST_SET_IPTOS,
   NCS_IP_CTRL_REQUEST_CLOSE,
   NCS_IP_CTRL_REQUEST_MAX       = NCS_IP_CTRL_REQUEST_CLOSE,

   NCS_IP_DATA_REQUEST_MIN,
   NCS_IP_DATA_REQUEST_SEND_DATA = NCS_IP_DATA_REQUEST_MIN,
   NCS_IP_DATA_REQUEST_PSEND_DATA,
   NCS_IP_REQUEST_MAX            = NCS_IP_DATA_REQUEST_PSEND_DATA
} NCS_IP_REQUESTS;





/*****************************************************************************

  ENUM NAME:            NCS_IP_TYPE

  DESCRIPTION:          This enum defines the type of IP connections available.
                  NCS_IP_TYPE_UDP, NCS_IP_TYPE_TCP and NCS_IP_TYPE_RAW
                  are the only required types.  It is provided in the 
                  NCS_IP_CTRL_REQUEST_OPEN and 
                  NCS_IP_CTRL_REQUEST_OPEN_ESTABLISH, to specify the
                  type of IP connection.

  VALUES:
   NCS_IP_TYPE_UDP:      This connection will use the UDP protocol.
   NCS_IP_TYPE_TCP:      This connection will use the TCP protocol.
   NCS_IP_TYPE_RAW:      This connection will use another (specified IP protocol).


  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_TYPE_UDP,
   NCS_IP_TYPE_TCP,
   NCS_IP_TYPE_RAW,
   NCS_IP_TYPE_MAX = NCS_IP_TYPE_RAW
} NCS_IP_TYPE;




/*****************************************************************************

  ENUM NAME:            NCS_IP_TOS_PRECEDENCE

  DESCRIPTION:          This enum defines the settings of precedence bits to be used
                     within the Type Of Service field in the IP Header.

  VALUES:
   NCS_IP_TOS_PRECEDENCE_DEFAULT:        Let the IP layer software select.

   The other Value Names correspond in name & value to rfc791.


  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_TOS_PRECEDENCE_DEFAULT = 0xFF,
   NCS_IP_TOS_PRECEDENCE_ROUTINE = 0x00,
   NCS_IP_TOS_PRECEDENCE_PRIORITY = 0x20,
   NCS_IP_TOS_PRECEDENCE_IMMEDIATE = 0x40,
   NCS_IP_TOS_PRECEDENCE_FLASH = 0x60,
   NCS_IP_TOS_PRECEDENCE_FLASHOVERRIDE = 0x80,
   NCS_IP_TOS_PRECEDENCE_CRITICECP = 0xA0,
   NCS_IP_TOS_PRECEDENCE_INTERNETWORK_CONTROL = 0xC0,
   NCS_IP_TOS_PRECEDENCE_NETWORK_CONTROL = 0xE0,
   NCS_IP_TOS_PRECEDENCE_SENTINEL
}  NCS_IP_TOS_PRECEDENCE;




/*****************************************************************************

  ENUM NAME:            NCS_IP_TOS_DELAY

  DESCRIPTION:          This enum defines the setting of the delay bit to use
                     within the Type Of Service field in the IP Header.

  VALUES:
   NCS_IP_TOS_DELAY_DEFAULT:     Let the IP layer software select.

   The other Value Names correspond in name & value to rfc791.



  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_TOS_DELAY_DEFAULT = 255,
   NCS_IP_TOS_DELAY_NORMAL  = 0x00,
   NCS_IP_TOS_DELAY_LOW = 0x10,
   NCS_IP_TOS_DELAY_SENTINEL
}  NCS_IP_TOS_DELAY;




/*****************************************************************************

  ENUM NAME:            NCS_IP_TOS_THROUGHPUT

  DESCRIPTION:          This enum defines the setting of the throughput bit to use
                     within the Type Of Service field in the IP Header.

  VALUES:
   NCS_IP_TOS_THROUGHPUT_DEFAULT:        Let the IP layer software select.

   The other Value Names correspond in name & value to rfc791.



  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_TOS_THROUGHPUT_DEFAULT = 255,
   NCS_IP_TOS_THROUGHPUT_NORMAL  = 0,
   NCS_IP_TOS_THROUGHPUT_HIGH = 0x08
}  NCS_IP_TOS_THROUGHPUT;




/*****************************************************************************

  ENUM NAME:            NCS_IP_TOS_RELIABILITY

  DESCRIPTION:          This enum defines the setting of the reliability bit to use
                     within the Type Of Service field in the IP Header.

  VALUES:
   NCS_IP_TOS_RELIABILITY_DEFAULT:       Let the IP layer software select.

   The other Value Names correspond in name & value to rfc791.



  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_TOS_RELIABILITY_DEFAULT = 255,
   NCS_IP_TOS_RELIABILITY_NORMAL  = 0,
   NCS_IP_TOS_RELIABILITY_HIGH = 0x04
}  NCS_IP_TOS_RELIABILITY;




/*****************************************************************************

  ENUM NAME:            NCS_IP_ERROR

  DESCRIPTION:          This enum defines the type of IP error indications.

  VALUES:

   NCS_IP_ERROR_NO_ERROR:            No error occurred. 

   NCS_IP_ERROR_IP_DOWN:         The IP Protocol Stack is not operating, 
   unavailable, or not yet initialized.

   NCS_IP_ERROR_NO_MEM:              Memory resources were not available to
   fulfill the request.

   NCS_IP_ERROR_FRAME_TOO_LARGE: If a Raw or UDP connection, the frame was 
   too large to send.

   NCS_IP_ERROR_CONN_UNSUPPORTED:    A connection orientated request was made
   of a UDP or Raw connection, or a datagram oriented request was made of
   a stream (tcp) connection.

   NCS_IP_ERROR_CONN_DOWN:           The specified connection is not ready - it
   has been aborted, reset or is not yet connection.

   NCS_IP_ERROR_CONN_UNKNOWN:        The specified connection is unknown.

   NCS_IP_ERROR_UNKNOWN:         Unspecified error condition occurred.

   NCS_IP_ERROR_SND_GIVEUP:      Send operation failed because the max retry count
                                for blocking sends is reached or the send request
                                can't be queued for queueing sends.

  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_ERROR_NO_ERROR = NCSSOCK_ERROR_TYPES_NO_ERROR,
   NCS_IP_ERROR_IP_DOWN = NCSSOCK_ERROR_TYPES_IP_DOWN,
   NCS_IP_ERROR_NO_MEM = NCSSOCK_ERROR_TYPES_NO_MEM,
   NCS_IP_ERROR_FRAME_TOO_LARGE = NCSSOCK_ERROR_TYPES_FRAME_TOO_LARGE,
   NCS_IP_ERROR_CONN_UNSUPPORTED = NCSSOCK_ERROR_TYPES_CONN_UNSUPPORTED,
   NCS_IP_ERROR_CONN_DOWN = NCSSOCK_ERROR_TYPES_CONN_DOWN,
   NCS_IP_ERROR_CONN_UNKNOWN = NCSSOCK_ERROR_TYPES_CONN_UNKNOWN,
   NCS_IP_ERROR_UNKNOWN = NCSSOCK_ERROR_TYPES_UNKNOWN,
   NCS_IP_ERROR_SND_GIVEUP = NCSSOCK_ERROR_TYPES_SND_GIVEUP,
   NCS_IP_ERROR_WOULDBLOCK = NCSSOCK_ERROR_TYPES_WOULDBLOCK,
   NCS_IP_ERROR_MAX = NCS_IP_ERROR_WOULDBLOCK
} NCS_IP_ERROR;


/*****************************************************************************

  ENUM NAME:            NCS_IP_SEND_POLICY

  DESCRIPTION:          This enum defines the send policy for a new connection.

  VALUES:
   NCS_IP_SEND_POLICY_BLOCK:       Let the IP layer block on send requests.
   NCS_IP_SEND_POLICY_QUEUE:       Let the IP layer queue send requests.

  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_SEND_POLICY_BLOCK,
   NCS_IP_SEND_POLICY_QUEUE
}  NCS_IP_SEND_POLICY;

/*****************************************************************************

  ENUM NAME:            NCS_IP_SEND_PRIORITY

  DESCRIPTION:          This enum defines the data send priority.

  VALUES:
   NCS_IP_SEND_PRIORITY_HI:        Hi priority.
   NCS_IP_SEND_PRIORITY_LOW:       Low priority.
   NCS_IP_SEND_PRIORITY_DROP:      Drop is send can not complete.

  NOTES:

*****************************************************************************/

typedef enum
{
   NCS_IP_SEND_PRIORITY_HI,
   NCS_IP_SEND_PRIORITY_LOW,
   NCS_IP_SEND_PRIORITY_DROP
}  NCS_IP_SEND_PRIORITY;

/*****************************************************************************

  ENUM NAME:            NCS_USRDATA_FORMAT

  DESCRIPTION:          User data can be passed between sysf_ip and the
                        application either as ptr to usrbuf or ptr to
            flat memory.

  VALUES:
   NCS_IP_USRBUF_DATA:   Data packed in userbufs
   NCS_IP_FLAT_DATA:     Data as flat memory

  NOTES:

*****************************************************************************/

typedef enum
{
            NCS_USRBUF_DATA = 0,
            NCS_USRFRAME_DATA = 0x7f /* some magic number */
} NCS_USRDATA_FORMAT;

/*****************************************************************************
 * STRUCTURE NAME:     NCS_IP_LAYER_HANDLE
 *
 * DESCRIPTION:        This structure represents the handle given to 
 *                     the LTCS by the target system at genesis to identify 
 *                     the IP protocol stack.
 *
 * NOTES:
 *   A 'len' field of zero indicates 'no handle'
 *
 *   Sizes of the 'handle' array and use of the 'len' field is a target-system
 *   design issue.
 *
 *   The symbol SYSF_IP_MAX_LAYER_HANDLE_SIZE is defined in a customer-
 *   modifiable file.  Typically it might be defined to be sizeof(void*)
 *   if the actual handle value is a pointer.
 *
 ****************************************************************************/
typedef struct ncs_ip_layer_handle_tag
{
   unsigned int len;     /* actual size of handle.  '0' means none. */
   uns8         data[SYSF_IP_MAX_LAYER_HANDLE_LEN];
} NCS_IP_LAYER_HANDLE;



/*****************************************************************************
 * STRUCTURE NAME:     NCS_IP_CLIENT_HANDLE
 *
 * DESCRIPTION:        This structure represents the handle returned to 
 *                     the LTCS by the target system by the  
 *                     NCS_IP_CTRL_REQUEST_BIND request.
 *
 * NOTES:
 *   A 'len' field of zero indicates 'no handle'
 *
 *   Sizes of the 'handle' array and use of the 'len' field is a target-system
 *   design issue.
 *
 *   The symbol SYSF_IP_MAX_CLIENT_HANDLE_SIZE is defined in a customer-
 *   modifiable file.  Typically it might be defined to be sizeof(void*)
 *   if the actual handle value is a pointer.
 *
 ****************************************************************************/
typedef struct ncs_ip_client_handle_tag
{
   unsigned int len;     /* actual size of handle.  '0' means none. */
   uns8         data[SYSF_IP_MAX_CLIENT_HANDLE_LEN];
} NCS_IP_CLIENT_HANDLE;


/*****************************************************************************

  Macro NAME:           NCS_IP_INDICATION

  DESCRIPTION:          A function matching this prototype must be provided 
                  when a IP user opens a new connection with the
                  NCS_IP_CTRL_REQUEST OPEN primitve.  This function 
                  will be used by the IP protocol stack deliver received 
                  IP data and control indications to this IP User.

  ARGUMENTS:
   ip_indication:       Pointer to a NCS_IP_INDICATION_INFO information block.

  RETURNS: 
   NCSCC_RC_SUCCESS: Success.  
   NCSCC_RC_FAILURE: Failure. 
  NOTES:

*****************************************************************************/

typedef uns32 (*NCS_IP_INDICATION) (struct ncs_ip_indication_info_tag *ip_indication);





/*****************************************************************************

  Macro NAME:           NCS_IP_REQUEST

  DESCRIPTION:          A function matching this prototype must be provided 
                  by the IP protocol stack prior to an IP user opening
                  any connections.  This function will be used by the IP
                  user to send IP control and data requests to the IP
                  stack.

  ARGUMENTS:
   ip_request:          Pointer to a NCS_IP_REQUEST_INFO information block.

  RETURNS: 
   NCSCC_RC_SUCCESS: Success.  
   NCSCC_RC_FAILURE: Failure. 
  NOTES:

*****************************************************************************/

typedef uns32 (*NCS_IP_REQUEST) (struct ncs_ip_request_info_tag *ip_request);



/* this is the strucutre which hold the packet information which we need to 
   get */
typedef struct ncs_ip_pkt_info_tag
{
   NCS_IP_ADDR src_addr;
   NCS_IP_ADDR dst_addr;
   uns32      if_index;
   uns32      TTL_value;
} NCS_IP_PKT_INFO;



/*****************************************************************************

  ENUM NAME:            NCS_IP_INDICATION_INFO

  DESCRIPTION:          This structure defines control block used in the
                  H&J IP interface.

  NOTES:
  This structure contains a union with a separate element for each primitive
  request type.  In this way, each primitive clearly defines what fields are
  required inputs and outputs.  The fields that exist outside of the union
  are required for all request types.

  Fields prefixed with "i_" are inputs to the request.
  Fields prefixed with "o_" are output from successfully completed requests.


*****************************************************************************/

typedef struct ncs_ip_indication_info_tag
{

   /* One of the NCS_IP_INDICATIONS enum specify the operation type. */
   NCS_IP_INDICATIONS       i_indication;

   /* IP protocol stack's opaque handle for the connection */
   NCSCONTEXT               i_ip_handle;

   /* The LMS handle */
   NCSCONTEXT               i_user_handle;

   /* IP User's opaque handle for the connection */
   NCSCONTEXT               i_user_connection_handle;
   NCS_IP_ADDR_TYPE         i_addr_family;

   /* This is a union of each indication type. */
   union
   {
      /* This is a union of each control plane indication type */
      union
      {

         /* This IP Connection is now Active */
         struct
         {
            /* The IP User's IP Address */
            NCS_IPV4_ADDR      i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16          i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR      i_remote_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR   i_ipv6_local_addr;
            NCS_IPV6_ADDR   i_ipv6_remote_addr;
#endif

            /* The destination's port */
            uns16          i_remote_port;

            /* Interface index to use for connection. */
            uns32          i_if_index;

         } connect;


         /* This IP Connection is shutdown and disabled */
         struct
         {
            /* The IP User's IP Address */
            NCS_IPV4_ADDR      i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16          i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR      i_remote_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR    i_ipv6_local_addr;
            NCS_IPV6_ADDR    i_ipv6_remote_addr;
#endif

            /* The destination's port */
            uns16          i_remote_port;

            /* Interface index to use for connection. */
            uns32          i_if_index;

         } disconnect;

         struct
         {
            /* The IP User's IP Address */
            NCS_IPV4_ADDR      i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16          i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR      i_remote_addr;
#if (NCS_IPV6 == 1)            
            NCS_IPV6_ADDR    i_ipv6_local_addr;
            NCS_IPV6_ADDR    i_ipv6_remote_addr;
#endif

            /* The destination's port */
            uns16          i_remote_port;

            /* Interface index to use for connection. */
            uns32          i_if_index;

            /* The error code being reported, one of NCS_IP_ERROR enum */
            NCS_IP_ERROR       i_error;

         } error;

      } ctrl;

      /* This is a union of each data plane indication type */
      union
      {
         struct
         {
            /* The IP User's IP Address */
            NCS_IPV4_ADDR      i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16          i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR      i_remote_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR    i_ipv6_local_addr;
            NCS_IPV6_ADDR    i_ipv6_remote_addr;
#endif            
            NCS_IP_PKT_INFO  pkt_info;

            /* The destination's port */
            uns16          i_remote_port;

            /* Interface index to use for connection. */
            uns32          i_if_index;

            /* The received data frame, as a pointer to a USRBUF chain */
            USRBUF            *i_usrbuf;

            /* The received data frame, as a pointer to a USRFRAME */
            USRFRAME          *i_usrframe;

            /* The TTL value received from this packet */
            uns8              o_ttl_value;

         } recv_data;
      } data;
   } info;

} NCS_IP_INDICATION_INFO;


/*****************************************************************************

  ENUM NAME:            NCS_IP_REQUEST_INFO

  DESCRIPTION:          This structure defines control block used in the
                  H&J IP interface.

  FIELDS:
   i_ip_layer_handle:   Handle to IP protocol stack handle, if available.
   i_ip_type:           Type of IP User, see NCS_IP_TYPE, either TCP or UDP. 
   i_ip_handle:     IP protocol stack's opaque handle for the IP user instance 
                  (as returned by the NCS_IP_CTRL_REQUEST OPEN primitive.
   i_user_handle:       Opaque handle for this IP user, as provided by IP user in
                  the NCS_IP_CTRL_REQUEST OPEN primitive invocation.

  NOTES:


*****************************************************************************/

typedef struct ncs_ip_request_info_tag
{

   /* One of the NCS_IP_REQUESTS enum specify the operation type. */
   NCS_IP_REQUESTS                i_request;

   NCSCONTEXT                     i_user_handle;

   /* Extended error information  */
   NCS_IP_ERROR                   o_xerror;

   /* address family */
   NCS_IP_ADDR_TYPE               addr_family_type;

   /* This is a union of each request type. */
   union
   {

      /* This is a union of each control plane request type */
      union
      {
         /* Bind IP Client to IP protocol stack. */
         struct
         {
            /* Handle to IP protocol stack instance. */
            NCS_IP_LAYER_HANDLE      i_ip_layer_handle;

            /* Return IP stack handle for this IP Client. */
            NCS_IP_CLIENT_HANDLE     o_ip_client_handle;

         } bind;

         /* Unbind IP Client to IP protocol stack. */
         struct
         {
            /* Handle to IP protocol stack instance. */
            NCS_IP_LAYER_HANDLE      i_ip_layer_handle;

            /* IP stack handle for this IP Client (returned by bind request) */
            NCS_IP_CLIENT_HANDLE     i_ip_client_handle;
         } unbind;

         struct
         {
            /* IP stack handle for this IP Client (returned by bind request) */
            NCS_IP_CLIENT_HANDLE   i_ip_client_handle;

            /* Type of IP connection, one of NCS_IP_TYPE enum. */
            NCS_IP_TYPE            i_ip_type;

            /* IP Protocol, if NCS_IP_TYPE_RAW. */
            uns8                  i_ip_protocol;

            /* IP User's opaque handle for this connection */
            NCSCONTEXT             i_user_connection_handle;

            /* If 1, interface is unnumbered */
            NCS_BOOL               i_unnumbered;

            /* The IP User's IP Address */
            NCS_IPV4_ADDR          i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16                 i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR          i_remote_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR          i_ipv6_local_addr;
            NCS_IPV6_ADDR          i_ipv6_remote_addr;
#endif

            /* The destination's port */
            uns16                 i_remote_port;

            /* Interface index to use for connection. */
            uns32                 i_if_index;

            /* Client Data indication handler, matching NCS_IP_INDICATION prototype */
            NCS_IP_INDICATION      i_ip_data_indication;

            /* Client Control indication handler, matching NCS_IP_INDICATION prototype */
            NCS_IP_INDICATION      i_ip_ctrl_indication;

            /* If 1, use IP_HDRINCL option, build complete IP header on sends - Raw Only */
            uns8                  i_ip_hdrincl;

            /* If 1, use BIND to Device option - Raw Only */
            NCS_BOOL               i_bindtodevice;

            /* If 1, enable receive with RouterAlert option - Raw Only */
            NCS_BOOL               i_router_alert;

            /* If 1, enable receving destination IP, ifindex and hop limit*/
            NCS_BOOL               i_enb_pkt_info;

            /* If 1, enable to open Link Local sockets */
            NCS_BOOL               i_link_loc_sock;

            /* If 1, enable to process Broadcast pkts */
            NCS_BOOL               i_broadcast;

            /* If 1, enable to process multicast pkts */
            NCS_BOOL               i_multicast;

            /* If TRUE, enable loopback on this socket */
            NCS_BOOL               i_mcast_loc_lbk;

      /* The send policy for the new connection.
         Used for NCS_IP_CTRL_REQUEST_POPEN and
         NCS_IP_CTRL_REQUEST_POPEN_ESTABLISH requests
      */
      NCS_IP_SEND_POLICY     i_send_policy;

            /* The expected format (usrbuf or usrframe) for receive data */
            NCS_USRDATA_FORMAT             i_rcv_usrdata_format;

            /* Returned IP protocol stack's opaque handle for this IP connection */
            NCSCONTEXT             o_ip_handle;

         } open;

         struct
         {
            /* IP stack handle for this IP Client (returned by bind request) */
            NCS_IP_CLIENT_HANDLE  i_ip_client_handle;

            /* Type of IP connection, one of NCS_IP_TYPE enum. */
            NCS_IP_TYPE           i_ip_type;

            /* IP User's opaque handle for this connection */
            NCSCONTEXT            i_user_connection_handle;

            /* If 1, interface is unnumbered */
            NCS_BOOL               i_unnumbered;

            /* The IP User's IP Address */
            NCS_IPV4_ADDR         i_local_addr;

            /* The IP User's UDP/TCP Port */
            uns16                i_local_port;

            /* The destination's IP Address */
            NCS_IPV4_ADDR         i_remote_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR         i_ipv6_local_addr;
            NCS_IPV6_ADDR         i_ipv6_remote_addr;
#endif

            /* The destination's port */
            uns16                i_remote_port;

            /* Interface index to use for connection. */
            uns32                i_if_index;

            /* Client Data indication handler, matching NCS_IP_INDICATION prototype */
            NCS_IP_INDICATION     i_ip_data_indication;

            /* Client Control indication handler, matching NCS_IP_INDICATION prototype */
            NCS_IP_INDICATION     i_ip_ctrl_indication;

      /* The send policy for the new connection.
         Used for NCS_IP_CTRL_REQUEST_POPEN and
         NCS_IP_CTRL_REQUEST_POPEN_ESTABLISH requests
      */
      NCS_IP_SEND_POLICY     i_send_policy;

            /* Returned IP protocol stack's opaque handle for this IP connection */
            NCSCONTEXT            o_ip_handle;

         } open_establish;

         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

         } close;

         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

            /* The Multicast Group Address */
            NCS_IPV4_ADDR         i_multicast_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR         i_ipv6_multicast_addr;
#endif

         } multicastjoin;

         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

            /* The Multicast Group Address */
            NCS_IPV4_ADDR         i_multicast_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR         i_ipv6_multicast_addr;
#endif

         } multicastleave;

         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

            /* The TTL value to be set in all subsequent packets */
            uns16                i_ttl_value;

            /* is it the ttl for multicast sends or unicast ? */
            NCS_BOOL              i_multicast; /* TRUE is multicast ttl */

         } setipttl;

         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

            /* The setting for the precedence bits within the IP Hdr TOS field */
            NCS_IP_TOS_PRECEDENCE i_precedence;

            /* The setting for the delay bit within the IP Hdr TOS field */
            NCS_IP_TOS_DELAY      i_delay;

            /* The setting for the throughput bit within the IP Hdr TOS field */
            NCS_IP_TOS_THROUGHPUT i_throughput;

            /* The setting for the reliability bit within the IP Hdr TOS field */
            NCS_IP_TOS_RELIABILITY i_reliability;

         } setiptos;

      } ctrl;


      /* This is a union of each data plane request type */
      union
      {
         struct
         {
            /* IP protocol stack's opaque handle for this IP connection 
             * as returned by open request 
             */
            NCSCONTEXT            i_ip_handle;

            /* The destination's IP Address */
            NCS_IPV4_ADDR         i_remote_addr;

            /* The destination's port */
            uns16                i_remote_port;

            NCS_USRDATA_FORMAT    i_usrdata_format;

            /* The frame to send, as a pointer to a USRBUF chain. */
            USRBUF              *i_usrbuf;

            /* The frame to send, as a pointer to USRFRAME. */
        USRFRAME        *i_usrframe;

            /* The TTL value to be set in this packet */
            uns16                i_ttl_value;

            /* For RAW IP:  Should packet be sent with Router Alert? */
            NCS_BOOL              i_router_alert;

            /* Enable to process broadcast pkts */
            NCS_BOOL              i_broadcast;

            /* The IP source address in HDRINCL option */
            NCS_IPV4_ADDR         i_ip_src_address;

            /* The IP Next Hop Address - from routing lookup */
            NCS_IPV4_ADDR         i_next_hop_ip_addr;
#if (NCS_IPV6 == 1)
            NCS_IPV6_ADDR         i_ipv6_src_addr;
            NCS_IPV6_ADDR         i_ipv6_remote_addr;
            NCS_IPV6_ADDR         i_ip_v6_next_hop_ip_addr;
#endif

            /* Outgoing if_index - from routing lookup. */
            uns32                i_if_index;

           /* The send priority.
              Used for the NCS_IP_DATA_REQUEST_PSEND_DATA request.
            */
            NCS_IP_SEND_PRIORITY  i_send_priority;


         } send_data;
      } data;

   } info;

} NCS_IP_REQUEST_INFO;


#ifdef  __cplusplus
}
#endif

#endif

