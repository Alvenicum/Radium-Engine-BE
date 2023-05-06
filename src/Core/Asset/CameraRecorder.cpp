#include "Core/Asset/Camera.hpp"
#include "Core/Types.hpp"
#include <Core/Asset/CameraRecorder.hpp>
#include <cstdlib>
#include <iterator>
#include <vector>

namespace Ra {
namespace Core {
namespace Asset {

CameraRecorder::CameraRecorder( Camera* camera )
:m_camera(camera)
{
    m_positions =new std::vector<Vector3>();
    m_directions=new std::vector<Vector3>();
};

CameraRecorder::~CameraRecorder(){
  delete m_positions;
  delete m_directions;
};

void CameraRecorder::interpolate(){
  
}

void CameraRecorder::record() {
  std::cout<<m_camera->getPosition() << std::endl;
  m_positions->emplace_back(m_camera->getPosition());
  m_positions->emplace_back(m_camera->getDirection());
}; 

void CameraRecorder::rewind( long time )
{};

} // namespace Asset
} // namespace Core
} // namespace Ra