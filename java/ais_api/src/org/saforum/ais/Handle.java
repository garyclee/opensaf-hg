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
 * The Handle interface represents a reference to the association between
 * an AIS client and an AIS service. The client code uses this
 * handle in subsequent communication with the AIS service. AIS libraries must
 * support several Handle instances from the same binary
 * program (e.g., process in the POSIX.1 world).
 *
 * <P><B>SAF Reference:</B> <code>Sa&lt;Area&gt;HandleT</code>
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 */
public interface Handle {

    /**
     * This method returns true if there is a currently pending callback for
     * this library handle.
     * <P>
     * Different threads of a process can invoke this method. As a consequence,
     * a single pending callback may be detected by several threads. It is up to
     * the application to provide concurrency control (for instance,
     * synchronizing on the library handle object), if needed.
     * <P>
     * Please note, that this method will internally use a platform specific
     * selection object for detecting pending callbacks. The selection object may be
     * cached internally by this Handle object. If the selection
     * object is not yet available (e.g. when this method is called for the
     * first time or it is not cached),
     * this method will try to obtain the selection object.
     * <P><B>SAF Reference:</B> <code>sa&lt;Area&gt;SelectionObjectGet()</code>
     *
     * @return true if there is a pending callback for this library handle.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01)
     *             The operation requested in this call is
     *             unavailable on this cluster node due to one of the two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership, but
     *             the associated AmfHandle object was acquired before the cluster
     *             node left the cluster membership.
     *             </UL>
     */
    public boolean hasPendingCallback()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method returns true if a pending callback occurs for this library
     * handle within the specified timeout.
     * <P>
     * Different threads of a process can invoke this method. As a consequence,
     * a single pending callback may be detected by several threads. It is up to
     * the application to provide concurrency control (for instance,
     * synchronizing on the library handle object), if needed.
     * <P>
     * Please note, that this method will internally use a platform specific
     * selection object for detecting pending callbacks. The selection object
     * may be cached internally by this Handle object. If the selection object
     * is not yet available (e.g. when this method is called for the first time
     * or it is not cached), this method will try to obtain the selection
     * object.
     * <P>
     * <B>SAF Reference:</B> <code>sa&lt;Area&gt;SelectionObjectGet()</code>
     *
     * @param timeout The maximum time duration (specified in nanoseconds) for
     *            which this method blocks if there is no pending callback.
     *            Commonly used time duration constants defined in
     *            {@link org.saforum.ais.Consts} can be used here.
     * @return true if there is a pending callback for this library handle.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the service library or the provider
     *             of the service is out of memory and cannot provide the
     *             service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01) The
     *             operation requested in this call is unavailable on this
     *             cluster node due to one of the two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership,
     *             but the associated AmfHandle object was acquired before the
     *             cluster node left the cluster membership.
     *             </UL>
     */
    public boolean hasPendingCallback( long timeout )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method returns a SelectableChannel object that indicates pending
     * callbacks on this associated Handle. (SelectableChannel objects can be
     * used by a NIO Selector to perform event-driven multiplexed input on
     * multiple I/O channels.) Please note that the returned SelectableChannel
     * instance has a one-to-one association with the area handle: it is cached
     * and returned by subsequent calls to this method.
     * <P>
     * It is foreseen that hidden threads (i.e. threads created and controlled
     * by the implementation, not by the client) may be needed for supporting the
     * implementation of this method. However, the implementation should try to
     * minimize the number of threads and should limit their existence as much
     * as possible. For example, per-application hidden threads should be
     * avoided. Also, hidden threads should be created dynamically, only when
     * the application first uses the API for which the threads are necessary.
     * <P>
     * Also note, that this method will internally use a platform specific
     * selection object for detecting pending callbacks. The selection object
     * may be cached internally by this Handle object. If the selection object
     * is not yet available (e.g. when this method is called for the first time
     * or it is not cached), this method will try to obtain the selection
     * object.
     * <P>
     * <B>SAF Reference:</B> <code>sa&lt;Area&gt;SelectionObjectGet()</code>
     *
     * @return a SelectableChannel object.
     * @throws AisLibraryException
     *             An unexpected problem occurred in the library (such as
     *             corruption). The library cannot be used anymore.
     * @throws AisTimeoutException
     *             An implementation-dependent timeout occurred before the call
     *             could complete. It is unspecified whether the call succeeded
     *             or whether it did not.
     * @throws AisTryAgainException
     *             The service cannot be provided at this time. The process may
     *             retry later.
     * @throws AisBadHandleException
     *             This library handle is invalid, since it is corrupted or has
     *             already been finalized.
     * @throws AisNoMemoryException
     *             Either the service library or the provider of the service is
     *             out of memory and cannot provide the service.
     * @throws AisNoResourcesException
     *             The system is out of required resources (other than memory).
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01)
     *             The operation requested in this
     *             call is unavailable on this cluster node due to one of the
     *             two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership,
     *             but the associated AmfHandle object was acquired before the
     *             cluster node left the cluster membership.
     *             </UL>
     */
    public SelectableChannel getSelectableChannel()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method invokes, in the context of the calling thread, pending
     * callbacks for the handle this Handle instance represents in a
     * way that is specified by the dispatchFlags parameter.
     * <P>
     * Different threads of a process can invoke this method. As a consequence,
     * several pending callbacks may be invoked concurrently. It is up to the
     * application to provide concurrency control (for instance, synchronizing
     * on the library handle object), if needed. If proper concurrency control
     * is not provided, then this method may not behave as expected: for
     * example, it may return without invoking a callback even if a previous
     * call to hasPendingCallback indicated a pending callback.
     *
     * <P><B>SAF Reference:</B> <code>sa&lt;Area&gt;Dispatch()</code>
     *
     * @param dispatchFlags - Flags that specify the callback execution behavior
     *            of the dispatch() method, as defined in DispatchFlags.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01)
     *             The operation requested in this call is
     *             unavailable on this cluster node due to one of the two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership, but
     *             the associated AmfHandle object was acquired before the cluster
     *             node left the cluster membership.
     *             </UL>
     * @see org.saforum.ais.DispatchFlags
     */
    public void dispatch( DispatchFlags dispatchFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException;


    /**
     * This method invokes, in the context of the calling thread, a single
     * pending callback for the handle this Handle instance
     * represents: if there is no pending callback when this method is called,
     * then it will block until one becomes available. The method returns after
     * excuting the callback.
     * <P>
     * Different threads of a process can invoke this method. As a consequence,
     * several pending callbacks may be invoked concurrently. It is up to the
     * application to provide concurrency control (for instance, synchronizing
     * on the library handle object), if needed. If proper concurrency control
     * is not provided, then this method may not behave as expected: for
     * example, it may return without invoking a callback even before the
     * timeout expires.
     * <P>
     * Please note, that this method will internally use a platform specific
     * selection object for detecting pending callbacks. The selection object is
     * cached internally by this AmfHandle object. If the selection
     * object is not yet available (e.g. when this method is called for the
     * first time), this method will try to obtain the selection object.
     *
     * <P><B>SAF Reference:</B> <code>sa&lt;Area&gt;Dispatch()</code>,
     *                   <code>sa&lt;Area&gt;SelectionObjectGet()</code>
     *
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01)
     *             The operation requested in this call is
     *             unavailable on this cluster node due to one of the two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership, but
     *             the associated AmfHandle object was acquired before the cluster
     *             node left the cluster membership.
     *             </UL>
     */
    public void dispatchBlocking()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method invokes, in the context of the calling thread, a single
     * pending callback for the handle this Handle instance represents: if there
     * is no pending callback when this method is called, then it will block
     * until one becomes available, but only for the specified timeout period.
     * The method returns after excuting the callback or after the specified
     * timeout.
     * <P>
     * Different threads of a process can invoke this method. As a consequence,
     * several pending callbacks may be invoked concurrently. It is up to the
     * application to provide concurrency control (for instance, synchronizing
     * on the library handle object), if needed. If proper concurrency control
     * is not provided, then this method may not behave as expected: for
     * example, it may return without invoking a callback even before the
     * timeout expires.
     * <P>
     * Please note, that this method will internally use a platform specific
     * selection object for detecting pending callbacks. The selection object
     * may be cached internally by this Handle object. If the selection object
     * is not yet available (e.g. when this method is called for the first time
     * or it is not cached), this method will try to obtain the selection
     * object.
     *
     * <P>
     * <B>SAF Reference:</B> <code>sa&lt;Area&gt;Dispatch()</code>,
     * <code>sa&lt;Area&gt;SelectionObjectGet()</code>
     *
     * @param timeout The maximum time duration (specified in nanoseconds) for
     *            which this method blocks if there is no pending callback.
     *            Commonly used time duration constants defined in
     *            {@link org.saforum.ais.Consts} can be used here.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the service library or the provider
     *             of the service is out of memory and cannot provide the
     *             service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01) The
     *             operation requested in this call is unavailable on this
     *             cluster node due to one of the two reasons:
     *             <UL>
     *             <LI>the cluster node has left the cluster membership;
     *             <LI>the cluster node has rejoined the cluster membership,
     *             but the associated AmfHandle object was acquired before the
     *             cluster node left the cluster membership.
     *             </UL>
     */
    public void dispatchBlocking( long timeout )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * The finalizeHandle() method closes the association represented by the Handle
     * parameter between the invoking process and the AIS Service. A process must
     * invoke this function once for each Handle it acquired by invoking the corresponding
     * factory.
     *
     * <P><B>SAF Reference:</B> <code>sa&lt;Area&gt;Finalize()</code>
     * (Note that sa&lt;Area&gt;Finalize() function is mapped to finalizeHandle() method, since finalize()
     * is a special <code>java.lang.Object</code> method.)
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     */
    public void finalizeHandle()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException;
}

/*  */
