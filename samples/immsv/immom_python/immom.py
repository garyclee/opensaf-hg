#! /usr/bin/python
"""
immom -- An IMM Object Manager in Python

  The Information Model Management (IMM) is the configuration module
  in Service Availability Forum (SAF). The "immom" module provides
  object management functions towards the IMM.

  "immom" uses the "immombin" module to make native calls to the
  IMM C-API functions.

  Most functions may raise an immom.AisException. The possible
  reasons are;

        "SA_AIS_ERR_LIBRARY",
        "SA_AIS_ERR_VERSION",
        "SA_AIS_ERR_INIT",
        "SA_AIS_ERR_TIMEOUT",
        "SA_AIS_ERR_TRY_AGAIN",
        "SA_AIS_ERR_INVALID_PARAM",
        "SA_AIS_ERR_NO_MEMORY",
        "SA_AIS_ERR_BAD_HANDLE",
        "SA_AIS_ERR_BUSY",
        "SA_AIS_ERR_ACCESS",
        "SA_AIS_ERR_NOT_EXIST",
        "SA_AIS_ERR_NAME_TOO_LONG",
        "SA_AIS_ERR_EXIST",
        "SA_AIS_ERR_NO_SPACE",
        "SA_AIS_ERR_INTERRUPT",
        "SA_AIS_ERR_NAME_NOT_FOUND",
        "SA_AIS_ERR_NO_RESOURCES",
        "SA_AIS_ERR_NOT_SUPPORTED",
        "SA_AIS_ERR_BAD_OPERATION",
        "SA_AIS_ERR_FAILED_OPERATION",
        "SA_AIS_ERR_MESSAGE_ERROR",
        "SA_AIS_ERR_QUEUE_FULL",
        "SA_AIS_ERR_QUEUE_NOT_AVAILABLE",
        "SA_AIS_ERR_BAD_FLAGS",
        "SA_AIS_ERR_TOO_BIG",
        "SA_AIS_ERR_NO_SECTIONS",
        "SA_AIS_ERR_NO_OP",
        "SA_AIS_ERR_REPAIR_PENDING",
        "SA_AIS_ERR_NO_BINDINGS",
        "SA_AIS_ERR_UNAVAILABLE"
  

"""
import sys
import immombin
from immombin import AisException

immombin.saImmOmInitialize()

_flagdict = {
    'MULTI_VALUE':0x00000001,
    'RDN':0x00000002,
    'CONFIG':0x00000100,
    'WRITABLE':0x00000200,
    'INITIALIZED':0x00000400,
    'RUNTIME':0x00010000,
    'PERSISTENT':0x00020000,
    'CACHED':0x00040000,
    }

def _flag_list(flags):
    l = []
    for k in _flagdict.keys():
        if flags & _flagdict[k]:
            l.append(k)
    return l

def _flag_parse(l):
    f = 0
    for n in l:
        f |= _flagdict[n]
    return f

def _get_rdn_attr(class_name):
    (c,a) = getclass(class_name)
    for (n,t,f,l) in a:
        if 'RDN' in f:
            return (n,t)

def split_dn(dn):
    """Split a distinguish (dn) name into a tuple; (rdn,parent)
    Handles the rather tricky case with "referende" rdn's like;
    "ref=one=1\,two=2\,three=3,top=1" which would split into
    ('ref=one=1\\,two=2\\,three=3', 'top=1')
    """
    dnitems = dn.split(',')
    i=0
    while dnitems[i][-1] == '\\':
        i = i+1
    rdn = ','.join(dnitems[:i+1])
    parent = ','.join(dnitems[i+1:])
    return (rdn,parent)

def getclass(name):
    """Get IMM Class Information.
    Retruns a tuple (category, attribute_list). The attribute_list consists
    of tuples (name, type, flag_list, default_list). Example;
    
    >>> immom.getclass('Common')
    ('CONFIG', [('userLabel', 'SASTRINGT', ['CONFIG', 'WRITABLE'],[]),
                ('SaImmAttrImplementerName', 'SASTRINGT', ['CONFIG'],[]),
                ('SaImmAttrClassName', 'SASTRINGT', ['CONFIG'],[]),
                ('SaImmAttrAdminOwnerName', 'SASTRINGT', ['CONFIG'],[]),
                ('CommonId', 'SANAMET', ['RDN', 'CONFIG', 'INITIALIZED'],[])])

    category := 'CONFIG' | 'RUNTIME'

    type := 'SAINT32T' | 'SAUINT32T' | 'SAINT64T' | 'SAUINT64T' |
            'SATIMET' | 'SANAMET' | 'SAFLOATT' | 'SADOUBLET' |
            'SASTRINGT' | 'SAANYT'
    
    flag := 'MULTI_VALUE' | 'RDN' | 'CONFIG' | 'WRITABLE' | 'INITIALIZED' |
            'RUNTIME' | 'PERSISTENT' | 'CACHED'

    """
    (c,a) = immombin.saImmOmClassDescriptionGet(name)
    return (c, [(n,t,_flag_list(f),l) for (n,t,f,l) in a])

def getclassnames():
    """Returns a list of all defined IMM classes.
    """
    dn='opensafImm=opensafImm,safApp=safImmService'
    for (n,t,v) in immombin.saImmOmAccessorGet(dn):
        if n == 'opensafImmClassNames':
            return v

def createclass(name, category, attrs):
    """Create a new IMM class.
    See the "getclass" function for parameter values. Any "SaImm*"
    attributes passed will be ignored.

    NOTE: Default values are not yet implemented.
    """
    attrs = [ (n,t,_flag_parse(f),l) for (n,t,f,l) in attrs
              if not n.startswith('SaImm') ]
    immombin.saImmOmClassCreate(name, category, attrs)

def deleteclass(name):
    """Delete an IMM class.
    No instantiated objects of the class may exist.
    """
    immombin.saImmOmClassDelete(name)

def getchildobjects(dn):
    """Get child objects of the passed object.
    An empty dn ('') represents the root.
    """
    return filter(lambda x: x != dn, immombin.saImmOmSearchSublevel(dn))

def getsubtree(dn):
    """Get ALL objects beneath the passed object.
    An empty dn ('') represents the root and will thus return all objects
    in the IMM. In the returned list parents will come before their child
    objects.
    """
    c = getchildobjects(dn)
    s = []
    for n in c:
        s.extend(getsubtree(n))
    c.extend(s)
    return c

def getinstanceof(dn, classname):
    """Get instances of a class beneath the passed object.
    An empty dn ('') represents the root and will thus return all instances
    of the class. Childs may come before their parents in the list (IMM
    search order is undefined).
    """
    return immombin.saImmOmInstanceOf(dn, classname)

def classof(dn):
    """Get the class of an object.
    """
    for (n,t,v) in getobject(dn):
        if n == 'SaImmAttrClassName':
            return v[0]
    
def getobject(dn):
    """Get an IMM object.
    Returns a list with all attributes as tuples (name, type, value_list).
    See the "getclass" function for types.
    """
    return immombin.saImmOmAccessorGet(dn)

def getattributes(dn):
    """Get the attributes of an IMM object as a dictionary.
    This is basically the same as the getobject function but returned as
    a convenient dictionary. The type info is however lost.
    """
    a = dict()
    for (n,t,v) in immombin.saImmOmAccessorGet(dn):
        a[n] = v
    return a

def deleteobjects(dn_list):
    """Delete IMM objects.
    Prerequisites: An admin owner and CCB must have been initiated.
    """
    immombin.saImmOmAdminOwnerSet('SA_IMM_ONE', dn_list)
    for dn in dn_list:
        immombin.saImmOmCcbObjectDelete(dn)

def deletesubtree(dn):
    """Delete a subtree in the IMM.
    Prerequisites: An admin owner and CCB must have been initiated.
    WARNING: This can be a very destructive command!
    """
    immombin.saImmOmAdminOwnerSet('SA_IMM_SUBTREE', [dn])
    immombin.saImmOmCcbObjectDelete(dn)

def adminowner_initialize(name):
    """Initialize an admin owner.
    "immom" allows only one active admin owner. An admin owner is
    required for a CCB.
    """
    immombin.saImmOmAdminOwnerInitialize(name)

def adminowner_clear(scope, dn_list):
    """Clear the admin owner for objects.

    scope := 'SA_IMM_ONE' | 'SA_IMM_SUBLEVEL' | 'SA_IMM_SUBTREE'
    """
    immombin.saImmOmAdminOwnerClear(scope, dn_list)

def adminowner_finalize():
    """Finalize the admin owner.
    Any ongoing CCB will be aborted.
    """
    immombin.saImmOmAdminOwnerFinalize()

def createobject(dn, class_name, attr_list):
    """Create an IMM object.
    The passed dn is the object to be created, NOT the parent. The rdn
    attribute will be created automatically and shall not be included
    in the attr_list. Any "SaImm*" and RDN attributes will be ignored.
    """
    (rdn,parent) = split_dn(dn)
    (rdn_attr,rdn_type) = _get_rdn_attr(class_name)
    attr_list = filter(lambda (n,t,v):
                       not n.startswith('SaImm') and n != rdn_attr, attr_list)
    attr_list.append( (rdn_attr, rdn_type, [ rdn ]) )
    if parent:
        immombin.saImmOmAdminOwnerSet('SA_IMM_ONE', [parent])
    immombin.saImmOmCcbObjectCreate(parent, class_name, attr_list)

def copyobject(src_dn, dst_dn):
    """Copy an IMM object.
    """
    attr_list = getobject(src_dn)
    for (n,t,v) in attr_list:
        if n == 'SaImmAttrClassName':
            class_name = v[0]
            break
    createobject(dst_dn, class_name, attr_list)

def modifyobject(dn, attr_list):
    """Modify an IMM object.
    Any "SaImm*" attributes will be ignored.
    """
    #immombin.saImmOmAdminOwnerSet('SA_IMM_ONE', [dn])
    attr_list = filter(lambda (n,t,v):
                       not n.startswith('SaImm'), attr_list)
    immombin.saImmOmCcbObjectModify(dn, attr_list)

def adminoperation(dn, op, attr_list):
    """Invoke an Administrative Operation.
    """
    immombin.saImmOmAdminOwnerSet('SA_IMM_ONE', [dn])
    immombin.saImmOmAdminOperationInvoke(dn, op + 0L, attr_list)

def ccb_initialize(flag=0):
    """Initialize a CCB.
    "immom" allows only one active CCB.  A CCB is required for
    create/modify/delete object operations. Setting the flag parameter to
    1 (SA_IMM_CCB_REGISTERED_OI) causes the ccb to fail if no implementer
    exist.
    Prerequisites: An admin owner must have been initiated.
    """
    immombin.saImmOmCcbInitialize(flag)

def ccb_apply():
    """Apply a CCB.
    All changes in the CCB will be executed in an atomic operation.
    """
    immombin.saImmOmCcbApply()

def ccb_finalize():
    """Finalize the CCB
    If "ccb_apply()" has NOT been called all operations for the CCB are
    aborted.
    """
    immombin.saImmOmCcbFinalize()

def _dumpclassattr(n, t, f, d):
    if n.startswith('Sa'):
        return
    print 'RDN' in f and '    <rdn>' or '    <attr>'
    print '      <name>%s</name>' % n
    print '      <type>SA_%s_T</type>' % t[2:-1]
    for fv in f:
        if fv == 'CONFIG':
            print '      <category>SA_CONFIG</category>'
        elif fv == 'RDN':
            pass
        elif fv == 'RUNTIME':
            print '      <category>SA_RUNTIME</category>'
        else:
            print '      <flag>SA_%s</flag>' % fv
    print 'RDN' in f and '    </rdn>' or '    </attr>'

def dumpclass(cn):
    """Dump a class in imm-xml format.
    """
    (c,a) = getclass(cn)
    print '  <class name="%s">' % cn
    print '    <category>SA_%s</category>' % c
    for (n,t,f,d) in a:
        _dumpclassattr(n,t,f,d)
    print '  </class>'

def _getaflags(ca, attr):
    for (n,t,f,d) in ca:
        if n == attr:
            return f

def dumpobj(dn):
    """Dump an object in imm-xml format.
    """
    a = getattributes(dn)
    c = a['SaImmAttrClassName'][0]
    (cat, ax) = getclass(c)
    if cat != 'CONFIG':
        return

    print '  <object class="%s">' % c
    print '    <dn>%s</dn>' % dn
    for (k,v) in a.iteritems():
        if k.startswith('Sa') or not v:
            continue
        f = _getaflags(ax, k)
        if 'RDN' in f:
            continue
        if 'RUNTIME' in f and not 'PERSISTENT' in f:
            continue
        print '    <attr>'
        print '      <name>%s</name>' % k
        for vv in v:
            print '      <value>' + str(vv) + '</value>'
        print '    </attr>'
    print '  </object>'

