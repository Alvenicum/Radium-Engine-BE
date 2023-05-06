#pragma once

#include <Core/Math/Math.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Observable.hpp>
#include <Core/Asset/Camera.hpp>

#include <vector>

namespace Ra {
namespace Core {
namespace Asset {

class RA_CORE_API CameraRecorder
{
  public:

    CameraRecorder( Camera* camera );

    ~CameraRecorder();

    void record();

    void rewind( long time);

    void interpolate();

  private:
  long time;
  Camera* m_camera;
  bool m_recording = false;
  std::vector<Core::Vector3>* m_positions,*m_directions;
};

}
}
}