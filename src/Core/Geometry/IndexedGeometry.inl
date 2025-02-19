#pragma once

#include <Core/Geometry/IndexedGeometry.hpp>
#include <memory>

namespace Ra {
namespace Core {
namespace Geometry {

//  GeometryIndexLayerBase
inline GeometryIndexLayerBase::GeometryIndexLayerBase( const GeometryIndexLayerBase& other ) :
    ObjectWithSemantic( other.semantics() ) {}

inline GeometryIndexLayerBase&
GeometryIndexLayerBase::operator=( const GeometryIndexLayerBase& other ) {
    CORE_UNUSED( other );
    CORE_ASSERT( semantics() == other.semantics(),
                 "Try to assign object with different semantics" );
    return *this;
}

inline GeometryIndexLayerBase& GeometryIndexLayerBase::operator=( GeometryIndexLayerBase&& other ) {
    CORE_UNUSED( other );
    CORE_ASSERT( semantics() == other.semantics(),
                 "Try to assign GeometryIndexLayer of different type" );
    return *this;
}

inline GeometryIndexLayerBase::~GeometryIndexLayerBase() {}

inline bool GeometryIndexLayerBase::operator==( const GeometryIndexLayerBase& ) const {
    return false;
}

// GeometryIndexLayer

template <typename T>
inline typename GeometryIndexLayer<T>::IndexContainerType& GeometryIndexLayer<T>::collection() {
    return m_collection;
}

template <typename T>
const typename GeometryIndexLayer<T>::IndexContainerType&
GeometryIndexLayer<T>::collection() const {
    return m_collection;
}

template <typename T>
inline bool GeometryIndexLayer<T>::append( const GeometryIndexLayerBase& other ) {
    if ( shareSemantic( other ) ) {
        const auto& othercasted = static_cast<const GeometryIndexLayer<T>&>( other );
        m_collection.insert(
            m_collection.end(), othercasted.collection().begin(), othercasted.collection().end() );
        return true;
    }
    return false;
}

template <typename T>
inline bool GeometryIndexLayer<T>::operator==( const GeometryIndexLayerBase& other ) const {
    if ( shareSemantic( other ) ) {
        const auto& othercasted = static_cast<const GeometryIndexLayer<T>&>( other );
        return othercasted.collection() == m_collection;
    }
    return false;
}

template <typename T>
inline size_t GeometryIndexLayer<T>::getSize() const {
    return m_collection.size();
}

template <typename T>
inline size_t GeometryIndexLayer<T>::getNumberOfComponents() const {
    return IndexType::RowsAtCompileTime;
}

template <typename T>
inline size_t GeometryIndexLayer<T>::getBufferSize() const {
    return m_collection.size() * sizeof( IndexType );
}

template <typename T>
inline int GeometryIndexLayer<T>::getStride() const {
    return sizeof( IndexType );
}

template <typename T>
inline const void* GeometryIndexLayer<T>::dataPtr() const {
    return m_collection.data();
}

template <typename T>
inline std::unique_ptr<GeometryIndexLayerBase> GeometryIndexLayer<T>::clone() {
    auto copy          = std::make_unique<GeometryIndexLayer<T>>( *this );
    copy->m_collection = m_collection;
    return copy;
}

// MultiIndexedGeometry
inline MultiIndexedGeometry::~MultiIndexedGeometry() {
    detachAll();
    clear();
}

inline bool
MultiIndexedGeometry::containsLayer( const MultiIndexedGeometry::LayerKeyType& layerKey ) const {
    return m_indices.find( layerKey ) != m_indices.end();
}

inline bool
MultiIndexedGeometry::containsLayer( const MultiIndexedGeometry::LayerSemanticCollection& semantics,
                                     const std::string& layerName ) const {
    return containsLayer( { semantics, layerName } );
}

inline size_t
MultiIndexedGeometry::countLayers( const MultiIndexedGeometry::LayerKeyType& layerKey ) const {
    return m_indices.count( layerKey );
}

inline size_t
MultiIndexedGeometry::countLayers( const MultiIndexedGeometry::LayerSemanticCollection& semantics,
                                   const std::string& layerName ) const {
    return countLayers( { semantics, layerName } );
}

inline const GeometryIndexLayerBase&
MultiIndexedGeometry::getLayer( const MultiIndexedGeometry::LayerKeyType& layerKey ) const {
    return *( m_indices.at( layerKey ).second.get() );
}

inline GeometryIndexLayerBase& MultiIndexedGeometry::getLayerWithLock(
    const MultiIndexedGeometry::LayerSemanticCollection& semantics,
    const std::string& layerName ) {
    return getLayerWithLock( { semantics, layerName } );
}

inline void
MultiIndexedGeometry::unlockLayer( const MultiIndexedGeometry::LayerSemanticCollection& semantics,
                                   const std::string& layerName ) {
    unlockLayer( { semantics, layerName } );
}

[[nodiscard]] inline auto MultiIndexedGeometry::layerKeys() const {
    return Utils::map_keys( m_indices );
}

// PointCloudIndexLayer
inline PointCloudIndexLayer::PointCloudIndexLayer() :
    GeometryIndexLayer( PointCloudIndexLayer::staticSemanticName ) {}
inline PointCloudIndexLayer::PointCloudIndexLayer( size_t n ) :
    GeometryIndexLayer( PointCloudIndexLayer::staticSemanticName ) {
    collection().resize( n );
    collection().getMap() = IndexContainerType::Matrix::LinSpaced( n, 0, n - 1 );
}
template <class... SemanticNames>
inline PointCloudIndexLayer::PointCloudIndexLayer( SemanticNames... names ) :
    GeometryIndexLayer( PointCloudIndexLayer::staticSemanticName, names... ) {}
// TriangleIndexLayer
inline TriangleIndexLayer::TriangleIndexLayer() :
    GeometryIndexLayer( TriangleIndexLayer::staticSemanticName ) {}
template <class... SemanticNames>
inline TriangleIndexLayer::TriangleIndexLayer( SemanticNames... names ) :
    GeometryIndexLayer( TriangleIndexLayer::staticSemanticName, names... ) {}
// QuadIndexLayer
inline QuadIndexLayer::QuadIndexLayer() :
    GeometryIndexLayer( QuadIndexLayer::staticSemanticName ) {}
template <class... SemanticNames>
inline QuadIndexLayer::QuadIndexLayer( SemanticNames... names ) :
    GeometryIndexLayer( QuadIndexLayer::staticSemanticName, names... ) {}
// PolyIndexLayer
inline PolyIndexLayer::PolyIndexLayer() :
    GeometryIndexLayer( PolyIndexLayer::staticSemanticName ) {}
template <class... SemanticNames>
inline PolyIndexLayer::PolyIndexLayer( SemanticNames... names ) :
    GeometryIndexLayer( PolyIndexLayer::staticSemanticName, names... ) {}
// LineIndexLayer
inline LineIndexLayer::LineIndexLayer() :
    GeometryIndexLayer( LineIndexLayer::staticSemanticName ) {}
template <class... SemanticNames>
inline LineIndexLayer::LineIndexLayer( SemanticNames... names ) :
    GeometryIndexLayer( LineIndexLayer::staticSemanticName, names... ) {}
// IndexedGeometry
template <typename T>
inline IndexedGeometry<T>::IndexedGeometry() {
    auto layer          = std::make_unique<DefaultLayerType>();
    m_mainIndexLayerKey = { layer->semantics(), "" };
    addLayer( std::move( layer ) );
}

template <typename T>

inline const typename IndexedGeometry<T>::IndexContainerType&
IndexedGeometry<T>::getIndices() const {
    const auto& abstractLayer = getLayer( m_mainIndexLayerKey );
    return static_cast<const IndexedGeometry<T>::DefaultLayerType&>( abstractLayer ).collection();
}
template <typename T>
inline typename IndexedGeometry<T>::IndexContainerType& IndexedGeometry<T>::getIndicesWithLock() {
    auto& abstractLayer = getLayerWithLock( m_mainIndexLayerKey );
    return static_cast<IndexedGeometry<T>::DefaultLayerType&>( abstractLayer ).collection();
}

template <typename T>
inline void IndexedGeometry<T>::indicesUnlock() {
    unlockLayer( m_mainIndexLayerKey );
}

template <typename T>
inline void IndexedGeometry<T>::setIndices( IndexContainerType&& indices ) {
    auto& abstractLayer = getLayerWithLock( m_mainIndexLayerKey );
    static_cast<IndexedGeometry<T>::DefaultLayerType&>( abstractLayer ).collection() =
        std::move( indices );
    notify();
}

template <typename T>
inline void IndexedGeometry<T>::setIndices( const IndexContainerType& indices ) {
    auto& abstractLayer = getLayerWithLock( m_mainIndexLayerKey );
    static_cast<IndexedGeometry<T>::DefaultLayerType&>( abstractLayer ).collection() = indices;
    notify();
}

template <typename T>
inline const typename IndexedGeometry<T>::LayerKeyType& IndexedGeometry<T>::getLayerKey() const {
    return m_mainIndexLayerKey;
}

} // namespace Geometry
} // namespace Core
} // namespace Ra
