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

  MODULE NAME: VDS_CKPT.H
  
******************************************************************************/

#ifndef VDS_CKPT_H
#define VDS_CKPT_H

struct vds_cb;
struct vds_vdest_db_info;


typedef struct vds_ckpt_attrib
{
  SaCkptHandleT ckpt_hdl;
  SaCkptCheckpointHandleT checkpointHandle; 
} VDS_CKPT_ATTRIB; 

/* Macro to retrieve the CKPT version */
#define m_VDS_CKPT_VER_GET(ver) \
{ \
   ver.releaseCode = 'B'; \
   ver.majorVersion = 0x01; \
   ver.minorVersion = 0x01; \
};

typedef enum
{
   VDS_CKPT_OPERA_WRITE = 1,
   VDS_CKPT_OPERA_OVERWRITE,
   VDS_CKPT_OPERA_DEFAULT
} VDS_CKPT_OPERATIONS;

/* Macro definitions for checkpoint name */
#define VDS_CKPT_DBINFO_CKPT_NAME      "vds_vdest_db_1"
#define VDS_CKPT_DBINFO_CKPT_NAME_LEN  m_NCS_OS_STRLEN(VDS_CKPT_DBINFO_CKPT_NAME)
 
/* Macro definitions for section ID */
#define VDS_CKPT_LATEST_VDEST_SECTIONID     "vds_new_vdest"
#define VDS_CKPT_LATEST_VDEST_SECTIONID_LEN m_NCS_OS_STRLEN(VDS_CKPT_LATEST_VDEST_SECTIONID)

/* Macro definitions for section of Version */
#define VDS_CKPT_VERSION_SECTIONID     "vds_version"
#define VDS_CKPT_VERSION_SECTIONID_LEN m_NCS_OS_STRLEN(VDS_CKPT_VERSION_SECTIONID)

#define VDS_CKPT_MAX_SEC_SIZE    (VDS_CKPT_LATEST_VDEST_SECTIONID_LEN + 2)

#define VDS_CKPT_MAX_ADESTS            10
#define VDS_CKPT_DBINFO_RET_TIME       SA_TIME_END
#define VDS_CKPT_MAX_DBINFO_SECS       1000  /* NCS_MDS_MAX_VDEST, reduced to 1000*/
#define VDS_CKPT_SEC_DBINFO_SIZE       sizeof(VDS_CKPT_DBINFO);
#define VDS_CKPT_DBINFO_SIZE           ((VDS_CKPT_MAX_DBINFO_SECS -1) * sizeof(VDS_CKPT_DBINFO))
#define VDS_CKPT_TIMEOUT               6000000000LL

#define VDS_CKPT_NO_OF_SECTIONS_TOWRITE 1
#define VDS_CKPT_NO_OF_SECTIONS_TOREAD  VDS_CKPT_NO_OF_SECTIONS_TOWRITE 

#define m_VDS_CKPT_SET_OPEN_FLAGS(ckpt_open_flags) \
   ckpt_open_flags = SA_CKPT_CHECKPOINT_CREATE|SA_CKPT_CHECKPOINT_READ|SA_CKPT_CHECKPOINT_WRITE;

#define m_VDS_CKPT_UPDATE_CREATE_ATTR(attr) \
   m_NCS_OS_MEMSET(&attr, 0, sizeof(attr)); \
   attr.creationFlags = SA_CKPT_CHECKPOINT_COLLOCATED | SA_CKPT_WR_ALL_REPLICAS; \
   attr.checkpointSize    = VDS_CKPT_DBINFO_SIZE; \
   attr.retentionDuration = VDS_CKPT_DBINFO_RET_TIME; \
   attr.maxSections       = VDS_CKPT_MAX_DBINFO_SECS; \
   attr.maxSectionSize    = VDS_CKPT_SEC_DBINFO_SIZE; \
   attr.maxSectionIdSize  = VDS_CKPT_MAX_SEC_SIZE; 

/* checkpoint buffer size */
#define VDS_CKPT_BUFFER_SIZE 349

typedef struct vds_ckpt_dbinfo
{
   SaNameT   vdest_name;     /* Key to this structure */
   MDS_DEST  vdest_id;
   NCS_BOOL  persistent;
   uns16     adest_count; 
   MDS_DEST  adest[VDS_CKPT_MAX_ADESTS];
} VDS_CKPT_DBINFO;


/* Function prototypes */
EXTERN_C uns32 vds_ckpt_initialize(struct vds_cb *cb);
EXTERN_C uns32 vds_ckpt_finalize(struct vds_cb *cb);
EXTERN_C uns32 vds_ckpt_write(struct vds_cb *cb,
                     struct vds_vdest_db_info *vdest_dbinfo,
                     NCS_BOOL vds_ckpt_write_flag);
EXTERN_C uns32 vds_ckpt_dbinfo_write(struct vds_cb *cb, 
                               struct vds_vdest_db_info *vdest_dbinfo);
EXTERN_C uns32 vds_ckpt_dbinfo_overwrite(struct vds_cb *cb,
                               struct vds_vdest_db_info *vdest_dbinfo);
EXTERN_C uns32 vds_ckpt_dbinfo_delete(struct vds_cb *cb, MDS_DEST *vdest_name);
EXTERN_C uns32 vds_ckpt_read(struct vds_cb *cb);

#endif  /* VDS_CKPT_H */


