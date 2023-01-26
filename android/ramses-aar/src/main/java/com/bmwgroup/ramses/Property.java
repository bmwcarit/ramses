//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses;

import org.jetbrains.annotations.Nullable;

import java.util.NoSuchElementException;

/**
 * This class enables the user to manipulate individual logic node properties to influence a
 * ramses scene during runtime. A property can be an input or output (readonly) property of a lua script or
 * a RamsesBinding (RamsesNodeBinding, RamsesAppearanceBinding, RamsesCameraBinding).
 * To get an overview about how properties fit into the picture refer to
 * https://ramses-logic.readthedocs.io/en/latest/api.html#overview
 * There is no special memory management needed for underlying native c++ objects.
 *
 * To get the value of a property the specific getter with the matching type has to be called so
 * e.g. getInt() for a property with an int value.
 * The setters are bit different. For primitive types and string the setter is generic and the user
 * just calls set(T value) with the type matching the value type of the property. So e.g.
 * set(10) works for a int value type property.
 * Setting vectors float or int the user has to pass float elements separately. So e.g. for setting
 * a vec2i one calls set(10, 11).
 * Setters on output properties don't work! They will throw an exception as output properties are readonly.
 *
 * Remember that after the referenced RamsesBundle got disposed you should not use this
 * Property anymore as this will result in an IllegalStateException.
 */
public class Property {
    Property(long nativeRlogicPropertyHandle, RamsesBundle ramsesBundle, boolean isOutput) {
        m_nativeRlogicPropertyHandle = nativeRlogicPropertyHandle;
        m_ramsesBundle = ramsesBundle;
        m_isOutput = isOutput;
    }

    /**
     * Gets the child of this property by the given name
     * @param childName name of the child to get
     * @return child property with the given name
     * @throws NoSuchElementException if the property doesn't have a child with the given name
     */
    public Property getChild(String childName) {
        ensureRamsesBundleNativeObjectCreated();
        long childPropertyPointer = getChildByName(m_nativeRlogicPropertyHandle, childName);
        if (childPropertyPointer == 0) {
            throw new NoSuchElementException("The child with name " + childName + " doesn't exist!");
        }

        return new Property(childPropertyPointer, m_ramsesBundle, m_isOutput);
    }

    /**
     * Gets the child of this property by the given index
     * @param index index of the child to get
     * @return child property with the given index
     * @throws NoSuchElementException if the property doesn't have a child with the given index
     */
    public Property getChild(int index) {
        ensureRamsesBundleNativeObjectCreated();
        long childPropertyPointer = getChildByIndex(m_nativeRlogicPropertyHandle, index);
        if (childPropertyPointer == 0) {
            throw new NoSuchElementException("The child at index " + index + " doesn't exist!");
        }

        return new Property(childPropertyPointer, m_ramsesBundle, m_isOutput);
    }

    /**
     * Gets the child count of this property
     * @return child count
     */
    public int getChildCount() {
        ensureRamsesBundleNativeObjectCreated();
        return getChildCount(m_nativeRlogicPropertyHandle);
    }

    /**
     * Tells whether this property has a child by the given name
     * @param childName the name of the child that should be checked for existence
     * @return whether the child exists
     */
    public boolean hasChild(String childName) {
        ensureRamsesBundleNativeObjectCreated();
        return hasChild(m_nativeRlogicPropertyHandle, childName);
    }

    /**
     * Get the int value of this property
     * @return int value
     * @throws PropertyTypeMismatchException if the value of the property is not int
     */
    public int getInt() {
        ensureRamsesBundleNativeObjectCreated();
        Integer potentialPropertyValue = getInputPropertyValueInt(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get the float value of this property
     * @return float value
     * @throws PropertyTypeMismatchException if the value of the property is not float
     */
    public float getFloat() {
        ensureRamsesBundleNativeObjectCreated();
        Float potentialPropertyValue = getInputPropertyValueFloat(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get boolean value of this property
     * @return boolean value
     * @throws PropertyTypeMismatchException if the value of the property is not boolean
     */
    public boolean getBoolean() {
        ensureRamsesBundleNativeObjectCreated();
        Boolean potentialPropertyValue = getInputPropertyValueBool(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get String value of this property
     * @return String value
     * @throws PropertyTypeMismatchException if the value of the property is not String
     */
    public String getString() {
        ensureRamsesBundleNativeObjectCreated();
        String potentialPropertyValue = getInputPropertyValueString(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec2i value of this property
     * @return int array of length 2
     * @throws PropertyTypeMismatchException if the value of the property is not vec2i
     */
    public int[] getVec2i() {
        ensureRamsesBundleNativeObjectCreated();
        int[] potentialPropertyValue = getInputPropertyValueVec2i(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec3i value of this property
     * @return int array of length 3
     * @throws PropertyTypeMismatchException if the value of the property is not vec3i
     */
    public int[] getVec3i() {
        ensureRamsesBundleNativeObjectCreated();
        int[] potentialPropertyValue = getInputPropertyValueVec3i(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec4i value of this property
     * @return int array of length 4
     * @throws PropertyTypeMismatchException if the value of the property is not vec4i
     */
    public int[] getVec4i() {
        ensureRamsesBundleNativeObjectCreated();
        int[] potentialPropertyValue = getInputPropertyValueVec4i(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec2f value of this property
     * @return float array of length 2
     * @throws PropertyTypeMismatchException if the value of the property is not vec2f
     */
    public float[] getVec2f() {
        ensureRamsesBundleNativeObjectCreated();
        float[] potentialPropertyValue = getInputPropertyValueVec2f(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec3f value of this property
     * @return float array of length 3
     * @throws PropertyTypeMismatchException if the value of the property is not vec3f
     */
    public float[] getVec3f() {
        ensureRamsesBundleNativeObjectCreated();
        float[] potentialPropertyValue = getInputPropertyValueVec3f(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get vec4f value of this property
     * @return float array of length 4
     * @throws PropertyTypeMismatchException if the value of the property is not vec4f
     */
    public float[] getVec4f() {
        ensureRamsesBundleNativeObjectCreated();
        float[] potentialPropertyValue = getInputPropertyValueVec4f(m_nativeRlogicPropertyHandle);
        if (potentialPropertyValue == null) {
            throw new PropertyTypeMismatchException("Trying to access Property value with invalid type");
        }
        return potentialPropertyValue;
    }

    /**
     * Get the name of the Property.
     * <p>
     * As the property name is not mandatory, this will return null if the property has no name. Array elements for example have no name.
     * @return this Properties name or null if the property has no name
     */
    public @Nullable String getName() {
        ensureRamsesBundleNativeObjectCreated();
        return getName(m_nativeRlogicPropertyHandle);
    }

    /**
     * Make sure the native object of the RamsesBundle that is referenced by this property is
     * not already disposed to avoid calling a functions on a null object
     * @throws IllegalStateException if the referenced RamsesBundle is already disposed
     */
    private void ensureRamsesBundleNativeObjectCreated() {
        if (m_ramsesBundle.nativeObjectDisposed()) {
            throw new IllegalStateException("Using Property after parent RamsesBundle was disposed!");
        }
    }

    private void ensureIsWritable() {
        if (m_isOutput) {
            throw new IllegalStateException("This method cannot be used on Outputs as they are readonly!");
        }
    }

    /**
     * Set the value to this property with type T
     * @param value the value the property should be set to
     * @param <T> the type of the value
     * @throws IllegalArgumentException if the type of the value is unsupported or if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public <T> void set(T value) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();

        boolean failed = false;
        if (value instanceof Integer) {
            if (!setInputPropertyValueInt(m_nativeRlogicPropertyHandle, (Integer) value)) {
                failed = true;
            }
        }
        else if (value instanceof Float) {
            if (!setInputPropertyValueFloat(m_nativeRlogicPropertyHandle, (Float) value)) {
                failed = true;
            }
        }
        else if (value instanceof Boolean) {
            if (!setInputPropertyValueBool(m_nativeRlogicPropertyHandle, (Boolean) value)) {
                failed = true;
            }
        }
        else if (value instanceof String) {
            if (!setInputPropertyValueString(m_nativeRlogicPropertyHandle, (String) value)) {
                failed = true;
            }
        }
        else{
            throw new IllegalArgumentException("Trying to set Property value with unsupported data type"
                    + value.getClass().getSimpleName());
        }
        if (failed) {
            throw new IllegalArgumentException("Setting Property value failed. Please look into"
                    + " ramses logic logs to find the exact problem; Caused by method: "
                    + value.getClass().getSimpleName());
        }
    }

    /**
     * Set the vec2i value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public void set(int value1, int value2) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if (!setInputPropertyValueVec2i(m_nativeRlogicPropertyHandle, value1, value2)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Set the vec3i value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @param value3 third vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public void set(int value1, int value2, int value3) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if (!setInputPropertyValueVec3i(m_nativeRlogicPropertyHandle, value1, value2, value3)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Set the vec4i value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @param value3 third vector element
     * @param value4 fourth vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public void set(int value1, int value2, int value3, int value4) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if(!setInputPropertyValueVec4i(m_nativeRlogicPropertyHandle, value1, value2, value3, value4)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Set the vec2f value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */

    public void set(float value1, float value2) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if (!setInputPropertyValueVec2f(m_nativeRlogicPropertyHandle, value1, value2)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Set the vec3f value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @param value3 third vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public void set(float value1, float value2, float value3) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if (!setInputPropertyValueVec3f(m_nativeRlogicPropertyHandle, value1, value2, value3)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Set the vec4f value to this property
     * @param value1 first vector element
     * @param value2 second vector element
     * @param value3 second vector element
     * @param value4 second vector element
     * @throws IllegalArgumentException if setting a linked Property, a non primitive (struct or array) or with a mismatching value type.
     */
    public void set(float value1, float value2, float value3, float value4) {
        ensureIsWritable();
        ensureRamsesBundleNativeObjectCreated();
        if (!setInputPropertyValueVec4f(m_nativeRlogicPropertyHandle, value1, value2, value3, value4)) {
            throw new IllegalArgumentException("Setting Property values failed. Please look into"
                    + " ramses logic logs to find the exact problem");
        }
    }

    /**
     * Checks if the property is linked
     * <p>
     * It is not possible to link already linked input properties before unlinking first. Also it's not possible to set the
     * values of properties which are linked. Use this method to check first!
     * </p>
     * @return true if the property is linked
     */
    public boolean isLinked() {
        ensureRamsesBundleNativeObjectCreated();
        return isLinked(m_nativeRlogicPropertyHandle);
    }

    long getNativeHandle() {
        return m_nativeRlogicPropertyHandle;
    }

    private final RamsesBundle m_ramsesBundle;
    private final long m_nativeRlogicPropertyHandle;
    private final boolean m_isOutput;

    private native long getChildByName(long handle, String childName);
    private native long getChildByIndex(long handle, int index);
    private native int getChildCount(long handle);
    private native boolean hasChild(long handle, String childName);
    private native String getName(long handle);
    private native boolean isLinked(long handle);

    private native Integer getInputPropertyValueInt(long handle);
    private native Float getInputPropertyValueFloat(long handle);
    private native Boolean getInputPropertyValueBool(long handle);
    private native String getInputPropertyValueString(long handle);
    private native int[] getInputPropertyValueVec2i(long handle);
    private native int[] getInputPropertyValueVec3i(long handle);
    private native int[] getInputPropertyValueVec4i(long handle);
    private native float[] getInputPropertyValueVec2f(long handle);
    private native float[] getInputPropertyValueVec3f(long handle);
    private native float[] getInputPropertyValueVec4f(long handle);

    private native boolean setInputPropertyValueInt(long handle, int value);
    private native boolean setInputPropertyValueFloat(long handle, float value);
    private native boolean setInputPropertyValueBool(long handle, boolean value);
    private native boolean setInputPropertyValueString(long handle, String value);
    private native boolean setInputPropertyValueVec2i(long handle, int value1, int value2);
    private native boolean setInputPropertyValueVec3i(long handle, int value1, int value2, int value3);
    private native boolean setInputPropertyValueVec4i(long handle, int value1, int value2, int value3, int value4);
    private native boolean setInputPropertyValueVec2f(long handle, float value1, float value2);
    private native boolean setInputPropertyValueVec3f(long handle, float value1, float value2, float value3);
    private native boolean setInputPropertyValueVec4f(long handle, float value1, float value2, float value3, float value4);

    /**
     * This exception is thrown if an action on a property is performed that requires a
     * different property type (e.g. getInt on float property)
     */
    public class PropertyTypeMismatchException extends RuntimeException {
        public PropertyTypeMismatchException(String message) {
            super(message);
        }
    }
}
