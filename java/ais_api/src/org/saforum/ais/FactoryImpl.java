/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
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
package org.saforum.ais;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Properties;

/**
 * Abstract Factory (and shared Impl) for creating AIS service handles in a
 * generic way. The following system properties will be defined by the platform:
 * <dl>
 * <dt><tt>SAF_AIS_&lt;AREA&gt;_IMPL_CLASSNAME</tt> (mandatory)</dt>
 * <dd>The fully qualified name of the implementation class that creates the
 * area library handle.</dd>
 * <dt><tt>SAF_AIS_&lt;AREA&gt;_IMPL_URL</tt> (optional)</dt>
 * <dd>If defined, the specified url path is used to locate the implementation
 * class. If not defined, the default class loader tries to load the
 * implementation class.</dd>
 * </dl>
 * By defining the above system properties, the platform ensures that client code
 * using these APIs for AIS services will be able to use the FactoryImpl class
 * without having to know or specify the name/path of the actual implementation
 * classes.
 * <P>
 * Whether the client code can override any of the above properties depends on
 * whether these properties are protected by a security manager. This should be
 * a platform decision. If the above properties are not protected, then the client
 * can override them and plug in its own implementation. We expect this only
 * available for testing purposes. If these properties are protected, then the
 * platform-defined properties will always be used. We expect this as the usual
 * behavior in real high availability clusters.
 *
 *
 * @version AIS-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AIS-B.01.01
 *
 */
public abstract class FactoryImpl<S extends Handle, C extends Callbacks> implements
        Factory<S, C> {

    private static final String PROP_PREFIX = "SAF_AIS_";
    private static final String PROP_POSTFIX_CLASSNAME = "_IMPL_CLASSNAME";
    private static final String PROP_POSTFIX_URL = "_IMPL_URL";
    private static final String ROOT_PACKAGE = "org.saforum.ais.";
    private static final String CALLBACKS_CLASSNAME = "Handle$Callbacks";

    /**
     * property key for the fully qualified classname of the implementation
     */
    private final String classNameProp;

    /**
     * property key for the URL to be used by the classloader if the impl class
     * can not be otherwise found
     */
    private final String classURLProp;

    /**
     *
     */
    private final String fqCallbackClassName;

    /**
     * properties (usually with defaults) that are used for the search
     */
    private final Properties factoryProps;

    /**
     * Creates a factory able to initialize an AIS service.
     *
     */
    public FactoryImpl() {
        /**
         * build the defaults
         */
        String _simpleName = this.getClass().getSimpleName();
        String _Area = _simpleName.substring( 0, _simpleName.indexOf("HandleFactory") );
        String _AREA = _Area.toUpperCase();
        classNameProp = PROP_PREFIX + _AREA + PROP_POSTFIX_CLASSNAME;
        classURLProp = PROP_PREFIX + _AREA + PROP_POSTFIX_URL;
        fqCallbackClassName = ROOT_PACKAGE + _Area.toLowerCase() + "." + _Area + CALLBACKS_CLASSNAME;

        /**
         * build the relevant system properties
         */
        factoryProps = new Properties();
        String _s;

        _s = System.getProperty(classNameProp);
        if (_s == null) {
            _s = ""; // NOTE: This will inevitably cause an exception later!
        }
        factoryProps.setProperty(classNameProp, _s);

        _s = System.getProperty(classURLProp);
        if (_s == null) {
            _s = ""; // NOTE: This can still work!
        }
        factoryProps.setProperty(classURLProp, _s);

    }

    /*
     * <P><B>An implementation note</B> (i.e. this behaviour is not part of the
     * method contract!): The implementation class must define the following method:
     * <UL>
     * <LI><tt>public static S initializeHandle( C, org.saforum.ais.Version )</tt>,
     * where S must be an &lt;Area&gt;Handle and C must be an
     * &lt;Area&gt;Handle.Callbacks object.
     * </UL>
     */
    public S initializeHandle(C callbacks, Version version)
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException,
	    AisUnavailableException {

        String _classname = factoryProps.getProperty(classNameProp);
        String _url = factoryProps.getProperty(classURLProp);

        try {
            ClassLoader _loader;
            if( _url == "" ){
                _loader = ClassLoader.getSystemClassLoader();
            }
            else{
                URL[] _path = new URL[1];
                _path[0] = new URL(_url);
                _loader = new URLClassLoader(_path,Thread.currentThread().getContextClassLoader());
            }
            Class<? extends Object> _implClass = _loader.loadClass(_classname);
            Method _sm = _implClass.getMethod( "initializeHandle",
                                               Class.forName( fqCallbackClassName ),
                                               Class.forName( "org.saforum.ais.Version" )
                                          );
            S _s = (S) (_sm.invoke( null, callbacks, version));
            return _s;

        } catch (InvocationTargetException it) {
            Throwable _c = it.getCause();
            if (_c instanceof AisLibraryException) {
                throw (AisLibraryException) _c;
            }
            if (_c instanceof AisTimeoutException) {
                throw (AisTimeoutException) _c;
            }
            if (_c instanceof AisTryAgainException) {
                throw (AisTryAgainException) _c;
            }
            if (_c instanceof AisInvalidParamException) {
                throw (AisInvalidParamException) _c;
            }
            if (_c instanceof AisNoMemoryException) {
                throw (AisNoMemoryException) _c;
            }
            if (_c instanceof AisNoResourcesException) {
                throw (AisNoResourcesException) _c;
            }
            if (_c instanceof AisVersionException) {
                throw (AisVersionException) _c;
            }
            throw new AisLibraryException("Implementation factory internal error",_c);
        } catch (Exception e) {
            throw new AisLibraryException( e + ": could not load " + _classname, e);
        }
    }


}
