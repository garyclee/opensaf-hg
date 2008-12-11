/*
 * This C header file has been generated by smidump 0.2.17.
 * It is intended to be used with the NET-SNMP package.
 *
 * This header is derived from the SNMPv2-MIB module.
 *
 * $Id: snmpv2-mib.h,v 1.9 2001/08/24 10:11:18 strauss Exp $
 */

#ifndef _SNMPV2_MIB_H_
#define _SNMPV2_MIB_H_

#include <stdlib.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/*
 * C type definitions for SNMPv2-MIB::system.
 */

typedef struct system {
    u_char    *sysDescr;
    size_t    _sysDescrLength;
    uint32_t  *sysObjectID;
    size_t    _sysObjectIDLength;
    uint32_t  *sysUpTime;
    u_char    *sysContact;
    size_t    _sysContactLength;
    u_char    *sysName;
    size_t    _sysNameLength;
    u_char    *sysLocation;
    size_t    _sysLocationLength;
    int32_t   *sysServices;
    uint32_t  *sysORLastChange;
    void      *_clientData;		/* pointer to client data structure */

    /* private space to hold actual values */

    u_char    __sysDescr[255];
    uint32_t  __sysObjectID[128];
    uint32_t  __sysUpTime;
    u_char    __sysContact[255];
    u_char    __sysName[255];
    u_char    __sysLocation[255];
    int32_t   __sysServices;
    uint32_t  __sysORLastChange;
} system_t;

/*
 * C manager interface stubs for SNMPv2-MIB::system.
 */

extern int
snmpv2_mib_mgr_get_system(struct snmp_session *s, system_t **system);

/*
 * C agent interface stubs for SNMPv2-MIB::system.
 */

extern int
snmpv2_mib_agt_read_system(system_t *system);
extern int
snmpv2_mib_agt_register_system();

/*
 * C type definitions for SNMPv2-MIB::sysOREntry.
 */

typedef struct sysOREntry {
    int32_t   *sysORIndex;
    uint32_t  *sysORID;
    size_t    _sysORIDLength;
    u_char    *sysORDescr;
    size_t    _sysORDescrLength;
    uint32_t  *sysORUpTime;
    void      *_clientData;		/* pointer to client data structure */
    struct sysOREntry *_nextPtr;	/* pointer to next table entry */

    /* private space to hold actual values */

    int32_t   __sysORIndex;
    uint32_t  __sysORID[128];
    u_char    __sysORDescr[255];
    uint32_t  __sysORUpTime;
} sysOREntry_t;

/*
 * C manager interface stubs for SNMPv2-MIB::sysOREntry.
 */

extern int
snmpv2_mib_mgr_get_sysOREntry(struct snmp_session *s, sysOREntry_t **sysOREntry);

/*
 * C agent interface stubs for SNMPv2-MIB::sysOREntry.
 */

extern int
snmpv2_mib_agt_read_sysOREntry(sysOREntry_t *sysOREntry);
extern int
snmpv2_mib_agt_register_sysOREntry();

/*
 * C type definitions for SNMPv2-MIB::snmp.
 */

typedef struct snmp {
    uint32_t  *snmpInPkts;
    uint32_t  *snmpOutPkts;
    uint32_t  *snmpInBadVersions;
    uint32_t  *snmpInBadCommunityNames;
    uint32_t  *snmpInBadCommunityUses;
    uint32_t  *snmpInASNParseErrs;
    uint32_t  *snmpInTooBigs;
    uint32_t  *snmpInNoSuchNames;
    uint32_t  *snmpInBadValues;
    uint32_t  *snmpInReadOnlys;
    uint32_t  *snmpInGenErrs;
    uint32_t  *snmpInTotalReqVars;
    uint32_t  *snmpInTotalSetVars;
    uint32_t  *snmpInGetRequests;
    uint32_t  *snmpInGetNexts;
    uint32_t  *snmpInSetRequests;
    uint32_t  *snmpInGetResponses;
    uint32_t  *snmpInTraps;
    uint32_t  *snmpOutTooBigs;
    uint32_t  *snmpOutNoSuchNames;
    uint32_t  *snmpOutBadValues;
    uint32_t  *snmpOutGenErrs;
    uint32_t  *snmpOutGetRequests;
    uint32_t  *snmpOutGetNexts;
    uint32_t  *snmpOutSetRequests;
    uint32_t  *snmpOutGetResponses;
    uint32_t  *snmpOutTraps;
    int32_t   *snmpEnableAuthenTraps;
    uint32_t  *snmpSilentDrops;
    uint32_t  *snmpProxyDrops;
    void      *_clientData;		/* pointer to client data structure */

    /* private space to hold actual values */

    uint32_t  __snmpInPkts;
    uint32_t  __snmpOutPkts;
    uint32_t  __snmpInBadVersions;
    uint32_t  __snmpInBadCommunityNames;
    uint32_t  __snmpInBadCommunityUses;
    uint32_t  __snmpInASNParseErrs;
    uint32_t  __snmpInTooBigs;
    uint32_t  __snmpInNoSuchNames;
    uint32_t  __snmpInBadValues;
    uint32_t  __snmpInReadOnlys;
    uint32_t  __snmpInGenErrs;
    uint32_t  __snmpInTotalReqVars;
    uint32_t  __snmpInTotalSetVars;
    uint32_t  __snmpInGetRequests;
    uint32_t  __snmpInGetNexts;
    uint32_t  __snmpInSetRequests;
    uint32_t  __snmpInGetResponses;
    uint32_t  __snmpInTraps;
    uint32_t  __snmpOutTooBigs;
    uint32_t  __snmpOutNoSuchNames;
    uint32_t  __snmpOutBadValues;
    uint32_t  __snmpOutGenErrs;
    uint32_t  __snmpOutGetRequests;
    uint32_t  __snmpOutGetNexts;
    uint32_t  __snmpOutSetRequests;
    uint32_t  __snmpOutGetResponses;
    uint32_t  __snmpOutTraps;
    int32_t   __snmpEnableAuthenTraps;
    uint32_t  __snmpSilentDrops;
    uint32_t  __snmpProxyDrops;
} snmp_t;

/*
 * C manager interface stubs for SNMPv2-MIB::snmp.
 */

extern int
snmpv2_mib_mgr_get_snmp(struct snmp_session *s, snmp_t **snmp);

/*
 * C agent interface stubs for SNMPv2-MIB::snmp.
 */

extern int
snmpv2_mib_agt_read_snmp(snmp_t *snmp);
extern int
snmpv2_mib_agt_register_snmp();

/*
 * C type definitions for SNMPv2-MIB::snmpSet.
 */

typedef struct snmpSet {
    int32_t   *snmpSetSerialNo;
    void      *_clientData;		/* pointer to client data structure */

    /* private space to hold actual values */

    int32_t   __snmpSetSerialNo;
} snmpSet_t;

/*
 * C manager interface stubs for SNMPv2-MIB::snmpSet.
 */

extern int
snmpv2_mib_mgr_get_snmpSet(struct snmp_session *s, snmpSet_t **snmpSet);

/*
 * C agent interface stubs for SNMPv2-MIB::snmpSet.
 */

extern int
snmpv2_mib_agt_read_snmpSet(snmpSet_t *snmpSet);
extern int
snmpv2_mib_agt_register_snmpSet();


typedef struct snmpv2_mib {
    system_t	system;
    sysOREntry_t	*sysOREntry;
    snmp_t	snmp;
    snmpSet_t	snmpSet;
} snmpv2_mib_t;

/*
 * Initialization function:
 */

void snmpv2_mib_agt_init(void);

#endif /* _SNMPV2_MIB_H_ */
