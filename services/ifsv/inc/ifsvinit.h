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
FILE NAME: IFSVINIT.H
DESCRIPTION: This file contains:      
      1) Function prototypes for IfD/IfND initialize.
      2) Structure used for IfD/IfND initialize.
******************************************************************************/
#ifndef IFSVINIT_H
#define IFSVINIT_H

/**** Ifsv component names ****/
#define m_IFD_COMP_NAME g_ifd_comp_name
#define m_IFND_COMP_NAME g_ifnd_comp_name
#define m_IFA_COMP_NAME "IfA"

/**** Macro to get the component name for the component type ****/
#define m_IFSV_TASKNAME(comp_type) ((comp_type == IFSV_COMP_IFD) || (comp_type == IFSV_COMP_IFND)) ? "Ifsv":"IfA"

/**** Macro for IFSV task priority ****/
#define m_IFSV_PRIORITY (5)
/**** Macro for IFSV task stack size ****/
#define m_IFSV_STACKSIZE (8000)

/* Maximum size of the component name */
#define IFSV_MAX_COMP_NAME (255)

/* Let as have the default vrid as 1 */
#define m_IFSV_DEF_VRID (1)

/*****************************************************************************
 * This is the component type which is used to identify different components
 *****************************************************************************/
typedef enum ifsv_comp_type
{
   IFSV_COMP_IFD =1 ,
   IFSV_COMP_IFND,
   IFSV_COMP_IFA,
   IFSV_COMP_MAX
}IFSV_COMP_TYPE;

/*****************************************************************************
 * structure which holds the create PWE structure.
 *****************************************************************************/
typedef struct ifsv_create_pwe
{
   uns32      vrid;   
   uns32      shelf_no;
   uns32      slot_no;   
   uns8       pool_id;   
   uns32      comp_type;   
   uns32      oac_hdl;
} IFSV_CREATE_PWE;
extern char g_ifd_comp_name[128];
extern char g_ifnd_comp_name[128];
#endif /* #ifndef IFSVINIT_H */
