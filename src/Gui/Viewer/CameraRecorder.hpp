#pragma once

#include <Core/Asset/Camera.hpp>
#include <Core/Math/Math.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Observable.hpp>
#include <Gui/RaGui.hpp>
#include <qobject.h>
#include <qtimer.h>
#include <vector>

namespace Ra {
namespace Gui {

class RA_GUI_API CameraRecorder : QObject
{
  public:
    CameraRecorder( Core::Asset::Camera* camera,int captureInterval = 1000);

    ~CameraRecorder();

    void toggleRecord(bool reset);

    void setCaptureInterval( int ms );

    int captureInterval();

    std::vector<Core::Vector3>* getPositions();

  private:

    long m_time;
    int m_captureInterval;
    void record();
    Core::Asset::Camera* m_camera;
    QTimer* m_timer;
    QMetaObject::Connection m_connection; 
    bool m_recording = false;
    std::vector<Core::Vector3>*m_positions, *m_directions,*m_up_vectors;
};

} // namespace Gui
} // namespace Ra