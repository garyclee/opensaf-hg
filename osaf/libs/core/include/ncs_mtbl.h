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

******************************************************************************
*/
/*
 * Module Inclusion Control...
 */
 
#ifndef NCS_MTBL_H
#define NCS_MTBL_H

#ifdef  __cplusplus
extern "C" {
#endif

/***************************************************************************
 * NCSMIB_TBL_ID
 *
 *   this enum has the list of all of the NetPlane Supported MIB IDs
 *   independent of product/subsystem. The Table ID is a unique, though 
 *   arbitrary ID value, and required in the interface to the NetPlane
 *   Universal MIB Access API prototype (See base/common/inc/ncs_mib.h).
 *   
 *   NOTES:
 *   ======
 *   * IF a MIB Table value is here, it implies that the corresponding 
 *   subsystem honors the Universal MIB Access API prototype as its means
 *   to perform MIB manipulations for that subsystem.
 *
 *   * The Universal MIB Access API is also required for any subsystem,
 *   be it NetPlane or Customer implemented, that wishes to participate 
 *   in MIB Access Broker (MAB) functionality. Thus Ultralinq product assists
 *   in distributed MIB operations.
 *
 *   * IF a customer wants to participate in MAB functionality, they would
 *   register their MIB Table IDs must be in the range of  NCSMIB_TBL_END 
 *   to MIB_UD_TBL_ID_END
 ***************************************************************************/

typedef enum   ncsmib_tbl_id
  {

  /*     A F S S u p p o r t e d   M I B  T a b l e  I D s     */

    NCSMIB_TBL_BASE=1,

  /*  DTSv Tables */
    NCSMIB_TBL_DTSV_BASE = NCSMIB_TBL_BASE,
    NCSMIB_TBL_DTSV_SCLR_GLOBAL = NCSMIB_TBL_DTSV_BASE,/* <NCS-DTSV-MIB>:<ncsDtsvScalars>           */
    NCSMIB_TBL_DTSV_NODE,                              /* NCS-DTSV-MIB:ncsDtsvNodeLogPolicyEntry    */
    NCSMIB_TBL_DTSV_SVC_PER_NODE,                      /* NCS-DTSV-MIB:ncsDtsvServiceLogPolicyEntry */
    NCSMIB_TBL_DTSV_CIRBUFF_OP,                        /*internal use                               */
    NCSMIB_TBL_DTSV_CMD,                               /* internal use                              */
    NCSMIB_TBL_DTSV_END = NCSMIB_TBL_DTSV_CMD,

   /*  AVSv Tables */
    NCSMIB_TBL_AVSV_BASE,
    NCSMIB_TBL_AVSV_NCS_SCALAR = NCSMIB_TBL_AVSV_BASE, /* NCS-AVSV-MIB:avsvScalars                   */
    NCSMIB_TBL_AVSV_CLM_SCALAR,                        /* SAF-CLM-MIB:safClmScalarObject             */
    NCSMIB_TBL_AVSV_AMF_SCALAR,                        /* SAF-AMF-MIB:saAmfScalars                   */
    NCSMIB_TBL_AVSV_CLM_NODE,                          /* SAF-CLM-MIB:saClmNodeTableEntry            */
    NCSMIB_TBL_AVSV_NCS_NODE,                          /* NCS-AVSV-MIB:ncsNDTableEntry               */
    NCSMIB_TBL_AVSV_AMF_NODE,                          /* SAF-AMF-MIB:saAmfNodeTableEntry            */
    NCSMIB_TBL_AVSV_NCS_SG,                            /* NCS-AVSV-MIB:ncsSGTableEntry               */
    NCSMIB_TBL_AVSV_AMF_SG,                            /* SAF-AMF-MIB:saAmfSGTableEntry              */
    NCSMIB_TBL_AVSV_NCS_SU,                            /* NCS-AVSV-MIB:ncsSUTableEntry               */
    NCSMIB_TBL_AVSV_AMF_SU,                            /* SAF-AMF-MIB:saAmfSUTableEntry              */
    NCSMIB_TBL_AVSV_NCS_SI,                            /* NCS-AVSV-MIB:ncsSITableEntry               */
    NCSMIB_TBL_AVSV_AMF_SI,                            /* SAF-AMF-MIB:saAmfSITableEntry              */
    NCSMIB_TBL_AVSV_AMF_SUS_PER_SI_RANK,               /* SAF-AMF-MIB:saAmfSUsperSIRankEntry         */
    NCSMIB_TBL_AVSV_AMF_SG_SI_RANK,                    /* SAF-AMF-MIB:saAmfSGSIRankEntry             */
    NCSMIB_TBL_AVSV_AMF_SG_SU_RANK,                    /* SAF-AMF-MIB:saAmfSGSURankEntry             */
    NCSMIB_TBL_AVSV_AMF_SI_SI_DEP,                     /* SAF-AMF-MIB:saAmfSISIDepTableEntry         */
    NCSMIB_TBL_AVSV_AMF_COMP,                          /* SAF-AMF-MIB:saAmfCompTableEntry            */
    NCSMIB_TBL_AVSV_AMF_COMP_CS_TYPE,                  /* SAF-AMF-MIB:saAmfCompCSTypeSupportedTableEntry */
    NCSMIB_TBL_AVSV_AMF_CSI,                           /* SAF-AMF-MIB:saAmfCSITableEntry             */
    NCSMIB_TBL_AVSV_AMF_CSI_PARAM,                     /* SAF-AMF-MIB:saAmfCSINameValueTableEntry    */
    NCSMIB_TBL_AVSV_AMF_CS_TYPE_PARAM,                 /* SAF-AMF-MIB:saAmfCSTypeParamEntry          */
    NCSMIB_TBL_AVSV_AMF_SU_SI,                         /* SAF-AMF-MIB:saAmfSUSITableEntry            */
    NCSMIB_TBL_AVSV_AMF_HLT_CHK,                       /* SAF-AMF-MIB:saAmfHealthCheckTableEntry     */
    NCSMIB_TBL_AVSV_AVD_END = NCSMIB_TBL_AVSV_AMF_HLT_CHK,
    NCSMIB_TBL_AVSV_AVND_BASE,
    NCSMIB_TBL_AVSV_NCS_NODE_STAT = NCSMIB_TBL_AVSV_AVND_BASE,/* NCS-AVSV-MIB:ncsSNDTableEntry       */
    NCSMIB_TBL_AVSV_NCS_SU_STAT,                              /* NCS-AVSV-MIB:ncsSSUTableEntry       */
    NCSMIB_TBL_AVSV_NCS_COMP_STAT,                            /* NCS-AVSV-MIB:ncsSCompTableEntry     */
    NCSMIB_TBL_AVSV_AMF_COMP_CSI,                             /*SAF-AMF-MIB:saAmfSCompCsiTableEntry  */    
    NCSMIB_TBL_AVSV_END = NCSMIB_TBL_AVSV_AMF_COMP_CSI,
    /* The following four tables dont have MIBLIB support and are used for traps.                    */
    NCSMIB_TBL_AVSV_AMF_TRAP_OBJ,                      /* SAF-AMF-MIB:saAmfTrapObject                */
    NCSMIB_TBL_AVSV_AMF_TRAP_TBL,                      /* SAF-AMF-MIB:saAmfTraps                     */
    NCSMIB_TBL_AVSV_CLM_TRAP_OBJ,                      /* SAF-CLM-MIB:saClmTrapObject                */
    NCSMIB_TBL_AVSV_CLM_TRAP_TBL,                      /* SAF-CLM-MIB:saClmTraps                     */
    NCSMIB_TBL_AVSV_NCS_TRAP_TBL,

    
   /*  AVM Tables */
    NCSMIB_TBL_AVM_BASE,
    NCSMIB_TBL_AVM_SCALAR         = NCSMIB_TBL_AVM_BASE,/* NCS-AVM-MIB: ncsAvmScalars */
    NCSMIB_TBL_AVM_ENT_DEPLOYMENT,                      /* NCS-AVM-MIB: ncsAvmEntDeployTable */
    NCSMIB_TBL_AVM_ENT_FAULT_DOMAIN,                    /* NCS-AVM-MIB: ncsAvmEntFaultDomainTable - Not Implemented  */
    NCSMIB_TBL_AVM_TRAPS,
    NCSMIB_TBL_AVM_END = NCSMIB_TBL_AVM_TRAPS,

    /*  SPSv Tables */
    NCSMIB_TBL_SPSV_BASE_UNUSED,
    NCSMIB_TBL_SPSV_CARD_UNUSED = NCSMIB_TBL_SPSV_BASE_UNUSED,
    NCSMIB_TBL_SPSV_SLOT_CARD_UNUSED,
    NCSMIB_TBL_SPSV_SLOT_NODE_UNUSED,
    NCSMIB_TBL_SPSV_LIBRARY_UNUSED,
    NCSMIB_TBL_SPSV_PROCESS_UNUSED,
    NCSMIB_TBL_SPSV_PROCESS_LIBRARY_UNUSED,
    NCSMIB_TBL_SPSV_REMOTE_LIBRARY_UNUSED,       
    NCSMIB_TBL_SPSV_END_UNUSED = NCSMIB_TBL_SPSV_REMOTE_LIBRARY_UNUSED,
   
    /* IFSV MIB Table IDs */
    NCSMIB_TBL_IFSV_BASE,
    NCSMIB_TBL_IFSV_SCLRS1 = NCSMIB_TBL_IFSV_BASE,
    NCSMIB_TBL_IFSV_SCLRS2,
    NCSMIB_TBL_IFSV_IFTBL,
    NCSMIB_TBL_IFSV_IFXTBL,
    NCSMIB_TBL_IFSV_BIND_IFTBL,
    NCSMIB_TBL_IFSV_IFMAPTBL,
    NCSMIB_TBL_IPXS_BASE,
    NCSMIB_TBL_IPXS_IPTBL = NCSMIB_TBL_IPXS_BASE,
    NCSMIB_TBL_IPXS_IFIPTBL,

    /* EDSv Tables */
    NCSMIB_TBL_EDSV_BASE,
    NCSMIB_TBL_EDSV_SCALAR=NCSMIB_TBL_EDSV_BASE,/*  SAF-EVT-MIB : safEvtScalarObject */
    NCSMIB_TBL_EDSV_CHAN_TBL,                   /*  SAF-EVT-MIB : SaEvtChannelEntry  */
    NCSMIB_TBL_EDSV_TRAPOBJ_TBL,
    NCSMIB_TBL_EDSV_TRAP_TBL,
    NCSMIB_TBL_EDSV_END=NCSMIB_TBL_EDSV_TRAP_TBL,
   
    /* PSR Tables */
    NCSMIB_TBL_PSR_START,
    NCSMIB_TBL_PSR_PROFILES = NCSMIB_TBL_PSR_START,/* NCS-PSR-MIB:ncsPSSvProfileTable */
    NCSMIB_SCLR_PSR_TRIGGER,                       /* NCS-PSR-MIB:ncsPSSvScalars      */
    NCSMIB_TBL_PSR_CMD,                            /* internal use                    */
    NCSMIB_TBL_PSR_END = NCSMIB_TBL_PSR_CMD,

    /* ATIS-BRIDGE-MIB Tables */
    NCSMIB_TBL_ATIS_BRIDGE_MIB_BASE,
    NCSMIB_TBL_ATISBRIDGETABLE = NCSMIB_TBL_ATIS_BRIDGE_MIB_BASE,
    NCSMIB_TBL_ATISBASEPORTTABLE,
    NCSMIB_TBL_ATISBLOCKEDEGRESSPORTTABLE,
    NCSMIB_TBL_ATISBASEPORTSTATSTABLE,
    NCSMIB_TBL_ATISVLANTABLE,
    NCSMIB_TBL_ATISVLANTAGGEDPORTTABLE,
    NCSMIB_TBL_ATISVLANUNTAGGEDPORTTABLE,
    NCSMIB_TBL_ATISPORTVLANTABLE,
    NCSMIB_TBL_ATISTRUNKTABLE,
    NCSMIB_TBL_ATISTRUNKCOMPONENTTABLE,
    NCSMIB_TBL_ATISTRUNKBLOCKEDEGRESSPORTTABLE,
    NCSMIB_TBL_ATISL2BRIDGECONTROLTABLE,
    NCSMIB_TBL_ATISL2PORTTABLE,
    NCSMIB_TBL_ATISL2FORWARDINGTABLE,
    NCSMIB_TBL_ATISL2MULTICASTBLOCKTABLE,
    NCSMIB_TBL_ATISL2PORTUNKNOWNUCASTBLOCKTABLE,
    NCSMIB_TBL_ATISL2PORTUNKNOWNMCASTBLOCKTABLE,
    NCSMIB_TBL_ATISL2PORTBROADCASTBLOCKTABLE,
    NCSMIB_TBL_ATISL3INTERFACETABLE,
    NCSMIB_TBL_ATISL3HOSTFORWARDINGTABLE,
    NCSMIB_TBL_ATISL3PREFIXFORWARDINGTABLE,
    NCSMIB_TBL_ATISL3MULTICASTFORWARDINGTABLE,
    NCSMIB_TBL_ATISL3MULTICASTBLOCKTABLE,
    NCSMIB_TBL_ATISL3MULTICASTL2SWITCHTABLE,
    NCSMIB_TBL_ATISFILTERTABLE,
    NCSMIB_TBL_ATISFILTERACTIONTABLE,
    NCSMIB_TBL_ATISMETERTABLE,
    NCSMIB_TBL_ATISFILTERINGRESSPORTTABLE,
    NCSMIB_TBL_ATISFILTEREGRESSPORTTABLE,
    NCSMIB_TBL_ATISFILTERPORTCOUNTERTABLE,
    NCSMIB_TBL_ATISMETERPORTCOUNTERTABLE,
    NCSMIB_TBL_ATISSTPTABLE,
    NCSMIB_TBL_ATISSTPPORTTABLE,
    NCSMIB_TBL_ATISNOTIFICATIONS,
    NCSMIB_TBL_ATIS_BRIDGE_MIB_END = NCSMIB_TBL_ATISNOTIFICATIONS,

    /* Tables from IF-MIB */
    NCSMIB_TBL_IF_MIB_BASE,
    NCSMIB_SCLR_IFNUMBER = NCSMIB_TBL_IF_MIB_BASE,
    NCSMIB_TBL_IFTABLE,
    NCSMIB_TBL_IFXTABLE,
    NCSMIB_TBL_IFSTACKTABLE,
    NCSMIB_TBL_IFTESTTABLE,
    NCSMIB_TBL_IFRCVADDRESSTABLE,
    NCSMIB_TBL_IFTRAP,
    NCSMIB_SCLR_IFTABLELASTCHANGE,
    NCSMIB_SCLR_IFSTACKLASTCHANGE,
    NCSMIB_TBL_IF_MIB_END = NCSMIB_SCLR_IFSTACKLASTCHANGE,

    /* MIB Table definitions for RMON-MIB. */
    NCSMIB_TBL_RMON_MIB_BASE,
    NCSMIB_TBL_ETHERSTATSTABLE = NCSMIB_TBL_RMON_MIB_BASE,
    NCSMIB_TBL_HISTORYCONTROLTABLE,
    NCSMIB_TBL_ETHERHISTORYTABLE,
    NCSMIB_TBL_ALARMTABLE,
    NCSMIB_TBL_HOSTCONTROLTABLE,
    NCSMIB_TBL_HOSTTABLE,
    NCSMIB_TBL_HOSTTIMETABLE,
    NCSMIB_TBL_HOSTTOPNCONTROLTABLE,
    NCSMIB_TBL_HOSTTOPNTABLE,
    NCSMIB_TBL_MATRIXCONTROLTABLE,
    NCSMIB_TBL_MATRIXSDTABLE,
    NCSMIB_TBL_MATRIXDSTABLE,
    NCSMIB_TBL_FILTERTABLE,
    NCSMIB_TBL_CHANNELTABLE,
    NCSMIB_TBL_BUFFERCONTROLTABLE,
    NCSMIB_TBL_CAPTUREBUFFERTABLE,
    NCSMIB_TBL_EVENTTABLE,
    NCSMIB_TBL_LOGTABLE,
    NCSMIB_TBL_RMON_MIB_END = NCSMIB_TBL_LOGTABLE,

    /* MIB Table definitions for EtherLike-MIB. */
    NCSMIB_TBL_ETHERLIKE_MIB_BASE,
    NCSMIB_TBL_DOT3STATSTABLE = NCSMIB_TBL_ETHERLIKE_MIB_BASE,
    NCSMIB_TBL_DOT3COLLTABLE,
    NCSMIB_TBL_DOT3CONTROLTABLE,
    NCSMIB_TBL_DOT3PAUSETABLE,
    NCSMIB_TBL_ETHERLIKE_MIB_END = NCSMIB_TBL_DOT3PAUSETABLE,


    /* MIB Table definitions for LHC-MIB. */
    NCSMIB_TBL_ATIS_LHC_MIB_BASE,
    NCSMIB_TBL_LHCTABLE = NCSMIB_TBL_ATIS_LHC_MIB_BASE,
    NCSMIB_TBL_LHCCOMTABLE,
    NCSMIB_TBL_LHCPROCTORTABLE,
    NCSMIB_TBL_LHCRESPONDERTABLE,
    NCSMIB_TBL_LHCL2INTERFACETABLE,
    NCSMIB_TBL_LHCPROCTORL2INTERFACETABLE,
    NCSMIB_TBL_LHCPROCTORRESPONDERGROUPTABLE,
    NCSMIB_TBL_LHCPROCTOREXPECTEDRESPONDERTABLE,
    NCSMIB_TBL_LHCAUTHORIZEDPROCTORTABLE,
    NCSMIB_TBL_ATIS_LHC_MIB_END = NCSMIB_TBL_LHCAUTHORIZEDPROCTORTABLE,

    NCSMIB_TBL_SNMP_STD_TRAPS_BASE, 
    NCSMIB_TBL_SNMP_STD_TRAPS = NCSMIB_TBL_SNMP_STD_TRAPS_BASE,

    /* MIB Table deffinitions for MQSv table */
    NCSMIB_TBL_MQSV_MIB_BASE,
    NCSMIB_TBL_MQSV_SCLROBJECTSTBL = NCSMIB_TBL_MQSV_MIB_BASE, /*SAF-MSG-MIB:safMsgScalarObject */
    NCSMIB_TBL_MQSV_MQGRPTBL,         /* SAF-MSG-MIB:saMsgQueueGroupTable        */
    NCSMIB_TBL_MQSV_MSGQTBL,          /* SAF-MSG-MIB:saMsgQueueTable             */
    NCSMIB_TBL_MQSV_MSGQPRTBL,        /* SAF-MSG-MIB:saMsgQueuePriorityTable     */
    NCSMIB_TBL_MQSV_MSGQGRPMBRSTBL,   /* SAF-MSG-MIB:saMsgQueueGroupMembersTable */
    NCSMIB_TBL_MQSV_TRAP_OBJECT,      
    NCSMIB_TBL_MQSV_TRAPS,
    NCSMIB_TBL_MQSV_MIB_END = NCSMIB_TBL_MQSV_TRAPS, 

   /* MIB Table definitions for CPSv MIBs */
    NCSMIB_TBL_CPSV_MIB_BASE,
    NCSMIB_TBL_CPSV_SCLRS = NCSMIB_TBL_CPSV_MIB_BASE,/* SAF-CKPT-MIB:safCkptScalarObject */
    NCSMIB_TBL_CPSV_CKPTTBL,  /* SAF-CKPT-MIB:saCkptCheckpointTable     */
    NCSMIB_TBL_CPSV_REPLOCTBL,/* SAF-CKPT-MIB:saCkptNodeReplicaLocTable */
    NCSMIB_TBL_CPSV_CKPT_TRAP_OBJECT,
    NCSMIB_TBL_CPSV_NOTIF,    
    NCSMIB_TBL_CPSV_MIB_END = NCSMIB_TBL_CPSV_NOTIF,

    /* MIB Table definitions for GLSv MIBs */
    NCSMIB_TBL_GLSV_MIB_BASE,
    NCSMIB_TBL_GLSV_SCLRS = NCSMIB_TBL_GLSV_MIB_BASE,/*safLckScalarObject */
    NCSMIB_TBL_GLSV_RSCTBL,/*saLckResourceTable */
    NCSMIB_TBL_GLSV_MIB_END = NCSMIB_TBL_GLSV_RSCTBL,
    
    /* IFSV_VIP MIB Table ID*/
    NCSMIB_TBL_IFSV_VIPTBL,/*  NCS-VIP-MIB: ncsVIP */

    /* MIB Table for LFM */
    NCSMIB_TBL_NCS_LFM,
    NCSMIB_TBL_LFM_NOTIF,

    /* MIB Table definitions for SUND software upgrade Table */
    NCSMIB_TBL_SUND_MIB_BASE,
    NCSMIB_TBL_SUND = NCSMIB_TBL_SUND_MIB_BASE,     /*NCS-SUND-MIB:ncsSundTables */
    NCSMIB_TBL_SUND_TRAPS,                          /*NCS-SUND-MIB:ncsSundNotifications */
    NCSMIB_TBL_SUND_END = NCSMIB_TBL_SUND_TRAPS,

    /* FUND TABLE */
    NCSMIB_FUND_TBL,

    NCSMIB_TBL_SUND_SUPP,
    NCSMIB_TBL_AVM_SUPP,
    NCSMIB_TBL_AVM_UPGRADE,


  /*     E n d  M a r k e r  ..........................................    */

  NCSMIB_TBL_NCS_END = 500, /* all the internal customers of Motorola should start using from NCSMIB_TBL_NCS_END marker */ 

  NCSMIB_TBL_END = 1000, /* This determines the end of Motorola provided MIB Tables*/

  /* All customer MIB tables should go here */
  MIB_UD_TBL_ID_BASE = (NCSMIB_TBL_END+1),

  MIB_UD_TBL_ID_END = 2000 /* Customers can not have the table-ids after MIB_UD_TBL_ID_END */ 

  }NCSMIB_TBL_ID;

#ifdef  __cplusplus
}
#endif

#endif

