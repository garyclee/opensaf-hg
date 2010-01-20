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

  D E S C R I P T I O N :

  This module contains all task configuration information, including
  priority, stack-size and task-name for all possible NetPlane tasks.
  The idea is that there is one file to visit to both understand and
  adjust any product thread value at compile time.

  T A K E   N O T E :

  This file has been set up in the LEAP 2.6 release. It may not be the
  case that all subsystems have yet adopted the advertised task 
  mnemonics called out below. It will take some time for all subsystems
  to make the migration.

  O V E R V I E W

  WHO AND WHERE ARE TASKS CREATED

  For the most part, NetPlane invokes m_NCS_TASK_CREATE outside of its base
  code to create tasks. This is because task creation and destruction is 
  considered a 'system' function that takes place in the context of the 
  system's initialization and destruction sequence.

  As such, NetPlane tasks are generally created in either demo code or 
  powercode, which NetPlane classifies as 'editable' (adjustable).

  TASK STACK SIZES

  The stack sizes used below work for our test environments. Different OS's
  may cause one to adjust these sizes. Relative size mnemonics are set up so
  that if your OS/Compiler needs/uses more or less, you can adjust them all
  at once.

  TASK NAMES

  Task names are here as much for convenience as for configuration. You
  may wish to change them, but system behavior is not contingent on this
  value in any way.

  TASK PRIORITIES

  NetPlane software TASKS cover the spectrum of system software:

  A - There are low level system services such as LEAP TIMER and 
      SYSF_IP services.

  B - Middleware infrastructure such as MDS's RCP.

  C - Control Plane Protocol services, which make up the bulk of the 
      NetPlane product set, and

  D - Management Plane services, such as NetPlane's CLI and MAB 
      Services.

  Roughly speaking, NetPlane task priorities are arranged highest (get
  CPU if in READY queue) to lowest in that order: A, B, C and D.
  
  Once said, there are exceptions and there is no hard and fast rule, 
  as different OSs mixed with different hardware platforms in monolith 
  or distributed contexts will behave differently.
 
  As such, you may find reason to tweek the shipped settings, as 
  reflected in this file at ship-time.

  Note that all possible NetPlane tasks are listed here. This in no way
  implies that these tasks are in fact running. Again, this is a system
  initialization sequence responsibility.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_TASKS_H
#define NCS_TASKS_H

/*************************************************************************
     
       A )    S Y S T E M   S E R V I C E S 

**************************************************************************/

/*************************************************************************
SYSF TIMER

    This thread drives the LEAP Timer Services.
    Function entry:  ncslmp_process_events()
    
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_TMR_STACKSIZE
#define NCS_TMR_STACKSIZE     NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_TMR_PRIORITY
#define NCS_TMR_PRIORITY      NCS_TASK_PRIORITY_1
#endif

#ifndef NCS_TMR_TASKNAME
#define NCS_TMR_TASKNAME      "SYSF_TMR"
#endif

/*************************************************************************
SYSF EXEC MOD

    This thread drives the LEAP Execute Module Services.
    Function entry:  ncslmp_process_events()
    
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_EXEC_MOD_STACKSIZE
#define NCS_EXEC_MOD_STACKSIZE     NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_EXEC_MOD_PRIORITY
#define NCS_EXEC_MOD_PRIORITY      NCS_TASK_PRIORITY_1
#endif

#ifndef NCS_EXEC_MOD_TASKNAME
#define NCS_EXEC_MOD_TASKNAME      "EXEC_SCR_MOD"
#endif
/*************************************************************************
SYSF IP  

      This thread drives the LEAP IP services; in particular, it 
      waits at the select() function.

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_SYSF_IP_STACKSIZE
#define NCS_SYSF_IP_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_SYSF_IP_PRIORITY
#define NCS_SYSF_IP_PRIORITY   NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_SYSF_IP_TASKNAME
#define NCS_SYSF_IP_TASKNAME   "SYSF_IP"
#endif

/*************************************************************************
     
       B )    M I D D L E W A R E   I N F R A S T R U C T U R E

        RCP   Generally, a high priority
        XLIM  Protocols depend on its synchronized info across a
              distributed platform
        RMS   Generally a lower priority so as not to distract from
              protocols on a mission.

**************************************************************************/

/*************************************************************************
NCS_MDS   Message Distribution Service (the product)

         MDS code has no thread of its own. However, components shipped
         with MDS do have threads.
         
         RCP     Reliable Connection Protocol thread (over UDP/ATM or UD)
                 Function entry: rcp_process_event()

         XLIM    Extended Logical Interface Mapper thread
                 Function entry: xlim_do_events()

                 NOTE: XLIM is here for product grouping.. Its priority
                 does not need to be as high as RCP.

         XOVR    Cross Over Session type is launched when NCSMDS_SSN_TYPE_XOVR
                 is enabled. This session type was designed for testing or
                 simulating distributed systems in a monolithic context.
                
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* RCP */

#ifndef NCS_RCP_STACKSIZE
#define NCS_RCP_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_RCP_PRIORITY
#define NCS_RCP_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_RCP_TASKNAME
#define NCS_RCP_TASKNAME     "RCP"
#endif

/* XLIM */

#ifndef NCS_XLIM_STACKSIZE
#define NCS_XLIM_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_XLIM_PRIORITY
#define NCS_XLIM_PRIORITY    NCS_TASK_PRIORITY_6
#endif

#ifndef NCS_XLIM_TASKNAME
#define NCS_XLIM_TASKNAME    "XLIM"
#endif

/* XOVR */

#ifndef NCS_XOVR_STACKSIZE
#define NCS_XOVR_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_XOVR_PRIORITY
#define NCS_XOVR_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_XOVR_TASKNAME
#define NCS_XOVR_TASKNAME    "XOVR"
#endif

/* MQ */

#ifndef NCS_MQ_STACKSIZE
#define NCS_MQ_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_MQ_PRIORITY
#define NCS_MQ_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_MQ_TASKNAME
#define NCS_MQ_TASKNAME    "MQ"
#endif

/* Defined for Unix Sockets*/
#ifndef NCS_UX_STACKSIZE
#define NCS_UX_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_UX_PRIORITY
#define NCS_UX_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_UX_TASKNAME
#define NCS_UX_TASKNAME    "UX"
#endif

/* TDS */

#ifndef NCS_TDS_STACKSIZE
#define NCS_TDS_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_TDS_PRIORITY
#define NCS_TDS_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef NCS_TDS_TASKNAME
#define NCS_TDS_TASKNAME    "TDS"
#endif

/*************************************************************************
NCS_RMS   

        RMS    Redundancy Management Service  ; Infrastructure product
               Function entry: rms_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_RMS_STACKSIZE
#define NCS_RMS_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_RMS_PRIORITY
#define NCS_RMS_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_RMS_TASKNAME
#define NCS_RMS_TASKNAME   "RMS"
#endif

/*************************************************************************
     
       C )    C O N T R O L    P L A N E   P R O T O C O L S

              - IETF 

              NOTE: Strictly speaking, some of the protocols below
              may be considered 'management plane', but are grouped
              here by product family.

**************************************************************************/

/*************************************************************************
APS_LMS  (LTCS)

       LMS    Label Management System (marketing name for LTCS  )
              Function entry: ncslms_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef APS_LMS_STACKSIZE
#define APS_LMS_STACKSIZE       NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_LMS_PRIORITY
#define APS_LMS_PRIORITY        NCS_TASK_PRIORITY_5
#endif

#ifndef APS_LMS_TASKNAME
#define APS_LMS_TASKNAME        "LMS"
#endif

#ifndef APS_LMS_LDP_STACKSIZE
#define APS_LMS_LDP_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_LMS_LDP_PRIORITY
#define APS_LMS_LDP_PRIORITY    NCS_TASK_PRIORITY_5
#endif

#ifndef APS_LMS_LDP_TASKNAME
#define APS_LMS_LDP_TASKNAME   "LDP"
#endif

#ifndef APS_LMS_RSVP_STACKSIZE
#define APS_LMS_RSVP_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_LMS_RSVP_PRIORITY
#define APS_LMS_RSVP_PRIORITY   NCS_TASK_PRIORITY_5
#endif

#ifndef APS_LMS_RSVP_TASKNAME
#define APS_LMS_RSVP_TASKNAME  "RSVP"
#endif

/*************************************************************************
APS_IPRP :

      BGP     Border Gateway Protocol
              Function entry: ncsbgrp_process_mbx()

      OSPF    Open Shortest Path First
              Function entry: ncsospf_process_mbx()

      ISIS    Intermediate system to Intermediate System
              Function entry: ncsisis_process_mbx()

      FIBC    Forwarding Information Base Client
              Function entry: ncsfib_process_mbx()

      FIBS    Forwarding Information Base Server
              Function entry: ncsfib_process_mbx()

      POLC    Policy Client
              Function entry: <no thread found?>

      POLS    Policy Server
              Function entry: ncspolser_process_mbx()

      TED     Traffic Engineering Database
              Function entry: ncsted_process_mbx()

      CSPF    Constrained Shortest Path First
              Function entry: ncscspf_pce_process_mbx()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* BGP */

/* if BGP is run using single thread */

#ifndef APS_BGP_STACKSIZE
#define APS_BGP_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_BGP_PRIORITY
#define APS_BGP_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_BGP_TASKNAME
#define APS_BGP_TASKNAME     "BGP"
#endif

/* If BGP is run in multi threaded mode*/
#ifndef APS_BGP_INTF_STACKSIZE
#define APS_BGP_INTF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_BGP_INTF_PRIORITY
#define APS_BGP_INTF_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_BGP_INTF_TASKNAME
#define APS_BGP_INTF_TASKNAME     "BGP_INTF"
#endif

#ifndef APS_BGP_ROUTE_STACKSIZE
#define APS_BGP_ROUTE_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_BGP_ROUTE_PRIORITY
#define APS_BGP_ROUTE_PRIORITY     NCS_TASK_PRIORITY_6
#endif

#ifndef APS_BGP_ROUTE_TASKNAME
#define APS_BGP_ROUTE_TASKNAME     "BGP_ROUTE"
#endif

#ifndef APS_BGP_CTRL_STACKSIZE
#define APS_BGP_CTRL_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_BGP_CTRL_PRIORITY
#define APS_BGP_CTRL_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_BGP_CTRL_TASKNAME
#define APS_BGP_CTRL_TASKNAME     "BGP_CTRL"
#endif

/* OSPF */

/* OSPF in single thread mode */

#ifndef APS_OSPF_STACKSIZE
#define APS_OSPF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_OSPF_PRIORITY
#define APS_OSPF_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_OSPF_TASKNAME
#define APS_OSPF_TASKNAME     "OSPF"
#endif

/* OSPF in multi threaded mode */

#ifndef APS_OSPF_INTF_STACKSIZE
#define APS_OSPF_INTF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_OSPF_INTF_PRIORITY
#define APS_OSPF_INTF_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_OSPF_INTF_TASKNAME
#define APS_OSPF_INTF_TASKNAME     "OSPF_INTF"
#endif

#ifndef APS_OSPF_RDB_STACKSIZE
#define APS_OSPF_RDB_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_OSPF_RDB_PRIORITY
#define APS_OSPF_RDB_PRIORITY     NCS_TASK_PRIORITY_6
#endif

#ifndef APS_OSPF_RDB_TASKNAME
#define APS_OSPF_RDB_TASKNAME     "OSPF_RDB"
#endif

#ifndef APS_OSPF_CTRL_STACKSIZE
#define APS_OSPF_CTRL_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_OSPF_CTRL_PRIORITY
#define APS_OSPF_CTRL_PRIORITY     NCS_TASK_PRIORITY_7
#endif

#ifndef APS_OSPF_CTRL_TASKNAME
#define APS_OSPF_CTRL_TASKNAME     "OSPF_CTRL"
#endif

/* ISIS */

/* ISIS in single thread mode */

#ifndef APS_ISIS_STACKSIZE
#define APS_ISIS_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_ISIS_PRIORITY
#define APS_ISIS_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_ISIS_TASKNAME
#define APS_ISIS_TASKNAME     "ISIS"
#endif

/* ISIS in multi threaded mode */

#ifndef APS_ISIS_INTF_STACKSIZE
#define APS_ISIS_INTF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_ISIS_INTF_PRIORITY
#define APS_ISIS_INTF_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_ISIS_INTF_TASKNAME
#define APS_ISIS_INTF_TASKNAME     "ISIS_INTF"
#endif

#ifndef APS_ISIS_RDB_STACKSIZE
#define APS_ISIS_RDB_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_ISIS_RDB_PRIORITY
#define APS_ISIS_RDB_PRIORITY     NCS_TASK_PRIORITY_6
#endif

#ifndef APS_ISIS_RDB_TASKNAME
#define APS_ISIS_RDB_TASKNAME     "ISIS_RDB"
#endif

#ifndef APS_ISIS_CTRL_STACKSIZE
#define APS_ISIS_CTRL_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_ISIS_CTRL_PRIORITY
#define APS_ISIS_CTRL_PRIORITY     NCS_TASK_PRIORITY_7
#endif

#ifndef APS_ISIS_CTRL_TASKNAME
#define APS_ISIS_CTRL_TASKNAME     "ISIS_CTRL"
#endif

/* FIBC */

#ifndef APS_FIBC_STACKSIZE
#define APS_FIBC_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_FIBC_PRIORITY
#define APS_FIBC_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef APS_FIBC_TASKNAME
#define APS_FIBC_TASKNAME    "FIBC"
#endif

/* FIBS */

#ifndef APS_FIBS_STACKSIZE
#define APS_FIBS_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_FIBS_PRIORITY
#define APS_FIBS_PRIORITY    NCS_TASK_PRIORITY_4
#endif

#ifndef APS_FIBS_TASKNAME
#define APS_FIBS_TASKNAME    "FIBS"
#endif

/* POLICY Server */
#ifndef NCS_POLS_STACKSIZE
#define NCS_POLS_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_POLS_PRIORITY
#define NCS_POLS_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_POLS_TASKNAME
#define NCS_POLS_TASKNAME  "POLS"
#endif

/* Policy Client */
#ifndef NCS_POLC_STACKSIZE
#define NCS_POLC_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_POLC_PRIORITY
#define NCS_POLC_PRIORITY    NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_POLC_TASKNAME
#define NCS_POLC_TASKNAME    "POLC"
#endif

/* TED */

#ifndef APS_TED_STACKSIZE
#define APS_TED_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_TED_PRIORITY
#define APS_TED_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_TED_TASKNAME
#define APS_TED_TASKNAME     "TED"
#endif

/* CSPF */

#ifndef APS_CSPF_STACKSIZE
#define APS_CSPF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_CSPF_PRIORITY
#define APS_CSPF_PRIORITY     NCS_TASK_PRIORITY_7
#endif

#ifndef APS_CSPF_TASKNAME
#define APS_CSPF_TASKNAME     "CSPF"
#endif

/* IGMP */

/* if IGMP is run using single thread */

#ifndef APS_IGMP_STACKSIZE
#define APS_IGMP_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_IGMP_PRIORITY
#define APS_IGMP_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_IGMP_TASKNAME
#define APS_IGMP_TASKNAME     "IGMP"
#endif

/* If IGMP is run in multi threaded mode*/
#ifndef APS_IGMP_INTF_STACKSIZE
#define APS_IGMP_INTF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_IGMP_INTF_PRIORITY
#define APS_IGMP_INTF_PRIORITY     NCS_TASK_PRIORITY_3
#endif

#ifndef APS_IGMP_INTF_TASKNAME
#define APS_IGMP_INTF_TASKNAME     "IGMP-INTF"
#endif

#ifndef APS_IGMP_MAIN_STACKSIZE
#define APS_IGMP_MAIN_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_IGMP_MAIN_PRIORITY
#define APS_IGMP_MAIN_PRIORITY     NCS_TASK_PRIORITY_5
#endif

#ifndef APS_IGMP_MAIN_TASKNAME
#define APS_IGMP_MAIN_TASKNAME     "IGMP-MAIN"
#endif

/* PIM */

/* if PIM is run using single thread */

#ifndef APS_PIM_STACKSIZE
#define APS_PIM_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_PIM_PRIORITY
#define APS_PIM_PRIORITY     NCS_TASK_PRIORITY_4
#endif

#ifndef APS_PIM_TASKNAME
#define APS_PIM_TASKNAME     "PIM"
#endif

/* If PIM is run in multi threaded mode*/
#ifndef APS_PIM_INTF_STACKSIZE
#define APS_PIM_INTF_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_PIM_INTF_PRIORITY
#define APS_PIM_INTF_PRIORITY     NCS_TASK_PRIORITY_5
#endif

#ifndef APS_PIM_INTF_TASKNAME
#define APS_PIM_INTF_TASKNAME     "PIM_INTF"
#endif

#ifndef APS_PIM_MAIN_STACKSIZE
#define APS_PIM_MAIN_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef APS_PIM_MAIN_PRIORITY
#define APS_PIM_MAIN_PRIORITY     NCS_TASK_PRIORITY_5
#endif

#ifndef APS_PIM_MAIN_TASKNAME
#define APS_PIM_MAIN_TASKNAME     "PIM_MAIN"
#endif

/*************************************************************************
NCS_LMP  

      LMP    Link Management Protocol 
             Function entry:  ncslmp_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_LMP_STACKSIZE
#define NCS_LMP_STACKSIZE     NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_LMP_PRIORITY
#define NCS_LMP_PRIORITY      NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_LMP_TASKNAME
#define NCS_LMP_TASKNAME      "LMP"
#endif

/*************************************************************************
     
       C )    C O N T R O L    P L A N E   P R O T O C O L S

              - ATM 

              NOTE: Strictly speaking, some of the protocols below
              may be considered 'management plane', but are grouped
              here by product family.

**************************************************************************/

/*************************************************************************
NCS_SIGL  ATM Signalling

        SIGL   ATM Signalling (Q.2931 and Q.2971) thread proper
               Function entry: ncsig_process_events()

        SAAL   Q.2110 and Q.2130, in your typical configuration.
               Function entry: sscop_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* SIGL */

#ifndef NCS_SIGL_STACKSIZE
#define NCS_SIGL_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_SIGL_PRIORITY
#define NCS_SIGL_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_SIGL_TASKNAME
#define NCS_SIGL_TASKNAME  "SIGL"
#endif

/* SAAL */

#ifndef NCS_SAAL_STACKSIZE
#define NCS_SAAL_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_SAAL_PRIORITY
#define NCS_SAAL_PRIORITY    NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_SAAL_TASKNAME
#define NCS_SAAL_TASKNAME    "SAAL"
#endif

/*************************************************************************
NCS_LANES

      LEC    ATM Lan Emulation client
             Function entry: lec_process_events()

      LES    ATM Lan Emulation server
             Function entry: ncsles_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* LEC */

#ifndef NCS_LANES_LEC_STACKSIZE
#define NCS_LANES_LEC_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_LANES_LEC_PRIORITY
#define NCS_LANES_LEC_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_LANES_LEC_TASKNAME
#define NCS_LANES_LEC_TASKNAME  "LEC"
#endif

/* LES */

#ifndef NCS_LANES_LES_STACKSIZE
#define NCS_LANES_LES_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_LANES_LES_PRIORITY
#define NCS_LANES_LES_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_LANES_LES_TASKNAME
#define NCS_LANES_LES_TASKNAME  "LES"
#endif

/*************************************************************************
NCS_ILMI  

       ILMI   Interum Layer Management Interface\
              Function entry: ncsil_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_ILMI_STACKSIZE
#define NCS_ILMI_STACKSIZE      NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_ILMI_PRIORITY
#define NCS_ILMI_PRIORITY       NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_ILMI_TASKNAME
#define NCS_ILMI_TASKNAME       "ILMI"
#endif

/*************************************************************************
NCS_PNNI Private Network to Network Protocol (ATM Routing)

      PNNI       PNNI as a single thread entity, 
                 Function entry: ncspnni_process_mbx()      
                 else a thread per component, as in.....

      PNNI_HELLO Hello Protocol thread
                 Function entry: ncspnni_hello_process_mbx()

      PNNI_PGLE  Peer Group Leader Election
                 Function entry: ncspnni_pgle_process_mbx()

      PNNI_DB    PNNI Routing Database
                 Function entry: ncspnni_db_process_mbx()

      PNNI_NBR   PNNI Neighbor
                 Function entry: ncspnni_process_nbr_mbx()
  
    
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* PNNI */

#ifndef NCS_PNNI_STACKSIZE
#define NCS_PNNI_STACKSIZE      NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_PNNI_PRIORITY
#define NCS_PNNI_PRIORITY       NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_PNNI_TASKNAME
#define NCS_PNNI_TASKNAME       "PNNI"
#endif

/* PNNI HELLO */

#ifndef NCS_PNNI_HELLO_STACKSIZE
#define NCS_PNNI_HELLO_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_PNNI_HELLO_PRIORITY
#define NCS_PNNI_HELLO_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_PNNI_HELLO_TASKNAME
#define NCS_PNNI_HELLO_TASKNAME  "PNNI_HELLO"
#endif

/* PNNI PGLE */

#ifndef NCS_PNNI_PGLE_STACKSIZE
#define NCS_PNNI_PGLE_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_PNNI_PGLE_PRIORITY
#define NCS_PNNI_PGLE_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_PNNI_PGLE_TASKNAME
#define NCS_PNNI_PGLE_TASKNAME  "PNNI_PGLE"
#endif

/* PNNI DB */

#ifndef NCS_PNNI_DB_STACKSIZE
#define NCS_PNNI_DB_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_PNNI_DB_PRIORITY
#define NCS_PNNI_DB_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_PNNI_DB_TASKNAME
#define NCS_PNNI_DB_TASKNAME   "PNNI_DB"
#endif

/* PNNI NBR */

#ifndef NCS_PNNI_NBR_STACKSIZE
#define NCS_PNNI_NBR_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_PNNI_NBR_PRIORITY
#define NCS_PNNI_NBR_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_PNNI_NBR_TASKNAME
#define NCS_PNNI_NBR_TASKNAME   "PNNI_NBR"
#endif

/*************************************************************************
NCS_IPOA   

       IPOA   IP Over ATM Classical
              Function entry: ipoa_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_IPOA_STACKSIZE
#define NCS_IPOA_STACKSIZE       NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_IPOA_PRIORITY
#define NCS_IPOA_PRIORITY        NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_IPOA_TASKNAME
#define NCS_IPOA_TASKNAME        "IPOA"
#endif

/*************************************************************************
NCS_CMS 

      CMS        Connection Management System as a single thread entity 
                 Function entry: rms_process_events()      
                 else a thread per component, as in.....

      CMS_CFG    Configuration thread
                 Function entry: cms_cfg_process_mbx()

      CMS_INTF   Interface card thread
                 Function entry: cms_intf_process_mbx()

      CMS_SMH    Signal Message Handler thread
                 Function entry: cms_smh_process_mbx()

      CMS_ROUTE  Route management thead
                 Function entry: cms_route_process_events()
  
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* CMS */

#ifndef NCS_CMS_STACKSIZE
#define NCS_CMS_STACKSIZE    NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_CMS_PRIORITY
#define NCS_CMS_PRIORITY     NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_CMS_TASKNAME
#define NCS_CMS_TASKNAME     "CMS"
#endif

/* CMS CFG */

#ifndef NCS_CMS_CFG_STACKSIZE
#define NCS_CMS_CFG_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_CMS_CFG_PRIORITY
#define NCS_CMS_CFG_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_CMS_CFG_TASKNAME
#define NCS_CMS_CFG_TASKNAME  "CMS_CFG"
#endif

/* CMS INTF */

#ifndef NCS_CMS_INTF_STACKSIZE
#define NCS_CMS_INTF_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_CMS_INTF_PRIORITY
#define NCS_CMS_INTF_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_CMS_INTF_TASKNAME
#define NCS_CMS_INTF_TASKNAME  "CMS_INTF"
#endif

/* CMS SMH */

#ifndef NCS_CMS_SMH_STACKSIZE
#define NCS_CMS_SMH_STACKSIZE   NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_CMS_SMH_PRIORITY
#define NCS_CMS_SMH_PRIORITY    NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_CMS_SMH_TASKNAME
#define NCS_CMS_SMH_TASKNAME    "CMS_SMH"
#endif

/* CMS ROUTE */

#ifndef NCS_CMS_ROUTE_STACKSIZE
#define NCS_CMS_ROUTE_STACKSIZE NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_CMS_ROUTE_PRIORITY
#define NCS_CMS_ROUTE_PRIORITY  NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_CMS_ROUTE_TASKNAME
#define NCS_CMS_ROUTE_TASKNAME  "CMS_RTE"
#endif

/*************************************************************************
NCS_FRF5  

    FRF5  Frame relay ATM adaptation protocol
          Function entry: <dont know>
                  
NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_FRF5_STACKSIZE
#define NCS_FRF5_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_FRF5_PRIORITY
#define NCS_FRF5_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_FRF5_TASKNAME
#define NCS_FRF5_TASKNAME   "FRF5"
#endif

/*************************************************************************
NCS_FRF8  

     FRF8  Frame Relay ATM adaptation protocol
           Function entry: <dont know>         

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

#ifndef NCS_FRF8_STACKSIZE
#define NCS_FRF8_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_FRF8_PRIORITY
#define NCS_FRF8_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_FRF8_TASKNAME
#define NCS_FRF8_TASKNAME   "FRF8"
#endif

/*************************************************************************
     
       C )    C O N T R O L    P L A N E   P R O T O C O L S

              - Frame Relay 

              NOTE: This product has both control plane and data plane 
              features.

**************************************************************************/

/*************************************************************************
NCS_HPFR  High Performance Frame Relay

        HPFR      LMI (and data plane)
                  Function entry: frs_process_events()

        HPFR_LMI  LMI only
                  Function entry: frs_lmi_process_events()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* HPFR */

#ifndef NCS_HPFR_STACKSIZE
#define NCS_HPFR_STACKSIZE      NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_HPFR_PRIORITY
#define NCS_HPFR_PRIORITY       NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_HPFR_TASKNAME
#define NCS_HPFR_TASKNAME       "HPFR"
#endif

/* HPFR LMI */

#ifndef NCS_HPFR_LMI_STACKSIZE
#define NCS_HPFR_LMI_STACKSIZE  NCS_STACKSIZE_MEDIUM
#endif

#ifndef NCS_HPFR_LMI_PRIORITY
#define NCS_HPFR_LMI_PRIORITY   NCS_TASK_PRIORITY_8
#endif

#ifndef NCS_HPFR_LMI_TASKNAME
#define NCS_HPFR_LMI_TASKNAME   "HPFR_LMI"
#endif
/*************************************************************************
     
       D )    M A N A G E M E N T   P L A N E   S U B S Y S T E M S

**************************************************************************/

/*************************************************************************
NCS_CLI   

     CLI      Command Line Interface; drives character IO operations.
              Function entry:  ncscli_begin()

     CLI SHIM Protects CLI thread from subsystem havoc by exporting 
              subsystem specific work to this dedicated thread.
              Function entry:  shim_mbx_handler()

NOTE: These mnemonics do not yet mapped to this task's creation properties.
**************************************************************************/

/* CLI */

#ifndef NCS_CLI_STACKSIZE
#define NCS_CLI_STACKSIZE      NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_CLI_PRIORITY
#define NCS_CLI_PRIORITY       NCS_TASK_PRIORITY_7
#endif

#ifndef NCS_CLI_TASKNAME
#define NCS_CLI_TASKNAME       "CLI"
#endif

/* CLI SHIM */

#ifndef NCS_CLI_SHIM_STACKSIZE
#define NCS_CLI_SHIM_STACKSIZE NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_CLI_SHIM_PRIORITY
#define NCS_CLI_SHIM_PRIORITY  NCS_TASK_PRIORITY_6
#endif

#ifndef NCS_CLI_SHIM_TASKNAME
#define NCS_CLI_SHIM_TASKNAME  "CLI_SHIM"
#endif

   /*    DTS Task    */
#ifndef NCS_DTS_STACKSIZE
#define NCS_DTS_STACKSIZE      NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_DTS_PRIORITY
#define NCS_DTS_PRIORITY       NCS_TASK_PRIORITY_7
#endif

#ifndef NCS_DTS_TASKNAME
#define NCS_DTS_TASKNAME       "DTS"
#endif

   /*    DTA Task    */
#ifndef NCS_DTA_STACKSIZE
#define NCS_DTA_STACKSIZE      NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_DTA_PRIORITY
#define NCS_DTA_PRIORITY       NCS_TASK_PRIORITY_7
#endif

#ifndef NCS_DTA_TASKNAME
#define NCS_DTA_TASKNAME       "DTA"
#endif

#endif   /* NCS_TASKS.H */
