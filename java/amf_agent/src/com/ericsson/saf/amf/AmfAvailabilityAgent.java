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

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.concurrent.ExecutorService;

import javax.availability.management.ActivationReason;
import javax.availability.management.AvailabilityAgent;
import javax.availability.management.AvailabilityContainerController;
import javax.availability.management.AvailabilityException;
import javax.availability.management.DeactivationReason;
import javax.availability.management.HealthState;

import org.saforum.ais.AisStatus;
import org.saforum.ais.Version;

import org.saforum.ais.amf.AmfHandle;
import org.saforum.ais.amf.AmfHandleFactory;
import org.saforum.ais.amf.CsiAttribute;
import org.saforum.ais.amf.CsiDescriptor;
import org.saforum.ais.amf.HaState;
import org.saforum.ais.amf.Healthcheck;
import org.saforum.ais.amf.HealthcheckCallback;
import org.saforum.ais.amf.RecommendedRecovery;
import org.saforum.ais.amf.RemoveCsiCallback;
import org.saforum.ais.amf.SetCsiCallback;
import org.saforum.ais.amf.TerminateComponentCallback;

/**
 * This class implements an AM4J agent. It acts as interface between JSR 319 and
 * AMF. It's initialized by the AE and deals with setup with AMF's callbacks
 * mechanism.
 *
 * This agent does not support contained units, as OpenSAF's AMF does not support
 * container-contained yet.
 *
 */
public class AmfAvailabilityAgent implements AvailabilityAgent, SetCsiCallback,
        RemoveCsiCallback, TerminateComponentCallback, HealthcheckCallback {

    private final String HEALTHCHECK_KEY_PROPERTY_NAME = "AMF_AGENT_HEALTHCHECK_KEY";
    private final String DEFAULT_HEALTHCHECK_KEY = "0202";
    private final char RELEASE_VERSION = 'B';
    private final short MAJOR_VERSION = (short) 1;
    private final short MINOR_VERSION = (short) 1;

    private Logger logger = Logger.getLogger(AmfAvailabilityAgent.class.getName());
    private AmfHandle.Callbacks amfCallbacks;
    private AvailabilityContainerController container;
    private boolean serverHealthcheckEnabled;
    private final Object terminateAgentLock;
    private AmfHandle amfHandle;
    private AmfHandleFactory amfFactory;
    private AmfDispatcher dispatcher;
    private byte[] healthcheckKey;
    private String componentName;

    /**
     * This class handles dispatching towards AMF
     */
    class AmfDispatcher implements Runnable {

        private boolean terminateCallbackCalled = false;
        private final AmfHandle amfHandle;

        public AmfDispatcher(AmfHandle amfHandle) {
            this.amfHandle = amfHandle;
        }

        public void stopDispatching() {
            this.terminateCallbackCalled = true;
        }

        /**
         * Run the dispatch thread.
         */
        @Override
        public void run() {
            logger.log(Level.INFO, "Running dispatcher");
            if (amfHandle == null) {
                throw new RuntimeException("Amf Handle is null!");
            }
            while (!terminateCallbackCalled) {
                try {
                    logger.log(Level.FINE, "Invoking blocking dispatch");
                    amfHandle.dispatchBlocking();
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }

            logger.log(Level.FINE, "Dispatch terminated");
        }
    }

    /**
     * Constructor for the AmfAvailabilityAgent. It initializes member fields,
     * sets the healthcheck key, and initializes the callbacks.
     *
     */
    public AmfAvailabilityAgent() {
        amfCallbacks = new AmfHandle.Callbacks();

        serverHealthcheckEnabled = false;

        terminateAgentLock = new Object();

        healthcheckKey = (System.getProperty(HEALTHCHECK_KEY_PROPERTY_NAME, DEFAULT_HEALTHCHECK_KEY)).getBytes();

        amfCallbacks.setCsiCallback = this;
        amfCallbacks.removeCsiCallback = this;
        amfCallbacks.terminateComponentCallback = this;
        amfCallbacks.healthcheckCallback = this;

        logger.log(Level.INFO, "AmfAvailabilityAgent created");
    }

    /**
     *
     * This method is invoked by the  Availability Executor (running in the application server)
     *
     * @param container a reference to the container to use
     * @param executor an executor for task execution.
     * @param properties to be used by the agent.
     */
    public void init(AvailabilityContainerController container,
            ExecutorService executor,
            Properties properties) throws AvailabilityException {
        this.init(container, executor);
    }

    /**
     *
     * This method is invoked by the  Availability Executor (running in the application server)
     *
     * @param container a reference to the container to use
     * @param executor an executor for task execution.
     */
    public void init(AvailabilityContainerController container, ExecutorService executor)
            throws AvailabilityException {
        logger.log(Level.INFO, "Initializing the AMF agent");

        this.container = container;

        try {
            Version amfVersion = new Version(RELEASE_VERSION, MAJOR_VERSION, MINOR_VERSION);

            amfFactory = new AmfHandleFactory();
            amfHandle = amfFactory.initializeHandle(amfCallbacks, amfVersion);

            componentName = amfHandle.getComponentRegistry().getComponentName();

            // initialize the container
            container.init(new AmfAvailabilityAgentService(this, componentName));

            // register the component
            amfHandle.getComponentRegistry().registerComponent();

            // dispatch the callbacks
            dispatcher = new AmfDispatcher(amfHandle);
            executor.execute(dispatcher);

            // start the health checking
            amfHandle.getHealthcheck().startHealthcheck(
                    componentName,
                    healthcheckKey,
                    Healthcheck.HealthcheckInvocation.AMF_INVOKED,
                    RecommendedRecovery.NO_RECOMMENDATION);
        } catch (Exception e) {
            logger.log(Level.SEVERE, "Failed to initialize the agent", e);
            throw new RuntimeException(e);
        }
    }

    /**
     *
     * This method is invoked to terminate the availability agent
     *
     */
    public void terminate() throws AvailabilityException {
        logger.log(Level.INFO, "Terminating the agent");

        try {

            // wait with termination until the OK response has been sent to AMF
            synchronized (terminateAgentLock) {
                terminateAgentLock.wait();
            }

            dispatcher.stopDispatching();
            amfHandle.getComponentRegistry().unregisterComponent();
            amfHandle.finalizeHandle();
            logger.log(Level.FINE, "Terminated the agent");
        } catch (Exception e) {
            logger.log(Level.WARNING, "Failed to terminate the agent", e);
            throw new AvailabilityException(e);
        }
    }

    /**
     * Reference to the HealthcheckCallback object that
     * the Availability Management Framework may invoke on the component.
     * 
     * @since AMF-B.01.01
     * @param invocation identifier
     * @param componentName
     * @param healthcheckKey
     */
    public void healthcheckCallback(long invocation, String componentName,
            byte[] healthcheckKey) {
        logger.log(Level.INFO, "Health checking {0}", componentName);

        try {
            HealthState state = HealthState.HEALTHY;

            // inquire on the app server health, if it's enabled
            if (serverHealthcheckEnabled) {
                state = container.checkHealth();
                logger.log(Level.INFO,
                        "Health of component {0} is {1}", new Object[]{componentName, state});
            }

            // forward response to AMF
            switch (state) {
                case HEALTHY: // intentional fall-through
                case ERROR_TRYING_TO_RECOVER:
                    respond(invocation, AisStatus.OK);

                    break;

                case ERROR_CANNOT_RECOVER:
                    respond(invocation, AisStatus.ERR_FAILED_OPERATION);

                    break;
            } // switch
        } // try
        catch (AvailabilityException e) {
            logger.log(Level.SEVERE, "Failed to perform the health check", e);
            respond(invocation, AisStatus.ERR_FAILED_OPERATION);

        } // catch
    }

    /**
     * Reference to the TerminateComponentCallback object that
     * the Availability Management Framework may invoke on the component.
     *
     * @since AMF-B.01.01
     * @param invocation identifier
     * @param componentName
     */
    public void terminateComponentCallback(long invocation, String componentName) {
        logger.log(Level.INFO, "Terminating {0}", componentName);

        try {
            // inform the app server to initiate termination process.
            container.terminate();
            // forward response to AMF that all is fine
            respond(invocation, AisStatus.OK);

            logger.log(Level.INFO, "Releasing the termination lock");
            synchronized (terminateAgentLock) {
                terminateAgentLock.notify();
            }
        } // try
        catch (AvailabilityException e) {
            logger.log(Level.SEVERE, "Termination failed", e);
            respond(invocation, AisStatus.ERR_FAILED_OPERATION);
        } // catch
    }

    /**
     * Reference to the SetCsiCallback object that
     * the Availability Management Framework may invoke on the component.
     *
     * @since AMF-B.01.01
     * @param invocation identifier
     * @param componentName
     * @param haState The new HA state to be assumed by the component for the component
     *                          service instance
     * @param csiDescriptor descriptor with information about the component service
     *                          instances targeted by this callback invocation
     */
    public void setCsiCallback(long invocation, String componentName,
            HaState haState, CsiDescriptor csiDescriptor) {

        logger.log(Level.INFO, "Setting CSI for {0} to {1}",
                new Object[] {componentName, haState});

        try {
            switch (haState) {
                case ACTIVE:
                    logger.log(Level.FINEST, "CsiDescriptor flags: ", csiDescriptor.csiFlags);

                    Map<String, String> attr = new HashMap<String, String>();

                    // collect the activation attributes
                    for (CsiAttribute csiAttribute : csiDescriptor.csiAttribute) {
                        attr.put(csiAttribute.name, csiAttribute.value);
                        logger.log(Level.INFO, "Attribute: {0} = {1}",
                                new Object[]{csiAttribute.name, csiAttribute.value});
                    }

                    container.activate(ActivationReason.START_UP, attr);

                    // forward response to AMF
                    logger.log(Level.INFO, "Activated OK. Responding to AMF.");
                    respond(invocation, AisStatus.OK);

                    break;

                case STANDBY: // intentional fall-through
                case QUIESCED: 
                    container.deactivate(DeactivationReason.SWITCH_OVER);

                    // forward response to AMF
                    respond(invocation, AisStatus.OK);

                    break;

                case QUIESCING:
                    container.deactivate(DeactivationReason.SWITCH_OVER);

                    // forward response to AMF
                    respond(invocation, AisStatus.OK);

                    // tell AMF that quiescing is complete
                    amfHandle.getCsiManager().completedCsiQuiescing(invocation, AisStatus.OK.getValue());

                    break;
            } // switch
        } // try
        catch (Exception e) {
            logger.log(Level.SEVERE, "AmfAvailabilityAgent activate failed", e);

            respond(invocation, AisStatus.ERR_FAILED_OPERATION);
        } // catch
    }

    /**
     * Reference to the RemoveCsiCallback object that
     * the Availability Management Framework may invoke on the component.
     *
     * @since AMF-B.01.01
     * @param invocation identifier
     * @param componentName
     * @param csiName
     * @param csiFlags
     */
    public void removeCsiCallback(long invocation, String componentName,
            String csiName, CsiDescriptor.CsiFlags csiFlags) {
        logger.log(Level.INFO, "Removing CSI for {0}", componentName);

        try {
            container.deactivate(DeactivationReason.SHUT_DOWN);
            // forward response to AMF
            respond(invocation, AisStatus.OK);

        } // try
        catch (AvailabilityException e) {
            logger.log(Level.SEVERE, "Failed to deactivate the container", e);
            respond(invocation, AisStatus.ERR_FAILED_OPERATION);
        } // catch
    }

    /**
     * Reports an error from the container
     * 
     * @param state
     */
    protected void reportContainerError(HealthState state) {
        if (state != HealthState.HEALTHY) {
            try {
                logger.log(Level.INFO, "Reporting error for: {0}", componentName);
                amfHandle.getErrorReporting().reportComponentError(componentName,
                        0, // detection time:
                        // 0 means it is assumed that the time at which the
                        // library received the error is the error detection time
                        RecommendedRecovery.NO_RECOMMENDATION, 0); // SA_NTF_IDENTIFIER_UNUSED
            } catch (Exception e) {
                logger.log(Level.SEVERE, "Failed to send error report", e);
                throw new RuntimeException(e);
            }
        }
    }

    /**
     * Enables health checks for the container
     */
    protected void enableContainerHealthchecks() {
        serverHealthcheckEnabled = true;
    }

    /**
     * Disables health checks for the container
     */
    protected void disableContainerHealthchecks() {
        serverHealthcheckEnabled = false;
    }

    /**
     * helper method to respond to AMF
     *
     * @param invocation identifier
     * @param status the status to send back to AMF
     */
    private void respond(long invocation, AisStatus status) {
        logger.log(Level.INFO, "Responding {0} to AMF.", status);

        try {
            amfHandle.response(invocation, status);
        } catch (Exception e) {
            logger.log(Level.SEVERE, "Failed to respond to AMF", e);
            throw new RuntimeException(e);
        }
    }
}
