///////////////////////////////////////////////////////////////////////////////
///
/// \file ConvexMesh.hpp
/// Declaration of the ConvexMesh class.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Geometry
{
class Hull3D;

///Simple mesh class to store information for rendering a mesh as well as the
///adjacency information between the mesh's vertices, edges, and faces. Used 
///primarily for debugging.
class ConvexMesh
{
public:
  struct Vertex;
  struct Edge;
  struct Face;
  typedef uint                FaceId;
  typedef Array<uint>   UintArray;
  typedef Array<Vertex> VertexArray;
  typedef Array<Edge>   EdgeArray;
  typedef Array<Face>   FaceArray;
  typedef Array<FaceId> FaceIdArray;

public:
  ConvexMesh(void);
  ~ConvexMesh(void);

  ///Copies the information generated from the given hulling algorithm.
  void CopyInfo(Hull3D& hull);

  ///Returns the number of vertices in the mesh.
  uint GetVertexCount(void) const;

  ///Returns the number of edges in the mesh.
  uint GetEdgeCount(void) const;

  ///Returns the number of faces in the mesh.
  uint GetFaceCount(void) const;

  ///Returns a pointer to an array of the mesh's vertices.
  const Vertex* GetVertices(void) const;

  ///Returns a pointer to an array of the mesh's edges.
  const Edge* GetEdges(void) const;

  ///Returns a pointer to an array of the mesh's faces.
  const Face* GetFaces(void) const;

  ///Returns the mesh's vertex at the given index.
  const Vertex& GetVertex(uint index) const;

  ///Returns the normal to the plane that the specified edge lies on.
  Vec3 GetEdgePlaneNormal(uint index) const;

  ///Returns the mesh's face at the given index.
  const Face& GetFace(uint index) const;

  ///Returns the number of faces that the specified vertex is a part of and
  ///modifies the "faces" parameter to point at those faces.
  uint GetFacesSharedByVertex(uint vertexIndex, const uint* faces) const;

  ///Provides the two faces connected by the specified edge.
  void GetFacesSharedByEdge(uint edgeIndex, const uint*& faces) const;

  ///Calculates and returns the unit-length normal to the specified face.
  Vec3 GetFaceNormal(uint faceIndex) const;

  ///Generates vertex and index buffer, returns the mesh's triangle count
  uint GenerateRenderData(Vec3** vertexBuffer, uint** indexBuffer);

  ///Renders the mesh using debug draw triangles.
  void Draw(void) const;

  ///Renders the Gauss map of the mesh, primarily used for debugging.
  void RenderGaussMap(void);

private:
  VertexArray mVertices;
  EdgeArray   mEdges;
  FaceArray   mFaces;

public:
  ///Defines each point on the mesh. Each point consists of a position and the
  ///N number of faces it is a part of.
  struct Vertex
  {
    Vertex(void);
    ~Vertex(void);

    Vec3        Position;
    uint        Id;
    FaceIdArray Faces;
  };

  ///Defines each edge connecting two points and two faces.
  struct Edge
  {
    Edge(void);

    ///Sets the faces of the edge. Should only be called twice, as an edge 
    ///cannot be a part of more than two faces.
    void SetFace(uint face);

    ///Calculates and returns the normal to the plane that this edge resides on.
    Vec3 GetPlaneNormal(const ConvexMesh& convexMesh) const;

    uint Points[2];
    uint Faces[2];
    uint Id;
  };

  ///Defines each face of the mesh. Each face consists of three points and
  ///three edges.
  struct Face
  {
    Face(void);

    uint Points[3];
    uint Id;
    uint Edges[3];
  };
};

typedef const ConvexMesh& MeshParam;
typedef ConvexMesh&       MeshRef;

}// namespace Geometry
