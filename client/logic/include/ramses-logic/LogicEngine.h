//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-client-api/ERotationType.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/Collection.h"
#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/ErrorData.h"
#include "ramses-logic/LogicEngineReport.h"
#include "ramses-logic/LuaConfig.h"
#include "ramses-logic/SaveFileConfig.h"
#include "ramses-logic/WarningData.h"
#include "ramses-logic/PropertyLink.h"
#include "ramses-logic/DataTypes.h"
#include "ramses-framework-api/EFeatureLevel.h"

#include <vector>
#include <string_view>

namespace ramses
{
    class Scene;
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;
}

namespace rlogic::internal
{
    class LogicEngineImpl;
}

namespace rlogic
{
    class LogicNode;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
    class Property;
    class RamsesNodeBinding;
    class RamsesAppearanceBinding;
    class RamsesCameraBinding;
    class RamsesRenderPassBinding;
    class RamsesRenderGroupBinding;
    class RamsesRenderGroupBindingElements;
    class RamsesMeshNodeBinding;
    class SkinBinding;
    class DataArray;
    class AnimationNode;
    class AnimationNodeConfig;
    class TimerNode;
    class AnchorPoint;
    enum class ELogMessageType;

    /**
    * Central object which creates and manages the lifecycle and execution
    * of scripts, bindings, and all other objects supported by the Ramses Logic library.
    * All objects created by this class' methods must be destroyed with #destroy!
    *
    * - Use the create[Type] methods to create various objects, use #destroy() to delete them.
    * - Use #link and #unlink to connect data properties between these objects
    * - use #update() to trigger the execution of all objects
    */
    class LogicEngine
    {
    public:
        /**
        * Constructor of #LogicEngine with a feature level specified.
        * See #ramses::EFeatureLevel for more details what features are available.
        *
        * @param featureLevel Feature level to activate in this #LogicEngine instance. A feature level always includes
        *                     a previous feature level if any, e.g. a feature level released after base level will contain
        *                     all features from base level, however base level will include only base level features
        *                     and none from a newer feature level. Use #ramses::EFeatureLevel_Latest to get all available features.
        */
        RAMSES_API explicit LogicEngine(ramses::EFeatureLevel featureLevel) noexcept;

        /**
        * Destructor of #LogicEngine
        */
        RAMSES_API ~LogicEngine() noexcept;

        /**
        * Returns the feature level this #LogicEngine instance was configured to use when created.
        * See #LogicEngine(ramses::EFeatureLevel) and #ramses::EFeatureLevel for more details.
        *
        * @return feature level this #LogicEngine instance was configured to use
        */
        [[nodiscard]] RAMSES_API ramses::EFeatureLevel getFeatureLevel() const;

        /**
        * Returns an iterable #rlogic::Collection of all instances of \c T created by this #LogicEngine.
        * \c T must be a concrete logic object type (e.g. #rlogic::LuaScript) or #rlogic::LogicObject which will retrieve
        * all logic objects created with this #LogicEngine (see #rlogic::LogicObject::as<Type> to convert to concrete type).
        *
        * @return an iterable #rlogic::Collection with all instances of \c T created by this #LogicEngine
        */
        template <typename T>
        [[nodiscard]] Collection<T> getCollection() const;

        /**
        * Returns a pointer to the first occurrence of an object with a given \p name of the type \c T.
        * \c T must be a concrete logic object type (e.g. #rlogic::LuaScript) or #rlogic::LogicObject which will search
        * any object with given name regardless of its type (see #rlogic::LogicObject::as<Type> to convert to concrete type).
        *
        * @param name the name of the logic object to search for
        * @return a pointer to the logic object, or nullptr if none was found
        */
        template <typename T>
        [[nodiscard]] const T* findByName(std::string_view name) const;

        /** @copydoc findByName(std::string_view) const */
        template <typename T>
        [[nodiscard]] T* findByName(std::string_view name);

        /**
        * Returns a pointer to the first occurrence of an object with a given \p id regardless of its type.
        * To convert the object to a concrete type (e.g. LuaScript) use #rlogic::LogicObject::as<Type>() e.g.:
        *   auto myLuaScript = logicEngine.findLogicObjectById(1u)->as<LuaScript>());
        * Be aware that this function behaves as \c dynamic_cast and will return nullptr (without error) if
        * given type doesn't match the objects type. This can later lead to crash if ignored.
        *
        * @param id the id of the logic object to search for
        * @return a pointer to the logic object, or nullptr if none was found
        */
        [[nodiscard]] RAMSES_API const LogicObject* findLogicObjectById(uint64_t id) const;
        /// @copydoc findLogicObjectById(uint64_t id) const
        [[nodiscard]] RAMSES_API LogicObject* findLogicObjectById(uint64_t id);

        /**
        * Creates a new Lua script from a source string. Refer to the #rlogic::LuaScript class
        * for requirements which Lua scripts must fulfill in order to be added to the #LogicEngine.
        * You can optionally provide Lua module dependencies via the \p config, they will be accessible
        * under their configured alias name for use by the script. The provided module dependencies
        * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param source the Lua source code
        * @param config configuration options, e.g. for module dependencies
        * @param scriptName name to assign to the script once it's created
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        * The script can be destroyed by calling the #destroy method
        */
        RAMSES_API LuaScript* createLuaScript(
            std::string_view source,
            const LuaConfig& config = {},
            std::string_view scriptName = "");

        /**
        * Creates a new Lua interface from a source string. Refer to the #rlogic::LuaInterface class
        * for requirements which Lua interface must fulfill in order to be added to the #LogicEngine.
        * Note: interfaces must have a non-empty name.
        * #rlogic::LuaInterface can be created with any non-empty name but if two or more instances
        * share same name there will be a warning during validation (#validate) as it is advised
        * for every Lua interface to have a unique name for clear identification from application logic.
        * You can optionally provide Lua module dependencies via the \p config, they will be accessible
        * under their configured alias name for use in the interface source. The provided module dependencies
        * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param source the Lua source code
        * @param interfaceName name to assign to the interface once it's created. This name must be unique!
        * @param config configuration options, e.g. for module dependencies
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        * The interface can be destroyed by calling the #destroy method
        */
        RAMSES_API LuaInterface* createLuaInterface(
            std::string_view source,
            std::string_view interfaceName,
            const LuaConfig& config);

        /**
        * Deprecated! Use #createLuaInterface(std::string_view, std::string_view, const LuaConfig&) instead.
        *
        * Same as #createLuaInterface(std::string_view, std::string_view, const LuaConfig&) but without
        * support for using #rlogic::LuaModule in the interface, also will ignore any \c modules
        * declaration within the provided interface source.
        *
        * @deprecated
        * @param source the Lua source code
        * @param interfaceName name to assign to the interface once it's created. This name must be unique!
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        * The interface can be destroyed by calling the #destroy method
        */
        RAMSES_API LuaInterface* createLuaInterface(
            std::string_view source,
            std::string_view interfaceName);

        /**
         * Creates a new #rlogic::LuaModule from Lua source code.
         * LuaModules can be used to share code and data constants across scripts or
         * other modules. See also #createLuaScript and #rlogic::LuaConfig for details.
         * You can optionally provide Lua module dependencies via the \p config, they will be accessible
         * under their configured alias name for use by the module. The provided module dependencies
         * must exactly match the declared dependencies in source code (see #extractLuaDependencies).
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param source module source code
         * @param config configuration options, e.g. for module dependencies
         * @param moduleName name to assign to the module once it's created
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The script can be destroyed by calling the #destroy method
         */
        RAMSES_API LuaModule* createLuaModule(
            std::string_view source,
            const LuaConfig& config = {},
            std::string_view moduleName = "");

        /**
         * Extracts dependencies from a Lua script, module or interface source code so that the corresponding
         * modules can be provided when creating #rlogic::LuaScript, #rlogic::LuaModule or #rlogic::LuaInterface.
         *
         * Any #rlogic::LuaScript, #rlogic::LuaModule or #rlogic::LuaInterface which has a module dependency,
         * i.e. it requires another #rlogic::LuaModule for it to work, must explicitly declare these
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
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param source source code of module or script to parse for dependencies
         * @param callbackFunc function callback will be called for each dependency found
         * @return \c true if extraction succeeded (also if no dependencies found) or \c false if
         * something went wrong. In that case, use #getErrors() to obtain errors.
         */
        RAMSES_API bool extractLuaDependencies(
            std::string_view source,
            const std::function<void(const std::string&)>& callbackFunc);

        /**
         * Creates a new #rlogic::RamsesNodeBinding which can be used to set the properties of a Ramses Node object.
         * The initial values of the binding's properties are loaded from the \p ramsesNode. Rotation values are
         * taken over from the \p ramsesNode only if the conventions are compatible (see \ref ramses::ERotationType).
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesNode the ramses::Node object to control with the binding.
         * @param rotationType the type of rotation to use (will affect the 'rotation' property semantics and type).
         * @param name a name for the new #rlogic::RamsesNodeBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesNodeBinding* createRamsesNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType = ramses::ERotationType::Euler_XYZ, std::string_view name = "");

        /**
         * Creates a new #rlogic::RamsesAppearanceBinding which can be used to set the properties of a Ramses Appearance object.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesAppearance the ramses::Appearance object to control with the binding.
         * @param name a name for the the new #rlogic::RamsesAppearanceBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesAppearanceBinding* createRamsesAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name = "");

        /**
         * Creates a new #rlogic::RamsesCameraBinding which can be used to set the properties of a Ramses Camera object.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesCamera the ramses::Camera object to control with the binding.
         * @param name a name for the the new #rlogic::RamsesCameraBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesCameraBinding* createRamsesCameraBinding(ramses::Camera& ramsesCamera, std::string_view name ="");

        /**
         * Same as #createRamsesCameraBinding but the created #rlogic::RamsesCameraBinding will have an input property
         * for each frustum plane also for perspective camera. See #rlogic::RamsesCameraBinding for details.
         * Note that ramses::OrthographicCamera binding will always have frustum planes as properties whether #createRamsesCameraBinding
         * or #createRamsesCameraBindingWithFrustumPlanes is used to create it.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesCamera the ramses::Camera object to control with the binding.
         * @param name a name for the the new #rlogic::RamsesCameraBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesCameraBinding* createRamsesCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name ="");

        /**
         * Creates a new #rlogic::RamsesRenderPassBinding which can be used to set the properties of a ramses::RenderPass object.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesRenderPass the ramses::RenderPass object to control with the binding.
         * @param name a name for the the new #rlogic::RamsesRenderPassBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesRenderPassBinding* createRamsesRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name ="");

        /**
         * Creates a new #rlogic::RamsesRenderGroupBinding which can be used to control some properties of a ramses::RenderGroup object.
         * #rlogic::RamsesRenderGroupBinding can be used to control render order of elements it contains on Ramses side - ramses::MeshNode or ramses::RenderGroup,
         * the elements to control must be provided explicitly at creation time, see #rlogic::RamsesRenderGroupBindingElements and #rlogic::RamsesRenderGroupBinding
         * to learn how these elements form the binding's input properties.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesRenderGroup the ramses::RenderGroup object to control with the binding.
         * @param elements collection of elements (MeshNode or RenderGroup) to control with this #rlogic::RamsesRenderGroupBinding.
         * @param name a name for the the new #rlogic::RamsesRenderGroupBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesRenderGroupBinding* createRamsesRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElements& elements, std::string_view name ="");

        /**
         * Creates a new #rlogic::RamsesMeshNodeBinding which can be used to control some properties of a ramses::MeshNode object.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param ramsesMeshNode the ramses::MeshNode object to control with the binding.
         * @param name a name for the the new #rlogic::RamsesMeshNodeBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API RamsesMeshNodeBinding* createRamsesMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name ="");

        /**
         * Creates a new #rlogic::SkinBinding which can be used for vertex skinning (bone animations).
         * Refer to #rlogic::SkinBinding and examples for all the information needed how to use this object.
         *
         * These conditions must be met in order for the creation to succeed:
         *  - \c joints must contain at least 1 joint and no null pointers
         *  - \c inverseBindMatrices must be of equal size as \c joints (i.e. matrix per joint)
         *  - \c jointMatInput must point to a valid uniform input of the ramses::Effect used in \c appearanceBinding
         *  - the shader uniform that \c jointMatInput points to must be of type array of ramses::EDataType::Matrix44F
         *    with number of elements matching number of joints
         *  - \c jointMatInput must not be bound to any data object in its Ramses appearance
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param joints bindings to Ramses nodes which will act as skeleton nodes.
         * @param inverseBindMatrices inverse transformation matrices (one for each joint node), values are expected ordered in column-major fashion.
         * @param appearanceBinding binding to Ramses appearance which specifies the effect/shader for vertex skinning.
         * @param jointMatInput Ramses appearance uniform input for the resulting joint matrices to be set.
         * @param name a name for the the new #rlogic::SkinBinding.
         * @return a pointer to the created object or nullptr if
         * something went wrong during creation. In that case, use #getErrors() to obtain errors.
         * The binding can be destroyed by calling the #destroy method
         */
        RAMSES_API SkinBinding* createSkinBinding(
            const std::vector<const RamsesNodeBinding*>& joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            RamsesAppearanceBinding& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name = {});

        /**
        * Creates a new #rlogic::DataArray to store data which can be used with animations.
        * Provided data must not be empty otherwise creation will fail.
        * See #rlogic::CanPropertyTypeBeStoredInDataArray and #rlogic::PropertyTypeToEnum
        * to determine supported types that can be used to create a #rlogic::DataArray.
        * When using std::vector<float> as element data type (corresponds to #rlogic::EPropertyType::Array),
        * the sizes of all the elements (std::vector<float> instances) must be equal, otherwise creation will fail.
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param data source data to move into #rlogic::DataArray, must not be empty.
        * @param name a name for the the new #rlogic::DataArray.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        */
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name ="");

        /**
        * Creates a new #rlogic::AnimationNode for animating properties.
        * Refer to #rlogic::AnimationNode for more information about its use.
        * There must be at least one channel provided in the #rlogic::AnimationNodeConfig,
        * please see #rlogic::AnimationChannel requirements for all the data.
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param config list of animation channels to be animated with this animation node and other configuration flags.
        * @param name a name for the the new #rlogic::AnimationNode.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        */
        RAMSES_API AnimationNode* createAnimationNode(const AnimationNodeConfig& config, std::string_view name = "");

        /**
        * Creates a new #rlogic::TimerNode for generate and/or propagate timing information.
        * Refer to #rlogic::TimerNode for more information about its use.
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param name a name for the the new #rlogic::TimerNode.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        */
        RAMSES_API TimerNode* createTimerNode(std::string_view name = "");

        /**
        * Creates a new #rlogic::AnchorPoint that can be used to calculate projected coordinates of given ramses::Node when viewed using given ramses::Camera.
        * See #rlogic::AnchorPoint for more details and usage of this special purpose logic node.
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param nodeBinding binding referencing ramses::Node to use for model transformation when calculating projected coordinates.
        * @param cameraBinding binding referencing ramses::Camera to use for view and projection transformation when calculating projected coordinates.
        * @param name a name for the the new #rlogic::AnchorPoint.
        * @return a pointer to the created object or nullptr if
        * something went wrong during creation. In that case, use #getErrors() to obtain errors.
        * The #rlogic::AnchorPoint can be destroyed by calling the #destroy method
        */
        RAMSES_API AnchorPoint* createAnchorPoint(RamsesNodeBinding& nodeBinding, RamsesCameraBinding& cameraBinding, std::string_view name ="");

        /**
         * Updates all #rlogic::LogicNode's which were created by this #LogicEngine instance.
         * The order in which #rlogic::LogicNode's are executed is determined by the links created
         * between them (see #link and #unlink). #rlogic::LogicNode's which don't have any links
         * between then are executed in arbitrary order, but the order is always the same between two
         * invocations of #update without any calls to #link or #unlink between them.
         * As an optimization #rlogic::LogicNode's are only updated, if at least one input of a #rlogic::LogicNode
         * has changed since the last call to #update. If the links between logic nodes create a loop,
         * this method will fail with an error and will not execute any of the logic nodes.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @return true if the update was successful, false otherwise
         * In case of an error, use #getErrors() to obtain errors.
         */
        RAMSES_API bool update();

        /**
        * Enables collecting of statistics during call to #update which can be obtained using #getLastUpdateReport.
        * Once enabled every subsequent call to #update will be instructed to collect various statistical data
        * which can be useful for profiling and optimizing the network of logic nodes.
        * Note that when enabled there is a slight performance overhead to collect the data, it is recommended
        * to use this only during a development phase.
        *
        * @param enable true or false to enable or disable update reports.
        */
        RAMSES_API void enableUpdateReport(bool enable);

        /**
        * Returns collection of statistics from last call to #update if reporting is enabled (#enableUpdateReport).
        * The report contains lists of logic nodes that were executed and not executed and other useful data collected
        * during last #update. See #rlogic::LogicEngineReport for details.
        * The report data is generated only if previously enabled using #enableUpdateReport and is empty otherwise.
        * The data is only relevant for the last #update and is overwritten during next #update.
        * Note that if #update fails the report contents are undefined.
        *
        * Attention! The #rlogic::LogicEngineReport is returned by value and owns all the reported data.
        *            Make sure to keep the object as long as its data is referenced.
        *
        * @return collected statistics from last #update.
        */
        [[nodiscard]] RAMSES_API LogicEngineReport getLastUpdateReport() const;

        /**
        * Set the logging rate, i.e. how often statistics will be logged. Logging rate of \c N means
        * every \c Nth call to #update statistics will be logged.
        * Whether the the logs are actually logged is also influenced by the statistics log level that can be set with #setStatisticsLogLevel.
        * The logging rate also determines how many collected sets will be used to calculate min/max and average.
        * These statistics include:
        *  - \p Time since last log in seconds
        *  - \p Update execution time in microseconds (Avg, Min, Max)
        *  - \p Time between #update calls in microseconds (Avg, Min, Max)
        *  - \p Count of nodes executed in percentage of total count (Avg, Min, Max)
        *  - \p Links activated (Avg, Min, Max)
        * When loggingRate is set to 0 the logging of statistics is disabled.
        * Note that there is a slight performance overhead for collecting the statistics data,
        * however on most platforms this should be marginal.
        * To get more detailed information about update execution timings see #getLastUpdateReport.
        *
        * @param loggingRate rate of \c N means statistics will be logged every \c Nth call to #update. By default loggingRate is 60.
        */
        RAMSES_API void setStatisticsLoggingRate(size_t loggingRate);

        /**
        * Update statistics default logLevel is #ELogMessageType::Debug. For the statistics to be logged the logLevel
        * has to be <= then the result returned from #rlogic::Logger::GetLogVerbosityLimit.
        * For example if #rlogic::Logger::GetLogVerbosityLimit returns #ELogMessageType::Info you have to
        * set statistics logLevel to #ELogMessageType::Info or a smaller level (e.g. #ELogMessageType::Warn) to display statistics.
        * Note that setting the statistics log level only influences the periodic statistic logs. All other logs are not influenced
        * by this method.
        * To control the rate after how many updates a log is produced refer to #setStatisticsLoggingRate.
        *
        * @param logLevel the logLevel of statistics messages
        */
        RAMSES_API void setStatisticsLogLevel(ELogMessageType logLevel);

        /**
         * Links a property of a #rlogic::LogicNode to another #rlogic::Property of another #rlogic::LogicNode.
         * After linking, calls to #update will propagate the value of \p sourceProperty to
         * the \p targetProperty. Creating links influences the order in which scripts
         * are executed - if node A provides data to node B, then node A will be executed
         * before node B. A single output property (\p sourceProperty) can be linked to any number of input
         * properties (\p targetProperty), but any input property can have at most one link to an output property
         * (links are directional and support a 1-to-N relationships).
         *
         * The #link() method will fail when:
         * - \p sourceProperty and \p targetProperty belong to the same #rlogic::LogicNode
         * - \p sourceProperty is not an output (see #rlogic::LogicNode::getOutputs())
         * - \p targetProperty is not an input (see #rlogic::LogicNode::getInputs())
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
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param sourceProperty the output property which will provide data for \p targetProperty
         * @param targetProperty the target property which will receive its value from \p sourceProperty
         * @return true if linking was successful, false otherwise. To get more detailed
         * error information use #getErrors()
         */
        RAMSES_API bool link(const Property& sourceProperty, const Property& targetProperty);

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
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param sourceProperty the output property which will provide data for \p targetProperty
        * @param targetProperty the target property which will receive its value from \p sourceProperty
        * @return true if linking was successful, false otherwise. To get more detailed
        * error information use #getErrors()
        */
        RAMSES_API bool linkWeak(const Property& sourceProperty, const Property& targetProperty);

        /**
         * Unlinks two properties which were linked with #link. After a link is destroyed,
         * calls to #update will no longer propagate the output value from the \p sourceProperty to
         * the input value of the \p targetProperty. The value of the \p targetProperty will remain as it was after the last call to #update -
         * it will **not** be restored to a default value or to any value which was set manually with calls to #rlogic::Property::set().
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param sourceProperty the output property which is currently linked to \p targetProperty
         * @param targetProperty the property which will no longer receive the value from \p sourceProperty
         * @return true if unlinking was successful, false otherwise. To get more detailed
         * error information use #getErrors().
         */
        RAMSES_API bool unlink(const Property& sourceProperty, const Property& targetProperty);

        /**
         * Checks if an input or output of a given LogicNode is linked to another LogicNode
         * @param logicNode the node to check for linkage.
         * @return true if the given LogicNode is linked to any other LogicNode, false otherwise.
         */
        [[nodiscard]] RAMSES_API bool isLinked(const LogicNode& logicNode) const;

        /**
         * Collect and retrieve all existing links between properties of logic nodes.
         * There will be a #rlogic::PropertyLink in the returned container for every existing link created
         * (using #link or #linkWeak).
         *
         * Note that the returned container will not be modified (even if new links are created or unlinked in #LogicEngine)
         * until #getPropertyLinks is called again.
         *
         * @return all existing links between properties of logic nodes.
         */
        [[nodiscard]] RAMSES_API const std::vector<PropertyLink>& getPropertyLinks() const;

        /**
         * Returns the list of all errors which occurred during the last API call to a #LogicEngine method
         * or any other method of its subclasses (scripts, bindings etc). Note that errors get wiped by all
         * mutable methods of the #LogicEngine.
         *
         * This method can be used in two different ways:
         * - To debug the correct usage of the Logic Engine API (e.g. by wrapping all API calls with a check
         *   for their return value and using this method to find out the cause of the error)
         * - To check for runtime errors of scripts which come from a dynamic source, e.g. by calling the
         *   method after an unsuccessful call to #update() with a faulty script
         *
         * @return a list of errors
         */
        [[nodiscard]] RAMSES_API const std::vector<ErrorData>& getErrors() const;

        /**
        * Performs a (potentially slow!) validation of the LogicEngine content and reports a list of warnings
        * for things which could be fixed/changed for better performance, consistency or resource usage.
        *
        * @return a list of warnings
        */
        [[nodiscard]] RAMSES_API const std::vector<WarningData>& validate() const;

        /**
        * Destroys an instance of an object created with #LogicEngine.
        * All objects created using #LogicEngine derive from a base class #rlogic::LogicObject
        * and can be destroyed using this method.
        *
        * In case of a #rlogic::LogicNode and its derived classes, if any links are connected to this #rlogic::LogicNode,
        * they will be destroyed too. Note that after this call, the execution order of #rlogic::LogicNode may change! See the
        * docs of #link and #unlink for more information.
        *
        * In case of a #rlogic::DataArray, destroy will fail if it is used in any #rlogic::AnimationNode's #rlogic::AnimationChannel.
        *
        * In case of a #rlogic::LuaModule, destroy will fail if it is used in any #rlogic::LuaScript.
        *
        * Attention! This method clears all previous errors! See also docs of #getErrors()
        *
        * @param object the object instance to destroy
        * @return true if object destroyed, false otherwise. Call #getErrors() for error details upon failure.
        */
        RAMSES_API bool destroy(LogicObject& object);

        /**
         * Writes the whole #LogicEngine and all of its objects to a binary file with the given filename. The RAMSES scene
         * potentially referenced by #rlogic::RamsesBinding objects is not saved - that is left to the application.
         * #LogicEngine saves the references to those object, and restores them after loading.
         * Thus, deleting Ramses objects which are being referenced from within the #LogicEngine
         * will result in errors if the Logic Engine is loaded from the file again. Note that it is not sufficient
         * to have objects with the same name, they have to be the exact same objects as during saving!
         * For more in-depth information regarding saving and loading, refer to the online documentation at
         * https://ramses-logic.readthedocs.io/en/latest/api.html#saving-loading-from-file
         *
         * Note: The method reports error and aborts if the #rlogic::RamsesBinding objects reference more than one
         * Ramses scene (this is acceptable during runtime, but not for saving to file).
         *
         * Note: This method fails and reports error if validation failed and was not disabled by
         * calling #rlogic::SaveFileConfig::setValidationEnabled with false
         *
         * Note: This method fails and reports error if there is any Lua script or module with debug log functions,
         * (see #rlogic::LuaConfig::enableDebugLogFunctions), these must be destroyed before saving to file.
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param filename path to file to save the data (relative or absolute). The file will be created or overwritten if it exists!
         * @param config optional configuration object with exporter and asset metadata info, see #rlogic::SaveFileConfig for details
         * @return true if saving was successful, false otherwise. To get more detailed
         * error information use #getErrors()
         */
        RAMSES_API bool saveToFile(std::string_view filename, const SaveFileConfig& config = {});

        /**
         * Loads the whole LogicEngine data from the given file. See also #saveToFile().
         * After loading, the previous state of the #LogicEngine will be overwritten with the
         * contents loaded from the file, i.e. all previously created objects (scripts, bindings, etc.)
         * will be deleted and pointers to them will be invalid. The (optionally) provided ramsesScene
         * will be used to resolve potential #rlogic::RamsesBinding objects which point to Ramses objects.
         * You can provide a nullptr if you know for sure that the
         * #LogicEngine loaded from the file has no #rlogic::RamsesBinding objects which point to a Ramses scene object.
         * Otherwise, the call to #loadFromFile will fail with an error. In case of errors, the #LogicEngine
         * may be left in an inconsistent state.
         * For more in-depth information regarding saving and loading, refer to the online documentation at
         * https://ramses-logic.readthedocs.io/en/latest/api.html#saving-loading-from-file
         *
         * Attention! This method clears all previous errors! See also docs of #getErrors()
         *
         * @param filename path to file from which to load content (relative or absolute)
         * @param ramsesScene pointer to the Ramses Scene which holds the objects referenced in the Ramses Logic file
         * @param enableMemoryVerification flag to enable memory verifier (a flatbuffers feature which checks bounds and ranges).
         *        Disable this only if the file comes from a trusted source and performance is paramount.
         * @return true if deserialization was successful, false otherwise. To get more detailed
         * error information use #getErrors()
         */
        RAMSES_API bool loadFromFile(std::string_view filename, ramses::Scene* ramsesScene = nullptr, bool enableMemoryVerification = true);

        /**
         * Loads the whole LogicEngine data from the given file descriptor. This method is equivalent to #loadFromFile().
         *
         * The file descriptor must be opened for read access and must support seeking.
         * It will be in closed state after this call.
         *
         * @param[in] fd Open and readable filedescriptor.
         * @param[in] offset Absolute starting position of LogicEngine data within fd.
         * @param[in] length Size of the data within fd.
         * @param ramsesScene pointer to the Ramses Scene which holds the objects referenced in the Ramses Logic file
         * @param enableMemoryVerification flag to enable memory verifier (a flatbuffers feature which checks bounds and ranges).
         *        Disable this only if the file comes from a trusted source and performance is paramount.
         * @return true if deserialization was successful, false otherwise. To get more detailed
         * error information use #getErrors()
         */
        RAMSES_API bool loadFromFileDescriptor(int fd, size_t offset, size_t length, ramses::Scene* ramsesScene = nullptr, bool enableMemoryVerification = true);

        /**
        * Loads the whole LogicEngine data from the given memory buffer. This method is equivalent to
        * #loadFromFile but allows to have the file-opening
        * logic done by the user and only pass the data as a buffer. The logic engine only reads the
        * data, does not take ownership of it and does not modify it. The memory can be freed or
        * modified after the call returns, the #LogicEngine keeps no references to it.
        *
        * @param rawBuffer pointer to the raw data in memory
        * @param bufferSize size of the data (bytes)
        * @param ramsesScene pointer to the Ramses Scene which holds the objects referenced in the Ramses Logic file
        * @param enableMemoryVerification flag to enable memory verifier (a flatbuffers feature which checks bounds and ranges).
        *        Disable this only if the file comes from a trusted source and performance is paramount.
        * @return true if deserialization was successful, false otherwise. To get more detailed
        * error information use #getErrors()
        */
        RAMSES_API bool loadFromBuffer(const void* rawBuffer, size_t bufferSize, ramses::Scene* ramsesScene = nullptr, bool enableMemoryVerification = true);

        /**
        * Calculates the serialized size of all objects contained in this LogicEngine instance.
        * Note that the returned size will differ from actual size when saved to a file but the difference should be no more than several bytes
        * (file header, meta information, etc.).
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both, see #rlogic::SaveFileConfig::setLuaSavingMode),
        *                      default is #rlogic::ELuaSavingMode::SourceAndByteCode.
        * @return size in bytes of the serialized LogicEngine.
        */
        [[nodiscard]] RAMSES_API size_t getTotalSerializedSize(ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode) const;

        /**
        * Calculates the serialized size of all objects of a specific type in this LogicEngine instance.
        * \c T must be a concrete logic object type (e.g. #rlogic::LuaScript). For the logic type LogicObject the size of all logic objects will be returned.
        *
        * @tparam T Logic object type to calculate size for
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both, see #rlogic::SaveFileConfig::setLuaSavingMode),
        *                      default is #rlogic::ELuaSavingMode::SourceAndByteCode (relevant only for object types containing Lua code).
        * @return size in bytes of the serialized objects.
        */
        template<typename T>
        [[nodiscard]] size_t getSerializedSize(ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode) const;

        /**
        * Attempts to parse feature level from a Ramses Logic file.
        *
        * @param[in] filename file path to Ramses Logic file
        * @param[out] detectedFeatureLevel feature level detected in given file (valid only if parsing successful!)
        * @return true if parsing was successful, false otherwise.
        */
        [[nodiscard]] RAMSES_API static bool GetFeatureLevelFromFile(std::string_view filename, ramses::EFeatureLevel& detectedFeatureLevel);

        /**
        * Attempts to parse feature level from a Ramses Logic buffer.
        *
        * @param[in] logname additional name for errors logging (e.g. Ramses Logic file name)
        * @param[in] buffer data buffer with Ramses Logic file content
        * @param[in] bufferSize size of the data buffer
        * @param[out] detectedFeatureLevel feature level detected in given file (valid only if parsing successful!)
        * @return true if parsing was successful, false otherwise.
        */
        [[nodiscard]] RAMSES_API static bool GetFeatureLevelFromBuffer(std::string_view logname, const void* buffer, size_t bufferSize, ramses::EFeatureLevel& detectedFeatureLevel);

        /**
        * Copy Constructor of LogicEngine is deleted because logic engines hold named resources and are not supposed to be copied
        *
        * @param other logic engine to copy from
        */
        LogicEngine(const LogicEngine& other) = delete;

        /**
        * Move Constructor of LogicEngine
        *
        * @param other logic engine to move from
        */
        RAMSES_API LogicEngine(LogicEngine&& other) noexcept;

        /**
        * Assignment operator of LogicEngine is deleted because logic engines hold named resources and are not supposed to be copied
        *
        * @param other logic engine to assign from
        */
        LogicEngine& operator=(const LogicEngine& other) = delete;

        /**
        * Move assignment operator of LogicEngine
        *
        * @param other logic engine to move from
        */
        RAMSES_API LogicEngine& operator=(LogicEngine&& other) noexcept;

        /**
        * Implementation detail of LogicEngine
        */
        std::unique_ptr<internal::LogicEngineImpl> m_impl;

    private:
        /**
        * Internal implementation of collection getter
        * @return collection of objects
        */
        template <typename T>
        [[nodiscard]] RAMSES_API Collection<T> getLogicObjectsInternal() const;

        /**
        * Internal implementation of type specific size getter
        * @tparam T Logic object type
        * @param luaSavingMode calculate with Lua code saved as source string, binary or both
        * @return size in bytes of the serialized objects.
        */
        template<typename T>
        [[nodiscard]] RAMSES_API size_t getSerializedSizeInternal(ELuaSavingMode luaSavingMode) const;

        /**
        * Internal implementation of object finder
        * @param name object name
        * @return found object
        */
        template <typename T>
        [[nodiscard]] RAMSES_API const T* findLogicObjectInternal(std::string_view name) const;

        /**
        * Internal implementation of object finder
        * @param name object name
        * @return found object
        */
        template <typename T>
        [[nodiscard]] RAMSES_API T* findLogicObjectInternal(std::string_view name);

        /**
        * Internal implementation of #createDataArray
        *
        * @param data source data
        * @param name name
        * @return a pointer to the created object or nullptr on error
        */
        template <typename T>
        [[nodiscard]] RAMSES_API DataArray* createDataArrayInternal(const std::vector<T>& data, std::string_view name);

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
    const T* LogicEngine::findByName(std::string_view name) const
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(name);
    }

    template <typename T>
    T* LogicEngine::findByName(std::string_view name)
    {
        StaticTypeCheck<T>();
        return findLogicObjectInternal<T>(name);
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
            std::is_same_v<T, RamsesNodeBinding> ||
            std::is_same_v<T, RamsesAppearanceBinding> ||
            std::is_same_v<T, RamsesCameraBinding> ||
            std::is_same_v<T, RamsesRenderPassBinding> ||
            std::is_same_v<T, RamsesRenderGroupBinding> ||
            std::is_same_v<T, RamsesMeshNodeBinding> ||
            std::is_same_v<T, SkinBinding> ||
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
