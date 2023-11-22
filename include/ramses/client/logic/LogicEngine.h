//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/ERotationType.h"
#include "ramses/client/SceneObject.h"
#include "ramses/client/logic/AnimationTypes.h"
#include "ramses/client/logic/Collection.h"
#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/logic/LogicEngineReport.h"
#include "ramses/client/logic/LuaConfig.h"
#include "ramses/client/logic/PropertyLink.h"
#include "ramses/client/logic/ELuaSavingMode.h"
#include "ramses/framework/DataTypes.h"
#include "ramses/framework/EFeatureLevel.h"

#include <vector>
#include <string_view>

namespace ramses
{
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;
    class RenderBuffer;
    class LogicObject;
}

namespace ramses::internal
{
    class LogicEngineImpl;
}

/**
 * @defgroup LogicAPI The Ramses Logic API
 * This group contains all of the Ramses Logic API types.
 */

namespace ramses
{
    class LogicNode;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
    class Property;
    class NodeBinding;
    class AppearanceBinding;
    class CameraBinding;
    class RenderPassBinding;
    class RenderGroupBinding;
    class RenderGroupBindingElements;
    class MeshNodeBinding;
    class SkinBinding;
    class RenderBufferBinding;
    class DataArray;
    class AnimationNode;
    class AnimationNodeConfig;
    class TimerNode;
    class AnchorPoint;

    /**
     * Logging mode for the periodic statistics.
     */
    enum class EStatisticsLogMode
    {
        Compact,   ///< general statistics about update, total time, number of nodes executed, etc.
        Detailed   ///< additionally logs slowest nodes with individual times (this might add slight performance overhead of measuring)
    };

    /**
    * @ingroup LogicAPI
    * Central object which creates and manages the lifecycle and execution
    * of scripts, bindings, and all other objects supported by the Ramses Logic library.
    *
    * - Use the create[Type] methods to create various objects, use #destroy() to delete them.
    * - Use #link and #unlink to connect data properties between these objects
    * - use #update() to trigger the execution of all objects
    */
    class RAMSES_API LogicEngine : public SceneObject
    {
    public:
        /**
        * Returns an iterable #ramses::Collection of all instances of \c T created by this #LogicEngine.
        * \c T must be a concrete logic object type (e.g. #ramses::LuaScript) or #ramses::LogicObject which will retrieve
        * all logic objects created with this #LogicEngine (see #ramses::LogicObject::as<Type> to convert to concrete type).
        *
        * @return an iterable #ramses::Collection with all instances of \c T created by this #LogicEngine
        */
        template <typename T>
        [[nodiscard]] Collection<T> getCollection() const;

        /**
        * Returns a pointer to the first occurrence of an object with a given \p name and type \c T (#ramses::LogicObject by default).
        * \c T must be a concrete logic object type (e.g. #ramses::LuaScript) or #ramses::LogicObject which will search
        * any object with given name regardless of its type (see #ramses::LogicObject::as<Type> to convert to concrete type).
        * Note that giving a concrete object template type might result in faster search because it is limited to only objects of that type.
        *
        * @param name the name of the logic object to search for
        * @return a pointer to the logic object, or nullptr if none was found
        */
        template <typename T = LogicObject>
        [[nodiscard]] const T* findObject(std::string_view name) const;

        /** @copydoc findObject(std::string_view) const */
        template <typename T = LogicObject>
        [[nodiscard]] T* findObject(std::string_view name);

        /**
        * Returns a pointer to an object with a given #ramses::sceneObjectId_t and type \c T (#ramses::LogicObject by default).
        * \c T must be a concrete logic object type (e.g. #ramses::LuaScript) or #ramses::LogicObject which will search
        * any object with given ID regardless of its type (see #ramses::LogicObject::as<Type> to convert to concrete type).
        * Note that giving a concrete object template type might result in faster search because it is limited to only objects of that type.
        *
        * @param id The id of the object to find.
        * @return a pointer to the logic object, or nullptr if none was found
        */
        template <typename T = LogicObject>
        [[nodiscard]] T* findObject(sceneObjectId_t id);

        /**
        * @copydoc findObject(sceneObjectId_t id)
        **/
        template <typename T = LogicObject>
        [[nodiscard]] const T* findObject(sceneObjectId_t id) const;

        /**
        * Creates a new Lua script from a source string. Refer to the #ramses::LuaScript class
        * for requirements which Lua scripts must fulfill in order to be added to the #LogicEngine.
        * You can optionally provide Lua module dependencies via the \p config, they will be accessible
        * under their configured alias name for use by the script. The provided module dependencies
        * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
        *
        * @param source the Lua source code
        * @param config configuration options, e.g. for module dependencies
        * @param scriptName name to assign to the script once it's created
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        * The script can be destroyed by calling the #destroy method
        */
        LuaScript* createLuaScript(
            std::string_view source,
            const LuaConfig& config = {},
            std::string_view scriptName = "");

        /**
        * Creates a new Lua interface from a source string. Refer to the #ramses::LuaInterface class
        * for requirements which Lua interface must fulfill in order to be added to the #LogicEngine.
        * Note: interfaces must have a non-empty name.
        * #ramses::LuaInterface can be created with any non-empty name but if two or more instances
        * share same name there will be a warning during validation (#validate) as it is advised
        * for every Lua interface to have a unique name for clear identification from application logic.
        * You can optionally provide Lua module dependencies via the \p config, they will be accessible
        * under their configured alias name for use in the interface source. The provided module dependencies
        * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
        *
        * @param source the Lua source code
        * @param interfaceName name to assign to the interface once it's created. This name must be unique!
        * @param config configuration options, e.g. for module dependencies
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        * The interface can be destroyed by calling the #destroy method
        */
        LuaInterface* createLuaInterface(
            std::string_view source,
            std::string_view interfaceName,
            const LuaConfig& config = {});

        /**
         * Creates a new #ramses::LuaModule from Lua source code.
         * LuaModules can be used to share code and data constants across scripts or
         * other modules. See also #createLuaScript and #ramses::LuaConfig for details.
         * You can optionally provide Lua module dependencies via the \p config, they will be accessible
         * under their configured alias name for use by the module. The provided module dependencies
         * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
         *
         * @param source module source code
         * @param config configuration options, e.g. for module dependencies
         * @param moduleName name to assign to the module once it's created
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The script can be destroyed by calling the #destroy method
         */
        LuaModule* createLuaModule(
            std::string_view source,
            const LuaConfig& config = {},
            std::string_view moduleName = "");

        /**
         * Extracts dependencies from a Lua script, module or interface source code so that the corresponding
         * modules can be provided when creating #ramses::LuaScript, #ramses::LuaModule or #ramses::LuaInterface.
         *
         * Any #ramses::LuaScript, #ramses::LuaModule or #ramses::LuaInterface which has a module dependency,
         * i.e. it requires another #ramses::LuaModule for it to work, must explicitly declare these
         * dependencies directly in their source code by calling function 'modules' in global space
         * and pass list of module names it depends on, for example:
         *   \code{.lua}
         *       modules("foo", "bar")
         *       function interface(IN,OUT)
         *         OUT.x = foo.myType()
         *       end
         *       function run(IN,OUT)
         *         OUT.x = bar.doSth()
         *       end
         *   \endcode
         * The 'modules' function does not affect any other part of the source code in any way,
         * it is used only for the purpose of explicit declaration and extraction of its dependencies.
         *
         * Please note that script runtime errors are ignored during extraction. In case a runtime
         * error prevents the 'modules' function to be called, this method will still succeed
         * but will not extract any modules, i.e. will not call \c callbackFunc. It is therefore
         * highly recommended to put the modules declaration always at the beginning of every script
         * before any other code so it will get executed even if there is runtime error later in the code.
         *
         * @param source source code of module or script to parse for dependencies
         * @param callbackFunc function callback will be called for each dependency found
         * @return \c true if extraction succeeded (also if no dependencies found) or \c false if
         * something went wrong. In that case, use #ramses::RamsesFramework::getLastError.
         */
        bool extractLuaDependencies(
            std::string_view source,
            const std::function<void(const std::string&)>& callbackFunc);

        /**
         * Creates a new #ramses::NodeBinding which can be used to set the properties of a Ramses Node object.
         * The initial values of the binding's properties are loaded from the \p ramsesNode. Rotation values are
         * taken over from the \p ramsesNode only if the conventions are compatible (see \ref ramses::ERotationType).
         * The creation will fail if provided #ramses::Node is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesNode the ramses::Node object to control with the binding.
         * @param rotationType the type of rotation to use (will affect the 'rotation' property semantics and type).
         * @param name a name for the new #ramses::NodeBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        NodeBinding* createNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType = ramses::ERotationType::Euler_XYZ, std::string_view name = "");

        /**
         * Creates a new #ramses::AppearanceBinding which can be used to set the properties of a Ramses Appearance object.
         * The creation will fail if provided #ramses::Appearance is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesAppearance the ramses::Appearance object to control with the binding.
         * @param name a name for the the new #ramses::AppearanceBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        AppearanceBinding* createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name = "");

        /**
         * Creates a new #ramses::CameraBinding which can be used to set the properties of a Ramses Camera object.
         * The creation will fail if provided #ramses::Camera is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesCamera the ramses::Camera object to control with the binding.
         * @param name a name for the the new #ramses::CameraBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        CameraBinding* createCameraBinding(ramses::Camera& ramsesCamera, std::string_view name ="");

        /**
         * Same as #createCameraBinding but the created #ramses::CameraBinding will have an input property
         * for each frustum plane also for perspective camera. See #ramses::CameraBinding for details.
         * Note that ramses::OrthographicCamera binding will always have frustum planes as properties whether #createCameraBinding
         * or #createCameraBindingWithFrustumPlanes is used to create it.
         *
         * @param ramsesCamera the ramses::Camera object to control with the binding.
         * @param name a name for the the new #ramses::CameraBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        CameraBinding* createCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name ="");

        /**
         * Creates a new #ramses::RenderPassBinding which can be used to set the properties of a ramses::RenderPass object.
         * The creation will fail if provided #ramses::RenderPass is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesRenderPass the ramses::RenderPass object to control with the binding.
         * @param name a name for the the new #ramses::RenderPassBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        RenderPassBinding* createRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name ="");

        /**
         * Creates a new #ramses::RenderGroupBinding which can be used to control some properties of a ramses::RenderGroup object.
         * #ramses::RenderGroupBinding can be used to control render order of elements it contains on Ramses side - ramses::MeshNode or ramses::RenderGroup,
         * the elements to control must be provided explicitly at creation time, see #ramses::RenderGroupBindingElements and #ramses::RenderGroupBinding
         * to learn how these elements form the binding's input properties.
         * The creation will fail if provided #ramses::RenderGroup or the elements are not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesRenderGroup the ramses::RenderGroup object to control with the binding.
         * @param elements collection of elements (MeshNode or RenderGroup) to control with this #ramses::RenderGroupBinding.
         * @param name a name for the the new #ramses::RenderGroupBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        RenderGroupBinding* createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name ="");

        /**
         * Creates a new #ramses::MeshNodeBinding which can be used to control some properties of a ramses::MeshNode object.
         * The creation will fail if provided #ramses::MeshNode is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param ramsesMeshNode the ramses::MeshNode object to control with the binding.
         * @param name a name for the the new #ramses::MeshNodeBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        MeshNodeBinding* createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name ="");

        /**
         * Creates a new #ramses::SkinBinding which can be used for vertex skinning (bone animations).
         * Refer to #ramses::SkinBinding and examples for all the information needed how to use this object.
         *
         * These conditions must be met in order for the creation to succeed:
         *  - \c joints must contain at least 1 joint and no null pointers
         *  - \c inverseBindMatrices must be of equal size as \c joints (i.e. matrix per joint)
         *  - \c jointMatInput must point to a valid uniform input of the ramses::Effect used in \c appearanceBinding
         *  - the shader uniform that \c jointMatInput points to must be of type array of ramses::EDataType::Matrix44F
         *    with number of elements matching number of joints
         *  - \c jointMatInput must not be bound to any data object in its Ramses appearance
         * The creation will also fail if any of the provided Ramses objects or bindings are not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param joints bindings to Ramses nodes which will act as skeleton nodes.
         * @param inverseBindMatrices inverse transformation matrices (one for each joint node), values are expected ordered in column-major fashion.
         * @param appearanceBinding binding to Ramses appearance which specifies the effect/shader for vertex skinning.
         * @param jointMatInput Ramses appearance uniform input for the resulting joint matrices to be set.
         * @param name a name for the the new #ramses::SkinBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        SkinBinding* createSkinBinding(
            const std::vector<const NodeBinding*>& joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            AppearanceBinding& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name = {});

        /**
         * Creates a new #ramses::RenderBufferBinding which can be used to change some properties of a #ramses::RenderBuffer object.
         * The creation will fail if provided #ramses::RenderBuffer is not from same #ramses::Scene as this #LogicEngine instance.
         *
         * @param renderBuffer the #ramses::RenderBuffer object to control with the binding.
         * @param name a name for the the new #ramses::RenderBufferBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
         * The binding can be destroyed by calling the #destroy method
         */
        RenderBufferBinding* createRenderBufferBinding(ramses::RenderBuffer& renderBuffer, std::string_view name ="");

        /**
        * Creates a new #ramses::DataArray to store data which can be used with animations.
        * Provided data must not be empty otherwise creation will fail.
        * See #ramses::CanPropertyTypeBeStoredInDataArray and #ramses::PropertyTypeToEnum
        * to determine supported types that can be used to create a #ramses::DataArray.
        * When using std::vector<float> as element data type (corresponds to #ramses::EPropertyType::Array),
        * the sizes of all the elements (std::vector<float> instances) must be equal, otherwise creation will fail.
        *
        * @param data source data to move into #ramses::DataArray, must not be empty.
        * @param name a name for the the new #ramses::DataArray.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        */
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name ="");

        /**
        * Creates a new #ramses::AnimationNode for animating properties.
        * Refer to #ramses::AnimationNode for more information about its use.
        * There must be at least one channel provided in the #ramses::AnimationNodeConfig,
        * please see #ramses::AnimationChannel requirements for all the data.
        *
        * @param config list of animation channels to be animated with this animation node and other configuration flags.
        * @param name a name for the the new #ramses::AnimationNode.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        */
        AnimationNode* createAnimationNode(const AnimationNodeConfig& config, std::string_view name = "");

        /**
        * Creates a new #ramses::TimerNode for generate and/or propagate timing information.
        * Refer to #ramses::TimerNode for more information about its use.
        *
        * @param name a name for the the new #ramses::TimerNode.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        */
        TimerNode* createTimerNode(std::string_view name = "");

        /**
        * Creates a new #ramses::AnchorPoint that can be used to calculate projected coordinates of given ramses::Node when viewed using given ramses::Camera.
        * See #ramses::AnchorPoint for more details and usage of this special purpose logic node.
        *
        * @param nodeBinding binding referencing ramses::Node to use for model transformation when calculating projected coordinates.
        * @param cameraBinding binding referencing ramses::Camera to use for view and projection transformation when calculating projected coordinates.
        * @param name a name for the the new #ramses::AnchorPoint.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #ramses::RamsesFramework::getLastError.
        * The #ramses::AnchorPoint can be destroyed by calling the #destroy method
        */
        AnchorPoint* createAnchorPoint(NodeBinding& nodeBinding, CameraBinding& cameraBinding, std::string_view name ="");

        /**
         * Updates all #ramses::LogicNode's which were created by this #LogicEngine instance.
         * The order in which #ramses::LogicNode's are executed is determined by the links created
         * between them (see #link and #unlink). #ramses::LogicNode's which don't have any links
         * between then are executed in arbitrary order, but the order is always the same between two
         * invocations of #update without any calls to #link or #unlink between them.
         * As an optimization #ramses::LogicNode's are only updated, if at least one input of a #ramses::LogicNode
         * has changed since the last call to #update. If the links between logic nodes create a loop,
         * this method will fail with an error and will not execute any of the logic nodes.
         *
         * @return true if the update was successful, false otherwise
         * In case of an error, use #ramses::RamsesFramework::getLastError.
         */
        bool update();

        /**
        * Enables collecting of statistics during call to #update which can be obtained using #getLastUpdateReport.
        * Once enabled every subsequent call to #update will be instructed to collect various statistical data
        * which can be useful for profiling and optimizing the network of logic nodes.
        * Note that when enabled there is a slight performance overhead to collect the data, it is recommended
        * to use this only during a development phase.
        * Note that using #setStatisticsLoggingRate with #EStatisticsLogMode::Detailed will also implicitly enable
        * generating of update report for internal use during periodic statistics logging.
        *
        * @param enable true or false to enable or disable update reports.
        */
        void enableUpdateReport(bool enable);

        /**
        * Returns collection of statistics from last call to #update if reporting is enabled (#enableUpdateReport).
        * The report contains lists of logic nodes that were executed and not executed and other useful data collected
        * during last #update. See #ramses::LogicEngineReport for details.
        * The report data is generated if previously enabled using #enableUpdateReport and is empty otherwise.
        * The data is only relevant for the last #update and is overwritten during next #update.
        * Note that if #update fails the report contents are undefined.
        *
        * Attention! The #ramses::LogicEngineReport is returned by value and owns all the reported data.
        *            Make sure to keep the object as long as its data is referenced.
        *
        * @return collected statistics from last #update.
        */
        [[nodiscard]] LogicEngineReport getLastUpdateReport() const;

        /**
        * Set the logging rate, i.e. how often statistics will be logged. Logging rate of \c N means
        * every \c Nth call to #update statistics will be logged.
        * The logging rate also determines how many collected sets will be used to calculate min/max and average.
        * These compact statistics include:
        *  - \p Time since last log in seconds
        *  - \p Update execution time in microseconds (Avg, Min, Max)
        *  - \p Time between #update calls in microseconds (Avg, Min, Max)
        *  - \p Count of nodes executed in percentage of total count (Avg, Min, Max)
        *  - \p Links activated (Avg, Min, Max)
        * The detailed statistics additionally logs slowest nodes and their individual maximum update times during measured period.
        * When loggingRate is set to 0 the logging of statistics is disabled.
        * Note that there is a slight performance overhead for collecting the statistics data,
        * however on most platforms this should be marginal.
        * Note that using #EStatisticsLogMode::Detailed will also implicitly enable
        * generating of update report (see #enableUpdateReport) for internal use during periodic statistics logging,
        * and vice versa if #EStatisticsLogMode::Compact is set it will disable update report generating (even if previously enabled by user).
        *
        * @param loggingRate rate of \c N means statistics will be logged every \c Nth call to #update. By default, loggingRate is 60.
        * @param mode logging mode of the statistics (See #EStatisticsLogMode) By default, mode is #EStatisticsLogMode::Compact.
        */
        void setStatisticsLoggingRate(size_t loggingRate, EStatisticsLogMode mode = EStatisticsLogMode::Compact);

        /**
         * Links a property of a #ramses::LogicNode to another #ramses::Property of another #ramses::LogicNode.
         * After linking, calls to #update will propagate the value of \p sourceProperty to
         * the \p targetProperty. Creating links influences the order in which scripts
         * are executed - if node A provides data to node B, then node A will be executed
         * before node B. A single output property (\p sourceProperty) can be linked to any number of input
         * properties (\p targetProperty), but any input property can have at most one link to an output property
         * (links are directional and support a 1-to-N relationships).
         *
         * The #link() method will fail when:
         * - \p sourceProperty and \p targetProperty belong to the same #ramses::LogicNode
         * - \p sourceProperty is not an output (see #ramses::LogicNode::getOutputs())
         * - \p targetProperty is not an input (see #ramses::LogicNode::getInputs())
         * - either \p sourceProperty or \p targetProperty is not a primitive property (you have to link sub-properties
         *   of structs and arrays individually)
         *
         * Creating links which form a cycle in the node dependency graph will not fail right away but during the next call
         * to #update(). Links are directional, it is OK to have A->B, A->C and B->C, but is not OK to have
         * A->B->C->A. Cycles are allowed only under special conditions when using a weak link (see #linkWeak).
         *
         * After calling #link, the value of the \p targetProperty will not change until the next call to #update. Creating
         * and destroying links generally has no effect until #update is called.
         *
         * @param sourceProperty the output property which will provide data for \p targetProperty
         * @param targetProperty the target property which will receive its value from \p sourceProperty
         * @return true if linking was successful, false otherwise. To get more detailed
         * error information use #ramses::RamsesFramework::getLastError
         */
        bool link(Property& sourceProperty, Property& targetProperty);

        /**
        * A weak link is for the most part equivalent to #link - it has the same requirements and data propagation behavior,
        * but with one crucial difference. Weak link is not considered for node dependency graph (logic node topology)
        * which determines the order of nodes execution during #update. This allows to form a cycle using weak link
        * within the dependency graph, which can be helpful when some feedback data is required (e.g. a controller
        * node managing a worker node using normal links, worker node reports progress back to controller via weak link).
        * However there are important things to consider to avoid unexpected problems:
        *   1. The value propagated from a weak link is old, it is the value that was calculated in previous #update.
        *      Example: consider A->B->C using normal links and a back loop C->A using weak link. When #update is called,
        *               the nodes are executed exactly in the order defined by normal link dependency: A, B, C (remember
        *               that weak links are not considered for execution order). This means however that when A is executed
        *               its input value from weak linked C is only from previous update because C is executed last.
        *   2. During first update (after creation or load from file) there is no value applied from weak link, instead
        *      the initial input property value will be used at the weak link target. This is a logical consequence of the
        *      limitation described above - during first update there was no previous update and therefore no value from weak link.
        *   3. Avoid infinite update loops - #update has an optimization to execute only nodes with modified inputs,
        *      this can greatly simplify update of complex node hierarchies. When using weak links there is a risk that
        *      there will be a never ending need of executions of nodes involved in a node graph cycle. Consider the last
        *      node providing new value via weak link to first node, which then generates again new value propagated
        *      to the last node, this will result in the need to execute those nodes in all upcoming #update calls.
        *      Try to avoid such case and if it is not possible at least try to limit this update loop to a minimum of nodes.
        *
        * @param sourceProperty the output property which will provide data for \p targetProperty
        * @param targetProperty the target property which will receive its value from \p sourceProperty
        * @return true if linking was successful, false otherwise. To get more detailed
        * error information use #ramses::RamsesFramework::getLastError
        */
        bool linkWeak(Property& sourceProperty, Property& targetProperty);

        /**
         * Unlinks two properties which were linked with #link. After a link is destroyed,
         * calls to #update will no longer propagate the output value from the \p sourceProperty to
         * the input value of the \p targetProperty. The value of the \p targetProperty will remain as it was after the last call to #update -
         * it will **not** be restored to a default value or to any value which was set manually with calls to #ramses::Property::set().
         *
         * @param sourceProperty the output property which is currently linked to \p targetProperty
         * @param targetProperty the property which will no longer receive the value from \p sourceProperty
         * @return true if unlinking was successful, false otherwise. To get more detailed
         * error information use #ramses::RamsesFramework::getLastError.
         */
        bool unlink(Property& sourceProperty, Property& targetProperty);

        /**
         * Checks if an input or output of a given LogicNode is linked to another LogicNode
         * @param logicNode the node to check for linkage.
         * @return true if the given LogicNode is linked to any other LogicNode, false otherwise.
         */
        [[nodiscard]] bool isLinked(const LogicNode& logicNode) const;

        /**
         * Collect and retrieve all existing links between properties of logic nodes.
         * There will be a #ramses::PropertyLink in the returned container for every existing link created
         * (using #link or #linkWeak).
         *
         * Note that the returned container will not be modified (even if new links are created or unlinked in #LogicEngine)
         * until #getPropertyLinks is called again.
         *
         * @return all existing links between properties of logic nodes.
         */
        [[nodiscard]] const std::vector<PropertyLinkConst>& getPropertyLinks() const;

        /**
         * @copydoc getPropertyLinks() const
         */
        [[nodiscard]] const std::vector<PropertyLink>& getPropertyLinks();

        /**
        * Destroys an instance of an object created with #LogicEngine.
        * All objects created using #LogicEngine derive from a base class #ramses::LogicObject
        * and can be destroyed using this method.
        *
        * In case of a #ramses::LogicNode and its derived classes, if any links are connected to this #ramses::LogicNode,
        * they will be destroyed too. Note that after this call, the execution order of #ramses::LogicNode may change! See the
        * docs of #link and #unlink for more information.
        *
        * In case of a #ramses::DataArray, destroy will fail if it is used in any #ramses::AnimationNode's #ramses::AnimationChannel.
        *
        * In case of a #ramses::LuaModule, destroy will fail if it is used in any #ramses::LuaScript.
        *
        * @param object the object instance to destroy
        * @return true if object destroyed, false otherwise. Call #ramses::RamsesFramework::getLastError for error details upon failure.
        */
        bool destroy(LogicObject& object);

        /**
        * Calculates the serialized size of all objects contained in this LogicEngine instance.
        * Note that the returned size will differ from actual size when saved to a file but the difference should be no more than several bytes
        * (file header, meta information, etc.).
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both, see #ramses::SaveFileConfig::setLuaSavingMode),
        *                      default is #ramses::ELuaSavingMode::SourceAndByteCode.
        * @return size in bytes of the serialized LogicEngine.
        */
        [[nodiscard]] size_t getTotalSerializedSize(ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode) const;

        /**
        * Calculates the serialized size of all objects of a specific type in this LogicEngine instance.
        * \c T must be a concrete logic object type (e.g. #ramses::LuaScript). For the logic type LogicObject the size of all logic objects will be returned.
        *
        * @tparam T Logic object type to calculate size for
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both, see #ramses::SaveFileConfig::setLuaSavingMode),
        *                      default is #ramses::ELuaSavingMode::SourceAndByteCode (relevant only for object types containing Lua code).
        * @return size in bytes of the serialized objects.
        */
        template<typename T>
        [[nodiscard]] size_t getSerializedSize(ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode) const;

        /**
         * Get the internal data for implementation specifics of #LogicEngine.
         */
        [[nodiscard]] internal::LogicEngineImpl& impl();

        /**
         * Get the internal data for implementation specifics of #LogicEngine.
         */
        [[nodiscard]] const internal::LogicEngineImpl& impl() const;

    protected:
        /**
        * Implementation detail of LogicEngine
        */
        internal::LogicEngineImpl& m_impl;

        /**
        * @brief Constructor for LogicEngine. Use #ramses::Scene::createLogicEngine to instantiate this class.
        * @param[in] impl Internal data for implementation specifics of LogicEngine (sink - instance becomes owner)
        */
        explicit LogicEngine(std::unique_ptr<internal::LogicEngineImpl> impl);

        /**
        * @brief Scene is the factory for creating LogicEngine instances.
        */
        friend class internal::SceneObjectRegistry;

    private:
        /**
        * Internal implementation of collection getter
        * @return collection of objects
        */
        template <typename T>
        [[nodiscard]] Collection<T> getLogicObjectsInternal() const;

        /**
        * Internal implementation of type specific size getter
        * @tparam T Logic object type
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both
        * @return size in bytes of the serialized objects.
        */
        template<typename T>
        [[nodiscard]] size_t getSerializedSizeInternal(ELuaSavingMode luaSavingMode) const;

        /**
        * Internal implementation of object finder
        * @param name object name
        * @return found object
        */
        template <typename T>
        [[nodiscard]] const T* findLogicObjectInternal(std::string_view name) const;

        /**
        * Internal implementation of object finder
        * @param name object name
        * @return found object
        */
        template <typename T>
        [[nodiscard]] T* findLogicObjectInternal(std::string_view name);

        /**
        * Internal implementation of object finder
        * @param id object id
        * @return found object
        */
        template <typename T>
        [[nodiscard]] const T* findLogicObjectInternal(sceneObjectId_t id) const;

        /**
        * Internal implementation of object finder
        * @param id object id
        * @return found object
        */
        template <typename T>
        [[nodiscard]] T* findLogicObjectInternal(sceneObjectId_t id);

        /**
        * Internal implementation of #createDataArray
        *
        * @param data source data
        * @param name name
        * @return a pointer to the created object or nullptr on error
        */
        template <typename T>
        [[nodiscard]] DataArray* createDataArrayInternal(const std::vector<T>& data, std::string_view name);

        /// Internal static helper to validate type
        template <typename T>
        static void StaticTypeCheck();
    };

    template <typename T>
    Collection<T> LogicEngine::getCollection() const
    {
        StaticTypeCheck<T>();
        return getLogicObjectsInternal<T>();
    }

    template <typename T>
    const T* LogicEngine::findObject(std::string_view name) const
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(name);
    }

    template <typename T>
    T* LogicEngine::findObject(std::string_view name)
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(name);
    }

    template <typename T>
    const T* LogicEngine::findObject(sceneObjectId_t id) const
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(id);
    }

    template <typename T>
    T* LogicEngine::findObject(sceneObjectId_t id)
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(id);
    }

    template <typename T>
    DataArray* LogicEngine::createDataArray(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE),
            "Unsupported data type, see createDataArray API doc to see supported types.");
        return createDataArrayInternal<T>(data, name);
    }

    template <typename T>
    void LogicEngine::StaticTypeCheck()
    {
        static_assert(
            std::is_same_v<T, LogicObject> ||
            std::is_same_v<T, LuaScript> ||
            std::is_same_v<T, LuaModule> ||
            std::is_same_v<T, LuaInterface> ||
            std::is_same_v<T, NodeBinding> ||
            std::is_same_v<T, AppearanceBinding> ||
            std::is_same_v<T, CameraBinding> ||
            std::is_same_v<T, RenderPassBinding> ||
            std::is_same_v<T, RenderGroupBinding> ||
            std::is_same_v<T, MeshNodeBinding> ||
            std::is_same_v<T, SkinBinding> ||
            std::is_same_v<T, RenderBufferBinding> ||
            std::is_same_v<T, DataArray> ||
            std::is_same_v<T, AnimationNode> ||
            std::is_same_v<T, TimerNode> ||
            std::is_same_v<T, AnchorPoint>,
            "Attempting to retrieve invalid type of object.");
    }

    template<typename T>
    size_t LogicEngine::getSerializedSize(ELuaSavingMode luaSavingMode) const
    {
        StaticTypeCheck<T>();
        return getSerializedSizeInternal<T>(luaSavingMode);
    }
}
