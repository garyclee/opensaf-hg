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

  DESCRIPTION:

  This module contains declarations pertaining to various conditional compile
  options for the Harris & Jeffries Soft-ATM Product Suite.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_OPT_H
#define NCS_OPT_H


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/******************************************************************************

                  D E P R I C A T I O N   N O T I C E
  
  As of LEAP 2.3, NetPlane subsystem packages are no longer obliged to
  honor the following compile flags:
  
  USE_MMGR_SIGNATURES
  USE_MMGR_CONTAINERS
  
  These are artifacts of a simpler time in NetPlane history and do not
  provide any significant benefit to NetPlane or our customers.
  
  The text that follows, is the orginal ncs_opt.h text and flag settings
  for these compile flags, which are now obsolete.
  
  =========

  The following flag causes a small (4-octet, usually) debug "signature"
  to be added and initialized into memory blocks allocated by the system
  by the various m_MMGR_... macros.  Use of this "signature" is purely
  for debug purposes, so that a programmer can more easily verify that
  structures are correct.  Disabling the flag (by setting to 0) will
  slightly reduce memory overhead and improve performance.
  
  #ifndef USE_MMGR_SIGNATURES
  #define USE_MMGR_SIGNATURES 0
  #endif


  The following flag causes a char pointer "Container" field to be created
  within the memory blocks allocated by the m_MMGR_... macros.  Use of this
  field is PORTATION-DEPENDENT.  It MAY BE used to point to the system memory
  block associated with freeing the structure.  The "Container" would not be
  necessary (for example) if the m_MMGR_ALLOC_... macros return the
  actual address used in freeing the block back to system memory.
  As a default, this field is enabled, which adds a pointer-sized overhead
  to each structure.  In order to conserve memory, targets not needing the
  "Container" can disable this at compile time by changing the constant to '0'.
  Refer to the Porting Guide for more information.
  
  #ifndef USE_MMGR_CONTAINERS
  #define USE_MMGR_CONTAINERS 0
  #endif

******************************************************************************/

/** Select the appropriate base packages that you have acquired from Harris
   and Jeffries, Inc.
 **/

#ifndef NCS_IPV6
#define NCS_IPV6             0   /* Enable IPV6 */
#endif

#ifndef NCS_SIGL
#define NCS_SIGL             0   /* UNI Signalling */
#endif

#ifndef NCS_SAAL
#define NCS_SAAL            0   /* SAAL */
#endif

#ifndef NCS_LANES_LEC
#define NCS_LANES_LEC        0   /* LAN Emulation Client */
#endif

#ifndef NCS_LANES_SERVER
#define NCS_LANES_SERVER     0   /* LAN Emulation Server Components */
#endif

#ifndef NCS_ILMI
#define NCS_ILMI             0   /* ILMI */
#endif

#ifndef NCS_PNNI
#define NCS_PNNI             0   /* PNNI */
#endif

#ifndef NCS_IPOA
#define NCS_IPOA             0   /* IPOA (IP Over ATM) */
#endif

#ifndef NCS_FRF8
#define NCS_FRF8             0   /* FRF8 (FR/ATM PVC Interworking) */
#endif

#ifndef NCS_HPFR
#define NCS_HPFR             0   /* High Performance Frame Relay */
#endif

#ifndef NCS_FRSIGL
#define NCS_FRSIGL           0   /* Frame Relay Signalling */
#endif

#ifndef NCS_Q922
#define NCS_Q922             0   /* Q922 stack */
#endif

#ifndef NCS_FRF5
#define NCS_FRF5             0   /* FRF5 (FR/ATM PVC Network Interworking) */
#endif

#ifndef NCS_MPC
#define NCS_MPC              0   /* Multiprotocol over ATM Client */
#endif

#ifndef NCS_MPS
#define NCS_MPS              0   /* Multiprotocol over ATM Server */
#endif

#ifndef APS_LMS
#define APS_LMS              0   /* Label Management System */
#endif

#ifndef NCS_RMS
#define NCS_RMS              0   /* Redundancy Management Software Subsystem */
#endif

#ifndef NCS_RMS_WBB
#define NCS_RMS_WBB          0   /* WBB Redundancy Management Software Subsystem */
#endif


#ifndef NCS_POWERCODE
#define NCS_POWERCODE        0   /* PowerCODE Test Tool */
#endif

#ifndef NCS_MARS
#define NCS_MARS             0   /* MARS (RFC2022) */
#endif

#ifndef NCS_CMS
#define NCS_CMS              0   /* Connection Management Software Subsystem */
#endif

#ifndef NCS_MDS
#define NCS_MDS              0   /* Message Dispatch Service */
#endif

#ifndef NCS_RCP
#define NCS_RCP              0   /* Reliable Connection Protocol (SSCOP) */
#endif

#ifndef APS_IPRP
#define APS_IPRP             0   /* IPRP Routing Protocol */
#endif  /* APS_IPRP */

/* NCS_CLI flag definition moved to LEAP-private gl_defs.h to ensure 
   it is included after ncsgl_defs.h for the NCS_CLI definition
*/

#ifndef APS_INTF_SUBSYSTEM
#define APS_INTF_SUBSYSTEM   0   /* Target Services implementation of Interface Subsystem */ 
#endif 

#ifndef NCS_MAB
#define NCS_MAB              0   /* MIB Access Broker */
#endif

#ifndef NCS_A2SIG
#define NCS_A2SIG            0   /* AAL 2 Signalling */
#endif

#ifndef NCS_LMP
#define NCS_LMP              0   /* Link Management Protocol */
#endif


#ifndef NCS_OGI
#define NCS_OGI              0   /* OUNI to GMPLS interworking */
#endif


/** Based on the above subsystem settings, automatically set the
 ** NCS_ATM_SERVICES compile flag so that ATM common code can be conditionally
 ** compiled.
 **/
#ifndef NCS_ATM_SERVICES

#if ((NCS_SIGL == 1)         || \
     (NCS_SAAL == 1)         || \
     (NCS_A2SIG == 1)        || \
     (NCS_LANES_LEC == 1)    || \
     (NCS_LANES_SERVER == 1) || \
     (NCS_ILMI == 1)         || \
     (NCS_PNNI == 1)         || \
     (NCS_IPOA == 1)         || \
     (NCS_MARS == 1)         || \
     (NCS_FRF8 == 1)         || \
     (NCS_FRF5 == 1)         || \
     (NCS_MPC == 1)          || \
     (APS_LMS == 1)          || \
     (NCS_LMP == 1)          || \
     (NCS_MPS == 1))       /*   || \
     (NCS_MDS == 1)) */

#define NCS_ATM_SERVICES                 1
#else
#define NCS_ATM_SERVICES                 0
#endif

#endif /* ifndef NCS_ATM_SERVICES */


/** Based on the above subsystem settings, automatically set the
 ** NCS_FR_SERVICES compile flag so that Frame Relay common code can be
 ** conditionally compiled.
 **/
#ifndef NCS_FR_SERVICES

#if ((NCS_HPFR == 1)    || \
     (NCS_FRSIGL == 1)  || \
     (APS_LMS == 1)     || \
     (NCS_Q922 == 1))
#define NCS_FR_SERVICES                 1
#else
#define NCS_FR_SERVICES                 0
#endif

#endif /* ifndef NCS_FR_SERVICES */

/** Set NCS_IP_SERVICES based on APS_LMS set above */
#ifndef NCS_IP_SERVICES
#if ((APS_LMS == 1)     || \
     (NCS_MDS == 1))
#define NCS_IP_SERVICES                 1
#else
#define NCS_IP_SERVICES                 0
#endif
#endif /* ifndef NCS_IP_SERVICES */

/** Some 64 bit machines (SPARC) expect allocated memory to start on an
    8 byte boundary. Other processors may have such boundary issues
    (though NetPlane knows of no other). NetPlane's instrumented 
    sysfpool.c mem-manager can put thisboundary requirement out of whack.
    
    The value of this #define changes the declaration size of an uns8 array
    embedded in the 'offending' sysfpool data structure. If 0, the array
    is benign (declaration is removed; no 'pad' overhead). 
    
    NOTE 1: Even setting this bias to 1 causes WINDOWS to pad by 4. Most
    compilers/machines probably round off in some fashion.
    NOTE 2: This bias may need to be revisited if certain sysfpool.h DEBUG
    flags are enabled.
**/
#ifndef NCS_MEM_ALLOC_PAD_BIAS
#define NCS_MEM_ALLOC_PAD_BIAS       0
#endif

/** Systems using the Intel x86 family processors or other processors where
    multi-octet values are stored LSB-first must set this flag to one. In
    systems where multi-octet values are stored MSB-first (Big-Endian),
    please set this flag to zero.
**/
#ifndef USE_LITTLE_ENDIAN
#define USE_LITTLE_ENDIAN       1
#endif

/** Target (machine) is Word Boundary Sensitive (all but chars on 4 byte
    boundaries, or ELSE!!). Intel x86 family processors are NOT word boudary 
    sensitive (default value 0). Solaris machines tend to be Word Boundary
    sensitive (set to 1).
**/

#ifndef TRGT_IS_WBS
#define TRGT_IS_WBS             0
#endif


/** NetPlane Subsystem products provide typedefs for uns8, uns16, uns32 and 
    other useful data types. If the target system also uses these typedef 
    names or wishes to map these types to target system specific definitions
    then this flag is relevant. IF this flag is set to 1 NetPlane Subsystem
    products will use typedefs provided by the target system as follows:
    - The NetPlane definitions found in /base/common/inc/gl_defs.h shall be
      ignored.
    - Operating system specific file osprims/<os-flavor>/os_defs.h shall
      include a companion header file called trg_defs.h, which will provide
      the system specific definitions. 
    - By default, all shipped trg_def.h definitions are identical to those
      found in gl_defs.h
 **/

#ifndef USE_TARGET_SYSTEM_TYPEDEFS
#define USE_TARGET_SYSTEM_TYPEDEFS      0
#endif


/** Some CPUs require that multibyte access operations be addressed on even
    addressed locations. This is significant when our protocol products must
   build or read a traffic frame in memory and are accessing a 16 or 32 byte
   value. In addition, at the same time of reading/writing traffic frame
   data, our products must consider network and host order of bytes. Therefore,
   our implementation for the LEAP API macros for converting network to host
   or host to network order bytes will also take into account any alignment
   requirements of the CPU by the use of the following flag.

    The default setting is 0, meaning, the CPU does not enforce multibyte access
   alignment.
 **/

#ifndef NCS_CPU_MULTIBYTE_ACCESS_ALIGNMENT
#define NCS_CPU_MULTIBYTE_ACCESS_ALIGNMENT      0
#endif


/** This is the new flag to control the version of the CPCS interface that
    is used by Soft-ATM subsystems.  This flag can coexist with the
    USE_CPCS_INTERFACE_VER flag, i.e., V1 or V2  AND later versions (e.g., V3)
    can coexist.  This is for backwards compatibility.  Subsystems which
    haven't become LEAP compliant will still be able to invoke the V2
    subroutines.  Also, if a newer version of CPCS (e.g., V4) becomes
    available at a later date, a new flag will be created (e.g.,
    ENABLE_CPCS_VERSION_4) and then both V3 and V4 can be independently
    selected for inclusion.

    To enable  Version 3:   Set this flag to 1.
    To disable Version 3:   Set this flag to 0.

 **/
#ifndef ENABLE_CPCS_VERSION_3
#define ENABLE_CPCS_VERSION_3     1
#endif


/** This flag controls the version of the CPCS interface that is used by
    Soft-ATM subsystems. The "v2" interface supports additional indications
    at the API boundary required by NCS-FRF.5 and NCS-FRF.8 subsystems.
    The older "v1" interface may be used with other Soft-ATM subsystems.
    However, H&J recommends that you upgrade your AAL5 implementation to
    support the "v2" primitives.

    To use Version 1:   Set this flag to 1.
    To use Version 2:   Set this flag to 2.

 **/
#ifndef USE_CPCS_INTERFACE_VER
#define USE_CPCS_INTERFACE_VER  2
#endif


/** This flag controls the version of the FRDRV interface that
    is used by HPFR subsystems.  Subsystems will use the highest
    version enabled. If a subsystem doesn't support the version
    enabled then it will use the highest version it supports.

    To enable Version 1:   Set this flag to 1.

 **/
#ifndef ENABLE_FRDRV_VERSION_1
#define ENABLE_FRDRV_VERSION_1  0
#endif

/** This flag controls the version of the FRDRV interface that
    is used by HPFR subsystems.  Subsystems will use the highest
    version enabled. If a subsystem doesn't support the version
    enabled then it will use the highest version it supports.

    To enable Version 2:   Set this flag to 1.

 **/
#ifndef ENABLE_FRDRV_VERSION_2
#define ENABLE_FRDRV_VERSION_2  1
#endif

/** This flag controls whether storage is allocated in the USRBUF
   structure for storage needed by the SAR.

   To allocate storage: Set this flag to 1.

 **/
#ifndef SAR_BUFR_STORAGE
#define SAR_BUFR_STORAGE  0
#endif

/** This flag should only be set to 1 IF 

    A) NCS_MDS subsystem is enabled (set to 1) in this files, AND
    B) A session type used is NCSMDS_SSN_TYPE_RAW_UD,         AND
    B) OSE is the Operating system,                          AND
    C) MDS is to use OSE's General Link Handler protocol

   Default == 0

 **/

#ifndef NCSMDS_OSE_UD 
#define NCSMDS_OSE_UD      0
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

      SIGNALLING OPTIONS RELEVANT TO OTHER SUBSYSTEMS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/** Below are the signalling options that may be compiled in. Take note that
        any combination can be enabled. Thus, in a single executable different
        ports can be configured to one of the versions enabled.
**/

/** The support for ATM Signalling V3.0.  Set the following option
    to 1 if you wish to include support for V3.0.
**/
#ifndef V3_0
#define V3_0                1
#endif


/** The support for ATM SAAL/SSCOP  QSAAL.  Set the following option
    to 1 if you wish to include support for QSAAL.
**/
#ifndef QSAAL
#define QSAAL               1
#endif


/** The support for ATM Signalling V3.1.  Set the following option
    to 1 if you wish to include support for V3.1.
**/
#ifndef V3_1
#define V3_1                1
#endif


/** The support for ATM Signalling V4.0 (UNI4_0).  Set the following option
   to 1 if you wish to include support for UNI4.0
**/
#ifndef UNI4_0
#define UNI4_0              1
#endif


/** The support for ITUT Q.2931/Q2971.  Set the following option
   to 1 if you wish to include support for Q.2931/Q.2971
**/
#ifndef Q2931
#define Q2931              1
#endif


/** The support for ATM Signalling for PNNI1.0.  Set the following option
   to 1 if you wish to include support for PNNI Signalling.
**/
#ifndef PNNI
#define PNNI                1
#endif

/** The support for PNNI Path and Connection trace.  Set the following option
   to 1 if you wish to include support for PNNI Path and Connection trace.
**/
#ifndef PNNI_TRACE
#define PNNI_TRACE          0
#endif


/** The support for ATM SAAL/SSCOP as defined in Q2110.  Set the following
    option to 1 if you wish to include support for Q2110.
**/
#ifndef Q2110
#define Q2110               1
#endif


/** The support for the network-side or user-side of ATM UNI signalling is
    optional. For example, a feeder mux may only have UNI user support
    while a switch may only support UNI network ports (if it has UNI
    interfaces at all).  Set the following options to 1 accordingly.
    NOTE: UNI_USER_SIDE_SUPPORT is shipped as 1, since prior releases
          have implicitly included it by default.
**/
#ifndef UNI_NETWORK_SIDE_SUPPORT
#define UNI_NETWORK_SIDE_SUPPORT    1
#endif

#ifndef UNI_USER_SIDE_SUPPORT
#define UNI_USER_SIDE_SUPPORT       1
#endif


/** The support for IISP is optional. Set the following option to 1 if you
    wish to include support for IISP. Note that this will automatically
    enable UNI_NETWORK_SIDE_SUPPORT.
**/
#ifndef IISP_SUPPORT
#define IISP_SUPPORT                1
#endif

#if (IISP_SUPPORT == 1)
#define UNI_NETWORK_SIDE_SUPPORT    1
#endif


/** The SSS base code allows the use of "non-standard" cause code values by
    setting this option to 1. A target may use this option to extend the set
    of codepoints allowed in UNI v3.1 (Section 5.4.5.15) to provide additional
    details about a failure. If this option is set to 0, the SSS will declare a
    non-standard cause code point to be "CONTENT ERROR" and take appropriate
    action.
**/
#ifndef ALLOW_SPL_CAUSE_CODES
#define ALLOW_SPL_CAUSE_CODES       0
#endif


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                FRAME RELAY SIGNALLING OPTIONS RELEVANT TO OTHER SUBSYSTEMS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/** The support for the network-side of FR UNI signalling is optional. Set the
 ** following option to 1 if you wish to include support for the network side.
**/
#ifndef FRSSS_UNI_NETWORK_SIDE_SUPPORT
#define FRSSS_UNI_NETWORK_SIDE_SUPPORT    0
#endif




/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

   REQUIREMENTS FOR MPOA SUBSYSTEMS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#if ((NCS_MPC == 1) || (NCS_MPS == 1))
#define NCS_ECM              1   /* Endsystem Connection Manager */
#define NCS_CACHING          1
#else
#define NCS_ECM              0   /* Endsystem Connection Manager */
#define NCS_CACHING          0
#endif


/** Set the following to 1 if you wish to turn off
 ** optional floating references.  This has no effect
 ** on code that requires the use of floating point.
**/
#ifndef NCS_NOFLOAT
#define NCS_NOFLOAT          0
#endif



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    SYSMON OPTIONS 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/* SYSM FLAG ENABLE Rules
 * 
 * If NCS_USE_SYSMON is NOT defined, it will be defined OFF
 * If NCS_USE_SYSMON is OFF, all other SYSM flags shall be forced OFF
 * if NCS_USE_SYSMON is ON, other SYSM flags MAY be enabled
 */

#ifndef NCS_USE_SYSMON 
#define NCS_USE_SYSMON   0   /* The MASTER SWITCH */
#endif

#if (NCS_USE_SYSMON == 0)

/* Rule says, since NCS_USE_SYSMON is OFF, all other SYSM flags forced OFF */
/*                                                                        */
/* DO NOT CHANGE THESE VALUES... Go to #else to change values !!!!        */

#undef  NCSSYSM_IPC_DBG_ENABLE
#define NCSSYSM_IPC_DBG_ENABLE   0

#undef  NCSSYSM_MEM_DBG_ENABLE
#define NCSSYSM_MEM_DBG_ENABLE   0

#undef  NCSSYSM_BUF_DBG_ENABLE
#define NCSSYSM_BUF_DBG_ENABLE   0

#undef  NCSSYSM_LOCK_DBG_ENABLE
#define NCSSYSM_LOCK_DBG_ENABLE  0

/* STATISTICS FLAGS: */

#undef  NCSSYSM_IPC_STATS_ENABLE
#define NCSSYSM_IPC_STATS_ENABLE   0

#undef  NCSSYSM_MEM_STATS_ENABLE
#define NCSSYSM_MEM_STATS_ENABLE   0

#undef  NCSSYSM_BUF_STATS_ENABLE
#define NCSSYSM_BUF_STATS_ENABLE   0

#undef  NCSSYSM_TMR_STATS_ENABLE
#define NCSSYSM_TMR_STATS_ENABLE   0

#undef  NCSSYSM_LOCK_STATS_ENABLE
#define NCSSYSM_LOCK_STATS_ENABLE  0

#undef  NCSSYSM_CPU_STATS_ENABLE
#define NCSSYSM_CPU_STATS_ENABLE   0

/* WATCH FLAGS: */

#undef  NCSSYSM_IPC_WATCH_ENABLE
#define NCSSYSM_IPC_WATCH_ENABLE   0

#undef  NCSSYSM_MEM_WATCH_ENABLE
#define NCSSYSM_MEM_WATCH_ENABLE   0

#undef  NCSSYSM_BUF_WATCH_ENABLE
#define NCSSYSM_BUF_WATCH_ENABLE   0

#undef  NCSSYSM_TMR_WATCH_ENABLE
#define NCSSYSM_TMR_WATCH_ENABLE   0

#undef  NCSSYSM_LOCK_WATCH_ENABLE
#define NCSSYSM_LOCK_WATCH_ENABLE  0

#undef  NCSSYSM_CPU_WATCH_ENABLE
#define NCSSYSM_CPU_WATCH_ENABLE   0

#undef  NCSSYSM_IPRA_WATCH_ENABLE
#define NCSSYSM_IPRA_WATCH_ENABLE  0


#else /* NCS_USE_SYSMON  is == 1 */

/* Here, we allow NCSSYSM flags to be set or overriden via compile line */
/*                                                                     */
/* CHANGE these values here or on the compiler line.........           */

#ifndef NCSSYSM_IPC_DBG_ENABLE
#define NCSSYSM_IPC_DBG_ENABLE   0
#endif
#ifndef NCSSYSM_MEM_DBG_ENABLE
#define NCSSYSM_MEM_DBG_ENABLE   0
#endif
#ifndef NCSSYSM_BUF_DBG_ENABLE
#define NCSSYSM_BUF_DBG_ENABLE   0
#endif
#ifndef NCSSYSM_LOCK_DBG_ENABLE
#define NCSSYSM_LOCK_DBG_ENABLE  0
#endif

/* STATISTICS FLAGS: */

#ifndef NCSSYSM_IPC_STATS_ENABLE
#define NCSSYSM_IPC_STATS_ENABLE   0
#endif
#ifndef NCSSYSM_MEM_STATS_ENABLE
#define NCSSYSM_MEM_STATS_ENABLE   0
#endif
#ifndef NCSSYSM_BUF_STATS_ENABLE
#define NCSSYSM_BUF_STATS_ENABLE   0
#endif
#ifndef NCSSYSM_TMR_STATS_ENABLE
#define NCSSYSM_TMR_STATS_ENABLE   0
#endif
#ifndef NCSSYSM_LOCK_STATS_ENABLE
#define NCSSYSM_LOCK_STATS_ENABLE  0
#endif
#ifndef NCSSYSM_CPU_STATS_ENABLE
#define NCSSYSM_CPU_STATS_ENABLE   0
#endif

/* WATCH FLAGS: */

#ifndef NCSSYSM_IPC_WATCH_ENABLE
#define NCSSYSM_IPC_WATCH_ENABLE    0
#endif
#ifndef NCSSYSM_MEM_WATCH_ENABLE
#define NCSSYSM_MEM_WATCH_ENABLE    0
#endif
#ifndef NCSSYSM_BUF_WATCH_ENABLE
#define NCSSYSM_BUF_WATCH_ENABLE    0
#endif
#ifndef NCSSYSM_TMR_WATCH_ENABLE
#define NCSSYSM_TMR_WATCH_ENABLE    0
#endif
#ifndef NCSSYSM_LOCK_WATCH_ENABLE
#define NCSSYSM_LOCK_WATCH_ENABLE   0
#endif
#ifndef NCSSYSM_CPU_WATCH_ENABLE
#define NCSSYSM_CPU_WATCH_ENABLE    0
#endif
#ifndef NCSSYSM_IPRA_WATCH_ENABLE
#define NCSSYSM_IPRA_WATCH_ENABLE   0
#endif

#endif

/* NCS_USE_RMT_CNSL defaults to OFF to reduce overhead */
#ifndef NCS_USE_RMT_CNSL 
#define NCS_USE_RMT_CNSL   0   
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    LEAP ENVIRONMENT INITIALIZATION FLAGS 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/* Init SYSMON */
#ifndef NCSL_ENV_INIT_SMON
#define NCSL_ENV_INIT_SMON 1
#endif

/* Init Handle Manager */
#ifndef NCSL_ENV_INIT_HM
#define NCSL_ENV_INIT_HM 1
#endif

/* Init MIB Transaction Manager */
#ifndef NCSL_ENV_INIT_MTM
#define NCSL_ENV_INIT_MTM 1
#endif

/* Init KMS */
#ifndef NCSL_ENV_INIT_KMS
#define NCSL_ENV_INIT_KMS 1
#endif

/* Init LEAP Timer Service */
#ifndef NCSL_ENV_INIT_TMR
#define NCSL_ENV_INIT_TMR 1
#endif

/* Init LEAP Lock Manager */
#ifndef NCSL_ENV_INIT_LM
#define NCSL_ENV_INIT_LM 1
#endif

/* Init LEAP Buffer Pool */
#ifndef NCSL_ENV_INIT_LBP
#define NCSL_ENV_INIT_LBP 1
#endif

/* Init LEAP Memory Manager */
#ifndef NCSL_ENV_INIT_MMGR
#define NCSL_ENV_INIT_MMGR 1
#endif

/* Init PATRICIA TREE Style */ 
#ifndef NCS_MTREE_PAT_STYLE_SUPPORTED
#define NCS_MTREE_PAT_STYLE_SUPPORTED 0 
#endif 

/* JEA - Changing default SAF option to Disable.
   Having SAF Enabled by default requires the SAF
   vob to be installed for LEAP to function.
   Modified: 2/20/2004
*/
#ifndef NCS_SAF
#define NCS_SAF 0
/* #define NCS_SAF 1 */
#endif

#endif /** NCS_OPT_H **/
