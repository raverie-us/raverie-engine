// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

template <typename ArrayType>
class BoundArrayRange;

/// Base functionality for binding a pre-existing array. Currently designed as
/// a templated base to reduce code duplication, especially while prototyping.
/// Possibly split to individual classes (due to differences, code
/// documentation, etc...) or make generic enough to use elsewhere later.
template <typename OwningType, typename DataTypeT>
class BoundArray : public SafeId32Object
{
public:
  typedef BoundArray<OwningType, DataTypeT> SelfType;
  typedef DataTypeT DataType;
  typedef Array<DataTypeT> ArrayType;
  typedef BoundArrayRange<SelfType> RangeType;

  BoundArray()
  {
    mBoundArray = nullptr;
  }

  BoundArray(ArrayType* arrayData)
  {
    mBoundArray = arrayData;
  }

  Vec3Param operator[](size_t index) const
  {
    return (*mBoundArray)[index];
  }

  DataTypeT Get(int arrayIndex) const
  {
    if (!ValidateArrayIndex(arrayIndex))
      return DataTypeT();

    return (*mBoundArray)[arrayIndex];
  }

  int GetCount() const
  {
    return (int)(*mBoundArray).Size();
  }

  RangeType GetAll()
  {
    return RangeType(this);
  }

  bool ValidateArrayIndex(int arrayIndex) const
  {
    int count = GetCount();
    if (arrayIndex >= count)
    {
      String msg = String::Format("Index %d is invalid. Array only contains %d element(s).", arrayIndex, count);
      DoNotifyException("Invalid index", msg);
      return false;
    }
    return true;
  }

  /// The array that this class represents. Assumes that this data cannot go
  /// away without this class also going away.
  ArrayType* mBoundArray;
};

template <typename ArrayTypeT>
class BoundArrayRange
{
public:
  typedef BoundArrayRange<ArrayTypeT> SelfType;
  typedef ArrayTypeT ArrayType;
  // Required for binding
  typedef typename ArrayTypeT::DataType FrontResult;

  BoundArrayRange()
  {
    mArray = nullptr;
    mIndex = 0;
  }

  BoundArrayRange(ArrayTypeT* array)
  {
    mArray = array;
    mIndex = 0;
  }

  bool Empty()
  {
    // Validate that the range hasn't been destroyed
    ArrayType* array = mArray;
    if (array == nullptr)
    {
      DoNotifyException("Range is invalid", "The array this range is referencing has been destroyed.");
      return true;
    }

    return mIndex >= (size_t)array->GetCount();
  }

  FrontResult Front()
  {
    // If the range is empty (or the range has been destroyed) then throw an
    // exception.
    if (Empty())
    {
      DoNotifyException("Invalid Range Operation", "Cannot access an item in an empty range.");
      return FrontResult();
    }
    return mArray->Get(mIndex);
  }

  void PopFront()
  {
    ++mIndex;
  }

  SelfType& All()
  {
    return *this;
  }

private:
  HandleOf<ArrayTypeT> mArray;
  size_t mIndex;
};

class IndexedHalfEdgeVertex;
class IndexedHalfEdge;
class IndexedHalfEdgeFace;
class IndexedHalfEdgeMesh;

class IndexedHalfEdgeMeshVertexArray : public BoundArray<IndexedHalfEdgeMesh, Vec3>
{
public:
  RaverieDeclareType(IndexedHalfEdgeMeshVertexArray, TypeCopyMode::ReferenceType);
};

class IndexedHalfEdgeMeshEdgeArray : public BoundArray<IndexedHalfEdgeMesh, IndexedHalfEdge*>
{
public:
  RaverieDeclareType(IndexedHalfEdgeMeshEdgeArray, TypeCopyMode::ReferenceType);
};

class IndexedHalfEdgeFaceEdgeIndexArray : public BoundArray<IndexedHalfEdgeFace, int>
{
public:
  RaverieDeclareType(IndexedHalfEdgeFaceEdgeIndexArray, TypeCopyMode::ReferenceType);
};

class IndexedHalfEdgeMeshFaceArray : public BoundArray<IndexedHalfEdgeMesh, IndexedHalfEdgeFace*>
{
public:
  RaverieDeclareType(IndexedHalfEdgeMeshFaceArray, TypeCopyMode::ReferenceType);
};

class IndexedHalfEdge : public SafeId32Object
{
public:
  RaverieDeclareType(IndexedHalfEdge, TypeCopyMode::ReferenceType);

  /// Index of the tail vertex in the vertex list.
  int mVertexIndex;
  /// Index of the twin edge (edge on adjacent face).
  int mTwinIndex;
  /// Index of the face that owns this edge.
  int mFaceIndex;
};

class IndexedHalfEdgeFace : public SafeId32Object
{
public:
  RaverieDeclareType(IndexedHalfEdgeFace, TypeCopyMode::ReferenceType);

  typedef Array<int> EdgeArray;
  typedef IndexedHalfEdgeFaceEdgeIndexArray BoundEdgeArray;

  IndexedHalfEdgeFace();

  /// The list of half-edges owned by this face.
  BoundEdgeArray* GetEdges();

  /// The actual edge array of indices.
  EdgeArray mEdges;
  /// The bound array of edges. This allows safe referencing in script.
  BoundEdgeArray mBoundEdges;
};

/// An index based half-edge mesh. This is an edge-centric mesh representation
/// that allows efficient traversal from edges to anywhere else. Each edge is
/// broken up into two half-edges, one for each face it's a part of. Most
/// sub-shapes contain indices back into the 'global' list (e.g. an edge
/// contains the index of the vertex in the mesh's vertex list). This mesh
/// format is meant for efficient traversal, but manipulation is not as easy
/// with indices. This should be loaded into a more efficient format (such as
/// pointers) if manipulating a mesh.
class IndexedHalfEdgeMesh : public ReferenceCountedObject
{
public:
  RaverieDeclareType(IndexedHalfEdgeMesh, TypeCopyMode::ReferenceType);

  typedef Array<Vec3> VertexArray;
  typedef Array<IndexedHalfEdge*> EdgeArray;
  typedef Array<IndexedHalfEdgeFace*> FaceArray;
  typedef IndexedHalfEdgeMeshVertexArray BoundVertexArray;
  typedef IndexedHalfEdgeMeshEdgeArray BoundEdgeArray;
  typedef IndexedHalfEdgeMeshFaceArray BoundFaceArray;

  IndexedHalfEdgeMesh();

  /// Create the buffers to store the provided mesh size.
  void Create(int vertexCount, int edgeCount, int faceCount);
  /// Clear all mesh data.
  void Clear();

  /// The list of vertices in this mesh.
  BoundVertexArray* GetVertices();
  /// The list of edge in this mesh.
  BoundEdgeArray* GetEdges();
  /// The list of faces in this mesh.
  BoundFaceArray* GetFaces();

  // The internal data that's efficient to work with at run-time
  VertexArray mVertices;
  EdgeArray mEdges;
  FaceArray mFaces;
  // The proxy arrays for binding. This allows safe referencing of the
  // underlying primitives.
  BoundVertexArray mBoundVertices;
  BoundEdgeArray mBoundEdges;
  BoundFaceArray mBoundFaces;
};

} // namespace Raverie
