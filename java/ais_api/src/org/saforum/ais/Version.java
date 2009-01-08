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

/**
 * The Version type is used to represent software versions of area
 * implementations. Application components can use instances of this type to
 * request compatibility with a particular version of an SA Forum Application
 * Interface area specification. The area referred to is implicit in this API.
 *
 * <P><B>SAF Reference:</B> <code>SaVersionT</code>
 * @version AIS-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AIS-B.01.01
 *
 */
public class Version implements java.io.Serializable {

    /**
     * tracks javadoc version numbers when structural changes occur
     */
    private static final long serialVersionUID = 5L;

    /**
     * The release code is a single ASCII capital letter [A-Z]. All
     * specifications and implementations with the same release code are
     * backward compatible. It is expected that the release code will change
     * very infrequently. Release codes are assigned exclusively by the SA
     * Forum.
     */
    public char releaseCode;

    /**
     * The major version is a number in the range [01..255]. An area
     * implementation with a given major version number implies compliance to
     * the interface specification bearing the same release code and major
     * version number. Changes to a specification requiring a revision of the
     * major version number are expected to occur at most once or twice a year
     * for the first few years, becoming less frequent as time goes on. Major
     * version numbers are assigned exclusively by the SA Forum.
     */
    public short majorVersion;

    /**
     * The minor version is a number in the range [01..255]. Successive updates
     * to an area implementation complying to an area interface specification
     * bearing the same release code and major version number have increasing
     * minor version number starting from 01. Increasing minor version numbers
     * only refer to enhancements of the implementation, like better performance
     * or bug fixes. Different values of the minor version may not impact the
     * compatibility and are not used to check whether required and supported
     * versions are compatible.
     * <P>
     * Successive updates to an area interface specification with the same
     * release code and major version number will also have increasing minor
     * version numbers starting from 01. However, such changes to a
     * specification are limited to editorial changes that do not impose changes
     * on any software implementations for the sake of compliance. Minor version
     * numbers are assigned independently by the SA Forum for interface
     * specifications and by members and licensed implementors for their
     * implementations.
     */
    public short minorVersion;

    /**
     * Creates a Version object with the specified parameters. Please note that
     * the validity of the specified version parameters is checked only when
     * this object is passed to an &lt;Area&gt;Factory.
     */
    public Version(char releaseCode, short majorVersion, short minorVersion) {
        this.releaseCode = releaseCode;
        this.majorVersion = majorVersion;
        this.minorVersion = minorVersion;
    }
}
