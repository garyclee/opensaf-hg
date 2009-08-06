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

  MODULE NAME:  ip_defs.c


..............................................................................

  DESCRIPTION:


  CONTENTS:


*******************************************************************************/

#include "ncs_opt.h"
#include "gl_defs.h"   /* Global defintions */

#include "ncs_ipprm.h"   /* Req'd for primitive interfaces */
#include "sysf_ip.h"
#include "sysf_def.h"

#define   MCAST_SIM_PORT (4500) 

#if (NCS_TS_SOCK_MULTICAST_READ_SOCKET == 1)
int
ncs_mcast_sim_ipv4_rcv_sock(NCS_IPV4_ADDR mcast_addr, fd_set *read_fd, uns32 if_index)
{
   uns32 res = NCSCC_RC_SUCCESS;
   int                 bio      = 1;
   int                 smode    = 1;
   struct sockaddr_in  saddr;
   int sock;
   
   do
   {
      sock = m_NCSSOCK_SOCKET (AF_INET, SOCK_DGRAM, 0);
      if (sock < 0)
      {         
         res = NCSCC_RC_FAILURE;
         break;
      }
      
      if (m_NCSSOCK_IOCTL (sock, m_NCSSOCK_FIONBIO ,&bio ) 
         == NCSSOCK_ERROR)
      {         
         m_NCSSOCK_CLOSE(sock);
         res = NCSCC_RC_FAILURE;
         break;
      }
      
      if (m_NCSSOCK_SETSOCKOPT (sock,
         SOL_SOCKET,
         SO_REUSEADDR,
         (char*) &smode,
         sizeof (smode)))
      {         
         m_NCSSOCK_CLOSE(sock);
         res = NCSCC_RC_FAILURE;
         break;
      }
      
      memset (&saddr, 0x00, sizeof (saddr));
      saddr.sin_family      = AF_INET;
      saddr.sin_port        = sysf_htons((uns16)(MCAST_SIM_PORT + if_index));
      /*saddr.sin_port = 0;*/
      saddr.sin_addr.s_addr = mcast_addr;
      /*saddr.sin_addr.s_addr = INADDR_ANY;*/
      
      
      if (m_NCSSOCK_BIND (sock,
         (struct sockaddr*)&saddr,sizeof(saddr))==NCSSOCK_ERROR)
      {
         m_NCSSOCK_CLOSE(sock);
         res = NCSCC_RC_FAILURE;
         break;
      }
      FD_SET (sock, read_fd);
   } while (0);
      
   return (sock);
}
#endif


#if (m_SYSF_NCSSOCK_SUPPORT_SENDMSG == 1)

int
ncssock_ipv6_send_msg (NCS_SOCKET_ENTRY *se, char *bufp, uns32 total_len, 
                     struct sockaddr_in6 *saddr)
{
  struct msghdr msg;
  struct iovec iov;
  struct cmsghdr  *cmsgptr;
  char adata [256];
  struct in6_pktinfo *pkt;
  int ret;

  msg.msg_name = (void *) saddr;
  msg.msg_namelen = sizeof (struct sockaddr_in6);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (void *) adata;
  msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));

  iov.iov_base = bufp;
  iov.iov_len = total_len;

  cmsgptr = (struct cmsghdr *)adata;
  cmsgptr->cmsg_len = CMSG_LEN(sizeof (struct in6_pktinfo));
  cmsgptr->cmsg_level = IPPROTO_IPV6;
  cmsgptr->cmsg_type = IPV6_PKTINFO;

  pkt = (struct in6_pktinfo *) CMSG_DATA (cmsgptr);
  memset (&pkt->ipi6_addr, 0, sizeof (struct in6_addr));
  pkt->ipi6_ifindex = se->if_index;

  ret = sendmsg (se->client_socket, &msg, 0);
  return (ret);
}

#endif

#if (m_SYSF_TMP_NCSSOCK_SUPPORT_RECVMSG != 0)
uns32 
ncssock_ipv6_rcv_msg (NCS_SOCKET_ENTRY *se, char *buf, uns32 buf_len, 
                     struct sockaddr_in6 *p_saddr, NCS_IP_PKT_INFO *pkt_info)
{
   struct msghdr msg;
   struct iovec iov;
   struct cmsghdr  *cmsgptr;   
   char adata[1024];   
   
   /* Fill in message and iovec. */   
   msg.msg_name    = (void *) p_saddr;
   msg.msg_namelen = sizeof (struct sockaddr_in6);
   msg.msg_iov     = &iov;
   msg.msg_iovlen  = 1;
   msg.msg_control = (void *) adata;
   msg.msg_controllen = sizeof (adata);
   iov.iov_base = buf;
   iov.iov_len  = buf_len;

   buf_len = recvmsg (se->server_socket, &msg, 0);
   
   memset(pkt_info, 0, sizeof(NCS_IP_PKT_INFO));
   for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL;  cmsgptr = CMSG_NXTHDR(&msg, cmsgptr))
   {
      /* I want interface index which this packet comes from. */
      if ((cmsgptr->cmsg_level == IPPROTO_IPV6) &&
         (cmsgptr->cmsg_type == IPV6_PKTINFO))
      {
         struct in6_pktinfo *ptr;
         ptr = (struct in6_pktinfo *) CMSG_DATA (cmsgptr);
         pkt_info->if_index      = ptr->ipi6_ifindex;
         pkt_info->dst_addr.type = NCS_IP_ADDR_TYPE_IPV6;
         memcpy(&pkt_info->dst_addr.info.v6, &ptr->ipi6_addr, 
            sizeof(NCS_IPV6_ADDR));         
      }
      
      /* Incoming packet's multicast hop limit. */
      if (cmsgptr->cmsg_level == IPPROTO_IPV6 &&
         cmsgptr->cmsg_type == IPV6_HOPLIMIT)
      {
         pkt_info->TTL_value = *((int *) CMSG_DATA (cmsgptr));
      }
   }

   return (buf_len);
}
#endif


uns32 ip_svc_mc_lin_set_snd_if(NCS_SOCKET_ENTRY* se)
{
#ifdef IP_MULTICAST_IF
      {
         struct ip_mreqn mreq;
       
         mreq.imr_address.s_addr = htonl (se->local_addr);
         mreq.imr_ifindex = NCS_GET_IF_INDEX(se->if_index);

         if (m_NCS_TS_SOCK_SETSOCKOPT (se->client_socket,
                                  IPPROTO_IP,
                                  IP_MULTICAST_IF,
                                  (char*) &mreq,
                                  sizeof (mreq)))
         {
            return NCSCC_RC_FAILURE;
         }
      }
#endif
  return NCSCC_RC_SUCCESS;
}

#if (NCS_IPV6 == 1)
uns32 ipv6_svc_mc_lin_set_snd_if(NCS_SOCKET_ENTRY* se)
{
#ifdef IPV6_MULTICAST_IF
      {
         uns32 if_index = se->if_index;
                 
         if (m_NCS_TS_SOCK_SETSOCKOPT (se->client_socket,
                                  IPPROTO_IPV6,
                                  IPV6_MULTICAST_IF,
                                  (char*) &if_index,
                                  sizeof (if_index)))
         {
            return NCSCC_RC_FAILURE;
         }
      }
#endif
  return NCSCC_RC_SUCCESS;
}

int
os_ipv6_to_ifidx(NCS_IPV6_ADDR hbo_ipv6)
{
  NCS_IPV6_ADDR        nbo_ipv6;
  unsigned int        ifindex  = -1;
  int                 s        = -1;
  int                 more     = 0;
  int                 length   = 0;
  char*               buffer   = NULL;
  struct ifreq*       ifr      = NULL;
  struct ifconf       ifc;
  struct ifreq        ifra;
  struct sockaddr_in6* sin;
  int                 i        = 0;
  uns32                ip_comp_result;

  
  memset (&nbo_ipv6, 0x0, m_IPV6_ADDR_SIZE);
  memset(&ifc,0,sizeof(ifc));
  
  memcpy (&nbo_ipv6, &hbo_ipv6, m_IPV6_ADDR_SIZE);
    
  if((s = socket(PF_INET6,SOCK_DGRAM,0)) < 0)
    return -1;
  
  
  for(more = sizeof(struct ifreq),ifc.ifc_len = 0;;ifc.ifc_len = length)
    {
    if((ioctl(s,SIOCGIFCONF,(int)&ifc) < 0) && (ifc.ifc_buf != NULL))
      {
      /*printf("\nSIOCGIFCONF failed:%d\n", errno);*/
      close(s);
      free(ifc.ifc_buf);
      return -1;
      }
    if((ifc.ifc_buf != NULL) && (ifc.ifc_len >= (length + more)))
      break;
    if(ifc.ifc_buf != NULL)
      {
      if(length == 0)
        length = 128;
      ifc.ifc_len = length <<= 1;
      }
    else
      {
      length = ifc.ifc_len;
      more = 0;
      }
    if((buffer = realloc(ifc.ifc_buf,ifc.ifc_len)) == NULL)
      {
      close(s);
      free(ifc.ifc_buf);
      }
    ifc.ifc_buf = buffer;
    }
  for(i = 0 ;(length >= sizeof(struct sockaddr_storage)) && 
     (i < (length/sizeof(struct sockaddr_storage)));i++, length -= sizeof(struct sockaddr_storage))
    {
      /** Walk though if config list.  For each, look for match on address using the 
       ** ifname to ifaddr mapping from SIOCGIIFADDR ioctl.
       ** When we find an address we like do it again to get the ifindex 
       ** with SIOCGIFINDEX request.
       **/
    ifr = &ifc.ifc_req[i];
    memset(&ifra,0,sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    if(ioctl(s,SIOCGIFADDR,(int)&ifra) < 0)
      { 
    /*printf("\nSIOCGIFADDR failed:%d\n", errno);*/
    break;
      }
    sin = (struct sockaddr_in6*)&ifra.ifr_addr;
    ncs_ipv6_addr_compare (&nbo_ipv6, (NCS_IPV6_ADDR*)(&sin->sin6_addr), 
       &ip_comp_result);
    if(ip_comp_result != IPV6_ADDR_EQUAL)
      {
       continue;
      }
    memset(&ifra,0,sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    if(ioctl(s,SIOCGIFINDEX,(int)&ifra) < 0)
      { 
    /*printf("\nSIOCGIFINDEX failed:%d\n", errno);*/
    break;
      }

      ifindex = ifra.ifr_ifindex;
      break;
    }
  
  close(s);
  free(ifc.ifc_buf); 

  return ifindex;
}



/****************************************************************************
 * os_ifidx_to_ipv6
 *
 * Maps interface index to IPv6 (host order) address mapping
 *
 ***************************************************************************/
uns32
os_ifidx_to_ipv6(unsigned int ifIndex, NCS_IPV6_ADDR *ip_addr)
{
  int                 s        = -1;
  int                 more     = 0;
  int                 length   = 0;
  char*               buffer   = NULL;
  struct ifreq*       ifr      = NULL;
  struct ifconf       ifc;
  struct ifreq        ifra;
  struct sockaddr_in6* sin;
  int                 i        = 0;

  memset(&ifc,0,sizeof(ifc));
  
  if((s = socket(PF_INET6,SOCK_DGRAM,0)) < 0)
    return 0;
  
  for(more = sizeof(struct ifreq),ifc.ifc_len = 0;;ifc.ifc_len = length)
  {
    if((ioctl(s,SIOCGIFCONF,(int)&ifc) < 0) && (ifc.ifc_buf != NULL))
    {
      /*printf("\nSIOCGIFCONF failed:%d\n", errno);*/
      close(s);
      free(ifc.ifc_buf);
      return -1;
    }
    if((ifc.ifc_buf != NULL) && (ifc.ifc_len >= (length + more)))
      break;
    if(ifc.ifc_buf != NULL)
    {
      if(length == 0)
        length = 128;
      ifc.ifc_len = length <<= 1;
    }
    else
    {
      length = ifc.ifc_len;
      more = 0;
    }
    if((buffer = realloc(ifc.ifc_buf,ifc.ifc_len)) == NULL)
    {
      close(s);
      free(ifc.ifc_buf);
    }
    ifc.ifc_buf = buffer;
  }
  for(i = 0;(length >= sizeof(struct sockaddr_storage)) && 
     (i < (length/sizeof(struct sockaddr_storage)));i++, length -= sizeof(struct sockaddr_storage))
  {
    ifr = &ifc.ifc_req[i];
    memset(&ifra, 0, sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    ioctl(s,SIOCGIFINDEX,(int)&ifra);
    if (ifra.ifr_ifindex == ifIndex)
    {
       if(ioctl(s,SIOCGIFADDR,(int)&ifra) < 0)
          break;
       sin = (struct sockaddr_in6*)&ifra.ifr_addr;
       memcpy (ip_addr, &sin->sin6_addr, m_IPV6_ADDR_SIZE);       
       break;
    }
  }
  
  close(s);
  free(ifc.ifc_buf); 
  return 0;
}

#endif /*(NCS_IPV6 == 1) */

#if (NCS_TS_SOCK_BINDTODEVICE_SUPPORTED == 1)

uns32 ip_svc_bind_to_iface(NCS_SOCKET_ENTRY* se)
{ 
#ifdef SIOCGIFNAME
#ifdef SO_BINDTODEVICE
/** Build a ifreq to get mapping of device index to name,
** since the  bind to device sockopt operates on name.
   **/
   struct ifreq ifr;
   memset(&ifr, '\0', sizeof ifr);
   ifr.ifr_ifindex = se->if_index;
   
   if (m_NCS_TS_SOCK_IOCTL(se->client_socket,
      SIOCGIFNAME,
      &ifr)==NCS_TS_SOCK_ERROR)
   {
      return NCSCC_RC_FAILURE;
   }
   
   if (m_NCS_TS_SOCK_SETSOCKOPT(se->client_socket,
      SOL_SOCKET,
      SO_BINDTODEVICE,
      ifr.ifr_name,
      IFNAMSIZ))
   {
      return NCSCC_RC_FAILURE;
   }
#endif
#endif
   return NCSCC_RC_SUCCESS;
}

#endif


/****************************************************************************
 * IPv4 address to interface index converstion 
 *
 *
 ***************************************************************************/
int
os_ipv4_to_ifidx(unsigned int hbo_ipv4)
{
  unsigned int        nbo_ipv4 = 0;
  unsigned int        ifindex  = -1;
  int                 s        = -1;
  int                 more     = 0;
  int                 length   = 0;
  char*               buffer   = NULL;
  struct ifreq*       ifr      = NULL;
  struct ifconf       ifc;
  struct ifreq        ifra;
  struct sockaddr_in* sin;
  int                 i        = 0;

  
  memset(&ifc,0,sizeof(ifc));
  
  nbo_ipv4 = htonl(hbo_ipv4);
  
  if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0)
    return -1;
  
  
  for(more = sizeof(struct ifreq),ifc.ifc_len = 0;;ifc.ifc_len = length)
    {
    if((ioctl(s,SIOCGIFCONF,(int)&ifc) < 0) && (ifc.ifc_buf != NULL))
      {
      /*printf("\nSIOCGIFCONF failed:%d\n", errno);*/
      close(s);
      free(ifc.ifc_buf);
      return -1;
      }
    if((ifc.ifc_buf != NULL) && (ifc.ifc_len >= (length + more)))
      break;
    if(ifc.ifc_buf != NULL)
      {
      if(length == 0)
        length = 128;
      ifc.ifc_len = length <<= 1;
      }
    else
      {
      length = ifc.ifc_len;
      more = 0;
      }
    if((buffer = realloc(ifc.ifc_buf,ifc.ifc_len)) == NULL)
      {
      close(s);
      free(ifc.ifc_buf);
      }
    ifc.ifc_buf = buffer;
    }
  for(i = 0 ;(length >= sizeof(struct sockaddr)) && (i < (length/sizeof(struct sockaddr)));i++, length -= sizeof(struct sockaddr))
    {
      /** Walk though if config list.  For each, look for match on address using the 
       ** ifname to ifaddr mapping from SIOCGIIFADDR ioctl.
       ** When we find an address we like do it again to get the ifindex 
       ** with SIOCGIFINDEX request.
       **/
    ifr = &ifc.ifc_req[i];
    memset(&ifra,0,sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    if(ioctl(s,SIOCGIFADDR,(int)&ifra) < 0)
      { 
    /*printf("\nSIOCGIFADDR failed:%d\n", errno);*/
    break;
      }
    sin = (struct sockaddr_in*)&ifra.ifr_addr;
    if(nbo_ipv4 != sin->sin_addr.s_addr)
      {
       continue;
      }
    memset(&ifra,0,sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    if(ioctl(s,SIOCGIFINDEX,(int)&ifra) < 0)
      { 
    /*printf("\nSIOCGIFINDEX failed:%d\n", errno);*/
    break;
      }

      ifindex = ifra.ifr_ifindex;
      break;
    }
  
  close(s);
  free(ifc.ifc_buf); 

  return ifindex;
}



/****************************************************************************
 * os_ifidx_to_ipv4
 *
 * Maps interface index to IPv4 (host order) address mapping
 *
 ***************************************************************************/
unsigned int
os_ifidx_to_ipv4(unsigned int ifIndex)
{
  uns32                 ipAddr = 0;
  int                 s        = -1;
  int                 more     = 0;
  int                 length   = 0;
  char*               buffer   = NULL;
  struct ifreq*       ifr      = NULL;
  struct ifconf       ifc;
  struct ifreq        ifra;
  struct sockaddr_in* sin;
  int                 i        = 0;

  memset(&ifc,0,sizeof(ifc));
  
  if((s = socket(PF_INET,SOCK_DGRAM,0)) < 0)
    return 0;
  
  for(more = sizeof(struct ifreq),ifc.ifc_len = 0;;ifc.ifc_len = length)
  {
    if((ioctl(s,SIOCGIFCONF,(int)&ifc) < 0) && (ifc.ifc_buf != NULL))
    {
      /*printf("\nSIOCGIFCONF failed:%d\n", errno);*/
      close(s);
      free(ifc.ifc_buf);
      return -1;
    }
    if((ifc.ifc_buf != NULL) && (ifc.ifc_len >= (length + more)))
      break;
    if(ifc.ifc_buf != NULL)
    {
      if(length == 0)
        length = 128;
      ifc.ifc_len = length <<= 1;
    }
    else
    {
      length = ifc.ifc_len;
      more = 0;
    }
    if((buffer = realloc(ifc.ifc_buf,ifc.ifc_len)) == NULL)
    {
      close(s);
      free(ifc.ifc_buf);
    }
    ifc.ifc_buf = buffer;
  }
  for(i = 0;(length >= sizeof(struct sockaddr)) && (i < (length/sizeof(struct sockaddr)));i++, length -= sizeof(struct sockaddr))
  {
    ifr = &ifc.ifc_req[i];
    memset(&ifra, 0, sizeof(ifra));
    strcpy (ifra.ifr_name, ifr->ifr_name);
    ioctl(s,SIOCGIFINDEX,(int)&ifra);
    if (ifra.ifr_ifindex == ifIndex)
    {
       if(ioctl(s,SIOCGIFADDR,(int)&ifra) < 0)
          break;
       sin = (struct sockaddr_in*)&ifra.ifr_addr;
       ipAddr = ntohl(sin->sin_addr.s_addr);
       break;
    }
  }
  
  close(s);
  free(ifc.ifc_buf); 
  return ipAddr;
}



