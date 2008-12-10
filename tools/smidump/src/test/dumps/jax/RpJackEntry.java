/*
 * This Java file has been generated by smidump 0.3.0. Do not edit!
 * It is intended to be used within a Java AgentX sub-agent environment.
 *
 * $Id: RpJackEntry.java,v 1.10 2002/03/05 14:27:10 strauss Exp $
 */

/**
    This class represents a Java AgentX (JAX) implementation of
    the table row rpJackEntry defined in MAU-MIB.

    @version 1
    @author  smidump 0.3.0
    @see     AgentXTable, AgentXEntry
 */

import jax.AgentXOID;
import jax.AgentXSetPhase;
import jax.AgentXResponsePDU;
import jax.AgentXEntry;

public class RpJackEntry extends AgentXEntry
{

    protected int rpJackIndex = 0;
    protected int rpJackType = 0;
    // foreign indices
    protected int rpMauGroupIndex;
    protected int rpMauPortIndex;
    protected int rpMauIndex;

    public RpJackEntry(int rpMauGroupIndex,
                       int rpMauPortIndex,
                       int rpMauIndex,
                       int rpJackIndex)
    {
        this.rpMauGroupIndex = rpMauGroupIndex;
        this.rpMauPortIndex = rpMauPortIndex;
        this.rpMauIndex = rpMauIndex;
        this.rpJackIndex = rpJackIndex;

        instance.append(rpMauGroupIndex);
        instance.append(rpMauPortIndex);
        instance.append(rpMauIndex);
        instance.append(rpJackIndex);
    }

    public int get_rpMauGroupIndex()
    {
        return rpMauGroupIndex;
    }

    public int get_rpMauPortIndex()
    {
        return rpMauPortIndex;
    }

    public int get_rpMauIndex()
    {
        return rpMauIndex;
    }

    public int get_rpJackIndex()
    {
        return rpJackIndex;
    }

    public int get_rpJackType()
    {
        return rpJackType;
    }

}

