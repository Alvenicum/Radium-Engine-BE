#include "Core/Asset/Camera.hpp"
#include "Core/Types.hpp"
#include <Gui/Viewer/CameraRecorder.hpp>
#include <cstdlib>
#include <iterator>
#include <vector>

namespace Ra {
namespace Gui {
CameraRecorder::CameraRecorder( Core::Asset::Camera* camera, int captureInterval ) :
    m_captureInterval( captureInterval ), m_camera( camera ) {
    m_positions  = new std::vector<Core::Vector3>();
    m_directions = new std::vector<Core::Vector3>();
    m_up_vectors  = new std::vector<Core::Vector3>();
    m_timer      = new QTimer( this );
}

CameraRecorder::~CameraRecorder() {
    delete m_positions;
    delete m_directions;
    delete m_up_vectors;
    delete m_timer;
};

/* void CameraRecorder::interpolate() {} */

std::vector<Core::Vector3>* CameraRecorder::getPositions() {
    return m_positions;
}

void CameraRecorder::record() {

    m_positions->emplace_back( m_camera->getPosition() );
    m_directions->emplace_back( m_camera->getDirection() );
    m_up_vectors->emplace_back( m_camera->getUpVector() );
}

void CameraRecorder::toggleRecord( bool reset ) {
    if ( m_recording ) {
        m_timer->stop();
        QObject::disconnect( m_connection );
    }
    else {
        if ( reset ) {
            m_positions->clear();
            m_directions->clear();
            m_up_vectors->clear();
        }
        m_timer->setInterval( m_captureInterval );
        m_connection = QObject::connect( m_timer, &QTimer::timeout, [this]() { this->record(); } );
        m_timer->start();
    }
};

void CameraRecorder::setCaptureInterval( int ms ) {
    m_captureInterval = ms;
}

int CameraRecorder::captureInterval() {
    return m_captureInterval;
}

} // namespace Gui
} // namespace Ra