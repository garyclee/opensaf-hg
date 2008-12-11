/*
 * This Java file has been generated by smidump 0.3.0. It
 * is intended to be edited by the application programmer and
 * to be used within a Java AgentX sub-agent environment.
 *
 * $Id: NetConfigEntryImpl.java,v 1.10 2002/03/05 14:27:09 strauss Exp $
 */

/**
    This class extends the Java AgentX (JAX) implementation of
    the table row netConfigEntry defined in RMON2-MIB.
 */

import jax.AgentXOID;
import jax.AgentXSetPhase;
import jax.AgentXResponsePDU;
import jax.AgentXEntry;

public class NetConfigEntryImpl extends NetConfigEntry
{

    // constructor
    public NetConfigEntryImpl(int ifIndex)
    {
        super(ifIndex);
    }

    public byte[] get_netConfigIPAddress()
    {
        return netConfigIPAddress;
    }

    public int set_netConfigIPAddress(AgentXSetPhase phase, byte[] value)
    {
        switch (phase.getPhase()) {
        case AgentXSetPhase.TEST_SET:
            break;
        case AgentXSetPhase.COMMIT:
            undo_netConfigIPAddress = netConfigIPAddress;
            netConfigIPAddress = new byte[value.length];
            for(int i = 0; i < value.length; i++)
                netConfigIPAddress[i] = value[i];
            break;
        case AgentXSetPhase.UNDO:
            netConfigIPAddress = undo_netConfigIPAddress;
            break;
        case AgentXSetPhase.CLEANUP:
            undo_netConfigIPAddress = null;
            break;
        default:
            return AgentXResponsePDU.PROCESSING_ERROR;
        }
        return AgentXResponsePDU.NO_ERROR;
    }
    public byte[] get_netConfigSubnetMask()
    {
        return netConfigSubnetMask;
    }

    public int set_netConfigSubnetMask(AgentXSetPhase phase, byte[] value)
    {
        switch (phase.getPhase()) {
        case AgentXSetPhase.TEST_SET:
            break;
        case AgentXSetPhase.COMMIT:
            undo_netConfigSubnetMask = netConfigSubnetMask;
            netConfigSubnetMask = new byte[value.length];
            for(int i = 0; i < value.length; i++)
                netConfigSubnetMask[i] = value[i];
            break;
        case AgentXSetPhase.UNDO:
            netConfigSubnetMask = undo_netConfigSubnetMask;
            break;
        case AgentXSetPhase.CLEANUP:
            undo_netConfigSubnetMask = null;
            break;
        default:
            return AgentXResponsePDU.PROCESSING_ERROR;
        }
        return AgentXResponsePDU.NO_ERROR;
    }
    public int get_netConfigStatus()
    {
        return netConfigStatus;
    }

    public int set_netConfigStatus(AgentXSetPhase phase, int value)
    {
        switch (phase.getPhase()) {
        case AgentXSetPhase.TEST_SET:
            break;
        case AgentXSetPhase.COMMIT:
            undo_netConfigStatus = netConfigStatus;
            netConfigStatus = value;
            break;
        case AgentXSetPhase.UNDO:
            netConfigStatus = undo_netConfigStatus;
            break;
        case AgentXSetPhase.CLEANUP:
            break;
        default:
            return AgentXResponsePDU.PROCESSING_ERROR;
        }
        return AgentXResponsePDU.NO_ERROR;
    }
}

