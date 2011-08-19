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

package org.opensaf.ais.clm;

import org.saforum.ais.AisBadHandleException;
import org.saforum.ais.AisInitException;
import org.saforum.ais.AisInvalidParamException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisNotExistException;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.AisVersionException;
import org.saforum.ais.AisUnavailableException;
import org.saforum.ais.TrackFlags;
import org.saforum.ais.clm.ClmHandle;
import org.saforum.ais.clm.ClusterMembershipManager;
import org.saforum.ais.clm.ClusterNode;
import org.saforum.ais.clm.ClusterNotificationBuffer;

public class ClusterMembershipManagerImpl implements ClusterMembershipManager {

	/**
     *
     */
    private ClmHandle clmLibraryHandle;

    /**
     * TODO
     *
     * @param clmLibraryHandle TODO
     */
    ClusterMembershipManagerImpl( ClmHandle clmLibraryHandle ) {
        // TODO Auto-generated constructor stub
        this.clmLibraryHandle = clmLibraryHandle;
    }

	public native ClusterNotificationBuffer getCluster() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException, AisVersionException, AisUnavailableException;
	
	public native ClusterNotificationBuffer getCluster( boolean local ) 
			throws AisLibraryException, AisTimeoutException, 
			AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException, 
			AisVersionException, AisUnavailableException;

	public native void getClusterAsync() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisVersionException, AisUnavailableException;

	public native void getClusterAsync( boolean local ) 
			throws AisLibraryException, AisTimeoutException, 
			AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, 
			AisNoResourcesException, AisVersionException, 
			AisUnavailableException;

	public native ClusterNode getClusterNode(int nodeId, long timeout)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			AisInvalidParamException, AisNoMemoryException,
			AisNoResourcesException,AisVersionException,
			AisUnavailableException;

	public native void stopClusterTracking() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException, AisNotExistException,
			AisUnavailableException;


	public native void getClusterAsyncThenStartTracking(TrackFlags trackFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisInitException,
			AisNoMemoryException, AisNoResourcesException, AisVersionException,
			AisUnavailableException;

	public native void getClusterAsyncThenStartTracking(TrackFlags trackFlags, boolean local, int trackStep )
            		throws AisLibraryException, AisTimeoutException, 
			AisTryAgainException, AisBadHandleException, 
			AisInitException, AisNoMemoryException, 
			AisNoResourcesException, AisVersionException, 
			AisUnavailableException;


	public native ClusterNotificationBuffer getClusterThenStartTracking(
			TrackFlags trackFlags) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisInitException, AisNoMemoryException, AisNoResourcesException,
			AisVersionException, AisUnavailableException;
	
	public native ClusterNotificationBuffer getClusterThenStartTracking(TrackFlags trackFlags, boolean local,
			int trackStep) throws AisLibraryException,AisTimeoutException, 
			AisTryAgainException, AisBadHandleException, AisInitException, 
			AisNoMemoryException, AisNoResourcesException, AisVersionException, 
			AisUnavailableException;

	public native void getClusterNodeAsync(long invocation, int nodeId)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			AisInvalidParamException, AisInitException,
			AisNoMemoryException, AisNoResourcesException,
		        AisUnavailableException, AisNotExistException;

	public native void startClusterTracking(TrackFlags trackFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException, AisInitException,
			AisNoMemoryException, AisNoResourcesException,AisVersionException,
			AisUnavailableException;;

	public native void startClusterTracking(TrackFlags trackFlags, boolean local, int trackStep  )
			throws AisLibraryException, AisTimeoutException, AisTryAgainException,
			AisBadHandleException, AisInitException, AisNoMemoryException, 
			AisNoResourcesException, AisVersionException, AisUnavailableException;
}
