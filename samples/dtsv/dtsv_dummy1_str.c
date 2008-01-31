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


#include "dtsv_dummy_ss.h"

uns32 dummy_log_str_lib_req(NCS_LIB_REQ_INFO *req_info); 
void reg_dummy1_ascii_tbl(void);
void de_reg_dummy1_ascii_tbl(void);

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIL 
 ******************************************************************************/

const NCSFL_STR dummy1_indx_long_set[] = 
  {
    { DUMMY_INDX,     "Printing for NCSFL_TYPE_TIL"      },
    { 0,0 }
  };


/******************************************************************************
 Logging stuff for NCSFL_TYPE_TI 
 ******************************************************************************/

const NCSFL_STR dummy1_indx_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TI "},
    { DUMMY_INDX1,        "Printing for NCSFL_TYPE_TII "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TILLL 
 ******************************************************************************/

const NCSFL_STR dummy1_indxlll_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TILLL "},
    { 0,0 }
  };
/******************************************************************************************/

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TC 
 ******************************************************************************/

const NCSFL_STR dummy1_char_str_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TC "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIA 
 ******************************************************************************/

const NCSFL_STR dummy1_ip_addr_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIA "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIM 
 ******************************************************************************/

const NCSFL_STR dummy1_indxm_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIM "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIP 
 ******************************************************************************/

const NCSFL_STR dummy1_pdu_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIP "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TID 
 ******************************************************************************/

const NCSFL_STR dummy1_memdump_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TID "},
    { 0,0 }
  };
/******************************************************************************
 Logging stuff for NCSFL_TYPE_ILTIS 
 ******************************************************************************/

const NCSFL_STR dummy1_iltis_set[] = 
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_ILTIS "},
    { 0,0 }
  };
/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIF 
 ******************************************************************************/

const NCSFL_STR dummy1_tif_set[] =
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIF "},
    { 0,0 }
  };
/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIN
 ******************************************************************************/

const NCSFL_STR dummy1_tin_set[] =
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIN "},
    { DUMMY_INDX_MIN,     "Printing for MIN in NCSFL_TYPE_TIN "},
    { DUMMY_INDX_MAX,     "Printing for MAX in NCSFL_TYPE_TIN "},
    { 0,0 }
  };

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIU
 ******************************************************************************/

const NCSFL_STR dummy1_tiu_set[] =
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIU "},
    { DUMMY_INDX_MIN,     "Printing for MIN in NCSFL_TYPE_TIU "},
    { DUMMY_INDX_MAX,     "Printing for MAX in NCSFL_TYPE_TIU "},
    { 0,0 }
  };
/******************************************************************************
 Logging stuff for NCSFL_TYPE_TIX
 ******************************************************************************/

const NCSFL_STR dummy1_tix_set[] =
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIX "},
    { DUMMY_INDX_MIN,     "Printing for MIN in NCSFL_TYPE_TIX "},
    { DUMMY_INDX_MAX,     "Printing for MAX in NCSFL_TYPE_TIX "},
    { 0,0 }
  };

/******************************************************************************************/

/******************************************************************************
 Logging stuff for NCSFL_TYPE_TINUXD
 ******************************************************************************/

const NCSFL_STR dummy1_indxmlll_set[] =
  {
    { DUMMY_INDX,         "Printing for NCSFL_TYPE_TIDNUX "},
    { DUMMY_INDX1,         "Printing for NCSFL_TYPE_TINUXD "},
    { 0,0 }
  };
/******************************************************************************************/


/******************************************************************************
 Build up the canned constant strings for the ASCII SPEC
 ******************************************************************************/

NCSFL_SET dummy1_str_set[] = 
  {
    { DUMMY_FC_PRT_INDX,           0, (NCSFL_STR*) dummy1_indx_set      },
    { DUMMY_FC_PRT_INDXL,          0, (NCSFL_STR*) dummy1_indx_long_set },
    { DUMMY_FC_PRT_INDXLLL,        0, (NCSFL_STR*) dummy1_indxlll_set   },
    { DUMMY_FC_PRT_CHAR_STR,       0, (NCSFL_STR*) dummy1_char_str_set  },
    { DUMMY_FC_PRT_IP_ADDR,        0, (NCSFL_STR*) dummy1_ip_addr_set       },
    { DUMMY_FC_PRT_INDXM,          0, (NCSFL_STR*) dummy1_indxm_set       },
    { DUMMY_FC_PRT_PDU,            0, (NCSFL_STR*) dummy1_pdu_set       },
    { DUMMY_FC_PRT_MEMDUMP,        0, (NCSFL_STR*) dummy1_memdump_set       },
    { DUMMY_FC_PRT_ILTIS,          0, (NCSFL_STR*) dummy1_iltis_set       },
    { DUMMY_FC_PRT_TIF,            0, (NCSFL_STR*) dummy1_tif_set         },
    { DUMMY_FC_PRT_TIN,            0, (NCSFL_STR*) dummy1_tin_set         },
    { DUMMY_FC_PRT_TIU,            0, (NCSFL_STR*) dummy1_tiu_set         },
    { DUMMY_FC_PRT_TIX,            0, (NCSFL_STR*) dummy1_tix_set         },
    { DUMMY_FC_PRT_INDXMLLL,       0, (NCSFL_STR*) dummy1_indxmlll_set    },
    { 0,0,0 }
  };

NCSFL_FMAT dummy1_fmat_set[] = 
  {
    { DUMMY_LOG_INDX,           NCSFL_TYPE_TI,             "DUMMY %s INDEXI          : %s\n"     },
    { DUMMY_LOG_INDXI,          NCSFL_TYPE_TII,            "DUMMY %s INDEXII         : %s %s\n"  },
    { DUMMY_LOG_INDXIC,         NCSFL_TYPE_TIIC,           "DUMMY %s INDEXIIC        : %s %s %s\n"},
    { DUMMY_LOG_INDXICC,        NCSFL_TYPE_TIICC,          "DUMMY %s INDEXIICC       : %s  \n %s %s  \n %s\n"},
    { DUMMY_LOG_INDXIIC,        NCSFL_TYPE_TIIIC,          "DUMMY %s INDEXIIIC       : %s  \n %s %s  \n %s\n"},
    { DUMMY_LOG_INDXIL,         NCSFL_TYPE_TIIL,           "DUMMY %s INDEXIIL        : %s  \n %s %d\n"  },
    { DUMMY_LOG_INDXL,          NCSFL_TYPE_TIL,            "DUMMY %s INDEX-LONG      : %s (%d)\n"},
    { DUMMY_LOG_INDXLLL,        NCSFL_TYPE_TILLL,          "DUMMY %s INDEX-L-L-L     : %s  \n 1 = %d, 2 = %d, 3 = %d. \n"},
    { DUMMY_LOG_CHAR_STR,       NCSFL_TYPE_TC,             "DUMMY %s CHAR STRING     : %s\n"     },
    { DUMMY_LOG_IP_ADDR,        NCSFL_TYPE_TIA,            "DUMMY %s  %s IP ADDRESS  : %s\n"     },
    { DUMMY_LOG_INDXM,          NCSFL_TYPE_TIM,            "DUMMY %s U16             : %s %d\n"      },
    { DUMMY_LOG_PDU,            NCSFL_TYPE_TIP,            "DUMMY %s PDU             : %s\n %s\n"    },
    { DUMMY_LOG_MEMDUMP,        NCSFL_TYPE_TID,            "DUMMY %s MEMDUMP         : %s\n %s\n"    },
    { DUMMY_LOG_ILTIS,          NCSFL_TYPE_ILTIS,          "DUMMY %s ILTIS: %d %s\n %s %d\n"    },
    { DUMMY_LOG_TIII,           NCSFL_TYPE_TIII,           "DUMMY %s TIII: %s %s %s \n" },
    { DUMMY_LOG_TCIC,           NCSFL_TYPE_TCIC,           "DUMMY %s TCIC: %s %s %s \n" },
    { DUMMY_LOG_TILL,           NCSFL_TYPE_TILL,           "DUMMY %s TILL: %s %d %d \n" },
    { DUMMY_LOG_TILLL,          NCSFL_TYPE_TILLL,          "DUMMY %s TILLL: %s %d %d %d \n" },
    { DUMMY_LOG_TILLLL,         NCSFL_TYPE_TILLLL,         "DUMMY %s TILLLL: %s %d %d %d %d \n" },
    { DUMMY_LOG_TILLLLL,        NCSFL_TYPE_TILLLLL,        "DUMMY %s TILLLLL: %s %d %d \n %d %d %d \n" },
    { DUMMY_LOG_TILLLLLL,       NCSFL_TYPE_TILLLLLL,       "DUMMY %s TILLLLLL: %s %d %d \n %d %d %d %d \n" },
    { DUMMY_LOG_TILLLLLLL,      NCSFL_TYPE_TILLLLLLL,      "DUMMY %s TILLLLLLL: %s %d %d \n %d %d %d %d %d \n" },
    { DUMMY_LOG_TILLLLLLLL,     NCSFL_TYPE_TILLLLLLLL,     "DUMMY %s TILLLLLLLL: %s %d %d \n %d %d %d %d %d %d \n" },
    { DUMMY_LOG_TILLLLLLLLL,    NCSFL_TYPE_TILLLLLLLLL,    "DUMMY %s TILLLLLLLLL: %s %d %d \n %d %d %d %d %d %d %d \n" },
    { DUMMY_LOG_TILLLLLLLLLL,   NCSFL_TYPE_TILLLLLLLLLL,   "DUMMY %s TILLLLLLLLLL: %s %d %d \n %d %d %d %d %d %d %d %d \n" },
    { DUMMY_LOG_TIMM,           NCSFL_TYPE_TIMM,           "DUMMY %s TIMM: %s %x %x \n" },
    { DUMMY_LOG_TIAIA,          NCSFL_TYPE_TIAIA,          "DUMMY %s TIAIA: %s  \n %s %s %s \n"},
    { DUMMY_LOG_TIAD,           NCSFL_TYPE_TIAD,           "DUMMY %s TIAD: %s  \n %s %s \n"},
    { DUMMY_LOG_ILTI,           NCSFL_TYPE_ILTI,           "DUMMY %s ILTI: %d  \n %s %s \n"},
    { DUMMY_LOG_ILTIM,          NCSFL_TYPE_ILTIM,          "DUMMY %s ILTIM: %d  \n %s %s %d \n"},
    { DUMMY_LOG_ILTIL,          NCSFL_TYPE_ILTIL,          "DUMMY %s ILTIL: %d  \n %s %s %d \n"},
    { DUMMY_LOG_ILTII,          NCSFL_TYPE_ILTII,          "DUMMY %s ILTII: %d  \n %s %s %s \n"},
    { DUMMY_LOG_ILTIIL,         NCSFL_TYPE_ILTIIL,         "DUMMY %s ILTIIL: %d %s  \n %s %s %d \n"},
    { DUMMY_LOG_ILTILL,         NCSFL_TYPE_ILTILL,         "DUMMY %s ILTILL: %d %s \n  %s  %d %d \n"},
    { DUMMY_LOG_ILTILLL,        NCSFL_TYPE_ILTILLL,        "DUMMY %s ILTILLL: %d %s \n  %s  %d %d %d \n"},
    { DUMMY_LOG_TIALAL,         NCSFL_TYPE_TIALAL,         "DUMMY %s TIALAL: %s %s \n %d %s %d \n"     },
    { DUMMY_LOG_TIALL,          NCSFL_TYPE_TIALL,          "DUMMY %s TIALL: %s %s %d \n %d \n"     },
    { DUMMY_LOG_ILLLL,          NCSFL_TYPE_ILLLL,          "DUMMY %s ILLLL: %d  \n %d %d %d \n"},
    { DUMMY_LOG_ILTL,           NCSFL_TYPE_ILTL,           "DUMMY %s ILTL: %d  \n %s %d \n"},
    { DUMMY_LOG_ILTCLIL,        NCSFL_TYPE_ILTCLIL,        "DUMMY %s ILTCLIL: %d  \n %s %s %d \n %s %d \n"},
    { DUMMY_LOG_ILTISC,         NCSFL_TYPE_ILTISC,         "DUMMY %s ILTISC: %d %s  \n %s %d %s \n"},
    { DUMMY_LOG_ILTISCLC,       NCSFL_TYPE_ILTISCLC,       "DUMMY %s ILTISCLC: %d %s  \n %s %d %s \n %d %s \n"},
    { DUMMY_LOG_ILTIC,          NCSFL_TYPE_ILTIC,          "DUMMY %s ILTIC: %d  \n %s %s %s \n"},
    { DUMMY_LOG_ILTICL,         NCSFL_TYPE_ILTICL,         "DUMMY %s ILTICL: %d  \n %s %s %s %d \n"},
    { DUMMY_LOG_ILTILI,         NCSFL_TYPE_ILTILI,         "DUMMY %s ILTILI: %d  \n %s %s \n %d %s\n"},
    { DUMMY_LOG_ILTIILM,        NCSFL_TYPE_ILTIILM,        "DUMMY %s ILTIILM: %d %s  \n %s %s %d %d \n"},
    { DUMMY_LOG_ILTIILL,        NCSFL_TYPE_ILTIILL,        "DUMMY %s ILTIILL: %d %s  \n %s %s %d %d \n"},
    { DUMMY_LOG_ILTIILML,       NCSFL_TYPE_ILTIILML,       "DUMMY %s ILTIILML: %d %s  \n %s %s %d  \n %d %d \n"},
    { DUMMY_LOG_ILTIILLL,       NCSFL_TYPE_ILTIILLL,       "DUMMY %s ILTIILLL: %d %s  \n %s %s %d %d %d \n"},
    { DUMMY_LOG_ILTILIL,        NCSFL_TYPE_ILTILIL,        "DUMMY %s ILTILIL: %d  \n %s %s %d \n %s %d \n"},
    { DUMMY_LOG_ILTLLLL,        NCSFL_TYPE_ILTLLLL,        "DUMMY %s ILTLLLL: %d  \n %s %d \n %d %d %d \n"},
    { DUMMY_LOG_ILTILLM,        NCSFL_TYPE_ILTILLM,        "DUMMY %s ILTILLM: %d %s \n  %s  %d %d %d \n"},
    { DUMMY_LOG_TILAL,          NCSFL_TYPE_TILAL,          "DUMMY %s TILAL      : %s (%d)\n %s %d"},
    { DUMMY_LOG_TILLII,         NCSFL_TYPE_TILLII,         "DUMMY %s TILLII: %s %d %d \n %s %s \n" },
    { DUMMY_LOG_TILI,           NCSFL_TYPE_TILI,           "DUMMY %s TILI: %s %d %s \n" },
    { DUMMY_LOG_ILI,            NCSFL_TYPE_ILI,            "DUMMY ILI: %s %d %s \n" },
    { DUMMY_LOG_TIALALLLL,      NCSFL_TYPE_TIALALLLL,      "DUMMY %s TIALALLLL: %s %s \n %d %s %d \n %d %d %d \n"     },
    { DUMMY_LOG_TIALALLLLL,     NCSFL_TYPE_TIALALLLLL,     "DUMMY %s TIALALLLLL: %s %s \n %d %s %d \n %d %d %d %d \n"     },
    { DUMMY_LOG_TIALALLLLLL,    NCSFL_TYPE_TIALALLLLLL,    "DUMMY %s TIALALLLLLL: %s %s \n %d %s %d \n %d %d %d %d %d\n"     },
    { DUMMY_LOG_TICL,           NCSFL_TYPE_TICL,           "DUMMY %s TICL: %s \n %s %d\n"     },
    { DUMMY_LOG_TICLL,          NCSFL_TYPE_TICLL,          "DUMMY %s TICLL: %s \n %s %d %d \n"     },
    { DUMMY_LOG_TICLLL,         NCSFL_TYPE_TICLLL,         "DUMMY %s TICLLL: %s \n %s %d %d %d\n"     },
    { DUMMY_LOG_TICLLLL,        NCSFL_TYPE_TICLLLL,        "DUMMY %s TICLLLL: %s \n %s %d %d %d %d \n"     },
    { DUMMY_LOG_TICLLLLL,       NCSFL_TYPE_TICLLLLL,       "DUMMY %s TICLLLLL: %s \n %s %d %d %d %d %d \n"     },
    { DUMMY_LOG_TICCLLLL,       NCSFL_TYPE_TICCLLLL,       "DUMMY %s TICCLLLL: %s \n %s %s \n %d %d %d %d\n"     },
    { DUMMY_LOG_TIIC,           NCSFL_TYPE_TIIC,           "DUMMY %s TIIC: %s %s %s \n"  },
    { DUMMY_LOG_TIICC,          NCSFL_TYPE_TIICC,          "DUMMY %s TIICC: %s %s %s %s \n"  },

    /* Added new code to print float values */
    { DUMMY_LOG_TIF,            NCSFL_TYPE_TIF,            "DUMMY %s TIF: %s %s \n" },
    { DUMMY_LOG_TIN,            NCSFL_TYPE_TIN,            "DUMMY %s TIN: %s %s \n" },
    { DUMMY_LOG_TIU,            NCSFL_TYPE_TIU,            "DUMMY %s TIU: %s %s \n" },
    { DUMMY_LOG_TIX,            NCSFL_TYPE_TIX,            "DUMMY %s TIX: %s %s \n" },
    { DUMMY_LOG_TIDNUX,         NCSFL_TYPE_TIDNUX,         "DUMMY %s TIDNUX:%s %s %s %s %s \n" },
    { DUMMY_LOG_TINUXD,         NCSFL_TYPE_TINUXD,         "DUMMY %s TINUXD:%s %s %s %s %s \n" },
    { 0, 0, 0 }
  };


NCSFL_ASCII_SPEC dummy1_ascii_spec = 
  {
    UD_DTSV_DEMO_SERVICE1,              /* DUMMY subsystem - Log using OpenSAF service ID*/
    2,                              /* DUMMY revision ID */
   "dummy",
    (NCSFL_SET*)  dummy1_str_set,       /* DUMMY const strings referenced by index */
    (NCSFL_FMAT*) dummy1_fmat_set,      /* DUMMY string format info */
    0,                              /* Place holder for str_set count */
    0                               /* Place holder for fmat_set count */
  };

uns32
dummy_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;
                                                                                
   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         reg_dummy1_ascii_tbl();
         break;
                                                                                
      case NCS_LIB_REQ_DESTROY:
         de_reg_dummy1_ascii_tbl();
         break;
                                                                                
      default:
         break;
   }
   return (res);
}

void reg_dummy1_ascii_tbl()
{

   NCS_DTSV_REG_CANNED_STR arg;

   /*
    * First register ASCII_SPEC table.
    */
   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &dummy1_ascii_spec;
   ncs_dtsv_ascii_spec_api(&arg);

}

void de_reg_dummy1_ascii_tbl()
{

   NCS_DTSV_REG_CANNED_STR arg;

   /*
    * De-register ASCII_SPEC table.
    */

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = UD_DTSV_DEMO_SERVICE1;
   ncs_dtsv_ascii_spec_api(&arg);
}

