#!/usr/bin/env python
# -*- coding: utf-8 -*-

import immom
import unittest

# Handle initialization
import immombin
immombin.saImmOmFinalize()


class Immom10BasicTest(unittest.TestCase):

    def setUp(self):
        immombin.saImmOmInitialize()
    def tearDown(self):
        immombin.saImmOmFinalize()

    def test0010_Setup(self):
        pass

    def test0020_Initialize(self):
        self.assertRaises(immom.AisException, immombin.saImmOmInitialize)
        immombin.saImmOmFinalize()
        self.assertRaises(immom.AisException, immombin.saImmOmFinalize)
        immombin.saImmOmInitialize()

    def test0030_AdminOwner(self):
        self.assertRaises(immom.AisException, 
                          immom.adminowner_initialize, '')
        immom.adminowner_initialize('TestAdmin')
        self.assertRaises(immom.AisException, 
                          immom.adminowner_initialize, 'TestAdmin')
        immom.adminowner_finalize()
        self.assertRaises(immom.AisException,
                          immom.adminowner_finalize)

    def test0040_Ccb(self):
        self.assertRaises(immom.AisException, immom.ccb_initialize)
        immom.adminowner_initialize('TestAdmin')
        immom.ccb_initialize()
        immom.ccb_apply()
        immom.ccb_finalize()
        self.assertRaises(immom.AisException, immom.ccb_apply)
        self.assertRaises(immom.AisException, immom.ccb_finalize)
        immom.ccb_initialize()
        immom.ccb_finalize()
        immom.adminowner_finalize()
        self.assertRaises(immom.AisException, immom.ccb_initialize)

    def test0050_Class(self):
        # Clean-up
        testclass = 'TestClassBasic'
        try:
            immom.deleteclass(testclass)
        except:
            pass
        classes = immom.getclassnames()
        self.assertTrue(classes)
        self.assertTrue('SaImmMngt' in classes)
        attrs = [('TestClassBasicId', 'SANAMET', [ 'CONFIG', 'RDN' ], [])]
        immom.createclass(testclass, 'CONFIG', attrs)
        classes = immom.getclassnames()
        self.assertTrue(testclass in classes)
        self.assertRaises(immom.AisException, 
                          immom.createclass, testclass, 'CONFIG', attrs)
        immom.deleteclass(testclass)
        self.assertRaises(immom.AisException,
                          immom.deleteclass, testclass)

class Immom20BasicObjectTest(unittest.TestCase):

    topobject = 'TestClassId=1'

    def cleanup(self):
        try:
            immom.getobject(self.topobject)
            immom.ccb_initialize()
            immom.deletesubtree(self.topobject)
            immom.ccb_apply()
            immom.ccb_finalize()
        except Exception, ex:
            pass
        try:
            immom.deleteclass('TestClass')
        except:
            pass
        try:
            immom.deleteclass('TestClassMod')
        except:
            pass

    def setUp(self):
        immombin.saImmOmInitialize()
        immom.adminowner_initialize('TestAdmin')
        self.cleanup()
        attrs = [('TestClassId', 'SANAMET', [ 'CONFIG', 'RDN' ], [])]
        immom.createclass('TestClass', 'CONFIG', attrs)

    def tearDown(self):
        self.cleanup()
        immom.adminowner_finalize()
        immombin.saImmOmFinalize()

    def test0010_TopObject(self):
        self.assertRaises(immom.AisException,
                          immom.getobject, self.topobject)
        self.assertRaises(immom.AisException,
                          immom.createobject, self.topobject, 'TestClass', [])
        immom.ccb_initialize()
        self.assertRaises(immom.AisException,
                          immom.createobject,self.topobject,'InvalidClass', [])
        immom.createobject(self.topobject, 'TestClass', [])
        self.assertRaises(immom.AisException,
                          immom.getobject, self.topobject)
        immom.ccb_apply()
        immom.ccb_finalize()

        o = immom.getobject(self.topobject)
        o.sort()
        self.assertEqual(
            o, [
            ('SaImmAttrAdminOwnerName', 'SASTRINGT', ['TestAdmin']),
            ('SaImmAttrClassName', 'SASTRINGT', ['TestClass']),
            ('SaImmAttrImplementerName', 'SASTRINGT', []),
            ('TestClassId', 'SANAMET', ['TestClassId=1'])
            ])

        immom.adminowner_clear('SA_IMM_SUBTREE', [self.topobject])
        o = immom.getobject(self.topobject)
        o.sort()
        self.assertEqual(
            o, [
            ('SaImmAttrAdminOwnerName', 'SASTRINGT', []),
            ('SaImmAttrClassName', 'SASTRINGT', ['TestClass']),
            ('SaImmAttrImplementerName', 'SASTRINGT', []),
            ('TestClassId', 'SANAMET', ['TestClassId=1'])
            ])

        self.assertRaises(immom.AisException,
                          immom.deleteclass, 'TestClass')

        immom.ccb_initialize()
        immom.deleteobjects([self.topobject])
        immom.getobject(self.topobject)
        immom.ccb_apply()
        immom.ccb_finalize()
        self.assertRaises(immom.AisException,
                          immom.getobject, self.topobject)

    def test0011_TopObject_noOI(self):
        immom.ccb_initialize(1)
        self.assertRaises(immom.AisException,
                          immom.createobject, self.topobject, 'TestClass', [])
        immom.ccb_finalize()
        immom.ccb_initialize(0)
        immom.createobject(self.topobject, 'TestClass', [])
        immom.ccb_finalize()


    def test0020_ObjectTree(self):
        immom.ccb_initialize()
        immom.createobject(self.topobject, 'TestClass', [])
        for n in xrange(1,5):
            dn = "TestClassId=%d,%s" % (n, self.topobject)
            immom.createobject(dn, 'TestClass', [])
        immom.ccb_apply()
        immom.ccb_finalize()
        c = immom.getchildobjects(self.topobject)
        self.assertEqual(
            c, [
            'TestClassId=1,TestClassId=1',
            'TestClassId=2,TestClassId=1',
            'TestClassId=3,TestClassId=1',
            'TestClassId=4,TestClassId=1'
            ])

        immom.ccb_initialize()
        for n in xrange(30,35):
            dn = "TestClassId=%d,TestClassId=3,%s" % (n, self.topobject)
            immom.createobject(dn, 'TestClass', [])
        for n in xrange(20,23):
            dn = "TestClassId=%d,TestClassId=2,%s" % (n, self.topobject)
            immom.createobject(dn, 'TestClass', [])
        immom.ccb_apply()
        immom.ccb_finalize()

        c = immom.getsubtree(self.topobject)
        self.assertEqual(
            c, [
            'TestClassId=1,TestClassId=1',
            'TestClassId=2,TestClassId=1',
            'TestClassId=3,TestClassId=1',
            'TestClassId=4,TestClassId=1',
            'TestClassId=20,TestClassId=2,TestClassId=1',
            'TestClassId=21,TestClassId=2,TestClassId=1',
            'TestClassId=22,TestClassId=2,TestClassId=1',
            'TestClassId=30,TestClassId=3,TestClassId=1',
            'TestClassId=31,TestClassId=3,TestClassId=1',
            'TestClassId=32,TestClassId=3,TestClassId=1',
            'TestClassId=33,TestClassId=3,TestClassId=1',
            'TestClassId=34,TestClassId=3,TestClassId=1'
            ])

        immom.ccb_initialize()
        immom.deletesubtree(self.topobject)
        immom.getobject(self.topobject)
        immom.ccb_apply()
        immom.ccb_finalize()
        self.assertRaises(immom.AisException,
                          immom.getobject, self.topobject)


    def test0030_ModifyObject(self):
        attrs = [
            ('TestClassModId', 'SANAMET', [ 'CONFIG', 'RDN' ],[]),
            ('saint32t', 'SAINT32T', [ 'CONFIG', 'WRITABLE' ],[-11]),
            ('sauint32t', 'SAUINT32T', [ 'CONFIG', 'WRITABLE' ],[22]),
            ('saint64t', 'SAINT64T', [ 'CONFIG', 'WRITABLE' ],[-33]),
            ('sauint64t', 'SAUINT64T', [ 'CONFIG', 'WRITABLE' ],[44]),
            ('satimet', 'SATIMET', [ 'CONFIG', 'WRITABLE' ],[]),
            ('sanamet', 'SANAMET', [ 'CONFIG', 'WRITABLE' ],[]),
            ('safloatt', 'SAFLOATT', [ 'CONFIG', 'WRITABLE' ],[]),
            ('sadoublet', 'SADOUBLET', [ 'CONFIG', 'WRITABLE' ],[]),
            ('sastringt', 'SASTRINGT', ['CONFIG','WRITABLE','MULTI_VALUE'],[]),
            ('saanyt', 'SAANYT', [ 'CONFIG', 'WRITABLE' ],[]),
            ]
        immom.createclass('TestClassMod', 'CONFIG', attrs)
        nonemptyattr = lambda (n,t,v): v and not n.startswith('SaImm')

        # Set-up
        modobject = 'TestClassModId=1,' + self.topobject
        immom.ccb_initialize()
        immom.createobject(self.topobject, 'TestClass', [])
        immom.createobject(modobject, 'TestClassMod', [])
        immom.ccb_apply()
        immom.ccb_finalize()

        a = filter(nonemptyattr, immom.getobject(modobject))
        a.sort()
        self.assertEqual(
            a, [
            ('TestClassModId', 'SANAMET', ['TestClassModId=1']),
            ('saint32t', 'SAINT32T', [-11]),
            ('saint64t', 'SAINT64T', [-33L]),
            ('sauint32t', 'SAUINT32T', [22]),
            ('sauint64t', 'SAUINT64T', [44L])
            ])

        immom.ccb_initialize()
        immom.modifyobject(modobject, [
            ('saint32t', 'SAINT32T', [-2147483648]),
            ('saint64t', 'SAINT64T', [-9223372036854775808L]),
            ])
        immom.ccb_apply()
        immom.ccb_finalize()
        a = filter(nonemptyattr, immom.getobject(modobject))
        a.sort()
        self.assertEqual(
            a, [
            ('TestClassModId', 'SANAMET', ['TestClassModId=1']),
            ('saint32t', 'SAINT32T', [-2147483648]),
            ('saint64t', 'SAINT64T', [-9223372036854775808L]),
            ('sauint32t', 'SAUINT32T', [22]),
            ('sauint64t', 'SAUINT64T', [44L])
            ])

        # 'SAUINT64T' does not work. It seems like correct values are
        # stored, but signed values are read.

        immom.ccb_initialize()
        immom.modifyobject(modobject, [
            ('saint32t', 'SAINT32T', [2147483647]),
            ('sauint32t', 'SAUINT32T', [ 4294967295 ]),
            ('saint64t', 'SAINT64T', [9223372036854775807L]),
            ('sauint64t', 'SAUINT64T', [9223372036854775807L]),
            ('sastringt', 'SASTRINGT', ['kalle', 'kula', 'was', 'here']),
            ('sanamet', 'SANAMET', ['a,b,c,d,e=90,f'])
            ])
        immom.ccb_apply()
        immom.ccb_finalize()
        a = filter(nonemptyattr, immom.getobject(modobject))
        a.sort()
        self.assertEqual(
            a, [
            ('TestClassModId', 'SANAMET', ['TestClassModId=1']),
            ('saint32t', 'SAINT32T', [2147483647]),
            ('saint64t', 'SAINT64T', [9223372036854775807L]),
            ('sanamet', 'SANAMET', ['a,b,c,d,e=90,f']),
            ('sastringt', 'SASTRINGT', ['kalle', 'kula', 'was', 'here']),
            ('sauint32t', 'SAUINT32T', [4294967295]),
            ('sauint64t', 'SAUINT64T', [9223372036854775807L])
            ])

        immom.ccb_initialize()
        immom.modifyobject(modobject, [
            ('saint32t', 'SAINT32T', []),
            ('saint64t', 'SAINT64T', []),
            ('sanamet', 'SANAMET', []),
            ('sastringt', 'SASTRINGT', []),
            ('sauint32t', 'SAUINT32T', []),
            ('sauint64t', 'SAUINT64T', [])
            ])
        immom.ccb_apply()
        immom.ccb_finalize()
        a = filter(nonemptyattr, immom.getobject(modobject))
        a.sort()
        self.assertEqual(
            a, [
            ('TestClassModId', 'SANAMET', ['TestClassModId=1'])
            ])

        # Clean-up
        immom.ccb_initialize()
        immom.deletesubtree(self.topobject)
        immom.ccb_apply()
        immom.ccb_finalize()
        immom.deleteclass('TestClassMod')



if __name__ == '__main__':
    unittest.main()
