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

 MODULE NAME:  NCS_KMS.H
..............................................................................

  The Key Map Service

  DESCRIPTION:
  ============

  This service provides a means for the AKE to store strict relationships 
  between a PWE-id and specific data-points that map to that PWE-id. 

  Example data points understood so far include:

  MDS PWE handle - (NCSCONTEXT) Used by MDS Normal Services in the context 
                   of such MDS services as INSTALL, SEND, BCAST and SUBSCRIBE.
  MIB Key        - (NCS_KEY) In any given system this may be a MAC key 
                   (distributed systems) or a subsystem specific key (MIB 
                   tables are local to client). 

  CLIENTS
  =======

  The AKE (see API ncskms_lm())
  ============================

  The AKE knows how to create() and destroy() the Key Map Service. At
  initialization and runtime it knows how to add() and rmv() key mappings
  as subsystem resources come and go.
  
  Subsystem Clients (see ncskms_ss())
  ==================================
  
  Subsystem clients, such as the CEF functions of the NetPlane CLI, need to
  identify which instance of a subystem they need to address. They can do
  this in one of two ways.. Get a key record 'by_name()' or 'by_pwe()' id
  value. This is happening at runtime we imagine.

******************************************************************************
*/

#ifndef NCS_KMS_H
#define NCS_KMS_H

/**************************************************************************

                          N e t P l a n e 
                          
              K e y    M a p p i n g    S e r v i c e
          
       U s e d    B y    S u b s y s t e m    C l i e n t s   ( S S )

 There is no Per Virtual Router requirement for this service as KMS is the 
 'bridging' service that binds all such partitioned 'virtual router' 
 Services/Resources into a common system view.

 A P I s   U s e d   b y   S u b S y s t e m  C l i e n t s

 These APIs are used by subsystem clients must find the proper 'key' in order
 to address the proper instance of some class of partitioned resource, such as
 OSPF or LTCS.
    
    Fetch-by-name      -  Fetch the PWE-id and all the keys that map to a 
                          particular name. The name must be unique and is 
                          assigned by the system configuration part of the AKE.
                          

    Fetch-by-PWE-id    -  Fetch the name and all the keys that map to a particular
                          PWE-id. The PWE-id must be unique. It maps to the MDS
                          assigned PWE-id value used to scope messaging services.

****************************************************************************/

/*****************************************************************************
 H J K M S _ B Y _ N A M E
 
  Fetch the NCSKMS_REC by name.
 *****************************************************************************/

typedef struct ncskms_by_name {
	char *i_name;		/* name of PWE-id Key record */
	NCS_SERVICE_ID i_ss_id;	/* service identifier of key desired */
	NCS_KEY *o_key;		/* If success, get a ptr to internal NCS_KEY */

} NCSKMS_BY_NAME;

/*****************************************************************************
 H J K M S _ B Y _ P W E 
 
  Fetch the NCSKMS_REC by PWE id.
 *****************************************************************************/

typedef struct ncskms_by_pwe {
	uns32 i_pwe;		/* PWE-id value mapping to proper record   */
	NCS_SERVICE_ID i_ss_id;	/* service identifier of key desired */
	NCS_KEY *o_key;		/* If success, get a ptr to internal NCS_KEY */

} NCSKMS_BY_PWE;

/* Operations mnemonics for KMS Services used by subsystem clients       */

typedef enum ncskms_ss_op {
	/* Subsystem clients use to get proper subsystem key value */

	NCSKMS_SS_GET_BY_NAME,
	NCSKMS_SS_GET_BY_PWE
} NCSKMS_SS_OP;

/***************************************************************************
  H J K M S _ S S _ A R G
 
  The Key Management Service used by a particular subsystem clients to get
  the associated key value they are looking for.

 ***************************************************************************/

typedef struct ncskms_ss_arg {

	NCSKMS_SS_OP i_op;

	union {

		/* used by subsystem clients to fetch proper NCS_KEY for target subsystem */

		NCSKMS_BY_NAME by_name;
		NCSKMS_BY_PWE by_pwe;

	} op;

} NCSKMS_SS_ARG;

/* The master entry point for KMS Service used by subsystem clients */

EXTERN_C LEAPDLL_API uns32 ncskms_ss(NCSKMS_SS_ARG *arg);

/**************************************************************************

                          N e t P l a n e 
                          
             K e y   M a n a g e m e n t   S e r v i c e s
             
              U s e d   b y   L a y e r   M g m t
                    

 Per Virtual Router API Structures and abstractions to support these svcs.  

    Create         - Create THE Key Map Service. There is only one per memory
                     space.
    Destroy        - Destroy THE Key Map Service. Recover all resources.
    
    bind_name      - associate a string name with a a PWE-id.
    add            - Add an NCS_KEY with a svc-name to a PWE-id.
    rmv            - Remove an NCS_KEY from a PWE-id that maps to a svc-name.

****************************************************************************/

/***************************************************************************
 H J K M S _ C R E A T E

 Create THE Key Map Service for this environment.
 ***************************************************************************/

typedef struct ncskms_create {
	uns32 i_meaningless;	/* keep compiler happy                     */

} NCSKMS_CREATE;

/***************************************************************************
 H J K M S _ D E S T R O Y

 Recover all system resources associated with THE Key Map Service.
 ***************************************************************************/

typedef struct ncskms_destroy {
	uns32 i_meaningless;	/* keep compiler happy                     */

} NCSKMS_DESTROY;

/***************************************************************************
 H J K M S _ A D D _ R E C

 Create a KMS record.

 ***************************************************************************/

typedef struct ncskms_add_rec {
	uns32 i_pwe_id;		/* The PWE-id to which these keys are mapped  */
	char *i_name;		/* The name that this PWE-id shall go by */

} NCSKMS_ADD_REC;

/***************************************************************************
 H J K M S _ R M V _ R E C

 Removes a KMS record.

 ***************************************************************************/

typedef struct ncskms_rmv_rec {
	uns32 i_pwe_id;		/* The PWE-id to which these keys are mapped  */

} NCSKMS_RMV_REC;

/***************************************************************************
 H J K M S _ A D D _ K E Y

 Add an NCS_KEY record to the set of keys associated with the PWE-id. 
 SUCCESS if key is well formed and no other key that goes by the
 Subsystem ID embedded in the NCS_KEY.

 ***************************************************************************/

typedef struct ncskms_add_key {
	uns32 i_pwe_id;		/* The PWE-id to which these keys are mapped  */
	NCS_KEY *i_key;		/* the keys id-ed via the exist bitmap to add */

} NCSKMS_ADD_KEY;

/***************************************************************************
 H J K M S _ R M V _ K E Y 

 Remove an NCS_KEY record from the set of keys associated with the PWE-id
 based on passed NCS_SERVICE_ID value.
 SUCCESS if key is found and destroyed.

 ***************************************************************************/

typedef struct ncskms_rmv_key {
	uns32 i_pwe_id;		/* The PWE-id to which keys shall be removed */
	NCS_SERVICE_ID i_ss_id;	/* the service ID of the Key to remove       */

} NCSKMS_RMV_KEY;

/* Operations mnemonics for a KMS Service used by The Layer Management */

typedef enum ncskms_lm_op {
	NCSKMS_LM_CREATE,
	NCSKMS_LM_DESTROY,

	NCSKMS_LM_ADD_REC,
	NCSKMS_LM_RMV_REC,
	NCSKMS_LM_ADD_KEY,
	NCSKMS_LM_RMV_KEY
} NCSKMS_LM_OP;

/***************************************************************************
  H J K M S _ L M _ A R G
 
  'The System' (or AKE) wants to create and destroy THE Key Map Service.
  The system may also wish to map or unmap particular keys to particular
  PWE-ids.
 ***************************************************************************/

typedef struct ncskms_lm_arg {
	NCSKMS_LM_OP i_op;

	union {

		NCSKMS_CREATE create;	/* Create THE Key Map Service              */
		NCSKMS_DESTROY destroy;	/* destroy THE Key Map Service             */

		NCSKMS_ADD_REC add_rec;	/* name is specified here */
		NCSKMS_RMV_REC rmv_rec;
		NCSKMS_ADD_KEY add_key;	/* Add keys that map to a particular PWE   */
		NCSKMS_RMV_KEY rmv_key;	/* Remove kesy that map to a particular PWE */

	} op;

} NCSKMS_LM_ARG;

/* The master entry point for Layer Management Service used by the AKE */

EXTERN_C LEAPDLL_API uns32 ncskms_lm(NCSKMS_LM_ARG *arg);

/**************************************************************
   KMS Locking Entry point interface....      
                                                                
 **************************************************************/

EXTERN_C LEAPDLL_API uns32 ncskms_lock(void);
EXTERN_C LEAPDLL_API uns32 ncskms_unlock(void);

#endif
