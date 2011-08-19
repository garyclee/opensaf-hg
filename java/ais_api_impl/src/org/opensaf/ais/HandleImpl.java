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
 * Author(s): Nokia Siemens Networks, OptXware Research & Development LLC.
 */

package org.opensaf.ais;

import java.io.IOException;
import java.nio.channels.SelectableChannel;
import java.util.ArrayList;
import java.util.List;

import org.saforum.ais.AisBadHandleException;
import org.saforum.ais.AisInvalidParamException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.AisUnavailableException;
import org.saforum.ais.DispatchFlags;
import org.saforum.ais.Handle;

public abstract class HandleImpl implements Handle {

	/**
	 * The native operating system handle, returned by the sa<Service>SelectionObjectGet
	 * function of the native AIS implementation, associated with the this
	 * library instance. The invoking process can use this handle to detect
	 * pending callbacks, instead of repeatedly invoking dispatch() for this
	 * purpose. In a POSIX environment, the operating system handle is a file
	 * descriptor that is used with the poll() or select() system calls to
	 * detect pending callbacks.
	 */
	protected long selectionObject = -1;

	private SelectionObjectMediator mediator;

	/**
	 * A flag indicating whether the selectionObject field contains a valid
	 * value.
	 */
	private boolean selectionObjectObtained = false;

	/**
	 * Selects a set of LibraryHandles with pending callbacks: this method
	 * performs a blocking selection operation. It returns only after at least
	 * one LibraryHandle is selected (i.e. there is a pending callback for that
	 * LibraryHandle).
	 *
	 * <P>
	 * Different threads of a process can invoke this method. As a consequence,
	 * a single pending callback may be detected by several threads. It is up to
	 * the application to provide concurrency control (for instance,
	 * synchronizing on the library handle object), if needed.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references that are checked for
	 *            pending callbacks.
	 * @return - an array of LibraryHhandle references that have pending
	 *         callbacks.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 * @throws AisNoResourcesException
	 * @throws AisBadHandleException
	 * @throws AisTryAgainException
	 * @throws AisTimeoutException
	 */
	public static Handle[] select(Handle[] handleArray)
			throws AisLibraryException, AisInvalidParamException,
			AisNoMemoryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException, AisNoResourcesException{
		long[] _selectionObjectArray = s_getSelectionObjectArray(handleArray);
		s_invokeSelect(_selectionObjectArray);
		return s_getHandleArray(handleArray, _selectionObjectArray);
	}

	/**
	 * Selects a set of LibraryHandles with pending callbacks: this method
	 * performs a blocking selection operation with a timout. It returns after
	 * at least one LibraryHandle is selected (i.e. there is a pending callback
	 * for that LibraryHandle), or when the timout period expires.
	 *
	 * <P>
	 * Different threads of a process can invoke this method. As a consequence,
	 * a single pending callback may be detected by several threads. It is up to
	 * the application to provide concurrency control (for instance,
	 * synchronizing on the library handle object), if needed.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references that are checked for
	 *            pending callbacks.
	 * @param timeout
	 *            The maximum time duration for which this method blocks if
	 *            there is no pending callback.
	 * @return - an array of LibraryHhandle references that have pending
	 *         callbacks. Can be null, if there is no pending callback.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 * @throws AisNoResourcesException
	 * @throws AisBadHandleException
	 * @throws AisTryAgainException
	 * @throws AisTimeoutException
	 */
	public static Handle[] select(Handle[] handleArray, long timeout)
			throws AisLibraryException, AisInvalidParamException,
			AisNoMemoryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException, AisNoResourcesException {
		long[] _selectionObjectArray = s_getSelectionObjectArray(handleArray);
		if (s_invokeSelect(_selectionObjectArray, timeout) > 0) {
			return s_getHandleArray(handleArray, _selectionObjectArray);
		} else {
			return null;
		}
	}

	/**
	 * Selects a set of LibraryHandles with pending callbacks: this method
	 * performs a non-blocking selection operation. It returns immediately with
	 * the set of LibraryHandles that already have a pending callback. only
	 * after at least one LibraryHandle is selected (i.e. there is a pending
	 * callback for that LibraryHandle).
	 *
	 * <P>
	 * Different threads of a process can invoke this method. As a consequence,
	 * a single pending callback may be detected by several threads. It is up to
	 * the application to provide concurrency control (for instance,
	 * synchronizing on the library handle object), if needed.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references that are checked for
	 *            pending callbacks.
	 * @return - an array of LibraryHhandle references that have pending
	 *         callbacks. Can be null, if there is no pending callback.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 * @throws AisNoResourcesException
	 * @throws AisBadHandleException
	 * @throws AisTryAgainException
	 * @throws AisTimeoutException
	 */
	public static Handle[] selectNow(Handle[] handleArray)
			throws AisLibraryException, AisInvalidParamException,
			AisNoMemoryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException, AisNoResourcesException {
		return select(handleArray, 0);
	}

	/**
	 * TBD.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references.
	 * @return an array of selection objects corresponding to the passed
	 *         LibraryHhandle references..
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 * @throws AisNoResourcesException
	 * @throws AisBadHandleException
	 * @throws AisTryAgainException
	 * @throws AisTimeoutException
	 * @throws AisNoMemoryException
	 * @throws AisLibraryException
	 */
	private static long[] s_getSelectionObjectArray(Handle[] handleArray)
			throws AisLibraryException, AisInvalidParamException,
			AisNoMemoryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException, AisNoResourcesException {
		if (handleArray == null) {
			throw new AisInvalidParamException(
					"A parameter is not set correctly."); // EXIT POINT!!!
		}
		int _len = handleArray.length;
		if (_len == 0) {
			throw new AisInvalidParamException(
					"A parameter is not set correctly."); // EXIT POINT!!!
		}
		long[] _selectionObjectArray = new long[_len];
		for (int _idx = 0; _idx < _len; _idx++) {
			HandleImpl _handle = (HandleImpl) handleArray[_idx];
			if (_handle != null) {
				_handle.ensureSelectionObjectObtained();
				_selectionObjectArray[_idx] = _handle.selectionObject;
			} else {
				throw new AisInvalidParamException(
						"A parameter is not set correctly."); // EXIT POINT!!!
			}
		}
		return _selectionObjectArray;
	}

	/**
	 * TBD.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references. that have been checked
	 *            for pending callbacks.
	 * @param selectionObjectArray -
	 *            an array of selection objects corresponding to the passed
	 *            LibraryHhandle references. However, the array elements that
	 *            represent unselected selection objects (i.e. there are no
	 *            pending callbacks on that object) have been set to null.
	 * @return an array of LibraryHhandle references that have been selected
	 *         (i.e. correspond to the non-null elements of the passed
	 *         selectionObjectArray).
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 * @throws AisNoResourcesException
	 * @throws AisBadHandleException
	 * @throws AisTryAgainException
	 * @throws AisTimeoutException
	 * @throws AisNoMemoryException
	 * @throws AisLibraryException
	 */
	private static Handle[] s_getHandleArray(Handle[] handleArray,
			long[] selectionObjectArray) throws AisLibraryException,
			AisNoMemoryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException {
		int _len = handleArray.length;
		List<Handle> _selectedLibraryHandleList = new ArrayList<Handle>(_len);
		for (int _idx = 0; _idx < _len; _idx++) {
			if (selectionObjectArray[_idx] != 0xFFFFFFFFFFFFFFFFL) {
				_selectedLibraryHandleList.add(handleArray[_idx]);
			}
		}
		return _selectedLibraryHandleList.toArray(new Handle[0]);
	}

	/**
	 * Selects a set of LibraryHandles with pending callbacks.
	 *
	 * This method performs a blocking selection operation. It returns only
	 * after at least one LibraryHandle is selected (i.e. there is a pending
	 * callback for that LibraryHandle).
	 *
	 * <P>
	 * Different threads of a process can invoke this method. As a consequence,
	 * a single pending callback may be detected by several threads. It is up to
	 * the application to provide concurrency control (for instance,
	 * synchronizing on the library handle object), if needed.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references that are checked for
	 *            pending callbacks. Upon return, it will contain only those
	 *            references that have a pending callback. The ones that have
	 *            not are removed (i.e. the corresponding array element is set
	 *            to null by this method.)
	 * @return The number of LibraryHandles with a pending callback.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 */
	private static native void s_invokeSelect(long[] selectionObjectArray)
			throws AisLibraryException, AisNoMemoryException;

	/**
	 * Selects a set of LibraryHandles with pending callbacks.
	 *
	 * This method performs a blocking selection operation. It returns after at
	 * least one LibraryHandle is selected (i.e. there is a pending callback for
	 * that LibraryHandle), or when the timout period expires.
	 *
	 * <P>
	 * Different threads of a process can invoke this method. As a consequence,
	 * a single pending callback may be detected by several threads. It is up to
	 * the application to provide concurrency control (for instance,
	 * synchronizing on the library handle object), if needed.
	 *
	 * @param handleArray -
	 *            an array of LibraryHhandle references that are checked for
	 *            pending callbacks. Upon return, it will contain only those
	 *            references that have a pending callback. The ones that have
	 *            not are removed (i.e. the corresponding array element is set
	 *            to null by this method.)
	 * @param timeout
	 *            The maximum time duration for which this method blocks if
	 *            there is no pending callback.
	 * @return The number of LibraryHandles with a pending callback. Can be
	 *         zero, if the timeout period expires.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 */
	private native static int s_invokeSelect(long[] selectionObjectArray,
			long timeout) throws AisLibraryException, AisNoMemoryException;

	public HandleImpl() {
		mediator = SelectionObjectMediator.getInstance();
	}

	public SelectableChannel getSelectableChannel() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		SelectableChannel channel = null;
		ensureSelectionObjectObtained();
		try {
			mediator.addHandle(selectionObject);
			channel = mediator.getAssociatedChannel(selectionObject);
		} catch (IOException e) {
			throw new AisLibraryException(e);
		}

		return channel;
	}

	public void dispatch(DispatchFlags dispatchFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException {
		try {
			mediator.pendingCallbacksDispatched(selectionObject);
		} catch (IOException e) {
			throw new AisLibraryException(e);
		}
	}

	public void dispatchBlocking() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		try {
			mediator.pendingCallbacksDispatched(selectionObject);
		} catch (IOException e) {
			throw new AisLibraryException(e);
		}
	}

	public void dispatchBlocking(long timeout) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		if (hasPendingCallback(timeout)) {
			dispatch(DispatchFlags.DISPATCH_ONE);
		}
	}

	public boolean hasPendingCallback() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException{
		return hasPendingCallback(0);
	}

	public boolean hasPendingCallback(long timeout) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		// make sure that we have a valid selection object
		ensureSelectionObjectObtained();
		// check the selection object
		return checkSelectionObject(timeout);
	}

	public void finalizeHandle() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException {
		try {
			mediator.removeHandle(selectionObject);
		} catch (IOException e) {
			throw new AisLibraryException(e);
		}
	}

	/**
	 * TODO
	 *
	 * @param timeout
	 *            TODO
	 * @return TODO
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisNoMemoryException
	 *             Either the AIS library or the provider of the service is out
	 *             of memory and cannot provide the service.
	 */
	public native boolean checkSelectionObject(long timeout)
			throws AisLibraryException, AisNoMemoryException;

	/**
	 * TODO
	 *
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
	 *             Either the Availability Management Framework library or the
	 *             provider of the service is out of memory and cannot provide
	 *             the service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 */
	// private void ensureSelectionObjectObtained() TODO check this
	protected void ensureSelectionObjectObtained() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			// AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException {
		// TODO consider using a separate lock object for selection object
		synchronized (this) {
			if (!selectionObjectObtained) {
				invokeSelectionObjectGet();
				selectionObjectObtained = true;
			}
		}
	}

	/**
	 * TODO
	 *
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
	 *             Either the Availability Management Framework library or the
	 *             provider of the service is out of memory and cannot provide
	 *             the service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 */
	abstract protected void invokeSelectionObjectGet()
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			// AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException;

}
