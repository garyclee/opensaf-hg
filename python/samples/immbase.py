############################################################################
#
# (C) Copyright 2011 The OpenSAF Foundation
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
# Author(s): Wind River Systems, Inc.
#
############################################################################

from pyosaf.saImmOm import *
from pyosaf.saAmf import *

IMM_VERSION = SaVersionT('A', 2, 1)

class SafException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return eSaAisErrorT.whatis(self.value)

class SafObject(object):
	resolver = {
		# SaAmfComp
			'saAmfCompPresenceState': eSaAmfPresenceStateT,
			'saAmfCompReadinessState': eSaAmfReadinessStateT,
			'saAmfCompOperState': eSaAmfOperationalStateT,
		# SaAmfNode
			'saAmfNodeOperState': eSaAmfOperationalStateT,
			'saAmfNodeAdminState': eSaAmfAdminStateT,
		# SaAmfCSIAssignment
			'saAmfCSICompHAReadinessState': eSaAmfReadinessStateT,
			'saAmfCSICompHAState': eSaAmfHAStateT,
		# SaAmfSIAssignment
			'saAmfSISUHAReadinessState': eSaAmfReadinessStateT,
			'saAmfSISUHAState': eSaAmfHAStateT,
		# SaAmfApplication
			'saAmfApplicationAdminState': eSaAmfAdminStateT,
		# SaClmNode
			'saClmNodeAdminState': eSaAmfAdminStateT,
		# SaAmfSU
			'saAmfSUPresenceState': eSaAmfPresenceStateT,
			'saAmfSUReadinessState': eSaAmfReadinessStateT,
			'saAmfSUAdminState': eSaAmfAdminStateT,
			'saAmfSUOperState': eSaAmfOperationalStateT,
		# SaAmfSG
			'saAmfSGAdminState': eSaAmfAdminStateT,
		# SaAmfSI
			'saAmfSIAdminState': eSaAmfAdminStateT,
			'saAmfSIAssignmentState': eSaAmfAssignmentStateT,
		# SaAmfCluster
			'saAmfClusterAdminState': eSaAmfAdminStateT,
		# TODO: Once we create saSmf.py
		# SaSmfProcedure
		# SaSmfStep
		# SaSmfCampaign
	}

	@staticmethod
	def resolveStates(attribs):
		for (attr, vals) in attribs.iteritems():
			enum = SafObject.resolver.get(attr)
			if enum:
				vals[1] = [enum.whatis(val) for val in vals[1]]

	@staticmethod
	def serialize(entry):
		return entry.attribs

	def __init__(self, dn, attribs, numeric=False):
		self.dn = dn
		self.attribs = {}
		attrList = unmarshalNullArray(attribs)
		for attr in attrList:
			attrRange = range(attr.attrValuesNumber)
			self.attribs[attr.attrName] = [
				eSaImmValueTypeT.whatis(attr.attrValueType),
				[unmarshalSaImmValue(
					attr.attrValues[val],
					attr.attrValueType) for val in attrRange]
				]
		if not numeric:
			SafObject.resolveStates(self.attribs)
