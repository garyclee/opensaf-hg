/*
 * This Java file has been generated by smidump 0.3.0. It
 * is intended to be edited by the application programmer and
 * to be used within a Java AgentX sub-agent environment.
 *
 * $Id: RingStationControl2EntryImpl.java,v 1.10 2002/03/05 14:27:10 strauss Exp $
 */

/**
    This class extends the Java AgentX (JAX) implementation of
    the table row ringStationControl2Entry defined in RMON2-MIB.
 */

import jax.AgentXOID;
import jax.AgentXSetPhase;
import jax.AgentXResponsePDU;
import jax.AgentXEntry;

public class RingStationControl2EntryImpl extends RingStationControl2Entry
{

    // constructor
    public RingStationControl2EntryImpl()
    {
        super();
    }

    public long get_ringStationControlDroppedFrames()
    {
        return ringStationControlDroppedFrames;
    }

    public long get_ringStationControlCreateTime()
    {
        return ringStationControlCreateTime;
    }

}

