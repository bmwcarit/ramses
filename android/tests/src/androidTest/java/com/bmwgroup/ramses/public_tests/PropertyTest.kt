//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

package com.bmwgroup.ramses.public_tests

import com.bmwgroup.ramses.Property
import com.bmwgroup.ramses.RamsesBundle
import kotlin.math.ceil
import kotlin.test.*
import androidx.test.platform.app.InstrumentationRegistry.getInstrumentation as getInstrumentation

class PropertyTest {
    companion object {
        private lateinit var ramsesBundle: RamsesBundle

        private lateinit var intfFloatInput: Property
        private lateinit var scriptInputs: Property
        private lateinit var scriptOutputs: Property
    }

    @BeforeTest
    fun setupRamsesBundleAndScene() {
        ramsesBundle = RamsesBundle(null)
        val logicFD =
            getInstrumentation().context.resources.assets.openFd("testLogic.rlogic")
        val sceneFD =
            getInstrumentation().context.resources.assets.openFd("testScene.ramses")

        assertTrue(
            ramsesBundle.loadScene(
                sceneFD.parcelFileDescriptor,
                sceneFD.startOffset,
                logicFD.parcelFileDescriptor,
                logicFD.startOffset
            )
        )

        intfFloatInput =
            ramsesBundle.getLogicNodeRootInput("intf").getChild("struct").getChild("floatInput")
        scriptInputs = ramsesBundle.getLogicNodeRootInput("script1")
        scriptOutputs = ramsesBundle.getLogicNodeRootOutput("script1")
    }

    @AfterTest
    fun disposeRamsesBundle() {
        ramsesBundle.dispose()
    }

    @Test
    fun property_canGetChildByName() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)
        assertFailsWith<NoSuchElementException> { structInput.getChild("invalidChildName") }

        val floatOutput = scriptOutputs.getChild("floatOutput")
        assertNotNull(floatOutput)
    }

    @Test
    fun property_canGetNestedChildByNameAndSetData() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)

        val propNested = structInput.getChild("nested")
        assertNotNull(propNested)

        val propertyData2 = structInput.getChild("nested").getChild("data2")
        propertyData2.set(10)
        assertEquals(propertyData2.int, 10)
    }

    @Test
    fun property_canGetChildCount() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)
        assertEquals(structInput.childCount, 1)

        assertEquals(scriptOutputs.childCount, 2)
    }

    @Test
    fun property_canGetChildByIndex() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)
        val childCount = structInput.childCount
        val propNested = structInput.getChild(childCount - 1)
        assertNotNull(propNested)

        assertFailsWith<NoSuchElementException> { structInput.getChild(childCount) }

        val outputsChildCount = scriptOutputs.childCount
        assertNotNull(scriptOutputs.getChild(outputsChildCount - 1))
        assertFailsWith<NoSuchElementException> { scriptOutputs.getChild(outputsChildCount) }
    }

    @Test
    fun property_canGetNestedChildByIndexAndSetData() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)

        val propNested = structInput.getChild("nested")
        assertNotNull(propNested)

        val childCount = propNested.childCount
        val data2 = propNested.getChild(childCount - 1)
        assertNotNull(data2)
        data2.set(123)
        assertEquals(data2.int, 123)
    }

    @Test
    fun property_canCheckIfPropertyHasChild() {
        val structInput = scriptInputs.getChild("structInput").getChild("nested")
        assertNotNull(structInput)

        assertTrue(structInput.hasChild("data1"))
        assertTrue(structInput.hasChild("data2"))

        assertFalse(structInput.hasChild("invalidChildName"))
        assertFailsWith<NoSuchElementException> { structInput.getChild("invalidChildName") }

        assertTrue(scriptOutputs.hasChild("floatOutput"))
        assertFalse(scriptOutputs.hasChild("invalidChildName"))
        assertFailsWith<NoSuchElementException> { scriptOutputs.getChild("invalidChildName") }
    }

    @Test
    fun property_cannotSetOutputs() {
        val floatOutput = scriptOutputs.getChild("floatOutput")
        assertNotNull(floatOutput)
        assertFailsWith<java.lang.IllegalStateException> { floatOutput.set(11.4f) }
    }

    @Test
    fun property_canGetAndSetArrayData() {
        val arrayInput = scriptInputs.getChild("arrayInput")
        assertNotNull(arrayInput)
        val childCount = arrayInput.childCount
        assertEquals(childCount, 9)

        val nestedArrayProp = arrayInput.getChild(childCount - 1)
        assertNotNull(nestedArrayProp)

        nestedArrayProp.set(12.0f)
        assertEquals(nestedArrayProp.float, 12.0f, 0.0f)
        assertEquals(nestedArrayProp.float, 12.0f, 0.0f)

        assertFailsWith<NoSuchElementException> { nestedArrayProp.getChild(childCount) }
    }

    @Test
    fun property_gettersAndSettersThrowExceptionOnUsageAfterRamsesBundleDisposed() {
        val structInput = scriptInputs.getChild("structInput")
        val propertyData1 = structInput.getChild("nested").getChild("data1")
        propertyData1.set("test")

        ramsesBundle.dispose()
        assertTrue(ramsesBundle.nativeObjectDisposed())

        assertFailsWith<IllegalStateException> { propertyData1.set("test") }

        assertFailsWith<IllegalStateException> { propertyData1.string }

        assertFailsWith<IllegalStateException> { structInput.getChild("nested") }

        assertFailsWith<IllegalStateException> { structInput.getChild(0) }

        assertFailsWith<IllegalStateException> { structInput.childCount }
    }

    @Test
    fun property_canGetSetInt() {
        val inputInt = scriptInputs.getChild("intInput")
        inputInt.set(17)
        assertEquals(inputInt.int, 17)

        assertFailsWith<IllegalArgumentException> { inputInt.set(35.0f) }
        assertNotEquals(inputInt.int, 35)

        assertFailsWith<Property.PropertyTypeMismatchException> { inputInt.float }
    }

    @Test
    fun property_canGetSetFloat() {
        intfFloatInput.set(11.3f)
        assertEquals(intfFloatInput.float, 11.3f, 0.0f)

        assertFailsWith<IllegalArgumentException> { intfFloatInput.set(22) }
        assertNotEquals(intfFloatInput.float, 22.0f)

        assertFailsWith<Property.PropertyTypeMismatchException> { intfFloatInput.boolean }
    }

    @Test
    fun property_canGetSetBool() {
        val boolInput = scriptInputs.getChild("boolInput")
        boolInput.set(true)
        assertTrue(boolInput.boolean)

        assertFailsWith<IllegalArgumentException> { boolInput.set(22) }
        assertTrue(boolInput.boolean)

        assertFailsWith<Property.PropertyTypeMismatchException> { boolInput.string }
    }

    @Test
    fun property_canGetSetString() {
        val stringInput = scriptInputs.getChild("stringInput")
        stringInput.set("testString")
        assertEquals(stringInput.string, "testString")

        assertFailsWith<IllegalArgumentException> { stringInput.set(22) }
        assertEquals(stringInput.string, "testString")

        assertFailsWith<Property.PropertyTypeMismatchException> { stringInput.int }
    }

    @Test
    fun property_canGetSetVec2f() {
        val vec2fInput = scriptInputs.getChild("vec2fInput")
        vec2fInput.set(11.3f, 43.4f)
        assertContentEquals(vec2fInput.vec2f, floatArrayOf(11.3f, 43.4f))

        assertFailsWith<IllegalArgumentException> { vec2fInput.set(22.0f) }

        assertFailsWith<IllegalArgumentException> { vec2fInput.set(10, 10) }

        assertContentEquals(vec2fInput.vec2f, floatArrayOf(11.3f, 43.4f))
        assertFailsWith<Property.PropertyTypeMismatchException> { vec2fInput.vec2i }
    }

    @Test
    fun property_canGetSetVec3f() {
        val vec3fInput = scriptInputs.getChild("vec3fInput")
        vec3fInput.set(11.3f, 43.4f, 10.0f)
        assertContentEquals(vec3fInput.vec3f, floatArrayOf(11.3f, 43.4f, 10.0f))

        assertFailsWith<IllegalArgumentException> { vec3fInput.set(22.0f) }

        assertFailsWith<IllegalArgumentException> { vec3fInput.set(10, 10, 10) }

        assertContentEquals(vec3fInput.vec3f, floatArrayOf(11.3f, 43.4f, 10.0f))

        assertFailsWith<Property.PropertyTypeMismatchException> { vec3fInput.vec2i }
    }

    @Test
    fun property_canGetSetVec4f() {
        val vec4fInput = scriptInputs.getChild("vec4fInput")
        vec4fInput.set(11.3f, 43.4f, 10.0f, 56.3f)
        assertContentEquals(vec4fInput.vec4f, floatArrayOf(11.3f, 43.4f, 10.0f, 56.3f))

        assertFailsWith<IllegalArgumentException> { vec4fInput.set(22.0f) }

        assertFailsWith<IllegalArgumentException> { vec4fInput.set(10, 10, 10, 10) }

        assertContentEquals(vec4fInput.vec4f, floatArrayOf(11.3f, 43.4f, 10.0f, 56.3f))

        assertFailsWith<Property.PropertyTypeMismatchException> { vec4fInput.vec4i }
    }

    @Test
    fun property_canGetSetVec2i() {
        val vec2iInput = scriptInputs.getChild("vec2iInput")
        vec2iInput.set(11, 43)
        assertContentEquals(vec2iInput.vec2i, intArrayOf(11, 43))

        assertFailsWith<IllegalArgumentException> { vec2iInput.set(22.0f) }

        assertFailsWith<IllegalArgumentException> { vec2iInput.set(10.0f, 10.0f) }

        assertContentEquals(vec2iInput.vec2i, intArrayOf(11, 43))

        assertFailsWith<Property.PropertyTypeMismatchException> { vec2iInput.vec3i }
    }

    @Test
    fun property_canGetSetVec3i() {
        val vec3iInput = scriptInputs.getChild("vec3iInput")
        vec3iInput.set(11, 43, 12)
        assertContentEquals(vec3iInput.vec3i, intArrayOf(11, 43, 12))

        assertFailsWith<IllegalArgumentException> { vec3iInput.set(22, 33, 33, 43) }

        assertFailsWith<IllegalArgumentException> { vec3iInput.set(10.0f, 10.0f, 10.0f) }

        assertContentEquals(vec3iInput.vec3i, intArrayOf(11, 43, 12))

        assertFailsWith<Property.PropertyTypeMismatchException> { vec3iInput.vec3f }
    }

    @Test
    fun property_canGetSetVec4i() {
        val vec4iInput = scriptInputs.getChild("vec4iInput")
        vec4iInput.set(11, 43, 12, 65)
        assertContentEquals(vec4iInput.vec4i, intArrayOf(11, 43, 12, 65))

        assertFailsWith<IllegalArgumentException> { vec4iInput.set(22, 33, 33) }

        assertFailsWith<IllegalArgumentException> { vec4iInput.set(10.0f, 10.0f, 10.0f, 10.0f) }

        assertContentEquals(vec4iInput.vec4i, intArrayOf(11, 43, 12, 65))

        assertFailsWith<Property.PropertyTypeMismatchException> { vec4iInput.vec4f }
    }

    @Test
    fun property_settingPropertyValueWithUnsupportedDataTypeFails() {
        assertFailsWith<IllegalArgumentException> { intfFloatInput.set(213.12) }
    }

    @Test
    fun property_canGetSetNodeBinding() {
        val nodeBinding = ramsesBundle.getLogicNodeRootInput("nodebinding")
        assertNotNull(nodeBinding)

        val translation = nodeBinding.getChild("translation")
        assertNotNull(translation)

        assertContentEquals(translation.vec3f, floatArrayOf(0.0f, 2.0f, 3.0f))
        val newFloatInputValue = 5.3f
        intfFloatInput.set(newFloatInputValue)

        assertTrue(ramsesBundle.updateLogic())
        assertTrue(ramsesBundle.flushRamsesScene())
        assertContentEquals(translation.vec3f, floatArrayOf(newFloatInputValue, 2.0f, 3.0f))
    }

    @Test
    fun property_canGetSetAppearanceBinding() {
        val appearanceBinding = ramsesBundle.getLogicNodeRootInput("appearancebinding")
        assertNotNull(appearanceBinding)
        val floatUniform = appearanceBinding.getChild("floatUniform")
        assertNotNull(floatUniform)

        assertEquals(floatUniform.float, 5.0f, 0.0f)
        val newFloatInputValue = 4.3f
        intfFloatInput.set(newFloatInputValue)
        assertTrue(ramsesBundle.updateLogic())
        assertTrue(ramsesBundle.flushRamsesScene())
        assertEquals(floatUniform.float, 5.0f + newFloatInputValue, 0.0f)
    }

    @Test
    fun property_canGetSetCameraBinding() {
        val cameraBinding = ramsesBundle.getLogicNodeRootInput("camerabinding")
        assertNotNull(cameraBinding)
        val cameraVpWidth = cameraBinding.getChild("viewport").getChild("width")
        assertNotNull(cameraVpWidth)

        assertEquals(cameraVpWidth.int, 100)
        val newFloatInputValue = 9.3f
        intfFloatInput.set(newFloatInputValue)
        assertTrue(ramsesBundle.updateLogic())
        assertTrue(ramsesBundle.flushRamsesScene())
        assertEquals(cameraVpWidth.int, 100 + ceil(newFloatInputValue).toInt())
    }

    @Test
    fun property_canGetSetAnimationNode() {
        val animationInputs = ramsesBundle.getLogicNodeRootInput("animNode")
        assertNotNull(animationInputs)

        assertTrue(ramsesBundle.updateLogic())
        assertTrue(ramsesBundle.flushRamsesScene())

        // Value with default state (progress == 0, keyframes = [1, 2]
        val animationOutputValue =
            ramsesBundle.getLogicNodeRootOutput("animNode").getChild("channel")
        assertNotNull(animationOutputValue)
        assertEquals(1.0f, animationOutputValue.float, 0.0f)

        val progress = animationInputs.getChild("progress")
        assertNotNull(progress)
        progress.set(1.0f)
        assertEquals(1.0f, progress.float, 0.0f)

        assertTrue(ramsesBundle.updateLogic())
        assertTrue(ramsesBundle.flushRamsesScene())

        // Progress = 1.0 -> animated value is 2.0f
        assertEquals(2.0f, animationOutputValue.float, 0.0f)
    }

    @Test
    fun property_canGetName() {
        val structInput = scriptInputs.getChild("structInput")
        assertNotNull(structInput)
        assertEquals("structInput", structInput.name)
        assertEquals(structInput.childCount, 1)
        assertEquals("nested", structInput.getChild(0).name)
    }

    @Test
    fun property_getNameReturnsNullIfPropertyHasNoName() {
        val arrayInput = scriptInputs.getChild("arrayInput")
        assertNotNull(arrayInput)
        val childCount = arrayInput.childCount
        assertEquals(childCount, 9)

        val nestedArrayProp = arrayInput.getChild(childCount - 1)
        assertNotNull(nestedArrayProp)
        assertNull(nestedArrayProp.name)
    }

    // TODO Daniel add test for islinked
}
