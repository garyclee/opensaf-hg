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
 */

#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncsusrbuf.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_mem.h>
#include "leaptest.h"

/********************************************************************

********************************************************************/

char testpattern[] = {0x01,0x09,0x00,0x00,0x21,0x29,0x31,0x39,
                      0x02,0x0a,0x12,0x1a,0x22,0x2a,0x32,0x3a,
                      0x03,0x0b,0x13,0x1b,0x23,0x2b,0x33,0x3b,
                      0x04,0x0c,0x14,0x1c,0x24,0x2c,0x34,0x3c,
                      0x05,0x0d,0x15,0x1d,0x25,0x2d,0x35,0x3d,
                      0x06,0x0e,0x16,0x1e,0x26,0x2e,0x36,0x3e,
                      0x07,0x0f,0x17,0x1f,0x27,0x2f,0x37,0x3f,
                      0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x00};

static void
data_at_end_test(void)
{
    USRBUF *ub1, *ub2;
    const char *ub1_msg = "Buffer One\n\0";
    const char *ub2_msg = "Buffer Two\n\0";
#define BOTH_MSGS 50
    int  sz_msg1  = m_NCS_STRLEN (ub1_msg);
    int  sz_msg2  = m_NCS_STRLEN (ub2_msg) + 1; /* add 1 to get terminating '\0' */
    char *cp = 0;
    uns16  cksum         = 0,
          *p_cksum_field = NULL;

    char store[BOTH_MSGS]; /* should = sz_msg1 + sz_msg2 */

    m_NCS_CONS_PRINTF("- Start Testing m_MMGR_DATA_AT_END -\n");

    if ((sz_msg1 + sz_msg2) > BOTH_MSGS)
    {
        m_NCS_CONS_PRINTF("  Coding Error: test array must be larger\n");
        goto GET_OUT;
    }

    /* allocate the USRBUFs */
    if ((ub1 = m_MMGR_ALLOC_BUFR(0)) == BNULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to allocate a USRBUF!!\n");
        goto FREE_LIST1;
    }

    /* TEST THE CHECKSUM ALGORITHM */
    /* Even byte boundary */
    ub1->count = sizeof(testpattern);
    memcpy((ub1->payload->Data),testpattern, sizeof(testpattern));
    m_MMGR_BUFR_CALC_CKSUM(ub1, ub1->count, &cksum);
    m_NCS_CONS_PRINTF ("\nCHECKSUM 1 RETURNED (%x) SHOULD BE 0x7cc5\n", cksum);
    /* place the cksum back in and do a recompute, 
     * the resultant should be 0 if all goes well 
     */
    p_cksum_field = (uns16*)(((uns8*)ub1->payload->Data)+2);
    *p_cksum_field = m_NCS_OS_NTOHS_P((uns8*)&cksum);
    m_MMGR_BUFR_CALC_CKSUM(ub1, ub1->count, &cksum);
    m_NCS_CONS_PRINTF ("CHECKSUM 1 RECOMPUTE RETURNED (%x) SHOULD BE 0\n", cksum);

    /* Odd byte boundary */
    ub1->start++;
    memcpy((ub1->payload->Data+1),testpattern, sizeof(testpattern));
    m_MMGR_BUFR_CALC_CKSUM(ub1, ub1->count, &cksum);
    m_NCS_CONS_PRINTF ("CHECKSUM 2 RETURNED (%x) SHOULD BE 0x7cc5\n", cksum);    
    /* place the cksum back in and do a recompute, 
     * the resultant should be 0 if all goes well 
     */
    p_cksum_field = (uns16*)(((uns8*)ub1->payload->Data)+3);
    *p_cksum_field = m_NCS_OS_NTOHS_P((uns8*)&cksum);
    m_MMGR_BUFR_CALC_CKSUM(ub1, ub1->count, &cksum);
    m_NCS_CONS_PRINTF ("CHECKSUM 2 RECOMPUTE RETURNED (%x) SHOULD BE 0\n", cksum);

    
    /* Reset Userbuff fields */
    ub1->count = 0;
    ub1->start = 0;
    memset(ub1->payload->Data, '\0', sizeof(testpattern)+1);
    
    if ((ub2 = m_MMGR_ALLOC_BUFR(0)) == BNULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to allocate a USRBUF!!\n");
        goto FREE_LIST2;
    }

    /* put messages into USRBUFs */
    if ((cp = m_MMGR_RESERVE_AT_START (&ub1, sz_msg1, char *)) == NULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to reserve space in ub1!!\n");
        goto FREE_LIST2;
    }
    memcpy (cp, ub1_msg, sz_msg1);

    if ((cp = m_MMGR_RESERVE_AT_START (&ub2, sz_msg2, char *)) == NULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to reserve space in ub2!!\n");
        goto FREE_LIST2;
    }
    memcpy (cp, ub2_msg, sz_msg2);


    /* combine both USRBUFs into one chain */
    m_MMGR_APPEND_DATA (ub1, ub2);

    /* read entire message from chain */
    if ((cp = m_MMGR_DATA_AT_END (ub1, sz_msg1+sz_msg2, store)) == NULL)
    {
        m_NCS_CONS_PRINTF("  !!Failed to read data from chain!!\n");
        goto FREE_LIST1;
    }


    m_NCS_CONS_PRINTF ("Entire Message:\n%s", cp);
    goto FREE_LIST1;


FREE_LIST2:
    m_MMGR_FREE_BUFR_LIST (ub2);
FREE_LIST1:
    m_MMGR_FREE_BUFR_LIST (ub1);

GET_OUT:
    m_NCS_CONS_PRINTF("- Complete Testing m_MMGR_DATA_AT_END -\n\n");
}


int
bufferManager_testSuite (int argc, char **argv)
{
    m_NCS_CONS_PRINTF("Buffer Manager Test Suite\n");
    ncs_mem_create ();

    data_at_end_test();

    ncs_mem_destroy ();
    m_NCS_CONS_PRINTF("-Exiting Buffer Manager Macro Test-\n\n");
    return 0;
}





