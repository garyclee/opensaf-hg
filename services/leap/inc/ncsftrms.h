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

  This module contains structure definitions that provide the interface hooks
  between the generic Fault Tolerant engine and a subscribing backend subhsystem.

******************************************************************************
*/


/*
 * Module Inclusion Control...
 */

#ifndef NCSFTRMS_H
#define NCSFTRMS_H

#include "ncsgl_defs.h"
#include "ncs_svd.h"
#include "ncsusrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  NCSFT_REDUNDANT_ENTITY_DESCRIPTOR
  ********************************

  This structure defines a redundant entity as it pertains to any subsystem.
  Each subsystem may map the internals of this entity as it so desires. 
  
  The redundant entity descriptor is used by the Generic State Machine to 
  map an incoming packet to the correct Fault Tolerant Control Block. Several
  FTCBs (Fault Tolerant Control Blocks) may relate to the same subsystem if
  several redundant entities could exist for a subsystem. For instance for 
  signalling, a redundant entity is an interface. Hence, there will be a FTCB
  that maps to each interface the signalling stack is backing up. 

  Secondly, the redundant entity descriptor is also used by the system services
  provided by the target environment to route packets from/to the primary and 
  backup in the redundant pair. For instance the target could have an IP address
  associated with the redundant entity descriptor so it can route packets when
  backup related data needs to be sent across processors. 

  The information in the redundnant entity descriptor is not stored in the 
  Generic header in the fault tolerant packets since it is received as a 
  parameter when data is received. 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/*
 **********************************
 * NCSFT Redundant Entity Descriptor 
 **********************************
 */

typedef struct ncsft_red
{
  /* H&J or non-H&J subsystem */
  uns16  subsystem; 

  /* The subsystem_version is returned when ncsft_register() is called
   * to register a subsystem with the Generic Fault Tolerant Engine.
   */
  uns16  subsystem_version;  

  /* This is a redundant unit within a subsystem. Each subsystem has 
   * different characteristics regarding it's components that can be
   * backed up. For instance for signalling this maps to an interface
   */
  uns32 redundant_entity_id[4]; 

  /* The collection identifier is used to group redundant entities
   * together. There are APIs provided that can change role by collection
   * ID for instance.
   */
  uns32 collection_id; 

  /* The collection priority is used in conjunction with the collection
   * ID to act on redundant entities. The priority determines the order
   * in which the entities are processed within a collection. 
   */
  uns32 collection_priority; 

  /* This can be used by the target environment - RMS does not modify
   * or use this field 
   */
  void* opaque;

} NCSFT_RED;

#define NCSFT_RED_NULL ((NCSFT_RED*)0)

#define NEW_STYLE_RED_MARKER  0xf000000f


#endif

