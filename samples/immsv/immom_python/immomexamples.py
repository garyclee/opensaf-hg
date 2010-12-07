#! /usr/bin/env python

import immom



def getimplementer(dn):
    """Returns the implmenter of an object or '' if there are no implementer.
    """
    for (n,t,v) in immom.getobject(dn):
        if n == 'SaImmAttrImplementerName':
            if v:
                return v[0]
            return ''

def getimplementers():
    """Returns a list of all active implmenters.
    """
    implementers = []
    for dn in immom.getsubtree(''):
        i = getimplementer(dn)
        if i and not i in implementers:
            implementers.append(i)
    return implementers

def getadminowner(dn):
    """Returns the adminowner of an object or '' if there are no adminowner.
    """
    for (n,t,v) in immom.getobject(dn):
        if n == 'SaImmAttrAdminOwnerName':
            if v:
                return v[0]
            return ''

def runtimeclasses():
    """Returns a set of all RUNTIME classes.
    """
    rtset = set()
    for cn in immom.getclassnames():
        (c,a) = immom.getclass(cn)
        if c == 'RUNTIME':
            rtset.add(cn)
    return rtset

def configclasses():
    """Returns a set of all CONFIG classes.
    """
    return set(immom.getclassnames()) - runtimeclasses()

def configrtclasses():
    """Returns a set of all CONFIG classes with RUNTIME attributes.
    Use; configclasses() - configrtclasses()
    to get a set of all pure config classes.
    """
    rtset = set()
    for cn in configclasses():
        (c,a) = immom.getclass(cn)
        for (n,t,f,d) in a:
            if 'RUNTIME' in f:
                rtset.add(cn)
    return rtset

def immdump():
    print '<?xml version="1.0"?>'
    print '<imm:IMM-contents>'
    for c in immom.getclassnames():
        immom.dumpclass(c)
    for o in immom.getsubtree(''):
        immom.dumpobj(o)
    print '</imm:IMM-contents>'
