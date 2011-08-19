/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**
** DATE: 
**   Wednesday November 19, 2008
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS.
**
** Copyright 2008 by the Service Availability Forum. All rights reserved.
**
** Permission to use, copy, and distribute this mapping specification for any
** purpose without fee is hereby granted, provided that this entire notice
** is included in all copies. No permission is granted for, and users are
** prohibited from, modifying or making derivative works of the mapping
** specification.
**
*******************************************************************************/

package org.saforum.ais;

import java.nio.channels.SelectableChannel;


/**
 * This abstract class provides the implementation of the helper methods dispatchBlocking(timeout)
 * and hasPendingCallback().
 *
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 */
public abstract class AbstractHandle implements Handle {

    /**
     */
    public final boolean hasPendingCallback()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException{
        return hasPendingCallback( 0 );
    }

    /**
     */
    public abstract boolean hasPendingCallback( long timeout )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;
            

    /**
     */
    public abstract SelectableChannel getSelectableChannel()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;
            

    /**
     */
    public abstract void dispatch( DispatchFlags dispatchFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException;


    /**
     */
    public abstract void dispatchBlocking()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     */
    public final void dispatchBlocking( long timeout )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException{
        if ( hasPendingCallback( timeout ) ) {
            dispatch( DispatchFlags.DISPATCH_ONE );
        }
    }

    /**
     */
    public abstract void finalizeHandle()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException;

}

/*  */
