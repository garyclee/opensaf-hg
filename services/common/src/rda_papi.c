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

  $Header:  $

  MODULE NAME: rda_papi.c

..............................................................................

  DESCRIPTION:  This file defines the interface API for RDF. 

******************************************************************************
*/

/*
** Includes
*/
#include "rda_papi.h"
#include <dlfcn.h>


/*
** Error strings for API users
*/
char *pcsrda_err_str[] =
{
    "PCSRDA_RC_SUCCESS",
    "PCSRDA_RC_TIMEOUT",
    "PCSRDA_RC_INVALID_PARAMETER",
    "PCSRDA_RC_LIB_LOAD_FAILED",
    "PCSRDA_RC_LIB_NOT_INITIALIZED",
    "PCSRDA_RC_LIB_NOT_FOUND",
    "PCSRDA_RC_LIB_SYM_FAILED",
    "PCSRDA_RC_IPC_CREATE_FAILED",
    "PCSRDA_RC_IPC_CONNECT_FAILED",
    "PCSRDA_RC_IPC_SEND_FAILED",
    "PCSRDA_RC_IPC_RECV_FAILED",
    "PCSRDA_RC_TASK_SPAWN_FAILED",
    "PCSRDA_RC_MEM_ALLOC_FAILED",
    "PCSRDA_RC_CALLBACK_REG_FAILED",
    "PCSRDA_RC_CALLBACK_ALREADY_REGD",
    "PCSRDA_RC_ROLE_GET_FAILED",
    "PCSRDA_RC_ROLE_SET_FAILED",
    "PCSRDA_RC_AVD_HB_ERR_FAILED",
    "PCSRDA_RC_AVND_HB_ERR_FAILED",
    "PCSRDA_RC_LEAP_INIT_FAILED",
    "PCSRDA_RC_FATAL_IPC_CONNECTION_LOST"
};

char *pcsrda_role_str[] =
{ 
    "PCS_RDA_ACTIVE",
    "PCS_RDA_STANDBY",
    "PCS_RDA_QUIESCED",
    "PCS_RDA_ASSERTING",
    "PCS_RDA_YIELDING",
    "PCS_RDA_UNDEFINED"

};

/*
** Shared library name
*/
#define PCS_RDA_SHARED_LIB_NAME       "librda.so"

/*
** Symbols and function pointers
*/
#define PCS_RDA_SYM_REG_CALLBACK       "pcs_rda_reg_callback"
#define PCS_RDA_SYM_UNREG_CALLBACK     "pcs_rda_unreg_callback"
#define PCS_RDA_SYM_SET_ROLE           "pcs_rda_set_role"
#define PCS_RDA_SYM_GET_ROLE           "pcs_rda_get_role"
#define PCS_RDA_SYM_AVD_HB_ERR         "pcs_rda_avd_hb_err"
#define PCS_RDA_SYM_AVND_HB_ERR        "pcs_rda_avnd_hb_err"
#define PCS_RDA_SYM_AVD_HB_RESTORE         "pcs_rda_avd_hb_restore"
#define PCS_RDA_SYM_AVND_HB_RESTORE        "pcs_rda_avnd_hb_restore"

typedef int (*PCS_RDA_PTR_REG_CALLBACK)   (uns32, PCS_RDA_CB_PTR, long*);
typedef int (*PCS_RDA_PTR_UNREG_CALLBACK) (uns32);
typedef int (*PCS_RDA_PTR_SET_ROLE)       (PCS_RDA_ROLE);
typedef int (*PCS_RDA_PTR_GET_ROLE)       (PCS_RDA_ROLE*);
typedef int (*PCS_RDA_PTR_AVD_HB_ERR)     (void);
typedef int (*PCS_RDA_PTR_AVND_HB_ERR)     (uns32);
typedef int (*PCS_RDA_PTR_AVD_HB_RESTORE)     (void);
typedef int (*PCS_RDA_PTR_AVND_HB_RESTORE)     (uns32);

/*
** Global data
*/
static void * pcs_rda_lib_handle    = NULL;
static long  pcs_rda_callback_cb   = 0;


/*
** Helper functions
*/
static int pcs_rda_lib_init          (void);
static int pcs_rda_lib_destroy       (void);
static int pcs_rda_lib_reg_callback  (uns32, PCS_RDA_CB_PTR cb_ptr);
static int pcs_rda_lib_unreg_callback(void);
static int pcs_rda_lib_set_role      (PCS_RDA_ROLE    role);
static int pcs_rda_lib_get_role      (PCS_RDA_ROLE   *role);
static int pcs_rda_lib_avd_hb_err    (void);
static int pcs_rda_lib_avnd_hb_err   (uns32 phy_slot_id);
static int pcs_rda_lib_avd_hb_restore(void);
static int pcs_rda_lib_avnd_hb_restore(uns32 phy_slot_id);

/****************************************************************************
 * Name          : pcs_rda_request
 *
 * Description   : This is the single entry API function is used to 
 *                 initialize/destroy the RDA shared library and  to interact 
 *                 with RDA to register callback function, set HA role and to 
 *                 get HA role. 
 *                 
 *
 * Arguments     : pcs_rda_req - Pointer to a structure of type PCS_RDA_REQ 
 *                               containing information pertaining to the request. 
 *                               This structure could be a staticallly allocated 
 *                               or dynamically allocated strcuture. If it is 
 *                               dynamically allocated it is the user's responsibility 
 *                               to free it. . 
 *
 * Return Values : None
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_request(PCS_RDA_REQ *pcs_rda_req)
{
        int ret = PCSRDA_RC_SUCCESS;
        
    switch (pcs_rda_req->req_type)
    {

    case PCS_RDA_LIB_INIT:
                ret = pcs_rda_lib_init (); 
                break;

    case PCS_RDA_LIB_DESTROY:
                ret = pcs_rda_lib_destroy (); 
                break;

    case PCS_RDA_REGISTER_CALLBACK:
                ret = pcs_rda_lib_reg_callback (pcs_rda_req->callback_handle, pcs_rda_req->info.call_back);
                break;

    case PCS_RDA_UNREGISTER_CALLBACK:
                ret = pcs_rda_lib_unreg_callback ();
                break;

    case PCS_RDA_SET_ROLE:
                ret = pcs_rda_lib_set_role (pcs_rda_req->info.io_role);
                break;

    case PCS_RDA_GET_ROLE:
                pcs_rda_req->info.io_role = PCS_RDA_UNDEFINED;
                ret = pcs_rda_lib_get_role (&pcs_rda_req->info.io_role);
                break;
    case PCS_RDA_AVD_HB_ERR:
                ret = pcs_rda_lib_avd_hb_err ();
                break;
    case PCS_RDA_AVND_HB_ERR:
                ret = pcs_rda_lib_avnd_hb_err (pcs_rda_req->info.phy_slot_id);
                break;
    case PCS_RDA_AVD_HB_RESTORE:
                ret = pcs_rda_lib_avd_hb_restore();
                break;
    case PCS_RDA_AVND_HB_RESTORE:
                ret = pcs_rda_lib_avnd_hb_restore(pcs_rda_req->info.phy_slot_id);
                break;
    

    default:
                ret = PCSRDA_RC_INVALID_PARAMETER;

    }/* switch */

    /*
    ** Done
    */
    return ret;

}

/*
** Helper functions
*/
static int pcs_rda_lib_init (void)
{

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle != NULL)
    {
        /* Yes */
        return PCSRDA_RC_SUCCESS;

    }

    pcs_rda_lib_handle = dlopen (PCS_RDA_SHARED_LIB_NAME, RTLD_LAZY);
    if (pcs_rda_lib_handle == NULL)
    {
         printf ("%s\n", dlerror());
         return PCSRDA_RC_LIB_LOAD_FAILED;
        
    }

    /*
    ** Done
    */
    return PCSRDA_RC_SUCCESS;
}


static int pcs_rda_lib_destroy (void)
{

    /*
    ** Is the libray loaded?
    */
    if (pcs_rda_lib_handle == NULL)
    {
        /* No */
        return PCSRDA_RC_SUCCESS;

    }

    /*
    ** Need to unregiter the callback
    */
    pcs_rda_lib_unreg_callback ();

    /*
    ** Close shared library
    */
    dlclose (pcs_rda_lib_handle);
    pcs_rda_lib_handle = NULL;
    
    /*
    ** Done
    */
    return PCSRDA_RC_SUCCESS;

}

static int pcs_rda_lib_reg_callback (uns32 callback_handle, PCS_RDA_CB_PTR cb_ptr)
{
    PCS_RDA_PTR_REG_CALLBACK pcs_rda_ptr_reg_callback;

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle == NULL)
    {
            /* No */
            return PCSRDA_RC_LIB_NOT_INITIALIZED;

    }

    /*
    ** Is the callback already registered?
    */
    if (pcs_rda_callback_cb != 0)
    {
            /* Yes */
            return PCSRDA_RC_CALLBACK_ALREADY_REGD;

    }

    pcs_rda_ptr_reg_callback = (PCS_RDA_PTR_REG_CALLBACK) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_REG_CALLBACK);
    if (pcs_rda_ptr_reg_callback == NULL)
    {
            printf ("%s\n", dlerror());
            return PCSRDA_RC_LIB_SYM_FAILED;
    
    }

    /*
    ** Register callback
    */
    return (*pcs_rda_ptr_reg_callback) (callback_handle, cb_ptr, &pcs_rda_callback_cb);

}

static int pcs_rda_lib_unreg_callback ()
{
    /*
    ** Is the libray loaded?
    */
    if (pcs_rda_lib_handle == NULL)
    {
        /* No */
        return PCSRDA_RC_SUCCESS;

    }

    /*
    ** Need to unregiter the callback
    */
    if (pcs_rda_callback_cb != 0)
    {
        PCS_RDA_PTR_UNREG_CALLBACK pcs_rda_ptr_unreg_callback;
        pcs_rda_ptr_unreg_callback = (PCS_RDA_PTR_UNREG_CALLBACK) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_UNREG_CALLBACK);
        if (pcs_rda_ptr_unreg_callback == NULL)
        {
            printf ("%s\n", dlerror());
            return PCSRDA_RC_LIB_SYM_FAILED;
        }

        (*pcs_rda_ptr_unreg_callback) (pcs_rda_callback_cb);
        pcs_rda_callback_cb = 0;

    }

    /*
    ** Done
    */
    return PCSRDA_RC_SUCCESS;

}


static int pcs_rda_lib_set_role (PCS_RDA_ROLE role)
{
        PCS_RDA_PTR_SET_ROLE pcs_rda_set_role;

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle == NULL)
        {
                /* No */
                return PCSRDA_RC_LIB_NOT_INITIALIZED;

        }

        pcs_rda_set_role = (PCS_RDA_PTR_SET_ROLE) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_SET_ROLE);
        if (pcs_rda_set_role == NULL)
        {
                printf ("%s\n", dlerror());
                return PCSRDA_RC_LIB_SYM_FAILED;
        
        }

        /*
        ** Set role
        */
        return (*pcs_rda_set_role) (role);

}


static int pcs_rda_lib_get_role (PCS_RDA_ROLE   *role)
{
    PCS_RDA_PTR_GET_ROLE pcs_rda_get_role;

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle == NULL)
        {
                /* No */
                return PCSRDA_RC_LIB_NOT_INITIALIZED;

        }

        pcs_rda_get_role = (PCS_RDA_PTR_GET_ROLE) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_GET_ROLE);
        if (pcs_rda_get_role == NULL)
        {
                printf ("%s\n", dlerror());
                return PCSRDA_RC_LIB_SYM_FAILED;
        
        }

        /*
        ** Set role
        */
        return (*pcs_rda_get_role) (role);

}

static int pcs_rda_lib_avd_hb_err (void)
{
    PCS_RDA_PTR_AVD_HB_ERR pcs_rda_avd_hb_err;

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle == NULL)
        {
                /* No */
                return PCSRDA_RC_LIB_NOT_INITIALIZED;

        }

        pcs_rda_avd_hb_err = (PCS_RDA_PTR_AVD_HB_ERR) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_AVD_HB_ERR);
        if (pcs_rda_avd_hb_err == NULL)
        {
                printf ("%s\n", dlerror());
                return PCSRDA_RC_LIB_SYM_FAILED;
        
        }

        /*
        ** Avd Heart beat error
        */
        return (*pcs_rda_avd_hb_err) ();

}

static int pcs_rda_lib_avnd_hb_err(uns32 phy_slot_id)
{
   PCS_RDA_PTR_AVND_HB_ERR pcs_rda_avnd_hb_err;

   /*
   ** Is the libray already loaded?
   */
   if (pcs_rda_lib_handle == NULL)
   {
      /* No */
      return PCSRDA_RC_LIB_NOT_INITIALIZED;
   }

   pcs_rda_avnd_hb_err = (PCS_RDA_PTR_AVND_HB_ERR) dlsym (pcs_rda_lib_handle, 
                                                      PCS_RDA_SYM_AVND_HB_ERR);
   if (pcs_rda_avnd_hb_err == NULL)
   {
      printf ("%s\n", dlerror());
      return PCSRDA_RC_LIB_SYM_FAILED;
   }

   /*
   ** handle avnd heartbeat loss
   */
   return (*pcs_rda_avnd_hb_err) (phy_slot_id);
}

static int pcs_rda_lib_avd_hb_restore (void)
{
    PCS_RDA_PTR_AVD_HB_RESTORE pcs_rda_avd_hb_restore;

    /*
    ** Is the libray already loaded?
    */
    if (pcs_rda_lib_handle == NULL)
        {
                /* No */
                return PCSRDA_RC_LIB_NOT_INITIALIZED;

        }

        pcs_rda_avd_hb_restore = (PCS_RDA_PTR_AVD_HB_RESTORE) dlsym (pcs_rda_lib_handle, PCS_RDA_SYM_AVD_HB_RESTORE);
        if (pcs_rda_avd_hb_restore == NULL)
        {
                printf ("%s\n", dlerror());
                return PCSRDA_RC_LIB_SYM_FAILED;
        
        }

        /*
        ** Avd Heart beat restore 
        */
        return (*pcs_rda_avd_hb_restore) ();

}
static int pcs_rda_lib_avnd_hb_restore(uns32 phy_slot_id)
{
   PCS_RDA_PTR_AVND_HB_RESTORE pcs_rda_avnd_hb_restore;

   /*
   ** Is the libray already loaded?
   */
   if (pcs_rda_lib_handle == NULL)
   {
      /* No */
      return PCSRDA_RC_LIB_NOT_INITIALIZED;
   }

   pcs_rda_avnd_hb_restore = (PCS_RDA_PTR_AVND_HB_RESTORE) dlsym (pcs_rda_lib_handle, 
                                                      PCS_RDA_SYM_AVND_HB_RESTORE);
   if (pcs_rda_avnd_hb_restore == NULL)
   {
      printf ("%s\n", dlerror());
      return PCSRDA_RC_LIB_SYM_FAILED;
   }

   /*
   ** handle avnd heartbeat restore
   */
   return (*pcs_rda_avnd_hb_restore) (phy_slot_id);
}
