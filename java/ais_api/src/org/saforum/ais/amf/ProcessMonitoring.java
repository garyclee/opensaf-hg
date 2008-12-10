/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-AMF-B.01.01
**
** DATE:
**   Wed Aug 6 2008
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

package org.saforum.ais.amf;

import org.saforum.ais.*;


/**
 * A ProcessMonitoring object contains API methods that enable components to
 * request passive monitoring of processes by the Availability Management
 * Framework.
 *
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface ProcessMonitoring {

    /**
     * This value requests the monitoring of processes exiting with a zero exit
     * status. Monitoring for several errors can be requested in a single call
     * by ORing different values.
     *
     */
    public static final int ZERO_EXIT = 1;

    /**
     * This value requests the monitoring of processes exiting with a non-zero
     * exit status. Monitoring for several errors can be requested in a single
     * call by ORing different values.
     */
    public static final int NON_ZERO_EXIT = 2;

    /**
     * Monitoring for several errors can be requested in a single call by
     * ORing different values.
     */
    public static final int ABNORMAL_END = 4;



    /**
     * This method requests the Availability Management Framework to start
     * passive monitoring of specific errors, which may occur to a process and
     * its descendents. Currently, only death of processes can be monitored. If
     * one of the errors being monitored occurs for the process or one of its
     * descendents, the Availability Management Framework will automatically
     * report an error on the component identified by componentName (see
     * reportComponentError for details regarding error reports). The
     * recommended recovery action will be set according to the
     * recommendedRecovery parameter.
     * <P><B>SAF Reference:</B> <code>saAmfPmStart()</code>
     *
     * @param componentName [in] The name of the component the monitored
     *            processes belong to.
     * @param processId [in] Identifier of a process to be monitored.
     * @param descendentsTreeDepth [in] Depth of the tree of descendents of the
     *            process, designated by processId, to be also monitored.
     *            <UL>
     *            <LI>0 indicates that no descendents of the designated process
     *            will be monitored,
     *            <LI>1 indicates that direct children of the designated
     *            process will be monitored,
     *            <LI>2 indicates that direct children and grand children of
     *            the designated process will be monitored, and so on.
     *            <LI>-1 indicates that descendents at any level in the
     *            descendents tree will be monitored.
     *            </UL>
     * @param pmErrors [in] Specifies the type of process errors to monitor.
     *            Monitoring for several errors can be requested in a single
     *            call by ORing different PmErrors values.
     *            <UL>
     *            <LI>PmErrors.NON_ZERO_EXIT requests the monitoring of
     *            processes exiting with a non-zero exit status.
     *            <LI>PmErrors.PM_ZERO_EXIT requests the monitoring of
     *            processes exiting with a with a zero exit status.
     *            </UL>
     * @param recommendedRecovery [in] Recommended recovery to be performed by
     *            the Availability Management Framework.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException Either one or both of the cases that follow
     *             apply:
     *             <UL>
     *             <LI>The component, identified by componentName, is not
     *             configured in the Availability Management Framework to
     *             execute on the local node.
     *             <LI>The process identified by processId does not execute on
     *             the local node.
     *             </UL>
     */
    public void startProcessMonitoring( String componentName,
                                              long processId,
                                              int descendentsTreeDepth,
                                              int pmErrors,
                                              RecommendedRecovery recommendedRecovery )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method requests the Availability Management Framework to stop
     * passive monitoring of specific errors, which may occur to a set of
     * processes belonging to a component.
     * <P><B>SAF Reference:</B> <code>saAmfPmStop()</code>
     *
     * @param componentName [in] The name of the component the monitored
     *            processes belong to.
     * @param stopQualifier [in] Qualifies which processes should stop being
     *            monitored.
     *            <UL>
     *            <LI>PmStopQualifier.PROC: The Availability Management
     *            Framework stops monitoring the process identified by
     *            processId.
     *            <LI>PmStopQualifier.PROC_AND_DESCENDENTS: The Availability
     *            Management Framework stops monitoring the process identified
     *            by processId and all its descendents.
     *            <LI>PmStopQualifier.ALL_PROCESSES: The Availability
     *            Management Framework stops monitoring of all processes, which
     *            belong to the component identified by componentName.
     *            </UL>
     * @param processId [in] Identifier of a process to be monitored.
     * @param pmErrors [in] Specifies the type of process errors that the
     *            Availability Management Framework should stop monitoring for
     *            the designated processes. Stopping the monitoring for several
     *            errors can be requested in a single call by ORing different
     *            PmErrors values.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException Either one, two or all cases that follow
     *             apply:
     *             <UL>
     *             <LI>The component, identified by compName, is not configured
     *             in the Availability Management Framework to execute on the
     *             local node.
     *             <LI>The process identified by processId does not execute on
     *             the local node.
     *             <LI>The process, identified by processId, was not monitored
     *             by the Availability Management Framework for errors specified
     *             by pmErrors.
     *             </UL>
     */
    public void stopProcessMonitoring( String componentName,
                                             PmStopQualifier stopQualifier,
                                             long processId,
                                             int pmErrors )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * Values for qualifiyng which processes should be stopped being monitored.
     * <P><B>SAF Reference:</B> <code>SaAmfPmStopQualifierT</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     */
    public enum PmStopQualifier {

        /**
         * The Availability Management Framework stops monitoring the specified
         * process.
         */
        PROC( 1 ),

        /**
         * The Availability Management Framework stops monitoring the specified
         * process and all its descendents.
         */
        PROC_AND_DESCENDENTS( 2 ),

        /**
         * The Availability Management Framework stops monitoring all
         * processes, which belong to the specified component.
         */
        ALL_PROCESSES( 3 );



        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;



        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private PmStopQualifier( int value ) {
            this.value = value;
        }



        /**
         * Returns the numerical value assigned to this constant by the AIS specification.
         * @return the numerical value assigned to this constant by the AIS specification.
         */
        public int getValue() {
            return this.value;
        }

    }

}


