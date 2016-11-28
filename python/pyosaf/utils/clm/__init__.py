############################################################################
#
# (C) Copyright 2015 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Ericsson
#
############################################################################
'''
    CLM common utilitites
'''

from pyosaf import saClm, saAis

from pyosaf.utils import decorate

# Decorate the raw saClm* functions with retry and raising exceptions
saClmInitialize                = decorate(saClm.saClmInitialize)
saClmInitialize_3              = decorate(saClm.saClmInitialize_3)
saClmInitialize_4              = decorate(saClm.saClmInitialize_4)
saClmSelectionObjectGet        = decorate(saClm.saClmSelectionObjectGet)
saClmDispatch                  = decorate(saClm.saClmDispatch)
saClmFinalize                  = decorate(saClm.saClmFinalize)
saClmClusterTrack              = decorate(saClm.saClmClusterTrack)
saClmClusterNodeGet            = decorate(saClm.saClmClusterNodeGet)
saClmClusterNotificationFree   = decorate(saClm.saClmClusterNotificationFree)
saClmClusterTrack_4            = decorate(saClm.saClmClusterTrack_4)
saClmClusterTrackStop          = decorate(saClm.saClmClusterTrackStop)
saClmClusterNotificationFree_4 = decorate(saClm.saClmClusterNotificationFree_4)
saClmClusterNodeGet_4          = decorate(saClm.saClmClusterNodeGet_4)
saClmClusterNodeGetAsync       = decorate(saClm.saClmClusterNodeGetAsync)
saClmResponse_4                = decorate(saClm.saClmResponse_4)

# Create the handle
HANDLE  = saClm.SaClmHandleT()

track_function = None

def track_callback(c_notification_buffer, c_number_of_members,
                   c_invocation_id, c_root_cause_entity,
                   c_correlation_ids, c_step,
                   c_time_supervision, c_error):

    if track_function:
        added   = []
        removed = []

        step = c_step

        if step == saClm.eSaClmChangeStepT.SA_CLM_CHANGE_COMPLETED:
            notification_buffer = c_notification_buffer.contents

            i = 0
            for notification in notification_buffer.notification:

                if i == notification_buffer.numberOfItems:
                    break
                else:
                    i = i + 1

                clm_cluster_node = notification.clusterNode
                cluster_node = cluster_node_instance_from_clm_cluster_node(clm_cluster_node)

                if notification.clusterChange == saClm.eSaClmClusterChangesT.SA_CLM_NODE_JOINED:
                    added.append(cluster_node)

                elif notification.clusterChange == saClm.eSaClmClusterChangesT.SA_CLM_NODE_LEFT:
                    removed.append(cluster_node)

        track_function(added, removed)

def node_get_callback(*args):
    pass

def cluster_node_instance_from_clm_cluster_node(clm_cluster_node):
    return ClusterNode(node_id=clm_cluster_node.nodeId,
                       node_address=clm_cluster_node.nodeAddress,
                       node_name=clm_cluster_node.nodeName,
                       execution_environment=clm_cluster_node.executionEnvironment,
                       member=clm_cluster_node.member,
                       boot_timestamp=clm_cluster_node.bootTimestamp,
                       initial_view_number=clm_cluster_node.initialViewNumber)


class ClusterNode:

    def __init__(self, node_id, node_address, node_name, execution_environment,
                 member,
                 boot_timestamp,
                 initial_view_number):

        self.node_id               = node_id
        self.node_address_value    = node_address.value
        self.node_address_family   = node_address.family
        self.node_name             = node_name.value
        self.execution_environment = execution_environment
        self.member                = member
        self.boot_timestamp        = boot_timestamp
        self.initial_view_number   = initial_view_number


def initialize(track_fn=None):

    global track_function

    track_function = track_fn

    # Set up callbacks for the member tracking
    callbacks = saClm.SaClmCallbacksT_4()

    callbacks.saClmClusterNodeGetCallback = saClm.SaClmClusterNodeGetCallbackT_4(node_get_callback)
    callbacks.saClmClusterTrackCallback = saClm.SaClmClusterTrackCallbackT_4(track_callback)

    # Define which version to use of the CLM API
    version = saAis.SaVersionT('B', 4, 1)

    # Initialize the CLM API
    saClmInitialize_4(HANDLE, callbacks, version)

def track(flags=saAis.saAis.SA_TRACK_CHANGES_ONLY):
    saClmClusterTrack_4(HANDLE, flags, None)

def get_members():
    notification_buffer = saClm.SaClmClusterNotificationBufferT_4()

    saClmClusterTrack_4(HANDLE, saAis.saAis.SA_TRACK_CURRENT, 
                        notification_buffer)

    cluster_nodes = []

    for i in range(notification_buffer.numberOfItems):
        notification = notification_buffer.notification[i]
        clusterNode  = notification.clusterNode

        node = cluster_node_instance_from_clm_cluster_node(clusterNode)

        cluster_nodes.append(node)

    return cluster_nodes

def dispatch(dispatchFlags):
    saClmDispatch(HANDLE, dispatchFlags)
