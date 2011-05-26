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

  MODULE NAME:  NCS_LOG.H

..............................................................................

  DESCRIPTION:

  Header file for LEAP implementation of FlexLog Logging service

******************************************************************************
*/

#ifndef NCS_LOG_H
#define NCS_LOG_H

#include "ncsgl_defs.h"

/************************************************************************

 M C G   D I S T R I B U T E D    L O G G I N G    S e r v i c e

 This File contains all the typedefs necessary to use the Flexlog
 logging service. Roughly there are two sets of abstractions:

   1) The run-time normalized form of logging data that is given to
      flexlog when it is time to actually log information. The normalized
      form is a very terse expression of the logging info. Once in this
      form, the data can be economically stored, filtered or moved. It
      can also be expanded into its printable form, when desired.

   2) The 'registration' set of structs allow a subystem to 'explain'
      all of its string formatting information so that flexlog can build
      printable strings for human consumtion.

************************************************************************/
typedef char *NCSFL_TYPE;

/************************************************************************

 F l e x l o g    R u n    T i m e   S t r u c t s

 This section reviews the data structures a logging subsystem will give
 to flexlog at log time. The principle ideas are:

 General logging info : there is a collection of data that any logging
                        subsystem must universally provide. This allows
                        for present and future functionality in areas such
                        as sorting, filtering and the like.

 A Data Format Scheme : there is a data structure and a corresponding
                        mnemonic that maps to a particular sequence of
                        arguments.

 F l e x L o g    F o r m a t    N a m i n g    C o n v e n t i o n s

 Naming for a FlexLog format type and memory overlay data structure shall
 abide by these conventions:

 NCSFL_TYPE_xxx - Prefix for a format identifier
 NCSFL_xxx      - Prefix for a memory overlay data structure

   Where xxx is a sequence of characters that mean:

 S     : (small)  uint8_t  value
 M     : (medium) uint16_t value
 L     : (large)  uns32 value
 F     : double value
 I     : Index value pair to find a canned string in a NCSFL_SET
 T     : Time value
 A     : IPV4 Address or mask to be printed in xxx.xxx.xxx.xxx format
 D     : Dump of memory stored as Octet string, of the form
         uint16_t = length,
         char* = 4 bytes of address + (length - 4) bytes of raw data.
         To be printed as...
         <mem addr> <16 bytes in hex format>                        <ascii form>
    ex.  0x098af44  33 34 35 36 20 20 20 20 33 34 35 36 2e 2e 2e 2e 3456    3456....

 P     : PDU as Octet string, of the form
         uint16_t = length,
         char* = length bytes of raw data
         To be printed as...
         <16 bytes in hex format>                        <ascii form>
    ex.  33 34 35 36 20 20 20 20 33 34 35 36 2e 2e 2e 2e 3456    3456....

 C     : Character string, NULL terminate stored as octet string
         of the form
         uint16_t = length,
         char* = strlen + 1 (the NULL).

************************************************************************/

#define  NCSFL_TYPE_T             "T"	/* tme                                          */
#define  NCSFL_TYPE_TI            "TI"	/* tme, idx,                                    */
#define  NCSFL_TYPE_TII           "TII"	/* tme, idx, idx                                */
#define  NCSFL_TYPE_TIIL          "TIIL"	/* tme, idx, idx, u32                           */
#define  NCSFL_TYPE_TIII          "TIII"	/* tme, u32, u32, u32                           */
#define  NCSFL_TYPE_TIIIC         "TIIIC"	/* tme, u32, u32, u32, C                        */
#define  NCSFL_TYPE_TCIC          "TCIC"	/* tme, C,   u32, C                             */
#define  NCSFL_TYPE_TC            "TC"	/* tme, C                                       */
#define  NCSFL_TYPE_TIM           "TIM"	/* tme, u32, u16                                */
#define  NCSFL_TYPE_TIMM          "TIMM"	/* tme, u32, u16, u16                           */
#define  NCSFL_TYPE_TIL           "TIL"	/* tme, idx, u32                                */
#define  NCSFL_TYPE_TIA           "TIA"	/* tme, idx, ipa                                */
#define  NCSFL_TYPE_TIAIA         "TIAIA"	/* tme, idx, ipa  idx, ipa                      */
#define  NCSFL_TYPE_TIAD          "TIAD"	/* tme, idx, ipa, u16                           */
#define  NCSFL_TYPE_TID           "TID"	/* tme, u32, mem                                */
#define  NCSFL_TYPE_TIP           "TIP"	/* tme, u32, pdu                                */
#define  NCSFL_TYPE_ILTI          "ILTI"	/* idx, u32, tme, idx                           */
#define  NCSFL_TYPE_ILTII         "ILTII"	/* idx, u32, tme, idx, idx                      */
#define  NCSFL_TYPE_ILTIS         "ILTIS"	/* idx, u32, tme, idx, u8                       */
#define  NCSFL_TYPE_ILTIM         "ILTIM"	/* idx, u32, tme, idx, u16                      */
#define  NCSFL_TYPE_ILTIL         "ILTIL"	/* idx, u32, tme, idx, u32                      */
#define  NCSFL_TYPE_ILTIIL        "ILTIIL"	/* idx, u32, tme, idx, idx, u32                 */
#define  NCSFL_TYPE_ILTILL        "ILTILL"	/* idx, u32, tme, idx, u32, u32                 */
#define  NCSFL_TYPE_ILTILLL       "ILTILLL"	/* idx, u32, tme, idx, u32, u32, u32            */
#define  NCSFL_TYPE_ILTL          "ILTL"	/* idx, u32, tme, u32                         */
#define  NCSFL_TYPE_ILTCLIL       "ILTCLIL"	/* idx, u32, tme, C,   u32, idx, u32          */
#define  NCSFL_TYPE_ILTISC        "ILTISC"	/* idx, u8,  tme, idx, u8,  C                 */
#define  NCSFL_TYPE_ILTISCLC      "ILTISCLC"	/* idx, u8,  tme, idx, u8,  C,   u32, C       */
#define  NCSFL_TYPE_ILTIC         "ILTIC"	/* idx, u32, tme, idx, C,                     */
#define  NCSFL_TYPE_ILTICL        "ILTICL"	/* idx, u32, tme, idx, C,   u32               */
#define  NCSFL_TYPE_ILTILI        "ILTILI"	/* idx, u32, tme, idx, u32, idx               */
#define  NCSFL_TYPE_ILTIILM       "ILTIILM"	/* idx, u32, tme, idx, idx, u32, u16          */
#define  NCSFL_TYPE_ILTIILL       "ILTIILL"	/* idx, u32, tme, idx, idx, u32, u32          */
#define  NCSFL_TYPE_ILTIILML      "ILTIILML"	/* idx, u32, tme, idx, idx, u32, u16, u32     */
#define  NCSFL_TYPE_ILTIILLL      "ILTIILLL"	/* idx, u32, tme, idx, idx, u32, u32, u32     */
#define  NCSFL_TYPE_ILTILIL       "ILTILIL"	/* idx, u32, tme, idx, u32, idx, u32,         */
#define  NCSFL_TYPE_ILTLLLL       "ILTLLLL"	/* idx, u32, tme, u32, u32, u32, u32,         */
#define  NCSFL_TYPE_ILTILLM       "ILTILLM"	/* idx, u32, tme, idx, u32, u32, u16          */

   /* Add new NCSFL_TYPE_ values as new sprintf templates emerge       */
#define  NCSFL_TYPE_TIALL         "TIALL"	/* tme, idx, u32, u32, u32,                    */
#define  NCSFL_TYPE_TIALAL        "TIALAL"	/* tme, idx, u32, u32, u32, u32                */
#define  NCSFL_TYPE_ILLLL         "ILLLL"	/* idx, u32, u32, u32, u32                     */
#define  NCSFL_TYPE_TIMSC         "TIMSC"	/* u32 */
#define  NCSFL_TYPE_TILAL         "TILAL"	/* tme, idx, u32, u32, u32                     */
#define  NCSFL_TYPE_TILLII        "TILLII"	/* tme, idx, u32, u32, idx, idx                */
#define  NCSFL_TYPE_TILI          "TILI"	/* tme, idx, u32, idx                          */
#define  NCSFL_TYPE_ILI           "ILI"	/* idx, u32, idx                               */

#define  NCSFL_TYPE_TIALALLLL     "TIALALLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32             */
#define  NCSFL_TYPE_TIALALLLLL    "TIALALLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32, u32        */
#define  NCSFL_TYPE_TIALALLLLLL   "TIALALLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32, u32, u32   */
#define  NCSFL_TYPE_TIF           "TIF"	/* tme, idx, dbl                                           */
#define  NCSFL_TYPE_TIN           "TIN"	/* tme, idx, long long                                           */
#define  NCSFL_TYPE_TIU           "TIU"	/* tme, idx, unsigned long long                                           */
#define  NCSFL_TYPE_TIX           "TIX"	/* tme, idx, long long (in hex format)                      */
#define  NCSFL_TYPE_TIFI          "TIFI"	/* tme, idx, dbl, idx                                      */
#define  NCSFL_TYPE_TIC           "TIC"	/* tme, idx, C                                             */
#define  NCSFL_TYPE_TICCLLLL      "TICCLLLL"	/* tme, idx, C, C, u32, u32, u32, u32                     */
#define  NCSFL_TYPE_TIIC          "TIIC"	/* tme, idx, idx, C                                        */
#define  NCSFL_TYPE_TIICC         "TIICC"	/* tme, idx, idx, C, C                                       */
#define  NCSFL_TYPE_TILL          "TILL"	/* tme, idx, u32, u23,                                     */
#define  NCSFL_TYPE_TILLL         "TILLL"	/* tme, idx, u32, u32, u32                                 */
#define  NCSFL_TYPE_TILLLL        "TILLLL"	/* tme, idx, u32, u32, u32, u32                            */
#define  NCSFL_TYPE_TILLLLL       "TILLLLL"	/* tme, idx, u32, u32, u32, u32, u32                       */
#define  NCSFL_TYPE_TILLLLLL      "TILLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32                  */
#define  NCSFL_TYPE_TILLLLLLL     "TILLLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32             */
#define  NCSFL_TYPE_TILLLLLLLL    "TILLLLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32, u32        */
#define  NCSFL_TYPE_TILLLLLLLLL   "TILLLLLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32, u32, u32   */
#define  NCSFL_TYPE_TILLLLLLLLLL  "TILLLLLLLLLL"	/* tme, idx, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32 */
#define  NCSFL_TYPE_TICL          "TICL"	/* tme, idx, C, u32                                         */
#define  NCSFL_TYPE_TICLL         "TICLL"	/* tme, idx, C, u32, u32                                    */
#define  NCSFL_TYPE_TICLLL        "TICLLL"	/* tme, idx, C, u32, u32, u32                               */
#define  NCSFL_TYPE_TICLLLL       "TICLLLL"	/* tme, idx, C, u32, u32, u32, u32                          */
#define  NCSFL_TYPE_TICLLLLL      "TICLLLLL"	/* tme, idx, C, u32, u32, u32, u32, u32                     */
#define  NCSFL_TYPE_TICLLLLLL     "TICLLLLLL"	/* tme, idx, C, u32, u32, u32, u32, u32, u32                */
#define  NCSFL_TYPE_TICLLLLLLL     "TICLLLLLLL"	/* tme, idx, C, u32, u32, u32, u32, u32, u32, u32         */
#define  NCSFL_TYPE_TIICL         "TIICL"	/* tme, idx, idx, C, uns32                                  */
#define  NCSFL_TYPE_TICI          "TICI"	/* tme, idx, C, idx                                         */
#define  NCSFL_TYPE_TILCL         "TILCL"	/* tme, idx, u32,C,u32  added for mqsv */
#define  NCSFL_TYPE_TCLIL         "TCLIL"	/* tme, C,u32,idx, u32  added for mqsv */
#define  NCSFL_TYPE_TCLILL        "TCLILL"	/* tme, C,u32,idx, u32,u32  added for edsv */
#define  NCSFL_TYPE_TCLILLF       "TCLILLF"	/* tme, C,u32,idx, u32,u32,u64  added for edsv */
#define  NCSFL_TYPE_TCLILLLL      "TCLILLLL"	/* tme, C,u32,idx, u32  added for glsv */
#define  NCSFL_TYPE_TIDNUX        "TIDNUX"	/*time idx mem_dump long long long long long long(hex) */
#define  NCSFL_TYPE_TINUXD        "TINUXD"	/*time idx long long long long long long(hex) mem_dump */
#define  NCSFL_TYPE_TIILLLL       "TIILLLL"
#define  NCSFL_TYPE_TIIII         "TIIII"
#define  NCSFL_TYPE_TICLCLL       "TICLCLL"     /*tme idx C long C long long added for glsv */
/************************************************************************
  FORMAT Structures for easy Memory Overlays

  NOTE: There is one data structure for each NCSFMAT_TYPE. As new types
  are created, so too will new data structures be created.

 ************************************************************************/

typedef struct ncsfl_mem {
	uint16_t len;		/* number of bytes of raw data in dump */
	char *addr;		/* address to be printed */
	char *dump;		/* contains the raw data to be printed */

} NCSFL_MEM;

typedef struct ncsfl_pdu {
	uint16_t len;
	char *dump;		/* dump contains len bytes of raw data */

} NCSFL_PDU;

/************************************************************************
  SEVERITY

  http:/search.ietf.org/internet-drafts/draft-ieft-syslog-syslog-01.txt
  calls out these Severity code-points (note 1).

  By decree, Flexlog declares this basic mapping between often used
  MCG logging categories and the reported Severity. Each subsystem
  is responsible for

  --------------------+-------------------------------------------------
  MCG Category   |  FlexLog Severity
  --------------------+-------------------------------------------------
  API                 |  (range) SEV_INFO   -to- SEV_ERROR
  HEADLINE            |  (range) SEV_INFO   -to- SEV_ERROR
  STATE               |  (range) SEV_INFO   -to- SEV_ERROR
  TIMER               |  (range) SEV_INFO   -to- SEV_NOTICE
  EVENT               |  (range) SEV_NOTICE -to- SEV_ALERT
  DATA                |  (range) SEV_DEBUG  -to- SEV_ERROR
  FSM                 |  (range) SEV_INFO   -to- SEV_ERROR
  SVC_PRVDR           |  (range) SEV_INFO   -to- SEV_ERROR
                      |
  CLI_COMMENTS        |  (range) SEV_INFO
  < OTHERS >          | < AS APPROPRIATE, by SUBSYSTEM >
  --------------------+-------------------------------------------------

    NOTES:
      1) this draft is expired, but at least its something.
      2) values are expressed as bit maps for easy multi-severity
         filtering (as per #defines below).

 ************************************************************************/

#define NCSFL_SEV_EMERGENCY  0x80	/* system is unusable               */
#define NCSFL_SEV_ALERT      0x40	/* action must be taken immediately */
#define NCSFL_SEV_CRITICAL   0x20	/* critical condition               */
#define NCSFL_SEV_ERROR      0x10	/* error conditions                 */
#define NCSFL_SEV_WARNING    0x08	/* warning condition                */
#define NCSFL_SEV_NOTICE     0x04	/* normal, but segnificant condition */
#define NCSFL_SEV_INFO       0x02	/* informational messages           */
#define NCSFL_SEV_DEBUG      0x01	/* debug-level messages             */

/***********************************************************************
  CATEGORY

  Common Logging Categories : Subystems continue to define whatever logging
  categories suit them, as is MCG tradition. Such categories are
  typically based on the particular, strategic needs of the developers,
  porters and integrators of that subsystem, which (again) will continue
  to be TRUE.

  Flexlog requires subsystem developers to the do their best to map each
  particular category to the general flexlog categories. These categories
  represent system-wide classes of logging information, which can be
  described by a single filtering criteria.

  NOTES:
    1) The system designer has control over what gets logged at development
       and product time, according to the 'actual implementation' of FlexLog
    2) CATEGORY values are independent of the SEVERITY values.

************************************************************************/

#define NCSFL_LC_API             0x80000000	/* API or Callback invoked         */
#define NCSFL_LC_HEADLINE        0x40000000	/* Basic noteworthy condition      */
#define NCSFL_LC_STATE           0x20000000	/* State-transition announcement   */
#define NCSFL_LC_TIMER           0x10000000	/* Timer stop/start/expire stuff   */
#define NCSFL_LC_FUNC_ENTRY      0x08000000	/* Function entry point */
#define NCSFL_LC_EVENT           0x04000000	/* Unusual, often async 'event'    */
#define NCSFL_LC_DATA            0x02000000	/* data (ex. PDU, memory) info     */
#define NCSFL_LC_STATE_MCH       0x01000000	/* Finite State Machine stuff      */
#define NCSFL_LC_SVC_PRVDR       0x00800000	/* reguarding my service providers */
#define NCSFL_LC_MEMORY          0x00400000	/* Memory related logs */
#define NCSFL_LC_LOCKS           0x00200000	/* Locks : created/destroyed/locked/release */
#define NCSFL_LC_INTERFACE       0x00100000	/* Interface: up or down */
#define NCSFL_LC_FUNC_RET_FAIL   0x00080000	/* Function returning failure */
#define NCSFL_LC_SYS_CALL_FAIL   0x00040000	/* System call fail */
#define NCSFL_LC_MSG_FMT_ERROR   0x00020000	/* Log : Message format errors */
#define NCSFL_LC_MSG_CKPT        0x00010000	/* Checkpointing related errors */

#define NCSFL_LC_MISC            0x00008000	/* The above categories don't fit  */

#endif
