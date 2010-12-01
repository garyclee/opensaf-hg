/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2010 The OpenSAF Foundation
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
 * Author(s): Ericsson AB.
 */
package com.ericsson.saf.amf;

import java.util.logging.Level;
import java.util.logging.Logger;
import javax.availability.management.AvailabilityAgentService;
import javax.availability.management.HealthState;

/**
 * Saf Availability Service. This is the interface used by the container to
 * communicate with agent.
 * 
 */
class AmfAvailabilityAgentService implements AvailabilityAgentService {

    private Logger logger = Logger.getLogger(AmfAvailabilityAgentService.class.getName());
    private AmfAvailabilityAgent agent;
    private String componentName;

    /**
     * Constructor
     * 
     * @param none
     */     
    AmfAvailabilityAgentService(AmfAvailabilityAgent agent, String componentName) {
        this.agent = agent;
        this.componentName = componentName;

        logger.log(Level.INFO, "Created AmfAvailabilityAgentService");
    }

    /**
     * Method invoked by the AE when an error condition is reported from the app
     * server.
     * 
     * @param HealthState state
     */  
    public void reportError(HealthState state) {
        logger.log(Level.INFO, "Reporting error with health state {0}", state);

        agent.reportContainerError(state);
    }

    /**
     * Enables health checks by the availability unit/container
     */
    public void enableHealthchecks() {
        logger.log(Level.INFO, "Enabling healthchecks for: ", componentName);

        agent.enableContainerHealthchecks();
    }

    /**
     * Disables health checks by the availability unit/container
     */
    public void disableHealthchecks() {
        logger.log(Level.INFO, "Disabling healthchecks for: {0}", componentName);

        agent.disableContainerHealthchecks();
    }
}
