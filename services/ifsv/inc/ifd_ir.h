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



#if(NCS_IFSV_BOND_INTF == 1)

uns32
ifd_binding_mib_send_evt (IFSV_CB *cb, uns8 portnum, NCS_IFSV_IFINDEX masterIfindex, NCS_IFSV_IFINDEX slaveIfIndex);

uns32
ifd_binding_check_ifindex_is_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex, NCS_IFSV_IFINDEX *binding_ifindex);

IFSV_INTF_DATA *
ifd_binding_change_master_ifindex(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex);

void
ifd_binding_change_all_master_intfs(IFSV_CB *ifsv_cb);

IFSV_INTF_DATA* ncsifsvbindgetnextindex(IFSV_CB *bind_ifsv_cb,uns32 bind_portnum,uns32 *next_index);


#endif








