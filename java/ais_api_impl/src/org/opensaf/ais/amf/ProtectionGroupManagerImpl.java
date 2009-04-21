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
import org.saforum.ais.AisInitException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisNotExistException;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.TrackFlags;
import org.saforum.ais.amf.AmfHandle;
import org.saforum.ais.amf.ProtectionGroupManager;
import org.saforum.ais.amf.ProtectionGroupNotification;

public class ProtectionGroupManagerImpl implements ProtectionGroupManager {

	/**
     *
     */
    private AmfHandle amfLibraryHandle;

    /**
     * TODO
     *
     * @param amfLibraryHandle TODO
     */
    ProtectionGroupManagerImpl( AmfHandle amfLibraryHandle ) {
        this.amfLibraryHandle = amfLibraryHandle;
    }

	public native ProtectionGroupNotification[] getProtectionGroup(String csiName)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisNoMemoryException,
			AisNoResourcesException, AisNotExistException;

	public native void getProtectionGroupAsync(String csiName)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisInitException,
			AisNoMemoryException, AisNoResourcesException,
			AisNotExistException;

	public native void startProtectionGroupTracking(
			String csiName, TrackFlags trackFlags) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisNotExistException;

	public native void stopProtectionGroupTracking(String csiName)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisNoMemoryException,
			AisNoResourcesException, AisNotExistException;

	public void getProtectionGroupAsyncThenStartTracking(String csiName,
			TrackFlags trackFlags) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisNotExistException {
		getProtectionGroupAsync(csiName);
		startProtectionGroupTracking(csiName, trackFlags);
	}

	public native ProtectionGroupNotification[] getProtectionGroupThenStartTracking(
			String csiName, TrackFlags trackFlags) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisNotExistException;

}
