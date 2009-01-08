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

package org.opensaf.ais.amf;

import org.saforum.ais.AisBadHandleException;
import org.saforum.ais.AisBadOperationException;
import org.saforum.ais.AisExistException;
import org.saforum.ais.AisInitException;
import org.saforum.ais.AisInvalidParamException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisNotExistException;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.amf.AmfHandle;
import org.saforum.ais.amf.ComponentRegistry;

public class ComponentRegistryImpl implements ComponentRegistry {

    /**
     *
     */
    private AmfHandle amfLibraryHandle;

    /**
     *
     */
    private String componentName;

    /**
     * TODO
     *
     * @param amfLibraryHandle TODO
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ComponentRegistry object is invalid, since it is corrupted or
     *             has already been finalized.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The Availability Management Framework is not
     *             aware of any component associated with the invoking process.
     */
    ComponentRegistryImpl( AmfHandle amfLibraryHandle )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            // AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException {
        this.amfLibraryHandle = amfLibraryHandle;
        componentName = invokeSaAmfComponentNameGet();
    }

	public String getComponentName() {
		return componentName;
	}

	public native void registerComponent() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisExistException;

	public native void registerProxiedComponent(String proxiedComponentName)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisInitException,
			AisInvalidParamException, AisNoMemoryException,
			AisNoResourcesException, AisNotExistException, AisExistException,
			AisBadOperationException;

	public native void unregisterComponent() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException,
			AisNotExistException, AisBadOperationException;

	public native void unregisterProxiedComponent(String proxiedComponentName)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			AisInvalidParamException, AisNoMemoryException,
			AisNoResourcesException, AisNotExistException,
			AisBadOperationException;

    /**
     * This method returns the name of the component the calling process belongs
     * to. This method is invoked only once, when this ComponentRegistry object
     * is constructed. The component name is then cached as a field of the
     * ComponentRegistry object and the cached value is used whenever the
     * component name is needed.
     *
     * @return The name of the component the process invoking this method
     *         belongs to.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ComponentRegistry object is invalid, since it is corrupted or
     *             has already been finalized.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The Availability Management Framework is not
     *             aware of any component associated with the invoking process.
     */
    private native String invokeSaAmfComponentNameGet()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            // AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

}
