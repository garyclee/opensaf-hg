/*
 * This Java file has been generated by smidump 0.3.0. It
 * is intended to be edited by the application programmer and
 * to be used within a Java AgentX sub-agent environment.
 *
 * $Id: MatrixControl2EntryImpl.java,v 1.10 2002/03/05 14:27:09 strauss Exp $
 */

/**
    This class extends the Java AgentX (JAX) implementation of
    the table row matrixControl2Entry defined in RMON2-MIB.
 */

import jax.AgentXOID;
import jax.AgentXSetPhase;
import jax.AgentXResponsePDU;
import jax.AgentXEntry;

public class MatrixControl2EntryImpl extends MatrixControl2Entry
{

    // constructor
    public MatrixControl2EntryImpl()
    {
        super();
    }

    public long get_matrixControlDroppedFrames()
    {
        return matrixControlDroppedFrames;
    }

    public long get_matrixControlCreateTime()
    {
        return matrixControlCreateTime;
    }

}

