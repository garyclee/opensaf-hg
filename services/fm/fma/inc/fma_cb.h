/****************************************************************************
*                                                                           *
* NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
*                                                                           *
* THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
* AGREEMENT FROM EMERSON, INC.                                              *
*                                                                           *
* THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
* SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
*                                                                           *
* IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
* PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
*                                                                           *
*****************************************************************************




..............................................................................

DESCRIPTION:

*****************************************************************************/


/** Needed for creating a TASK **/
#define NCS_FMA_NAME       "FMA" 
#define NCS_FMA_PRIORITY   0 
#define NCS_FMA_STACK_SIZE 8192

#define NCS_FMA_LOG 1

/** Validate the FMA version **/
#define FMA_MAJOR_VERSION 0x01
#define FMA_MINOR_VERSION 0x01
#define m_FMA_VER_IS_VALID(ver) \
              ( (ver->releaseCode == 'B') && \
                (ver->majorVersion <= FMA_MAJOR_VERSION) )

#define m_FMA_ASSERT( condition ) if(!(condition)) m_NCS_OS_ASSERT(0)

/** FMA Agent Control Block **/

typedef struct fma_cb
{
   uns32      cb_hdl;    /* CB hdl returned by hdl mngr */
   uns32      slot;
   uns8       pool_id;   /* pool-id used by hdl mngr */
   NCS_LOCK   lock;      /* CB lock */
   uns32      pend_dispatch;  /* Number of pending dispatches */

   /* mds parameters */
   NCSCONTEXT      mds_hdl;    /* mds handle */
   MDS_DEST        my_dest;    /* FMA A-dest */
   MDS_DEST        fm_dest;  /* FM A-dest */
   NCS_SEL_OBJ_SET sel_obj;
   /** Mailbox **/
   SYSF_MBX mbx;

   uns32 fm_up;

   uns32 my_node_id;

   /* FMA handle database */
   FMA_HDL_DB hdl_db;

} FMA_CB;


EXTERN_C uns32 fma_get_slot_site_from_ent_path(SaHpiEntityPathT ent_path, uns8 *o_slot, uns8 *o_site);
EXTERN_C void fma_get_ent_path_from_slot_site(SaHpiEntityPathT *o_ent_path, uns8 slot, uns8 site);

