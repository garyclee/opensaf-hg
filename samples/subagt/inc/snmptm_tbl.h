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

  MODULE NAME: SNMPTM_TBL.H 

..............................................................................

  DESCRIPTION:  Defines SNMPTM table structures                                                              
..............................................................................

******************************************************************************/

#ifndef  SNMPTM_TBL_H
#define  SNMPTM_TBL_H

/* Scalar table, has support for different base types */
typedef struct snmptm_scalar_tbl
{
   uns32       scalar_uns32;
   int         scalar_int;
   NCS_IP_ADDR scalar_ip_addr;
   char        scalar_notify[255];
   uns8        scalar_depricate;
   uns16       scalar_not_access; 
} SNMPTM_SCALAR_TBL;

#define SNMPTM_SCALAR_TBL_NULL ((SNMPTM_SCALAR_TBL *)0)

/* 
 * Definition of an table_one key, used to access the table_one
 * parameters from the table_one patricia tree.
 */
typedef struct snmptm_tbl_key
{
   NCS_IP_ADDR  ip_addr;
} SNMPTM_TBL_KEY;



/* Non-Scalar table, has support for different base types */
typedef struct snmptm_tblone
{
   NCS_PATRICIA_NODE  tblone_pat_node; 
   SNMPTM_TBL_KEY     tblone_key;
   char               tblone_disp_str[255];
   NCS_IP_ADDR        tblone_mac_addr;
   int                tblone_test_incr;
   uns32              tblone_cntr32;
   int                tblone_discrete;
   uns8               tblone_row_status;   
   long long          tblone_cntr64;
   uns32              tblone_g32_discrete;
   char               tblone_oct_str_fixed_len[10];  
   char               tblone_oct_str_var_len[100];  
   char               tblone_oct_str_discrete[100];  
   uns16              tblone_obj_id_len;
   uns32              tblone_obj_id[128];
} SNMPTM_TBLONE;

#define SNMPTM_TBLONE_NULL ((SNMPTM_TBLONE *)0)


/* Currently for this table, OpenSAF MibLib support is not there. 
 * Once the support is available, need to implement the MIB 
 * for this table */
typedef struct snmptm_two_tbl
{
   NCS_PATRICIA_NODE  tbltwo_pat_node; 
   uns32              tbltwo_cntr32;
   uns32              tbltwo_cntr64[2];
   char               *tbltwo_obj_idntfr;
} SNMPTM_TWO_TBL;


/* table-four details */
typedef struct snmptm_tblfour
{
    uns32                  index;          /* TBL FOUR Index_ID */
    uns32                  int_val;    
    NCSMIB_ROWSTATUS       status;      
    NCSCONTEXT             o_row_hdl;      /* row handle given by OAC */ 
    struct snmptm_tblfour  *next;
}SNMPTM_TBLFOUR ;

#define SNMPTM_TBLFOUR_NULL ((SNMPTM_TBLFOUR *)0)

/* Data struct for table three */
typedef struct snmptm_tblthree
{
   NCS_PATRICIA_NODE  tblthree_pat_node; 
   SNMPTM_TBL_KEY     tblthree_key;
   uns8               tblthree_row_status;   
} SNMPTM_TBLTHREE;

#define SNMPTM_TBLTHREE_NULL ((SNMPTM_TBLTHREE *)0)

/* Data struct for table five */
typedef struct snmptm_tblfive
{
   NCS_PATRICIA_NODE  tblfive_pat_node; 
   SNMPTM_TBL_KEY     tblfive_key;
   uns8               tblfive_row_status;   
} SNMPTM_TBLFIVE;

#define SNMPTM_TBLFIVE_NULL ((SNMPTM_TBLFIVE *)0)

/* Data struct for table six */
typedef struct snmptm_tblsix_key
{
   uns32        count;
} SNMPTM_TBLSIX_KEY;
typedef struct snmptm_tblsix
{
   NCS_PATRICIA_NODE  tblsix_pat_node; 
   SNMPTM_TBLSIX_KEY  tblsix_key;
   uns32              tblsix_data;   
   char               tblsix_name[9]; /* Maximum 8 characters */
} SNMPTM_TBLSIX;

#define SNMPTM_TBLSIX_NULL ((SNMPTM_TBLSIX *)0)

typedef struct snmptm_tblseven
{
    uns8        string[128]; 
    uns32       length; 
    uns32       move_me; 
    uns32       status; 
    NCSCONTEXT  o_row_hdl; /* row handle given by OAC */ 
    struct snmptm_tblseven *next; 
}SNMPTM_TBLSEVEN; 
#define SNMPTM_TBLSEVEN_NULL ((SNMPTM_TBLSEVEN *)0)

/* Data struture for table eight and nine */
typedef struct snmptm_tbleight_key
{
   uns32 ifIndex;
   uns32 tbleight_unsigned32;
   uns32 tblfour_unsigned32;
} SNMPTM_TBLEIGHT_KEY;

/* Table nine is augmented table of table eight */
typedef struct snmptm_tbleight
{
   NCS_PATRICIA_NODE     tbleight_pat_node;
   SNMPTM_TBLEIGHT_KEY   tbleight_key;
   uns32                 tbleight_row_status;
   uns32                 tblnine_unsigned32;
} SNMPTM_TBLEIGHT;

#define  SNMPTM_TBLEIGHT_NULL ((SNMPTM_TBLEIGHT *)0)
#define  SNMPTM_TBLNINE_NULL  SNMPTM_TBLEIGHT_NULL

/* Data struture for table ten */
typedef struct snmptm_tblten_key
{
   uns32 tblten_unsigned32;
   uns32 tblten_int;
} SNMPTM_TBLTEN_KEY;

/* Table ten MIB objects are all index elements. */
typedef struct snmptm_tblten
{
   NCS_PATRICIA_NODE     tblten_pat_node;
   SNMPTM_TBLTEN_KEY     tblten_key;
} SNMPTM_TBLTEN;

#define  SNMPTM_TBLTEN_NULL ((SNMPTM_TBLTEN *)0)

/* Command Ids, to check MASv-CLI feature */
typedef enum snmptm_cmdids
{
   tblone_create_row_cmdid = 1, /* Create a single ROW of TBLONE */
   tblfive_create_row_cmdid,    /* Create a single ROW of TBLFIVE */
   tblone_show_all_rows_cmdid,  /* Display the complete TBLONE data */
   tblfive_show_all_rows_cmdid, /* Display the complete TBLFIVE data */
   tblsix_show_all_rows_cmdid, /* Display the complete TBLSIX data */
   tblone_tblfive_show_data_cmdid,  /* REQESTING data of both tables, two USRBUFs*/ 
   tblone_tblfive_show_glb_data_cmdid  /* REQESTING data of both tables, two USRBUFs*/ 
} SNMPTM_CMDIDS;

typedef struct snmptm_hdr 
{
  uns32 table_id;
  uns32 inst_id;  
} SNMPTM_HDR;

#define SNMPTM_HDR_SIZE sizeof(SNMPTM_HDR)

typedef struct snmptm_tblone_entry
{
   uns32 ip_addr;
   uns32 cntrs;
   uns32 row_status;
} SNMPTM_TBLONE_ENTRY;

#define SNMPTM_TBLONE_ENTRY_SIZE sizeof(SNMPTM_TBLONE_ENTRY)

typedef struct snmptm_tblfive_entry
{
   uns32 ip_addr;
   uns32 row_status;
} SNMPTM_TBLFIVE_ENTRY;

#define SNMPTM_TBLFIVE_ENTRY_SIZE sizeof(SNMPTM_TBLFIVE_ENTRY)

/**************************************************************************
                   Function Prototypes
**************************************************************************/
/* Related to snmptm_mib.c */
struct snmptm_cb;

EXTERN_C uns32 snmptm_reg_with_oac(struct snmptm_cb*);
EXTERN_C uns32 snmptm_unreg_with_oac(struct snmptm_cb *);
EXTERN_C uns32 snmptm_miblib_reg(void);



/* Defined in snmptm_tblone.c */
EXTERN_C struct snmptm_tblone *snmptm_create_tblone_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBL_KEY *tblone_key);
EXTERN_C void  snmptm_delete_tblone(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tblone_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLONE *tblone);
EXTERN_C uns32 snmptm_tblone_gen_trap(struct snmptm_cb *snmptm, 
                                      NCSMIB_ARG *args,
                                      SNMPTM_TBLONE *tblone);

/* Defined in snmptm_tblthree.c */
EXTERN_C struct snmptm_tblthree *snmptm_create_tblthree_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBL_KEY *tblthree_key);
EXTERN_C void  snmptm_delete_tblthree(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tblthree_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLTHREE *tblthree);

/* Defined in snmptm_tblfive.c */
EXTERN_C struct snmptm_tblfive *snmptm_create_tblfive_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBL_KEY *tblfive_key);
EXTERN_C void  snmptm_delete_tblfive(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tblfive_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLFIVE *tblfive);

/* Defined in snmptm_tblsix.c */
EXTERN_C struct snmptm_tblsix *snmptm_create_tblsix_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBLSIX_KEY *tblsix_key);
EXTERN_C void  snmptm_delete_tblsix(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tblsix_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLSIX *tblsix);

/* Defined in snmptm_tbleight.c */
EXTERN_C struct snmptm_tbleight *snmptm_create_tbleight_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBLEIGHT_KEY *tbleight_key);
EXTERN_C void  snmptm_delete_tbleight(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tbleight_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLEIGHT *tbleight);

/* Defined in snmptm_tblten.c */
EXTERN_C struct snmptm_tblten *snmptm_create_tblten_entry(struct snmptm_cb *snmptm,
                                          SNMPTM_TBLTEN_KEY *tblten_key);
EXTERN_C void  snmptm_delete_tblten(struct snmptm_cb *snmptm);

EXTERN_C void snmptm_delete_tblten_entry(struct snmptm_cb *snmptm,
                                         SNMPTM_TBLTEN *tblten);

/**********************************************************************
               Related to snmptm_scalar_mib.c
**********************************************************************/
EXTERN_C uns32 ncstestscalars_tbl_reg(void); 
EXTERN_C uns32 snmptm_scalar_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstestscalars_set(NCSCONTEXT cb,
                                  NCSMIB_ARG *arg,
                                  NCSMIB_VAR_INFO *var_info,
                                  NCS_BOOL test_flag);
EXTERN_C uns32 ncstestscalars_extract(NCSMIB_PARAM_VAL *param, 
                                      NCSMIB_VAR_INFO  *var_info,
                                      NCSCONTEXT data,
                                      NCSCONTEXT buffer);
EXTERN_C uns32 ncstestscalars_get(NCSCONTEXT snmptm,
                                  NCSMIB_ARG *arg,
                                  NCSCONTEXT *data);
EXTERN_C uns32 ncstestscalars_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstestscalars_next(NCSCONTEXT snmptm,
                                   NCSMIB_ARG *arg,
                                   NCSCONTEXT* data,
                                   uns32* next_inst_id,
                                   uns32* next_inst_id_len);
EXTERN_C uns32 ncstestscalars_rmvrow(NCSCONTEXT cb_hdl, 
                                     NCSMIB_IDX *idx);

/**********************************************************************
               Related to snmptm_tblone_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttableoneentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblone_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttableoneentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttableoneentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttableoneentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttableoneentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttableoneentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

/**********************************************************************
               Related to snmptm_tblthree_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttablethreeentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblthree_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttablethreeentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttablethreeentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttablethreeentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttablethreeentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttablethreeentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

/**********************************************************************
               Related to snmptm_tblfour_mib.c
**********************************************************************/
EXTERN_C uns32 snmptm_oac_tblfour_register(struct snmptm_cb *inst);
EXTERN_C uns32 ncstesttablefourentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblfour_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttablefourentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttablefourentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttablefourentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttablefourentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttablefourentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

/**********************************************************************
               Related to snmptm_tblfive_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttablefiveentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblfive_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttablefiveentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttablefiveentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttablefiveentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttablefiveentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttablefiveentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

 
/**********************************************************************
               Related to snmptm_tblsix_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttablesixentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblsix_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttablesixentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttablesixentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttablesixentry_rmvrow(NCSCONTEXT cb_hdl, 
                                            NCSMIB_IDX *idx);
EXTERN_C uns32 ncstesttablesixentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttablesixentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttablesixentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

 

/**********************************************************************
               Related to snmptm_tblcmd.c
**********************************************************************/
EXTERN_C uns32 snmptm_oac_tblcmd_register(struct snmptm_cb *snmptm);
EXTERN_C uns32 snmptm_tblcmd_tbl_req(struct ncsmib_arg *args);

/**********************************************************************
               Related to snmptm_tbltwo_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttabletwoentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tbltwo_tbl_req(struct ncsmib_arg *args);

/**********************************************************************
               Related to snmptm_tblseven_mib.c
**********************************************************************/
EXTERN_C uns32 snmptm_oac_tblseven_register(struct snmptm_cb *inst);
EXTERN_C uns32 ncstesttablesevenentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblseven_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttablesevenentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttablesevenentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttablesevenentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttablesevenentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttablesevenentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

EXTERN_C uns32 ncstesttablesevenentry_rmvrow(NCSCONTEXT cb_hdl, NCSMIB_IDX *idx);

/**********************************************************************
               Related to snmptm_tbleight_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttableeightentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tbleight_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttableeightentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttableeightentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttableeightentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttableeightentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttableeightentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);

EXTERN_C uns32 ncstesttableeightentry_verify_instance (NCSMIB_ARG *);

#define snmptm_tblnine_tbl_req  snmptm_tbleight_tbl_req
EXTERN_C uns32 ncstesttablenineentry_tbl_reg(void);
#define  ncstesttablenineentry_set  ncstesttableeightentry_set
#define  ncstesttablenineentry_get  ncstesttableeightentry_get
#define  ncstesttablenineentry_extract  ncstesttableeightentry_extract
#define  ncstesttablenineentry_setrow  ncstesttableeightentry_setrow
#define  ncstesttablenineentry_next  ncstesttableeightentry_next
#define  ncstesttablenineentry_verify_instance  ncstesttableeightentry_verify_instance

/**********************************************************************
               Related to snmptm_tblten_mib.c
**********************************************************************/
EXTERN_C uns32 ncstesttabletenentry_tbl_reg(void);
EXTERN_C uns32 snmptm_tblten_tbl_req(struct ncsmib_arg *args);
EXTERN_C uns32 ncstesttabletenentry_set(NCSCONTEXT cb,
                                        NCSMIB_ARG *arg,
                                        NCSMIB_VAR_INFO *var_info,
                                        NCS_BOOL test_flag);
EXTERN_C uns32 ncstesttabletenentry_extract(NCSMIB_PARAM_VAL *param, 
                                            NCSMIB_VAR_INFO  *var_info,
                                            NCSCONTEXT data,
                                            NCSCONTEXT buffer);
EXTERN_C uns32 ncstesttabletenentry_get(NCSCONTEXT snmptm,
                                        NCSMIB_ARG *arg,
                                        NCSCONTEXT *data);
EXTERN_C uns32 ncstesttabletenentry_setrow(NCSCONTEXT cb,
                            NCSMIB_ARG *args,
                            NCSMIB_SETROW_PARAM_VAL *summ_addr_tbl_row,
                            struct ncsmib_obj_info *obj_info,
                            NCS_BOOL testrow_flag);
EXTERN_C uns32 ncstesttabletenentry_next(NCSCONTEXT snmptm,
                                         NCSMIB_ARG *arg,
                                         NCSCONTEXT *data,
                                         uns32 *next_inst_id,
                                         uns32 *next_inst_id_len);
EXTERN_C uns32 ncstesttabletenentry_rmvrow(NCSCONTEXT cb_hdl, 
                                            NCSMIB_IDX *idx);

#endif  /* SNMPTM_TBL_H */


