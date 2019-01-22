//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/Timer.h"
#include "ramses-citymodel/CitymodelScene.h"
#include "ramses-citymodel/CullingNode.h"
#include "ramses-citymodel/Name.h"
#include "ramses-citymodel/Name2D.h"
#include "ramses-citymodel/Tile.h"
#include "ramses-citymodel/AnimationPath.h"

#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformStringUtils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-utils.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "Math3d/Vector3.h"
#include "RendererLib/RendererConfigUtils.h"

#include "sstream"
#include "iomanip"
#include "random"

Citymodel::Citymodel(int argc, char* argv[], ramses::RamsesFramework& framework, ramses::sceneId_t sceneId)
    : m_framework(framework)
    , m_pitch(1.0f, 0.1f, 4.0f)
    , m_yaw(1.0f, 0.1f, 3.0f)
    , m_distance(1.0f, 0.05f, 4.0f)
    , m_xPosition(1.0f, 0.1f, 4.0f)
    , m_yPosition(1.0f, 0.1f, 4.0f)
    , m_zPosition(1.0f, 0.1f, 4.0f)
    , m_destinationPosition(7670.512695f, 5716.101562f, 8.422584f)
    , m_sceneId(sceneId)
{
    m_xPosition.reset(m_destinationPosition.x);
    m_yPosition.reset(m_destinationPosition.y);
    m_zPosition.reset(m_destinationPosition.z);
    m_pitch.reset(m_destinationPitch);
    m_distance.reset(m_destinationDistance);

    init(argc, argv);
}

Citymodel::~Citymodel()
{
    m_guiOverlay.deinit();
    delete m_ramsesClient;
    m_ramsesClient = nullptr;

    delete m_reader;
    m_reader = nullptr;
}

ramses::sceneId_t Citymodel::getSceneId() const
{
    return m_sceneId;
}

void Citymodel::init(int argc, char* argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    parseArguments(parser);
    if (!m_exit)
    {
        m_ramsesClient = new ramses::RamsesClient("citymodel", m_framework);

        // create a scene for distributing content
        m_ramsesScene = m_ramsesClient->createScene(m_sceneId, ramses::SceneConfig(), "citymodel scene");
        m_camera      = m_ramsesScene->createRemoteCamera();
        m_renderPass = m_ramsesScene->createRenderPass();
        m_renderPass->setClearFlags(ramses::EClearFlags_None);
        m_renderPass->setCamera(*m_camera);

        m_renderGroup = m_ramsesScene->createRenderGroup();
        assert(m_renderGroup != NULL);
        m_renderPass->addRenderGroup(*m_renderGroup);

        m_reader = new Reader(*this);

        // when a specific frame is set, we don't do animation
        if (m_staticFrameToShow >= 0)
        {
            m_showAnimation = false;
        }

        computeProjectionMatrix();

        createEffects();
        createMarkerGeometry();

        m_rootCameraTranslate = m_ramsesScene->createNode();

        m_cameraRotate    = m_ramsesScene->createNode();
        m_cameraTranslate = m_ramsesScene->createNode();

        m_cameraTranslate->addChild(*m_cameraRotate);
        m_cameraRotate->addChild(*m_rootCameraTranslate);
        m_rootCameraTranslate->addChild(*m_camera);

        readScene();
        if (m_showRoute)
        {
            m_route = new LineContainer(ramses_internal::Vector3(1.0f, 0.8f, 0.0f),
                                        ramses_internal::Vector3(0.8f, 0.4f, 0.0f),
                                        1.0f,
                                        ramses_internal::Vector3(0.0f, 0.0f, 1.0f),
                                        *m_ramsesScene,
                                        *m_ramsesClient,
                                        *m_renderGroup);

            m_routePoints = m_scene->getRoutePoints();
            createRoute();
        }

        if (m_showNaming)
        {
            m_namingPoints = m_scene->getNamePoints();
            createNaming();
        }

        const float aspect = m_windowSize.x / m_windowSize.y;
        m_frustum.init(m_fovy, aspect, 1500.0);

        if (!m_showAnimation)
        {
            m_frame = m_staticFrameToShow;
        }

        m_guiOverlay.init(m_ramsesClient, m_ramsesScene, m_windowSize);
    }
}

bool Citymodel::shouldExit() const
{
    return m_exit;
}

void Citymodel::parseArguments(ramses_internal::CommandLineParser& parser)
{
    CitymodelArguments arguments(parser);

    m_fovy         = ramses_internal::ArgumentFloat(parser, "fov", "camera-field-of-view", 19.0f);
    m_windowSize.x = static_cast<ramses_internal::Float>(ramses_internal::ArgumentUInt32(parser, "w", "width", 1280));
    m_windowSize.y = static_cast<ramses_internal::Float>(ramses_internal::ArgumentUInt32(parser, "h", "height", 480));

    if (m_sceneId == 0)
    {
        m_sceneId = arguments.m_sceneId;
    }
    m_showRoute             = !arguments.m_noRoute;
    m_showNaming            = !arguments.m_noNaming;
    m_staticFrameToShow     = arguments.m_staticFrame;
    m_showPerformanceValues = arguments.m_showPerformanceValues;
    m_roundsToDrive         = arguments.m_roundsToDrive;
    m_filePath              = ramses_internal::String(arguments.m_filePath).c_str();
    m_secondMap             = arguments.m_secondMap;

    if (arguments.m_help)
    {
        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
        arguments.print();
        m_exit = true;
    }
}

void Citymodel::readScene()
{
    m_reader->open(m_filePath + "/ramses-citymodel.rex");

    TileResourceContainer globalResources;
    m_scene = static_cast<CitymodelScene*>(m_reader->read(0, globalResources, false));

    if (!m_scene)
    {
        printf("Could not read scene !!!\n");
        exit(1);
    }

    buildTree();

    ramses::Node* carsorModel = m_scene->getCarsor();

    const float f = 0.5f;

    m_carsor              = m_ramsesScene->createNode();
    m_carsorTranslation   = m_ramsesScene->createNode();
    m_carsorRotation      = m_ramsesScene->createNode();
    ramses::Node* scaler = m_ramsesScene->createNode();
    scaler->setScaling(f, f, f);

    m_carsor->addChild(*m_carsorTranslation);
    m_carsorTranslation->addChild(*m_carsorRotation);
    m_carsorRotation->addChild(*scaler);

    if (nullptr != carsorModel)
    {
        scaler->addChild(*carsorModel);
    }
}

void Citymodel::buildTree()
{
    m_cullingTree = new CullingNode(m_scene->getTiles(), this);
}

void Citymodel::doAnimation()
{
    const float dt = m_showAnimation ? m_frameTime.getTime() : 0.0f;
    m_frameTime.reset();
    AnimationPath::Key* key = m_scene->getAnimationPath().getKey(m_frame);

    if (nullptr != key)
    {
        if (m_carsor)
        {
            m_carsorTranslation->setTranslation(key->getCarPosition().x, key->getCarPosition().y, key->getCarPosition().z + 1.0f);
            m_carsorRotation->setRotation(key->getCarRotation().x, key->getCarRotation().y, key->getCarRotation().z);
        }
    }

    float pitch   = m_pitch.compute(dt, m_destinationPitch);
    float xRotate = -90.0f + pitch;

    float xPosition;
    float yPosition;
    float zPosition;
    if (key && (m_interactionMode == eFollowCarsor))
    {
        xPosition = key->getCarPosition().x;
        yPosition = key->getCarPosition().y;
        zPosition = key->getCarPosition().z;

        m_destinationYaw = key->getCarRotation().z;

        float currentAngle = m_yaw.get();
        float delta        = m_destinationYaw - currentAngle;
        while (fabs(delta) > 180.0)
        {
            if (delta >= 0.0)
            {
                delta -= 360.0;
            }
            else
            {
                delta += 360.0;
            }
        }
        m_destinationYaw = currentAngle + delta;

        m_destinationPosition.x = xPosition;
        m_destinationPosition.y = yPosition;
        m_destinationPosition.z = zPosition;
        m_xPosition.reset(xPosition);
        m_yPosition.reset(yPosition);
        m_zPosition.reset(zPosition);

        setCarPosInMaterials(m_destinationPosition);
    }
    else
    {
        if (m_lastInputTime.getTime() > 10.0)
        {
            m_destinationPosition.x = key->getCarPosition().x;
            m_destinationPosition.y = key->getCarPosition().y;
            m_destinationPosition.z = key->getCarPosition().z;
            m_destinationPitch      = 90.0;
            m_destinationDistance   = 400.0;
        }

        xPosition = m_xPosition.compute(dt, m_destinationPosition.x);
        yPosition = m_yPosition.compute(dt, m_destinationPosition.y);
        zPosition = m_zPosition.compute(dt, m_destinationPosition.z);
        setCarPosInMaterials(ramses_internal::Vector3(xPosition, yPosition, zPosition));
    }

    const float zRotate = m_yaw.compute(dt, m_destinationYaw);

    m_cameraRotate->setRotation(xRotate, 0.0, zRotate);
    m_cameraTranslate->setTranslation(xPosition, yPosition, zPosition);

    const float distance = m_distance.compute(dt, m_destinationDistance);
    m_rootCameraTranslate->setTranslation(0.0f, 0.0f, distance);

    ramses_internal::Matrix44f viewMatrix    = Name2D::GetObjectSpaceMatrixOfNode(*m_camera);
    ramses_internal::Matrix44f invViewMatrix = viewMatrix.inverse();

    ramses_internal::Vector3 camPos(invViewMatrix.m14, invViewMatrix.m24, invViewMatrix.m34);

    const float lightConeFactor = 35.0f / distance;
    m_scene->setLightConeFactor(lightConeFactor);

    if (m_naming)
    {
        const ramses_internal::Matrix44f viewProjectionMatrix = mProjectionMatrix * viewMatrix;
        m_naming->update(viewProjectionMatrix);
    }
}

void Citymodel::doFrame()
{
    m_reader->getSceneLock().lock();

    doAnimation();
    doCulling();
    doPaging();

    // const uint32_t framesToBuffer = 3;
    // TODO Bring back ESceneFlushMode_Asynchronous
    m_ramsesScene->flush(ramses::ESceneFlushMode_SynchronizedWithResources, ramses::InvalidSceneVersionTag);

    if (!m_ramsesScene->isPublished() && (m_pager.getNumTilesToLoad() == 0))
    {
        m_ramsesScene->publish();
    }

    m_reader->getSceneLock().unlock();

    if (m_showAnimation)
    {
        m_frame += 1;
        const uint32_t frames = m_scene->getAnimationPath().getNumberOfKeys();
        if (m_frame >= frames)
        {
            m_numRoundsDriven++;
            m_frame = 0;

            if (m_roundsToDrive != 0 && m_numRoundsDriven == m_roundsToDrive)
            {
                m_exit = true;
            }
        }
    }

    if (m_showPerformanceValues)
    {
        const uint32_t measurePeriod(60);

        if (++m_measureFrames == measurePeriod)
        {
            float curTime = m_fpsMeasureTimer.getTime();
            float fps     = float(measurePeriod) / curTime;
            float cpuTime = m_fpsMeasureTimer.getCpuTime();
            float cpuLoad = cpuTime / curTime * 100.0f;

            m_measureFrames = 0;

            updatePerformanceValuesString(fps, cpuLoad);
            m_fpsMeasureTimer.reset();
        }
    }
}

void Citymodel::doPaging()
{
    m_openTilesToLoad += static_cast<int32_t>(m_tilesAddToRead.size());
    m_pager.add(m_tilesAddToRead);
    m_tilesAddToRead.clear();

    m_pager.remove(m_tilesRemoveToRead);
    m_tilesRemoveToRead.clear();

    std::vector<Tile*> loadedTiles;
    m_pager.get(loadedTiles);
    for (uint32_t i = 0; i < loadedTiles.size(); i++)
    {
        loadedTiles[i]->loaded();
    }
    m_openTilesToLoad -= static_cast<int32_t>(loadedTiles.size());
    assert(m_openTilesToLoad >= 0);

    std::set<Tile*>::iterator rit = m_tilesToDelete.begin();
    while (rit != m_tilesToDelete.end())
    {
        Tile* tile = *rit;
        rit++;
        tile->decDeleteCounter();
    }
}

void Citymodel::doCulling()
{
    ramses_internal::Matrix44f invViewMatrix = Name2D::GetWorldSpaceMatrixOfNode(*m_camera);
    ramses_internal::Matrix44f newmatrix     = invViewMatrix;
    m_frustum.transform(newmatrix);
    if (m_cullingTree)
    {
        m_cullingTree->computeVisible(0x01f, m_frustum);
    }
}

Reader& Citymodel::getReader()
{
    return *m_reader;
}

TilePager& Citymodel::getTilePager()
{
    return m_pager;
}

void Citymodel::createRoute()
{
    if (m_namingPoints.size() >= 2)
    {
        for (uint32_t i = 0; i < m_namingPoints.size() - 1; i += 2)
        {
            std::vector<ramses_internal::Vector3> namingPoints;
            namingPoints.push_back(m_namingPoints[i]);
            namingPoints.push_back(m_namingPoints[i + 1]);
            m_route->addPolyline(namingPoints, LineContainer::ECapType_Round, LineContainer::ECapType_Round);
        }
    }

    m_route->addPolyline(m_routePoints, LineContainer::ECapType_Round, LineContainer::ECapType_Round);
    m_route->updateGeometry();
}

void Citymodel::createEffects()
{
    ramses::EffectDescription effectDesc;

    effectDesc.setUniformSemantic("u_mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    effectDesc.setUniformSemantic("u_mvMatrix", ramses::EEffectUniformSemantic_ModelViewMatrix);

    m_reader->addEffect(createEffect(effectDesc, "res/ramses-citymodel-tile.vert", "res/ramses-citymodel-tile.frag"));
    m_reader->addEffect(
        createEffect(effectDesc, "res/ramses-citymodel-untextured.vert", "res/ramses-citymodel-untextured.frag"));

    m_markerEffect = createEffect(effectDesc, "res/ramses-citymodel-marker.vert", "res/ramses-citymodel-marker.frag");
}

ramses::Effect* Citymodel::createEffect(ramses::EffectDescription& effectDesc,
                                         const char*                vertexShaderFile,
                                         const char*                fragmentShaderFile)
{
    effectDesc.setVertexShaderFromFile(vertexShaderFile);
    effectDesc.setFragmentShaderFromFile(fragmentShaderFile);

    return m_ramsesClient->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "");
}

void Citymodel::createNaming()
{
    m_naming =
        new NamingManager(*m_ramsesClient, *m_ramsesScene, m_camera, m_windowSize.x, m_windowSize.y, m_fovy, m_showAnimation);

    std::vector<std::string>& names = m_scene->getNames();

    std::vector<ramses_internal::Vector3>& points = m_scene->getNamePoints();

    const uint32_t n = static_cast<const uint32_t>(points.size());

    const uint32_t nameCount = n / 2;

    std::minstd_rand randomNumberGenerator;

    for (uint32_t i = 0; i < nameCount; i++)
    {
        const uint32_t nameIndex = randomNumberGenerator() % (names.size() - 1);
        createName(points[i * 2], points[i * 2 + 1], names[nameIndex]);
    }
}

void Citymodel::createName(const ramses_internal::Vector3& p0,
                            const ramses_internal::Vector3& p1,
                            const std::string&              nameString)
{
    Name2D* name = new Name2D(nameString, *m_ramsesScene, *m_ramsesClient, *m_renderGroup, p0, p1);
    m_naming->add(name);
}

void Citymodel::computeProjectionMatrix()
{
    const float aspect = m_windowSize.x / m_windowSize.y;
    float       n      = 0.2f;
    float       f      = 1500.0f;

    const float tangent = ramses_internal::PlatformMath::Tan(ramses_internal::PlatformMath::Deg2Rad(m_fovy / 2.0f));
    const float t       = n * tangent;
    const float b       = -t;
    const float r       = t * aspect;
    const float l       = -r;

    mProjectionMatrix = ramses_internal::Matrix44f((2.0f * n) / (r - l),
                                                   0.0f,
                                                   (r + l) / (r - l),
                                                   0.0f,
                                                   0.0f,
                                                   (2.0f * n) / (t - b),
                                                   (t + b) / (t - b),
                                                   0.0f,
                                                   0.0f,
                                                   0.0f,
                                                   -(f + n) / (f - n),
                                                   (-2.0f * f * n) / (f - n),
                                                   0.0f,
                                                   0.0f,
                                                   -1.0f,
                                                   0.0f);
}

void Citymodel::updatePerformanceValuesString(float fps, float cpuLoad)
{
    if (m_naming)
    {
        if (m_statusName)
        {
            m_naming->removePermanent(m_statusName);
            delete m_statusName;
            m_statusName = 0;
        }

        std::ostringstream stringStream;

        stringStream << "FPS: " << std::fixed << std::setprecision(1) << fps << " - CPU: " << std::setprecision(2)
                     << cpuLoad << "%";

        m_statusName = new Name(stringStream.str(), *m_ramsesScene, *m_ramsesClient, *m_renderGroup);

        m_naming->addPermanent(m_statusName);

        const ramses_internal::Vector2& minBounding = m_statusName->minBounding();
        const ramses_internal::Vector2& maxBounding = m_statusName->maxBounding();

        ramses::Node* node = m_statusName->topNode();
        assert(node);
        node->setTranslation(
            (maxBounding.x - minBounding.x) * 0.5 + 8, m_windowSize.y - (maxBounding.y - minBounding.y) * 0.5 - 8, 0.0);
    }
}

void Citymodel::addTileToRead(Tile* tile)
{
    m_tilesAddToRead.push_back(tile);
}

void Citymodel::removeTileToRead(Tile* tile)
{
    m_tilesRemoveToRead.push_back(tile);
}

void Citymodel::addTileToDelete(Tile* tile)
{
    m_tilesToDelete.insert(tile);
}

void Citymodel::removeTileToDelete(Tile* tile)
{
    m_tilesToDelete.erase(tile);
}

void Citymodel::setCarPosInMaterials(const ramses_internal::Vector3& carPos)
{
    m_scene->setCarPos(carPos);
}

ramses::Scene& Citymodel::getRamsesScene()
{
    return *m_ramsesScene;
}

ramses::RamsesClient& Citymodel::getRamsesClient()
{
    return *m_ramsesClient;
}

ramses::RenderGroup& Citymodel::getRenderGroup()
{
    return *m_renderGroup;
}

void Citymodel::mouseEvent(ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY)
{
    ramses_internal::Vector2i mousePos(mousePosX, mousePosY);

    const float doubleClickTime    = 0.25;
    const float destinationSpeed   = 20.0f;
    const float maxDistance        = 1000.0f;
    const float minDistance        = 50.0f;
    const float speedAtMaxDistance = 2.0f;

    if (m_leftButtonPressed)
    {
        ramses_internal::Vector2i mousePosDelta = mousePos - m_mousePos;
        m_destinationPitch += static_cast<float>(mousePosDelta.y) * 0.8f;

        const float minPitch = 10.0f;
        if (m_destinationPitch < minPitch)
        {
            m_destinationPitch = minPitch;
        }

        const float maxPitch = 90.0f;
        if (m_destinationPitch > maxPitch)
        {
            m_destinationPitch = maxPitch;
        }

        m_destinationYaw += static_cast<float>(mousePosDelta.x) * 0.4f;
    }

    if (m_middleButtonPressed)
    {
        ramses_internal::Vector2i mousePosDelta = mousePos - m_mousePos;
        const float               v             = m_distance.get() / maxDistance * speedAtMaxDistance;
        float                     dx            = -static_cast<float>(mousePosDelta.x) * v;
        float                     dy            = static_cast<float>(mousePosDelta.y) * v;

        ramses_internal::Matrix33f rotation = ramses_internal::Matrix33f::RotationEulerZYX(0.0f, 0.0f, m_yaw.get());

        ramses_internal::Vector3 movement = rotation * ramses_internal::Vector3(dx, dy, 0.0f);

        m_destinationPosition.x += movement.x;
        m_destinationPosition.y += movement.y;
    }

    switch (eventType)
    {
    case ramses::EMouseEvent_LeftButtonDown:
    {
        m_leftButtonPressed   = true;
        m_mousePosLButtonDown = mousePos;

        if (m_lastMouseLButtonClickTime.getTime() < doubleClickTime)
        {
            computeFocusPoint(mousePos);
        }
        m_lastMouseLButtonClickTime.reset();

        break;
    }
    case ramses::EMouseEvent_LeftButtonUp:
    {
        if (m_mousePosLButtonDown == mousePos)
        {
            m_destinationPitch = m_pitch.get();
            m_destinationYaw   = m_yaw.get();
        }

        m_leftButtonPressed = false;
        break;
    }
    case ramses::EMouseEvent_MiddleButtonDown:
    {
        m_middleButtonPressed = true;
        m_mousePosMButtonDown = mousePos;
        break;
    }
    case ramses::EMouseEvent_MiddleButtonUp:
    {
        if (m_mousePosMButtonDown == mousePos)
        {
            m_destinationPosition.x = m_xPosition.get();
            m_destinationPosition.y = m_yPosition.get();
        }

        m_middleButtonPressed = false;
        break;
    }
    case ramses::EMouseEvent_WheelUp:
    {
        m_destinationDistance -= destinationSpeed;

        if (m_destinationDistance < minDistance)
        {
            m_destinationDistance = minDistance;
        }

        break;
    }
    case ramses::EMouseEvent_WheelDown:
    {
        m_destinationDistance += destinationSpeed;

        if (m_destinationDistance > maxDistance)
        {
            m_destinationDistance = maxDistance;
        }
        break;
    }
    case ramses::EMouseEvent_RightButtonDown:
    {
        switch (m_interactionMode)
        {
        case eEditRoute:
        {
            if (m_lastMouseRButtonClickTime.getTime() < doubleClickTime)
            {
                deleteLastRoutePoint();
            }
            else
            {
                addRoutePoint(mousePos);
            }
            break;
        }
        case eEditNamingPoints:
        {
            if (m_lastMouseRButtonClickTime.getTime() < doubleClickTime)
            {
                deleteLastNamingPoint();
            }
            else
            {
                addNamingPoint(mousePos);
            }
            break;
        }

        case eFollowCarsor:
        {
            if (m_interactionModeCanBeSwitched)
            {
                setInteractionMode(eFreeMove);
            }
            break;
        }

        case eFreeMove:
        {
            if (m_interactionModeCanBeSwitched)
            {
                setInteractionMode(eFollowCarsor);
            }
            break;
        }
        }

        m_lastMouseRButtonClickTime.reset();
        break;
    }
    default:
        break;
    }

    m_mousePos = mousePos;
    m_lastInputTime.reset();
}

void Citymodel::touchEvent(ramses::ETouchEvent eventType, int32_t id, int32_t touchPosX, int32_t touchPosY)
{
    ramses_internal::Vector2i touchPos(touchPosX, touchPosY);

    auto                      lastPosI = m_touchPoint.find(id);
    ramses_internal::Vector2i lastTouchPos;

    const float maxDistance        = 1000.0f;
    const float minDistance        = 100.0f;
    const float speedAtMaxDistance = 1.0f;

    if (lastPosI != m_touchPoint.end())
    {
        lastTouchPos = lastPosI->second;

        if (m_touchPoint.size() == 1)
        {
            ramses_internal::Vector2i touchPosDelta = touchPos - lastTouchPos;

            if (m_touchRotateMode)
            {
                m_destinationPitch += static_cast<float>(touchPosDelta.y) * 0.4f;
                const float minPitch = 10.0f;
                if (m_destinationPitch < minPitch)
                {
                    m_destinationPitch = minPitch;
                }
                const float maxPitch = 90.0f;
                if (m_destinationPitch > maxPitch)
                {
                    m_destinationPitch = maxPitch;
                }
                m_destinationYaw += static_cast<float>(touchPosDelta.x) * 0.2f;
            }
            else
            {
                const float v  = m_distance.get() / maxDistance * speedAtMaxDistance;
                float       dx = -static_cast<float>(touchPosDelta.x) * v;
                float       dy = static_cast<float>(touchPosDelta.y) * v;

                ramses_internal::Matrix33f rotation =
                    ramses_internal::Matrix33f::RotationEulerZYX(0.0f, 0.0f, m_yaw.get());

                ramses_internal::Vector3 movement = rotation * ramses_internal::Vector3(dx, dy, 0.0f);

                m_destinationPosition.x += movement.x;
                m_destinationPosition.y += movement.y;
            }

            if (eventType == ramses::ETouchEvent_Up)
            {
                if ((m_initialTouchPoint[id] - touchPos).length() <= 4)
                {
                    m_destinationPosition.x = m_xPosition.get();
                    m_destinationPosition.y = m_yPosition.get();
                    m_destinationPitch      = m_pitch.get();
                    m_destinationYaw        = m_yaw.get();
                }
            }
        }
    }
    switch (eventType)
    {
    case ramses::ETouchEvent_Down:
    {
        m_touchPoint[id]        = touchPos;
        m_initialTouchPoint[id] = touchPos;

        if (m_touchPoint.size() == 1)
        {
            m_touchRotateMode = m_guiOverlay.checkRotateIconPressed(touchPos);
        }
        break;
    }
    case ramses::ETouchEvent_Up:
    {
        if (m_touchPoint.size() == 1)
        {
            if (m_interactionModeCanBeSwitched)
            {
                if (m_interactionMode == eFollowCarsor)
                {
                    setInteractionMode(eFreeMove);
                }

                if (m_interactionMode == eFreeMove && m_guiOverlay.checkBackButtonPressed(touchPos))
                {
                    setInteractionMode(eFollowCarsor);
                }
            }
        }

        m_touchPoint.erase(id);
        m_initialTouchPoint.erase(id);
        m_twoTouchPointDistance = 0.0f;
        break;
    }
    case ramses::ETouchEvent_Move:
    {
        m_touchPoint[id] = touchPos;
        break;
    }
    default:
    {
        break;
    }
    }

    if (m_touchPoint.size() == 2)
    {
        float twoTouchPointDistance = (m_touchPoint.begin()->second - (++m_touchPoint.begin())->second).length();
        if (m_twoTouchPointDistance != 0.0f)
        {
            float deltaDistance = m_twoTouchPointDistance - twoTouchPointDistance;

            m_destinationDistance += deltaDistance;
            if (m_destinationDistance < minDistance)
            {
                m_destinationDistance = minDistance;
            }
            if (m_destinationDistance > maxDistance)
            {
                m_destinationDistance = maxDistance;
            }
        }
        m_twoTouchPointDistance = twoTouchPointDistance;
    }

    m_lastInputTime.reset();
}

void Citymodel::createMarkerGeometry()
{
    m_markerGeometry = m_ramsesScene->createGeometryBinding(*m_markerEffect);

    const float vertexPositionsArray[] = {-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
                                          1.0f,  -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
                                          -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  -1.0f};

    const uint16_t indexArray[] = {0, 1, 2, 0, 2, 3, 4, 5, 1, 4, 1, 0, 5, 6, 2, 5, 2, 1,
                                   6, 7, 3, 6, 3, 2, 7, 4, 0, 7, 0, 3, 5, 4, 7, 5, 7, 6};

    const ramses::Vector3fArray* vertexPositions = m_ramsesClient->createConstVector3fArray(8, vertexPositionsArray);
    const ramses::UInt16Array*   indices         = m_ramsesClient->createConstUInt16Array(36, indexArray);

    ramses::AttributeInput positionsInput;
    m_markerEffect->findAttributeInput("a_position", positionsInput);
    m_markerGeometry->setInputBuffer(positionsInput, *vertexPositions);
    m_markerGeometry->setIndices(*indices);
}


void Citymodel::createMarker(const ramses_internal::Vector3& position, float size)
{
    ramses::Appearance* appearance = m_ramsesScene->createAppearance(*m_markerEffect);
    ramses::MeshNode*   meshNode   = m_ramsesScene->createMeshNode();
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*m_markerGeometry);
    m_renderGroup->addMeshNode(*meshNode, 4);
    meshNode->setTranslation(position.x, position.y, position.z);
    meshNode->setScaling(size, size, size);
}

bool Citymodel::computeIntersection(const ramses_internal::Vector3& p,
                                     const ramses_internal::Vector3& d,
                                     ramses_internal::Vector3&       position)
{
    m_reader->getSceneLock().lock();

    float                      r     = std::numeric_limits<float>::max();
    const std::vector<Tile*>& tiles = m_scene->getTiles();
    for (auto tile : tiles)
    {
        tile->computeIntersection(p, d, r);
    }
    m_reader->getSceneLock().unlock();

    if (r != std::numeric_limits<float>::max())
    {
        position = p + d * r;
        return true;
    }
    else
    {
        return false;
    }
}

void Citymodel::addRoutePoint(const ramses_internal::Vector2i& mousePos)
{
    ramses_internal::Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        position.z += 2.0;

        m_routePoints.push_back(position);
        createRoute();
    }
    m_frameTime.reset();
}

void Citymodel::addNamingPoint(const ramses_internal::Vector2i& mousePos)
{
    ramses_internal::Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        position.z += 2.0;

        m_namingPoints.push_back(position);
        createRoute();
    }
    m_frameTime.reset();
}

void Citymodel::deleteLastRoutePoint()
{
    if (!m_routePoints.empty())
    {
        m_routePoints.pop_back();
        if (!m_routePoints.empty())
        {
            m_routePoints.pop_back();
        }
        createRoute();
    }
}

void Citymodel::deleteLastNamingPoint()
{
    if (!m_namingPoints.empty())
    {
        m_namingPoints.pop_back();
        if (!m_namingPoints.empty())
        {
            m_namingPoints.pop_back();
        }
        createRoute();
    }
}

void Citymodel::computeFocusPoint(const ramses_internal::Vector2i& mousePos)
{
    ramses_internal::Vector3 position;
    if (computeIntersection(mousePos, position))
    {
        printf("CECMClient::setFocusPoint\n");
        m_destinationPosition = position;
    }
    m_frameTime.reset();
}

bool Citymodel::computeIntersection(const ramses_internal::Vector2i& mousePos, ramses_internal::Vector3& position)
{
    ramses_internal::Matrix44f viewMatrix = Name2D::GetWorldSpaceMatrixOfNode(*m_camera);
    const float                distance   = 40.0f;
    const float tangens = ramses_internal::PlatformMath::Tan(ramses_internal::PlatformMath::Deg2Rad(m_fovy / 2.0f));

    float xNormalizedScreen = 2.0f * (static_cast<float>(mousePos.x) / m_windowSize.x - 0.5f);
    float yNormalizedScreen = 2.0f * (0.5f - static_cast<float>(mousePos.y) / m_windowSize.y);

    const float              z      = -distance;
    const float              y      = distance * tangens * yNormalizedScreen;
    const float              aspect = m_windowSize.x / m_windowSize.y;
    const float              x      = distance * tangens * aspect * xNormalizedScreen;
    ramses_internal::Vector4 p(x, y, z, 1.0f);
    ramses_internal::Vector4 pWorld = viewMatrix * p;

    ramses_internal::Vector3 pWorld3(pWorld.x, pWorld.y, pWorld.z);

    ramses_internal::Vector4 pEye = viewMatrix * ramses_internal::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    ramses_internal::Vector3 pEye3(pEye.x, pEye.y, pEye.z);

    return computeIntersection(pEye3, pWorld3 - pEye3, position);
}

void Citymodel::printRoute()
{
    printf("CECMClient::printRoute\n");
    for (auto point : m_routePoints)
    {
        printf("P(%ff, %ff, %ff)\n", point.x, point.y, point.z);
    }
}

void Citymodel::printNamingPoints()
{
    printf("CECMClient::printNamingPoints\n");
    for (auto point : m_namingPoints)
    {
        printf("P(%ff, %ff, %ff)\n", point.x, point.y, point.z);
    }
}

void Citymodel::setInteractionMode(EInteractionMode interactionMode)
{
    m_interactionMode = interactionMode;

    switch (m_interactionMode)
    {
    case eFollowCarsor:
    {
        m_destinationDistance = 225.0f;
        m_destinationPitch    = 25.0f;
        break;
    }
    default:
        break;
    }

    m_guiOverlay.setShowRotateIcon(m_interactionMode == eFreeMove, m_interactionModeCanBeSwitched);
}

void Citymodel::enableInteractionModeSwitchable(bool value)
{
    m_interactionModeCanBeSwitched = value;
}

bool Citymodel::getSecondMap() const
{
    return m_secondMap;
}
