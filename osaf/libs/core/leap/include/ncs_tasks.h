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
     
       D )    M A N A G E M E N T   P L A N E   S U B S Y S T E M S

**************************************************************************/

/* DTS Task */

#ifndef NCS_DTS_STACKSIZE
#define NCS_DTS_STACKSIZE      NCS_STACKSIZE_HUGE
#endif

#ifndef NCS_DTS_PRIORITY
#define NCS_DTS_PRIORITY       NCS_TASK_PRIORITY_7
#endif

#ifndef NCS_DTS_TASKNAME
#define NCS_DTS_TASKNAME       "DTS"
#endif

/* DTA Task */

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
