#include <Headless/CLIViewer.hpp>

#include <Core/Asset/Camera.hpp>
#include <Core/Asset/FileLoaderInterface.hpp>
#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Utils/Log.hpp>

#include <Engine/Data/ViewingParameters.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Scene/DefaultCameraManager.hpp>
#include <Engine/Scene/DirLight.hpp>
#include <Engine/Scene/GeometrySystem.hpp>
#include <Engine/Scene/SkeletonBasedAnimationSystem.hpp>
#include <Engine/Scene/SystemDisplay.hpp>

namespace Ra {
namespace Headless {
using namespace Ra::Core::Utils;

constexpr int defaultSystemPriority = 1000;

CLIViewer::CLIViewer( const glbinding::Version& glVersion ) :
    CLIBaseApplication(), m_glContext { glVersion } {
    // add ->required() to force user to give a filename;
    addOption( "-f,--file", m_parameters.m_dataFile, "Data file to process." )
        ->check( CLI::ExistingFile );
    addOption( "-s,--size", m_parameters.m_size, "Size of the computed image." )->delimiter( 'x' );
    addFlag( "-a,--animation", m_parameters.m_animationEnable, "Enable Radium Animation system." );
}

CLIViewer::~CLIViewer() {
    if ( m_engineInitialized ) {
        m_glContext.makeCurrent();
        m_renderer.reset();
        m_engine->cleanup();
        Ra::Engine::RadiumEngine::destroyInstance();
        m_glContext.doneCurrent();
    }
}

const CLIViewer::ViewerParameters& CLIViewer::getCommandLineParameters() const {
    return m_parameters;
}

int CLIViewer::init( int argc, const char* argv[] ) {
    int parseResult = CLIBaseApplication::init( argc, argv );
    if ( parseResult != 0 ) {
        LOG( logERROR ) << "Invalid command line argument, the application can't run";
        return 1;
    };
    // Do the Viewer init
    if ( !m_glContext.isValid() ) {
        LOG( logERROR ) << "Invalid openglContext, the application can't run";
        return 1;
    }

    m_glContext.resize( m_parameters.m_size );
    LOG( logINFO ) << "CLIViewer :\n" << m_glContext.getInfo();

    // Initialize the Radium engine environment

    // Create engine
    m_engine = Ra::Engine::RadiumEngine::createInstance();
    m_engine->initialize();
    m_engineInitialized = true;

    // Register the GeometrySystem converting loaded assets to meshes
    m_engine->registerSystem(
        "GeometrySystem", new Ra::Engine::Scene::GeometrySystem, defaultSystemPriority );

    if ( m_parameters.m_animationEnable ) {
        // Register the SkeletonBasedAnimationSystem converting loaded assets to
        // skeletons and skinning data
        m_engine->registerSystem( "SkeletonBasedAnimationSystem",
                                  new Ra::Engine::Scene::SkeletonBasedAnimationSystem,
                                  defaultSystemPriority );
    }

    // initialize OpenGL part of the Engine
    m_glContext.makeCurrent();
    m_engine->initializeGL();
    m_glContext.doneCurrent();

    // register listeners on the OpenGL Context
    m_glContext.resizeListener().attach(
        [this]( int width, int height ) { resize( width, height ); } );
    // Init is OK
    return 0;
}

int CLIViewer::oneFrame( float timeStep ) {
    if ( m_parameters.m_animationEnable ) {
        auto animationSystem = dynamic_cast<Ra::Engine::Scene::SkeletonBasedAnimationSystem*>(
            m_engine->getSystem( "SkeletonBasedAnimationSystem" ) );
        if ( animationSystem ) { animationSystem->toggleSkeleton( false ); }
        m_engine->setConstantTimeStep( timeStep );
        m_engine->step();
    }

    Ra::Core::TaskQueue tasks( std::thread::hardware_concurrency() - 1 );
    m_engine->getTasks( &tasks, Scalar( timeStep ) );
    tasks.startTasks();
    tasks.waitForTasks();
    tasks.flushTaskQueue();

    Ra::Engine::Data::ViewingParameters data {
        m_camera->getViewMatrix(), m_camera->getProjMatrix(), timeStep };
    m_renderer->render( data );

    return 0;
}

std::unique_ptr<unsigned char[]> CLIViewer::grabFrame( size_t& w, size_t& h ) const {
    return m_renderer->grabFrame( w, h );
}

void CLIViewer::setRenderer( Ra::Engine::Rendering::Renderer* renderer ) {
    m_glContext.makeCurrent();

    m_renderer.reset( renderer );
    m_renderer->initialize( m_parameters.m_size[0], m_parameters.m_size[1] );

    m_glContext.doneCurrent();
}

void CLIViewer::addDataFileLoader( Ra::Core::Asset::FileLoaderInterface* loader ) {
    m_engine->registerFileLoader( std::shared_ptr<Ra::Core::Asset::FileLoaderInterface>( loader ) );
}

void CLIViewer::loadScene() {
    m_engine->loadFile( m_parameters.m_dataFile );
}

void CLIViewer::compileScene() {
    if ( m_renderer ) {
        m_renderer->buildAllRenderTechniques();
        if ( !m_renderer->hasLight() ) {
            auto headlight = new Ra::Engine::Scene::DirectionalLight(
                Ra::Engine::Scene::SystemEntity::getInstance(), "headlight" );
            headlight->setColor( Ra::Core::Utils::Color::Grey( 2.0_ra ) );
            headlight->setDirection( m_camera->getDirection() );
            m_renderer->addLight( headlight );
        }
    }
}

void CLIViewer::openGlAddOns( std::function<void()> f ) {
    m_glContext.makeCurrent();
    f();
    m_glContext.doneCurrent();
}

void CLIViewer::bindOpenGLContext( bool on ) {
    if ( on ) { m_glContext.makeCurrent(); }
    else {
        m_glContext.doneCurrent();
    }
}

void CLIViewer::setCamera( Ra::Core::Utils::Index camIdx ) {
    auto cameraManager = static_cast<Ra::Engine::Scene::CameraManager*>(
        Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );

    if ( camIdx.isInvalid() || cameraManager->count() <= size_t( camIdx ) ) {
        m_camera = new Ra::Core::Asset::Camera( m_parameters.m_size[0], m_parameters.m_size[1] );
        m_camera->setFOV( 60.0_ra * Ra::Core::Math::toRad );
        m_camera->setZNear( 0.1_ra );
        m_camera->setZFar( 100_ra );

        auto aabb      = Ra::Engine::RadiumEngine::getInstance()->computeSceneAabb();
        Scalar f       = m_camera->getFOV();
        Scalar a       = m_camera->getAspect();
        const Scalar r = ( aabb.max() - aabb.min() ).norm() / 2_ra;
        const Scalar x = r / std::sin( f / 2_ra );
        const Scalar y = r / std::sin( f * a / 2_ra );
        Scalar d       = std::max( std::max( x, y ), 0.001_ra );

        m_camera->setPosition(
            Ra::Core::Vector3( aabb.center().x(), aabb.center().y(), aabb.center().z() + d ) );
        m_camera->setDirection( Ra::Core::Vector3( 0_ra, 0_ra, -1_ra ) );
        Scalar zfar =
            std::max( d + ( aabb.max().z() - aabb.min().z() ) * 2_ra, m_camera->getZFar() );
        m_camera->setZFar( zfar );
    }
    else {
        cameraManager->activate( camIdx );
        m_camera = cameraManager->getActiveCamera();
    }
}

void CLIViewer::setImageNamePrefix( std::string s ) {
    m_parameters.m_imgPrefix = std::move( s );
}

void CLIViewer::showWindow( bool on, OpenGLContext::EventMode mode, float delay ) {
    m_exposedWindow = on;
    if ( m_exposedWindow ) {
        m_glContext.resize( m_parameters.m_size );
        m_glContext.show( mode, delay );
    }
    else {
        m_glContext.hide();
    }
}

void CLIViewer::renderLoop( std::function<void( float )> render ) {
    m_glContext.renderLoop( render );
}

void CLIViewer::resize( int width, int height ) {
    if ( m_renderer ) {
        m_renderer->resize( width, height );
        m_camera->setViewport( width, height );
    }
}

} // namespace Headless
} // namespace Ra
