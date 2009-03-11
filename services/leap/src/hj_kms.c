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

..............................................................................

  PUBLIC FUNCTIONS INCLUDED in this module:

                                           ncskms_lm()
                                           ncskms_ss()
                                           ncskms_lock()
                                           ncskms_unlock()

  PRIVATE FUNCTIONS INCLUDED in this module:


******************************************************************************/

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"

/** Get the key abstractions for this thing...**/
#include "ncs_svd.h"
#include "ncs_kms.h"
#include "ncssysf_mem.h"
#include "ncssysf_def.h"

static uns32 kms_lm_create (NCSKMS_CREATE*  create );
static uns32 kms_lm_destroy(NCSKMS_DESTROY* destroy);
static uns32 kms_lm_add_rec(NCSKMS_ADD_REC* add_rec);
static uns32 kms_lm_rmv_rec(NCSKMS_RMV_REC* rmv_rec);
static uns32 kms_lm_add_key(NCSKMS_ADD_KEY* add_key);
static uns32 kms_lm_rmv_key(NCSKMS_RMV_KEY* rmv_key);

static uns32 kms_ss_get_by_name(NCSKMS_BY_NAME* by_name);
static uns32 kms_ss_get_by_pwe (NCSKMS_BY_PWE*  by_pwe );


/**********************************************************************/

/*****************************************************************************
 K M S _ K E Y _ E N T R Y
 
  A way to link all the keys that map to a PWE-id together.
  
 *****************************************************************************/

typedef struct kms_key_entry
  {
  struct kms_key_entry*  next;
  struct kms_key_entry*  prev;
  NCS_KEY                 key;

  } KMS_KEY_ENTRY;

static KMS_KEY_ENTRY* kms_kke_find_by_ss_id(KMS_KEY_ENTRY** anchor,
                                            NCS_SERVICE_ID   ss_id );
static uns32          kms_kke_rmv_all      (KMS_KEY_ENTRY*  anchor);
static uns32          kms_kke_rmv_entry    (KMS_KEY_ENTRY** anchor,
                                            KMS_KEY_ENTRY*  entry );
static uns32          kms_kke_insert       (KMS_KEY_ENTRY** anchor,
                                            NCS_KEY*         key   );


/* move these macros */

#define m_MMGR_ALLOC_KMS_KEY_ENTRY   (KMS_KEY_ENTRY*)                         \
                                      m_NCS_MEM_ALLOC(sizeof(KMS_KEY_ENTRY),   \
                                                     NCS_MEM_REGION_PERSISTENT,\
                                                     NCS_SERVICE_ID_COMMON,    \
                                                     31);


#define m_MMGR_FREE_KMS_KEY_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,\
                                                      NCS_SERVICE_ID_COMMON,    \
                                                      31) 

/*****************************************************************************
 K M S _ R E C
 
  The actual record of data points associated with a PWE-id. In this scheme, 
  1) a new KMS_SET is dynamically added to the list
  2) the PWE-id can be assigned an alias 'name'
 *****************************************************************************/

typedef struct kms_rec  
  {   
  uns32            pwe_id;       /* pwe_id; the persistent identity value    */
  char*            name;         /* a string name for this same PWE-id rec   */
  KMS_KEY_ENTRY*   list;         /*ptr to the list of KMS_KEY_ENTRY instances*/

  } KMS_REC;

/*****************************************************************************
 K M S _ R E C _ E N T R Y
 *****************************************************************************/

typedef struct kms_rec_entry
{
  struct kms_rec_entry* next;
  struct kms_rec_entry* prev;

  KMS_REC rec;

} KMS_REC_ENTRY;

static uns32           kms_kre_rmv_all     (KMS_REC_ENTRY*    anchor);
static uns32           kms_kre_rmv_entry   (KMS_REC_ENTRY**   anchor,
                                            KMS_REC_ENTRY*    entry );
static uns32           kms_kre_insert      (KMS_REC_ENTRY**   anchor,
                                            uns32             pwe_id,
                                            char*             name  );
static KMS_REC_ENTRY*  kms_kre_find_by_pwe (KMS_REC_ENTRY**   anchor,
                                            uns32             pwe_id);
static KMS_REC_ENTRY*  kms_kre_find_by_name(KMS_REC_ENTRY**   anchor,
                                            char*             name  );
/* move these macros */

#define m_MMGR_ALLOC_KMS_REC_ENTRY   (KMS_REC_ENTRY*)                         \
                                      m_NCS_MEM_ALLOC(sizeof(KMS_REC_ENTRY),   \
                                                     NCS_MEM_REGION_PERSISTENT,\
                                                     NCS_SERVICE_ID_COMMON,    \
                                                     32)


#define m_MMGR_FREE_KMS_REC_ENTRY(p) m_NCS_MEM_FREE(p, NCS_MEM_REGION_PERSISTENT,\
                                                      NCS_SERVICE_ID_COMMON,    \
                                                      32) 

static KMS_REC_ENTRY*  gl_kms_anchor = NULL;
static NCS_LOCK         gl_kms_lock;

/**| PUBLIC |****************************************************************/

uns32 ncskms_lock  (void)
{
  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_READ);
  return NCSCC_RC_SUCCESS;
}

uns32 ncskms_unlock(void)
{
  m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_READ);
  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncskms_lm

  DESCRIPTION:
    KMS entry point

  RETURNS:
*****************************************************************************/

uns32 ncskms_lm( NCSKMS_LM_ARG* arg)
  {

  switch(arg->i_op)
  {
  case NCSKMS_LM_CREATE:
    return kms_lm_create(&arg->op.create);
  case NCSKMS_LM_DESTROY:
    return kms_lm_destroy(&arg->op.destroy);
  case NCSKMS_LM_ADD_REC:
    return kms_lm_add_rec(&arg->op.add_rec);
  case NCSKMS_LM_RMV_REC:
    return kms_lm_rmv_rec(&arg->op.rmv_rec);
  case NCSKMS_LM_ADD_KEY:
    return kms_lm_add_key(&arg->op.add_key);
  case NCSKMS_LM_RMV_KEY:
    return kms_lm_rmv_key(&arg->op.rmv_key);
  default:
    return NCSCC_RC_FAILURE;
  }
    return NCSCC_RC_SUCCESS;
  }


/*****************************************************************************

  PROCEDURE NAME:   ncskms_ss

  DESCRIPTION:
    KMS entry point

  RETURNS:
*****************************************************************************/

uns32 ncskms_ss( NCSKMS_SS_ARG* arg )
  {

  switch(arg->i_op)
  {
  case NCSKMS_SS_GET_BY_NAME:
    return kms_ss_get_by_name(&arg->op.by_name);
  case NCSKMS_SS_GET_BY_PWE:
    return kms_ss_get_by_pwe(&arg->op.by_pwe);
  default:
    return NCSCC_RC_FAILURE;
  }
    return NCSCC_RC_SUCCESS;
  }

/***| PRIVATE |********************************************************************/

uns32 kms_lm_create (NCSKMS_CREATE*  create )
{
  if(m_NCS_LOCK_INIT(&gl_kms_lock) != NCSCC_RC_SUCCESS)
  {
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }
  return NCSCC_RC_SUCCESS;
}

uns32 kms_lm_destroy(NCSKMS_DESTROY* destroy)
{
  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  kms_kre_rmv_all(gl_kms_anchor);

  m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  if(m_NCS_LOCK_DESTROY(&gl_kms_lock) != NCSCC_RC_SUCCESS)
  {
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }
  return NCSCC_RC_SUCCESS;
}

uns32 kms_lm_add_rec(NCSKMS_ADD_REC* add_rec)
{
  return kms_kre_insert(&gl_kms_anchor,add_rec->i_pwe_id,add_rec->i_name);
}

uns32 kms_lm_rmv_rec(NCSKMS_RMV_REC* rmv_rec)
{
  KMS_REC_ENTRY* kre = NULL;
 
  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  if((kre = kms_kre_find_by_pwe(&gl_kms_anchor,rmv_rec->i_pwe_id)) != NULL)
  {
    kms_kre_rmv_entry(&gl_kms_anchor,kre);
    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
  }
  else
  {
    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }

  return NCSCC_RC_SUCCESS;
}

uns32 kms_lm_add_key(NCSKMS_ADD_KEY* add_key)
{
  KMS_REC_ENTRY* kre = NULL;

  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  kre = kms_kre_find_by_pwe(&gl_kms_anchor,add_key->i_pwe_id);
  if(kre != NULL)
  {
    if(kms_kke_insert(&kre->rec.list,add_key->i_key) == NCSCC_RC_FAILURE)
    {
      m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }
    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
  }
  else
  {
    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }

  return NCSCC_RC_SUCCESS;
}

uns32 kms_lm_rmv_key(NCSKMS_RMV_KEY* rmv_key)
{
  KMS_REC_ENTRY* kre = NULL;

  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  kre = kms_kre_find_by_pwe(&gl_kms_anchor,rmv_key->i_pwe_id);
  if(kre != NULL)
  {
    KMS_KEY_ENTRY* kke = NULL;

    if((kke = kms_kke_find_by_ss_id(&kre->rec.list,rmv_key->i_ss_id)) == NULL)
    {
      m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

    kms_kke_rmv_entry(&kre->rec.list,kke);


    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
  }
  else
  {
    m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }

  return NCSCC_RC_SUCCESS;
}


uns32 kms_ss_get_by_name(NCSKMS_BY_NAME* by_name)
{
  KMS_REC_ENTRY* kre = NULL;

  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  kre = kms_kre_find_by_name(&gl_kms_anchor,by_name->i_name);
  if(kre != NULL)
  {
    KMS_KEY_ENTRY* kke = NULL;

    if((kke = kms_kke_find_by_ss_id(&kre->rec.list,by_name->i_ss_id)) == NULL)
    {
      by_name->o_key = NULL;
    }
    else
    {
      by_name->o_key = &kke->key;
    }
  }
  else
  {
    by_name->o_key = NULL;
  }

  m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}

uns32 kms_ss_get_by_pwe (NCSKMS_BY_PWE*  by_pwe )
{
  KMS_REC_ENTRY* kre = NULL;

  m_NCS_LOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  kre = kms_kre_find_by_pwe(&gl_kms_anchor,by_pwe->i_pwe);
  if(kre != NULL)
  {
    KMS_KEY_ENTRY* kke = NULL;

    if((kke = kms_kke_find_by_ss_id(&kre->rec.list,by_pwe->i_ss_id)) == NULL)
    {
      by_pwe->o_key = NULL;
    }
    else
    {
      by_pwe->o_key = &kke->key;
    }
  }
  else
  {
    by_pwe->o_key = NULL;
  }

  m_NCS_UNLOCK(&gl_kms_lock,NCS_LOCK_WRITE);

  return NCSCC_RC_SUCCESS;
}




uns32 kms_kre_rmv_all(KMS_REC_ENTRY* anchor)
{
  KMS_REC_ENTRY* del;
  KMS_REC_ENTRY* cur = anchor;

  if(cur != NULL)
  {
    do
    {
      del = cur;
      cur = cur->next;
      kms_kke_rmv_all(del->rec.list);
      m_MMGR_FREE_KMS_REC_ENTRY(del);
    }
    while(cur);
  }

  return NCSCC_RC_SUCCESS;
}

uns32 kms_kre_rmv_entry(KMS_REC_ENTRY** anchor,
                        KMS_REC_ENTRY* entry)
{
  if(entry->prev != NULL)
    entry->prev->next = entry->next;
  else
  {
    /* we are deleting the head */
  if((anchor == NULL) || (*anchor != entry))
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  *anchor = entry->next;
  }
  if(entry->next != NULL)
    entry->next->prev = entry->prev;

  kms_kke_rmv_all(entry->rec.list);
  m_MMGR_FREE_KMS_REC_ENTRY(entry);

  return NCSCC_RC_SUCCESS;
}


uns32 kms_kre_insert(KMS_REC_ENTRY** anchor,
                     uns32           pwe_id,
                     char*           name  )
{
  KMS_REC_ENTRY* new_kre = NULL;

  if(anchor == NULL)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  new_kre = m_MMGR_ALLOC_KMS_REC_ENTRY;
  if(new_kre == NULL)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  memset(new_kre,0,sizeof(KMS_REC_ENTRY));
  new_kre->rec.pwe_id = pwe_id;
  new_kre->rec.name = m_NCS_MEM_ALLOC((m_NCS_STRLEN(name) + 1) * sizeof(char),
                                      NCS_MEM_REGION_PERSISTENT,
                                      NCS_SERVICE_ID_COMMON,30); 
    
  if(new_kre->rec.name == NULL)
  {
     m_MMGR_FREE_KMS_REC_ENTRY(new_kre);
     return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }
  strcpy(new_kre->rec.name,name);

  new_kre->next = *anchor;
  if(new_kre->next != NULL)
    new_kre->next->prev = new_kre;
  new_kre->prev = NULL;
  *anchor = new_kre;

  return NCSCC_RC_SUCCESS;
}


KMS_REC_ENTRY* 
kms_kre_find_by_pwe(KMS_REC_ENTRY** anchor,
                    uns32           pwe_id)
{
  KMS_REC_ENTRY* kre;

  if((anchor == NULL) || (*anchor == NULL))
    {
    m_LEAP_DBG_SINK(NULL);
    return NULL;
    }

  for(kre = *anchor;kre != NULL;kre = kre->next)
  {
    if(kre->rec.pwe_id == pwe_id)
    {
      return kre;
    }
  }

  return NULL;
}


KMS_REC_ENTRY* 
kms_kre_find_by_name(KMS_REC_ENTRY** anchor,
                     char*           name  )
{
  KMS_REC_ENTRY* kre;

  if((anchor == NULL) || (*anchor == NULL))
  {
    m_LEAP_DBG_SINK(NULL);
    return NULL;
  }

  for(kre = *anchor;kre != NULL;kre = kre->next)
  {
    if(m_NCS_STRCMP(kre->rec.name,name) == 0)
    {
      return kre;
    }
  }

  return NULL;
}



KMS_KEY_ENTRY* 
kms_kke_find_by_ss_id(KMS_KEY_ENTRY** anchor,
                      NCS_SERVICE_ID   ss_id )
{
  KMS_KEY_ENTRY* kke;

  if((anchor == NULL) || (*anchor == NULL))
  {
    m_LEAP_DBG_SINK(NULL);
    return NULL;
  }

  for(kke = *anchor;kke != NULL;kke = kke->next)
  {
    if(kke->key.svc == ss_id)
    {
      return kke;
    }
  }


  return NULL;
}


uns32 kms_kke_rmv_all(KMS_KEY_ENTRY* anchor)
{
  KMS_KEY_ENTRY* del;
  KMS_KEY_ENTRY* cur = anchor;

  if(cur != NULL)
  {
    do
    {
      del = cur;
      cur = cur->next;
      m_MMGR_FREE_KMS_KEY_ENTRY(del);
    }
    while(cur);
  }

  return NCSCC_RC_SUCCESS;
}

uns32 kms_kke_rmv_entry(KMS_KEY_ENTRY** anchor,
                        KMS_KEY_ENTRY*  entry )
{
  if(entry->prev != NULL)
    entry->prev->next = entry->next;
  else
  {
    /* we are deleting the head */
  if((anchor == NULL) || (*anchor != entry))
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  *anchor = entry->next;
  }
  if(entry->next != NULL)
    entry->next->prev = entry->prev;

  m_MMGR_FREE_KMS_KEY_ENTRY(entry);
  return NCSCC_RC_SUCCESS;
}
uns32 kms_kke_insert(KMS_KEY_ENTRY** anchor,
                     NCS_KEY*         key   )
{
  KMS_KEY_ENTRY* new_kke = NULL;

  if((anchor == NULL) || (key == NULL))
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  new_kke = m_MMGR_ALLOC_KMS_KEY_ENTRY;
  if(new_kke == NULL)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  memset(new_kke,0,sizeof(KMS_KEY_ENTRY));
  new_kke->key  = *key;

  new_kke->next = *anchor;
  if(new_kke->next != NULL)
    new_kke->next->prev = new_kke;
  new_kke->prev = NULL;
  *anchor = new_kke;

  return NCSCC_RC_SUCCESS;
}




