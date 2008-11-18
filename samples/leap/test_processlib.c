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
#include <opensaf/ncssysf_def.h>
#include "leaptest.h"

static uns32 lt_test_invoke_dl_routine(void);


int
lt_processlib_ops(int argc, char **argv)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    rc = lt_test_invoke_dl_routine( );

    return rc;
}

static uns32 lt_test_invoke_dl_routine( )
{
    int8    lib_name[255] = {0};
    int8    func_name[255] = {0};
    int8    gbl_var_name[255] = {0};
    uns32       status = NCSCC_RC_FAILURE;
    uns32       (*app_routine)(int arg) = NULL; 
    uns32       (*global_value) = NULL;
    NCS_OS_DLIB_HDL     *lib_hdl = NULL;
    int8        *dl_error = NULL; 


    m_NCS_STRCPY(&lib_name, "liblttest_invalid_dl_app.so");
    m_NCS_CONS_PRINTF("\nPerforming load on invalid library name... \n");
    lib_hdl = m_NCS_OS_DLIB_LOAD(lib_name, m_NCS_OS_DLIB_ATTR); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_NCS_CONS_PRINTF("\nLibrary could not be loaded: %s\n",lib_name);
        m_NCS_CONS_PRINTF("\tReported error : %s\n",dl_error);
    }

    m_NCS_CONS_PRINTF("\nPerforming load on a valid library name... \n");
    m_NCS_STRCPY(&lib_name, "liblttest_dl_app.so");
    lib_hdl = m_NCS_OS_DLIB_LOAD(lib_name, m_NCS_OS_DLIB_ATTR); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_NCS_CONS_PRINTF("\nLibrary could not be loaded: %s\n",lib_name);
        m_NCS_CONS_PRINTF("\tReported error : %s\n",dl_error);
        return NCSCC_RC_FAILURE; 
    }
    m_NCS_CONS_PRINTF("\nLoaded library : %s\n",lib_name);

    /* get the function pointer for invoking. */ 
    m_NCS_STRCPY(func_name, "lt_dl_app_routine");

    m_NCS_CONS_PRINTF("\nPerforming lookup on a valid symbol... \n");
    app_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, func_name); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_NCS_CONS_PRINTF("\nFATAL FAILURE : Symbol(%s) not found\n",func_name);
        m_NCS_CONS_PRINTF("\tReported error : %s\n",dl_error);
        m_NCS_OS_DLIB_CLOSE(lib_hdl);
        return NCSCC_RC_FAILURE; 
    }
    m_NCS_CONS_PRINTF("\nSymbol(%s) looked up successful \n",func_name);
    
    /* Invoke the function now, with argument 1. */
    m_NCS_CONS_PRINTF("\nInvoking Symbol(%s) with argument = 1\n",
    func_name);
    status = (*app_routine)(1); 
    if (status != NCSCC_RC_SUCCESS)
    {
        m_NCS_OS_DLIB_CLOSE(lib_hdl);
        return status;
    }
    
    /* Invoke the function now, with argument 2. */
    m_NCS_CONS_PRINTF("\nInvoking Symbol(%s) with argument = 2\n",
    func_name);
    status = (*app_routine)(2); 
    if (status != NCSCC_RC_SUCCESS)
    {
        m_NCS_OS_DLIB_CLOSE(lib_hdl);
        return status;
    }

    /* Lookup for an invalid symbol. */
    m_NCS_CONS_PRINTF("\nPerforming lookup on an invalid symbol... \n");
    m_NCS_STRCPY(func_name, "lt_dl_app_wrong_routine");
    app_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, func_name); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_NCS_CONS_PRINTF("\nLookup failed. Symbol(%s) not found\n",func_name);
        m_NCS_CONS_PRINTF("\tReported error : %s\n",dl_error);
    }

    /* Lookup for a Global variable . */
    m_NCS_CONS_PRINTF("\nPerforming lookup on a Global variable... \n");
    m_NCS_STRCPY(&gbl_var_name, "gl_dl_app_status");
    global_value = m_NCS_OS_DLIB_SYMBOL(lib_hdl, gbl_var_name); 
    if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) 
    {
        /* log the error returned from dlopen() */
        m_NCS_CONS_PRINTF("\nLookup on Global value failed. Global variable(%s) not found\n", 
        gbl_var_name);
        m_NCS_CONS_PRINTF("\tReported error : %s\n",dl_error);
    }
    m_NCS_CONS_PRINTF("\tGlobal variable retrieved, %s : %x\n", 
    gbl_var_name, *global_value);
    

    m_NCS_OS_DLIB_CLOSE(lib_hdl);

    m_NCS_CONS_PRINTF("\nTest successful... \n");
    return NCSCC_RC_SUCCESS;
}
