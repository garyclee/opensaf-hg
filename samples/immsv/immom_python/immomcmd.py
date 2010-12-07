#!/usr/bin/env python
# -*- coding: utf-8 -*-

import immom
import immombin
import string
import cmd
import shlex

def lsNode(i,n,long,maxlevel):
    if i >= 0:
        print "%s%s" % (' '*i, long and n or immom.split_dn(n)[0])
    if i >= maxlevel:
        return
    for e in immom.getchildobjects(n):
        lsNode(i+1,e,long,maxlevel)

def valString(t,v):
    if t == 'SASTRINGT' or t == 'SANAMET':
        return v
    elif t == 'SAINT32T' or t == 'SAINT64T':
        return "%d" % (v)
    elif  t == 'SAUINT32T' or t == 'SAUINT64T':
        return "%u" % (v)
    elif t == 'SATIMET':
        return "%d" % (v)
    else:
        return "(TYPE NOT IMPLEMENTED)"

def stringVal(t,v):
    '''Convert a string to an appropriate value'''
    if t == 'SASTRINGT' or t == 'SANAMET':
        return v
    elif t == 'SAINT32T' or t == 'SAINT64T':
        return int(v)
    elif  t == 'SAUINT32T' or t == 'SAUINT64T':
        return int(v)
    elif t == 'SATIMET':
        return int(v)
    else:
        raise immom.AisException('SA_AIS_ERR_INVALID_PARAM')
    

def catclass(name):
    try:
        (c,a) = immom.getclass(name)
    except immom.AisException:
        print "Invalid class [%s]" % name
        return
    print "Category: %s" % c
    for (n,t,f,l) in a:
        if l:
            # Has default
            print "%-30s %-12s %s (%s)" %(n,t,string.join(f),valString(t,l[0]))
        else:
            print "%-30s %-12s %s" % (n,t,string.join(f))

def attrType(ca, n):
    '''Get the type for an attribute or "".'''
    for (a,t,f,l) in ca:
        if n == a:
            return t
    return ''

def parseAttrs(ca,argv):
    '''Parse arguments in the form "attr=value" to an attribute list.
    The returned list is suitable for creating and modifying objects.
    '''
    alist=[]
    for a in argv:
        (n,v) = a.split('=',1)
        t = attrType(ca, n)
        if not t:
            raise immom.AisException('SA_AIS_ERR_INVALID_PARAM')
        v = stringVal(t,v)
        alist.append( (n,t, [v]) )
    return alist

class ImmomCmd(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)

    def do_hist(self, args):
        """Print a list of commands that have been entered"""
        print self._hist

    def do_exit(self, args):
        """Exits from the ImmomCmd"""
        return -1

    def do_EOF(self, args):
        """Exit on system end of file character"""
        return self.do_exit(args)

    def emptyline(self):    
        """Do nothing on empty input line"""
        pass

    def do_help(self, args):
        """Get help on commands
        'help' or '?' with no arguments prints a list of commands for
        which help is available 'help <command>' or '? <command>'
        gives help on <command>
        """
        ## The only reason to define this method is for the help text
        ## in the doc string cmd.Cmd.do_help(self, args)
        cmd.Cmd.do_help(self, args)

    def preloop(self):
        """
        Initialization before prompting user for commands.  Despite
        the claims in the Cmd documentaion, Cmd.preloop() is not a
        stub.
        """
        cmd.Cmd.preloop(self)   ## sets up command completion
        self.identchars += ',='
        self.hasCcb = False
        self.cwd = ''
        self.lastcwd = ''
        self._hist = []
        self.prompt = "/: "

    def precmd(self, line):
        """ This method is called after the line has been input but before
        it has been interpreted. If you want to modifdy the input line
        before execution (for example, variable substitution) do it here.
        """
        self._hist += [ line.strip() ]
        return line

    def getwo(self, args, validate=True):
        if not args:
            wo = self.cwd
        elif args[0] == '/':
            wo = args[1:]
        elif args == '..':
            wo = immom.split_dn(self.cwd)[1]
        elif args == '-':
            wo = self.lastcwd
        else:
            if self.cwd == '':
                wo = args
            else:
                wo = '%s,%s' % (args, self.cwd)
        wo = wo.strip()
        if validate and wo != '':
            immom.getobject(wo)
        return wo

    def ls(self, args, long):
        """List sub-objects.
        Syntax: ls [-r] [ '/' | '..' | '-' | rdn | /dn ]
        """
        arg = ''
        maxlevel = 0
        for n in args.split():
            if n == '-r':
                maxlevel = 30000
            else:
                arg = n
                break
        try:
            wo = self.getwo(arg)
            lsNode(-1, wo, long, maxlevel)
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)

    def do_cd(self, args):
        """Change Current Working Object (CWO)
        Syntax: cd [ '/' | '..' | '-' | rdn | /dn ]
        """
        try:
            wo = self.getwo(args)
            self.lastcwd = self.cwd
            self.cwd = wo
            self.prompt = "/%s: " % self.cwd
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)

    def do_ls(self, args):
        """List sub-objects.
        Syntax: ls [-r] [ '/' | '..' | '-' | rdn | /dn ]
        """
        self.ls(args, False)

    def do_ll(self, args):
        """List sub-objects (long).
        Syntax: ll [-r] [ '/' | '..' | '-' | rdn | /dn ]
        """
        self.ls(args, True)

    def do_cat(self, args):
        """Print an object.
        Syntax: cat [ '/' | '..' | '-' | rdn | /dn ]
        """
        try:
            wo = self.getwo(args)
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
            return

        if not wo:
            print 'No Working Object (top)'
            return
        for (n,t,v) in immom.getobject(wo):
            if len(v) == 0:
                print '%-12s%-28s= <empty>' % (t,n)
            elif len(v) == 1:
                print '%-12s%-28s= %s' % (t,n, valString(t,v[0]))
            else:
                print '%-12s%-28s= [' % (t,n)
                for x in v:
                    print '    %s' % (valString(t,x))
                print '  ]'

    def do_class(self, args):
        """Print class information for an object.
        Syntax: class [ '/' | '..' | '-' | rdn | /dn ]
        """
        try:
            wo = self.getwo(args)
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
            return
        if not wo:
            print 'No Working Object (top)'
            return
        for (n,t,v) in immom.getobject(wo):
            if n == 'SaImmAttrClassName':
                name = v[0]
                print 'Class: %s' % name
                break
        catclass(name)

    def do_lsclasses(self, args):
        """List existing classes
        """
        for n in immom.getclassnames():
            print n

    def do_catclass(self, args):
        """Print class information for a class
        Syntax: catclass <class_name>
        """
        if args:
            catclass(args)

    def do_instanceof(self, args):
        """Print object instances of a class.
        Syntax: instanceof <class_name> [ '/' | '..' | '-' | rdn | /dn ]
        """
        if not args:
            return
        argv = args.split()
        wo = ''
        if len(argv) > 1:
            wo = argv[1]
        for dn in immom.getinstanceof(self.getwo(wo, False), argv[0]):
            print dn

    def do_adminowner_clear(self, args):
        """Clear the admin-owner for objects.
        Syntax: adminowner_clear [-r] [ '/' | '..' | '-' | rdn | /dn ]
        """
        arg = ''
        scope = 'SA_IMM_ONE'
        for n in args.split():
            if n == '-r':
                scope = 'SA_IMM_SUBTREE'
            else:
                arg = n
                break
        try:
            wo = self.getwo(arg)
            if not wo:
                print 'No Working Object (top)'
            else:
                immom.adminowner_clear(scope, [ wo ])
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)

    def do_ccb(self, args):
        """CCB operation.
        Syntax. ccb <create|apply|abort>
        """
        if args == 'create':
            if self.hasCcb:
                print "CCB already created"
            else:
                immom.adminowner_initialize('immomcmd')
                immom.ccb_initialize()
                self.hasCcb = True
        elif args == 'apply':
            if self.hasCcb:
                immom.ccb_apply()
                immom.ccb_finalize()
                immom.adminowner_finalize()
                self.hasCcb = False
            else:
                print "CCB not created"
        elif args == 'abort':
            if self.hasCcb:
                immom.ccb_finalize()
                immom.adminowner_finalize()
                self.hasCcb = False
            else:
                print "CCB not created"
        else:
            print 'Invalid ccb operation [%s]' % args

    def do_delete(self, args):
        """Delete an object.
        If no CCB is active a temporary CCB is used.
        Syntax: delete [ '/' | '..' | '-' | rdn | /dn ]
        """

        ## FIX: Delete current object !!!
        try:
            wo = self.getwo(args)
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
            return
        if not wo:
            print 'No Working Object (top)'
            return

        try:
            if self.hasCcb:
                immom.deleteobject([wo])
            else:
                self.do_ccb('create')
                immom.deleteobjects([wo])
                self.do_ccb('apply')
        except immom.AisException, ex:
            self.do_ccb('abort')
            print 'Failed (ccb aborted): ' + str(ex)

    def do_create(self, args):
        """Create an object.
        If no CCB is active a temporary CCB is used.
        Syntax: create <rdn | /dn> classname [attr=value ...]
        """
        argv = shlex.split(args)
        if len(argv) < 2:
            print "Too few parameters"
            return

        wo = self.getwo(argv[0], False)
        if not wo:
            print 'No Working Object (top)'
            return

        try:
            (c,ca) = immom.getclass(argv[1])
        except immom.AisException, ex:
            print 'getclass: ' + str(ex)
            return
        try:
            if self.hasCcb:
                immom.createobject(wo, argv[1], parseAttrs(ca, argv[2:]))
            else:
                self.do_ccb('create')
                immom.createobject(wo, argv[1], parseAttrs(ca, argv[2:]))
                self.do_ccb('apply')
        except immom.AisException, ex:
            self.do_ccb('abort')
            print 'Failed (ccb aborted): ' + str(ex)

    def do_modify(self, args):
        '''Modify an object.
        If no CCB is active a temporary CCB is used.
        Syntax: modify <'..' | '-' | rdn | /dn > [attr=value ...]
        '''
        argv = shlex.split(args)
        try:
            wo = self.getwo(argv[0])
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
            return
        if not wo:
            print 'No Working Object (top)'
            return
        for (n,t,v) in immom.getobject(wo):
            if n == 'SaImmAttrClassName':
                classname = v[0]
                break
        (c,ca) = immom.getclass(classname)
        attrs = parseAttrs(ca, argv[1:])
        try:
            if self.hasCcb:
                immom.modifyobject(wo, attrs)
            else:
                self.do_ccb('create')
                immom.modifyobject(wo, attrs)
                self.do_ccb('apply')
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)


    def do_adminop(self, args):
        '''Invoke an administrative operation.
        Syntax: adminop <'..' | '-' | rdn | /dn > <op> [attr=value ...]
        All attributes are assumed to have type=SASTRINGT (for now)
        '''
        argv = shlex.split(args)
        try:
            wo = self.getwo(argv[0])
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
            return
        if not wo:
            print 'No Working Object (top)'
            return
        immom.adminowner_initialize('immomcmd')
        alist = [];
        for a in argv[2:]:
            (n,v) = a.split('=',1)
            alist.append((n,'SASTRINGT', [v]))
        try:
            immom.adminoperation(wo, int(argv[1]), alist)
        except immom.AisException, ex:
            print 'Failed: ' + str(ex)
        immom.adminowner_finalize()

if __name__ == '__main__':
    immomcmd = ImmomCmd()
    immomcmd.cmdloop() 
