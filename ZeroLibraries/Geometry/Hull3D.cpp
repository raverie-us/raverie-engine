///////////////////////////////////////////////////////////////////////////////
///
/// \file Hull3D.cpp
/// Algorithm to take a point set and generate a convex mesh from it.
///
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Geometry
{

Zero::Memory::Pool* Hull3D::sVertexPool = new Zero::Memory::Pool("Hull3D::Vertices", Zero::Memory::GetRoot(), sizeof(Vertex), 12);
Zero::Memory::Pool* Hull3D::sEdgePool = new Zero::Memory::Pool("Hull3D::Edges", Zero::Memory::GetRoot(), sizeof(Edge), 30);
Zero::Memory::Pool* Hull3D::sFacePool = new Zero::Memory::Pool("Hull3D::Faces", Zero::Memory::GetRoot(), sizeof(Face), 20);

ImplementOverloadedNewWithAllocator(Hull3D::Vertex,Hull3D::sVertexPool);
ImplementOverloadedNewWithAllocator(Hull3D::Edge,Hull3D::sEdgePool);
ImplementOverloadedNewWithAllocator(Hull3D::Face,Hull3D::sFacePool);

// The epsilon for visible checks.
real epsilon = real(0.00001);

void DrawTriangleList(Hull3D::FaceList& list, uint drawFlags, ByteColor color)
{
  Hull3D::FaceList::range faces = list.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Hull3D::Face& face = faces.Front();
    Vec3 p0 = face.Vertices[0]->Position;
    Vec3 p1 = face.Vertices[1]->Position;
    Vec3 p2 = face.Vertices[2]->Position;
    //Setting the filled flag on triangles to false does not make them wireframe.
    //For now do this check and draw wireframe as 3 lines.
    if((drawFlags & HullDebugDrawFlags::WireFrameHull) != 0)
    {
      Zero::gDebugDraw->Add(Zero::Debug::Line(p0, p1).Color(color));
      Zero::gDebugDraw->Add(Zero::Debug::Line(p1, p2).Color(color));
      Zero::gDebugDraw->Add(Zero::Debug::Line(p2, p0).Color(color));
    }
    else
      Zero::gDebugDraw->Add(Zero::Debug::Triangle(p0,p1,p2).Color(color).Border(true).Alpha(100));
  }
}

void DrawPointList(Hull3D::VertexList& list, ByteColor color)
{
  Hull3D::VertexList::range vertices = list.All();
  for(; !vertices.Empty(); vertices.PopFront())
  {
    Vec3 pos = vertices.Front().Position;
    Zero::Debug::Sphere sphere(pos, real(0.025));
    Zero::gDebugDraw->Add(sphere.Color(color));
  }
}

Hull3D::Vertex::Vertex(uint id, Vec3Param pos)
{
  Id = id;
  Position = pos;
  mNewEdge = nullptr;
}

Hull3D::Edge::Edge(uint id, Vertex* v0, Vertex* v1)
{
  Id = id;
  SetFaces(nullptr, nullptr);
  SetVertices(v0, v1);
}

void Hull3D::Edge::SetVertices(Vertex* v0, Vertex* v1)
{
  EndPoints[0] = v0;
  EndPoints[1] = v1;
}

void Hull3D::Edge::SetFaces(Face* f0, Face* f1)
{
  AdjFaces[0] = f0;
  AdjFaces[1] = f1;
}

Hull3D::Face::Face(uint id, Vertex* v0, Vertex* v1, Vertex* v2)
{
  Id = id;
  IsVisible = false;
  SetEdges(nullptr, nullptr, nullptr);
  SetVertices(v0,v1,v2);
}

void Hull3D::Face::SetVertices(Vertex* v0, Vertex* v1, Vertex* v2)
{
  Vertices[0] = v0;
  Vertices[1] = v1;
  Vertices[2] = v2;
}

void Hull3D::Face::SetEdges(Edge* e0, Edge* e1, Edge* e2)
{
  Edges[0] = e0;
  Edges[1] = e1;
  Edges[2] = e2;
}

Vec3 Hull3D::Face::GetNormal()
{
  Vec3 e0 = Vertices[1]->Position - Vertices[0]->Position;
  Vec3 e1 = Vertices[2]->Position - Vertices[0]->Position;
  Vec3 normal = Math::Cross(e0, e1);
  normal.AttemptNormalize();
  return normal;
}

bool Hull3D::Face::VerifyVisible(Vertex* v)
{
  Vec3 normal = GetNormal();
  //check all 3 points for floating point robustness
  real distance1 = Math::Dot(v->Position - Vertices[0]->Position,normal);
  real distance2 = Math::Dot(v->Position - Vertices[1]->Position,normal);
  real distance3 = Math::Dot(v->Position - Vertices[2]->Position,normal);

  //if any projection is on the back face then we consider it on the back face
  if(distance1 < epsilon)
    return false;
  else if(distance2 < epsilon)
    return false;
  else if(distance3 < epsilon)
    return false;
  return true;
}

bool Hull3D::Face::TestVisible(Vertex* v)
{
  IsVisible = VerifyVisible(v);

  return IsVisible;
}

Hull3D::Hull3D()
{
  mEdgeCount = 0;
  mFaceCount = 0;
  mFailed = false;
}

Hull3D::~Hull3D()
{
  ClearHullData();
}

bool Hull3D::Build(Zero::Array<Vec3>& verties)
{
  return Build(verties.Data(),verties.Size());
}

bool Hull3D::Build(const Vec3* vertices, uint size)
{
  ClearHullData();

  //can't build a convex hull if there's not enough points
  if(size < 4)
    return false;

  //convert the vertices to a more workable format (intrusive list with ids)
  BuildVertices(vertices,size);
  //build the initial tetrahedron. If this fails then the
  //data points are the same, collinear or coplanar.
  if(ConstructInitialTetrahedron() == false)
    return false;

  //Call cleanup to put new edges and faces into the main lists.
  Cleanup();

#ifdef Hull3DDebug
  //debug sanity check.
  VerifyHull();
#endif

  //as long as there are vertices to add to the hull, keep adding.
  while(!mVertices.Empty())
  {
    //add one new point to incrementally expand the hull, if the add failed then we're broken
    if(!AddVertex())
      return false;
    //delete old faces and edges. Swap new edges and faces into the main list.
    Cleanup();

#ifdef Hull3DDebug
    //debug sanity check
    VerifyHull();
#endif
  }
  //sort down the ids for edges and faces, also count the
  //total number of edges and faces in this pass
  FixEdgeIndices();
  FixFaceIndices();
  //we successfully created a convex hull.
  return true;
}

bool Hull3D::DebugBuild(Vec3Ptr vertices, uint size)
{
  //can't build a convex hull if there's not enough points
  if(size < 4)
    return false;

  //convert the vertices to a more workable format (intrusive list with ids)
  BuildVertices(vertices,size);
  //build the initial tetrahedron. If this fails then the
  //data points are the same, collinear or coplanar.
  if(!ConstructInitialTetrahedron())
    return false;
  return true;
}

bool Hull3D::DebugStep()
{
  //don't continue if we've ever failed, we're already dead and can't recover
  if(mFailed)
    return true;

  //Call cleanup to put new edges and faces into the main lists.
  Cleanup();

  //if we're out of vertices then we're done
  if(mVertices.Empty())
    return true;

  //add one new point to incrementally expand the hull
  if(!AddVertex())
    return true;

  //do a debug check, this is a debug function after all
  VerifyHull();
  //we need to fix the indices so whoever looks at this will have valid indices.
  FixEdgeIndices();
  FixFaceIndices();

  //we're done if we ran out of vertices
  if(mVertices.Empty())
  {
    //since we're done, move the new edges and faces to the correct lists
    Cleanup();
    //since we did cleanup, we need to fix the edges and faces again
    FixEdgeIndices();
    FixFaceIndices();
    return true;
  }
  return false;
}

bool Hull3D::DebugDraw(uint drawFlags)
{
  //draw the vertices
  if(drawFlags & HullDebugDrawFlags::UnProcessedPoints)
    DrawPointList(mVertices,Color::Yellow);
  if(drawFlags & HullDebugDrawFlags::HullPoints)
    DrawPointList(mHullVertices,Color::Red);
  if(drawFlags & HullDebugDrawFlags::InteriorPoints)
    DrawPointList(mInternalVertices,Color::Green);

  //Zero::Debug::DefaultConfig config;
  //if((drawFlags & HullDebugDrawFlags::WireFrameHull) == 0)
  //  config.Alpha(100).Border(true);
  //else
  //  config.Filled(false);

  //draw the faces
  if(drawFlags & HullDebugDrawFlags::CurrentHull)
    DrawTriangleList(mFaces,drawFlags,Color::Orange);
  if(drawFlags & HullDebugDrawFlags::NewHull)
    DrawTriangleList(mNewFaces,drawFlags,Color::Blue);
  if(drawFlags & HullDebugDrawFlags::DeletedHull)
    DrawTriangleList(mVisibleFaces,drawFlags,Color::Red);

  //we're finished if we're out of vertices
  return mVertices.Empty();
}

Hull3D::VertexList::range Hull3D::GetHullVertices()
{
  return mHullVertices.All();
}

Hull3D::VertexList::range Hull3D::GetInternalVertices()
{
  return mInternalVertices.All();
}

Hull3D::EdgeList::range Hull3D::GetEdges()
{
  return mEdges.All();
}

Hull3D::FaceList::range Hull3D::GetFaces()
{
  return mFaces.All();
}

uint Hull3D::GetHullVertexCount()
{
  uint vertexCount = 0;
  VertexList::range vertices = mHullVertices.All();
  for(; !vertices.Empty(); vertices.PopFront())
    ++vertexCount;
  return vertexCount;
}

uint Hull3D::GetEdgeCount()
{
  return mEdgeCount;
}

uint Hull3D::GetFaceCount()
{
  return mFaceCount;
}

void Hull3D::ClearHullData(void)
{
  //delete all of the vertices, edges and face
  DeleteList(mVertices);
  DeleteList(mHullVertices);
  DeleteList(mInternalVertices);
  DeleteList(mEdges);
  DeleteList(mFaces);

  //these edges should already be empty, but make sure they're deleted anyways
  DeleteList(mToDeleteEdges);
  DeleteList(mNewEdges);
  DeleteList(mVisibleFaces);

  mEdgeCount = 0;
  mFaceCount = 0;
  mFailed = false;
}

void Hull3D::BuildVertices(const Vec3* vertices, uint size)
{
  // Keep track of the largest absolute value on each axis (needed to compute a proper epsilon)
  Vec3 maxVals = Vec3::cZero;
  //convert all the vertices to the list format
  for(uint i = 0; i < size; ++i)
  {
    Vertex* v = new Vertex(i,vertices[i]);
    mVertices.PushBack(v);

    Vec3 absV = Math::Abs(vertices[i]);
    maxVals = Math::Max(maxVals, absV);
  }

  // Formula taken from Dirk's Quick-Hull presentation (likely from "Matrix Computations").
  // If we use a hard-coded epsilon then either large meshes or small meshes will work, but not both.
  real epsilon = 3 * Math::Max(maxVals.x, Math::Max(maxVals.y, maxVals.z)) * FLT_EPSILON;


  //See if any points are really close to each other. These points can
  //cause numerical issues in the building of the hull. To fix this
  //just merge points together that are too close.
  VertexList::range r1 = mVertices.All();
  for(; !r1.Empty(); r1.PopFront())
  {
    Vertex& v1 = r1.Front();
    VertexList::range r2 = r1;
    r2.PopFront();
    while(!r2.Empty())
    {
      Vertex& v2 = r2.Front();
      r2.PopFront();

      real lengthSq = (v1.Position - v2.Position).LengthSq();
      if(lengthSq <= epsilon)
      {
        mVertices.Unlink(&v2);
        mInternalVertices.PushBack(&v2);
      }
    }
  }
}

bool Hull3D::ConstructInitialTetrahedron()
{
  //grab the first 3 points
  VertexList::range r = mVertices.All();
  Vertex* tet[4];
  tet[0] = &r.Front();
  r.PopFront();
  tet[1] = &r.Front();
  r.PopFront();
  tet[2] = &r.Front();
  r.PopFront();

  //check to see if the points are collinear
  Vec3 normal = Math::Cross(tet[1]->Position - tet[0]->Position, tet[2]->Position - tet[0]->Position);
  while(normal.LengthSq() == real(0.0))
  {
    //if we ran out of points, then all points are collinear
    if(r.Empty())
      return false;

    //if the points are collinear, shift all the points down by 1 and test the next set of 3 points
    tet[0] = tet[1];
    tet[1] = tet[2];
    tet[2] = &r.Front();
    r.PopFront();
    normal = Math::Cross(tet[1]->Position - tet[0]->Position, tet[2]->Position - tet[0]->Position);
  }

  //now get the next point and see if it is coplanar or not
  tet[3] = &r.Front();
  r.PopFront();
  real distance = Math::Dot(tet[3]->Position - tet[0]->Position,normal);
  while(distance == 0)
  {
    //if we ran out of points and didn't get a valid
    //tetrahedron then there is no valid convex hull
    if(r.Empty())
      return false;

    //the point was coplanar, grab another and test again
    tet[3] = &r.Front();
    r.PopFront();
    distance = Math::Dot(tet[3]->Position - tet[0]->Position,normal);
  }

  //take all of the points of the initial
  //tetrahedron and move them to the hull list.
  VertexList::Unlink(tet[0]);
  VertexList::Unlink(tet[1]);
  VertexList::Unlink(tet[2]);
  VertexList::Unlink(tet[3]);
  mHullVertices.PushBack(tet[0]);
  mHullVertices.PushBack(tet[1]);
  mHullVertices.PushBack(tet[2]);
  mHullVertices.PushBack(tet[3]);

  //if the 3rd point was on the positive side of the plane, then the
  //triangle is clockwise. Swap two vertices to make it counter-clockwise.
  if(distance > 0)
    Math::Swap(tet[0],tet[1]);

  //make all 4 faces, the order here is set up to make every face counter-clockwise.
  Face* f0 = MakeNewFace(tet[0],tet[1],tet[2]);
  Face* f1 = MakeNewFace(tet[1],tet[0],tet[3]);
  Face* f2 = MakeNewFace(tet[2],tet[1],tet[3]);
  Face* f3 = MakeNewFace(tet[0],tet[2],tet[3]);
  //now make all 6 new edges
  Edge* e0 = MakeNewEdge(tet[0],tet[1]);
  Edge* e1 = MakeNewEdge(tet[0],tet[2]);
  Edge* e2 = MakeNewEdge(tet[0],tet[3]);
  Edge* e3 = MakeNewEdge(tet[1],tet[2]);
  Edge* e4 = MakeNewEdge(tet[1],tet[3]);
  Edge* e5 = MakeNewEdge(tet[2],tet[3]);

  //set the correct edges for each face and the correct faces for each edge.
  f0->SetEdges(e0,e1,e3);
  f1->SetEdges(e0,e4,e2);
  f2->SetEdges(e3,e5,e4);
  f3->SetEdges(e1,e2,e5);
  e0->SetFaces(f0,f1);
  e1->SetFaces(f0,f3);
  e2->SetFaces(f1,f3);
  e3->SetFaces(f0,f2);
  e4->SetFaces(f1,f2);
  e5->SetFaces(f2,f3);
  return true;
}

bool Hull3D::AddVertex()
{
  //grab and remove the next vertex from the list
  Vertex* v = &mVertices.Front();
  mVertices.PopFront();

  //check the visibility of each face with this vertex
  if(!CheckVisibility(v))
  {
    //if the vertex was inside of every face then the vertex is inside the hull
    mInternalVertices.PushBack(v);
    return true;
  }

  //Now we know that the vertex is outside the hull.
  //There are now 3 cases for each edge based upon its adjacent faces.
  // 1. Both faces are visible. The edge will be removed so move it to the edges to delete.
  // 2. Both faces are not visible. The edge should stay in the list. Nothing to do.
  // 3. One face is visible, one isn't. This edge is on the border and needs to create a new face.
  //In case 3, the visible face will get deleted later, so we need to create a new face
  //that goes from that edge to the new vertex. New edges will need to be created
  //from each vertex on the edge to the new vertex. This is all done in the CreateNewConeFace function.

  bool valid = true;

  EdgeList::range edges = mEdges.All();
  while(!edges.Empty())
  {
    Edge& edge = edges.Front();
    edges.PopFront();

    //Since we add new edges to a separate list that we don't iterate through until we
    //finish adding this point, we should never have an edge that is missing a face.
    //If we do have one then our graph is invalid so just gracefully fail.
    if(edge.AdjFaces[0] == nullptr || edge.AdjFaces[1] == nullptr)
    {
      valid = false;
      break;
    }

    //if both faces are visible, we'll delete this edge later.
    if(edge.AdjFaces[0]->IsVisible && edge.AdjFaces[1]->IsVisible)
    {
      EdgeList::Unlink(&edge);
      mToDeleteEdges.PushBack(&edge);
      continue;
    }

    //if both faces are not visible, there's nothing to do
    if(!edge.AdjFaces[0]->IsVisible && !edge.AdjFaces[1]->IsVisible)
      continue;

    //create a new face from this edge to the new vertex.
    CreateNewConeFace(&edge,v);
  }

  //the new vertex is on the hull
  mHullVertices.PushBack(v);
  return valid;
}

bool Hull3D::CheckVisibility(Vertex* v)
{
  //see if any face is visible to the new vertex,
  //if not the point is inside the hull.
  bool isVisible = false;
  FaceList::range r = mFaces.All();
  while(!r.Empty())
  {
    Face* f = &r.Front();
    r.PopFront();

    if(f->TestVisible(v))
    {
      //mark that this vertex is visible to at least one face
      isVisible = true;
      //also, we know this face will get removed later, so move it to the
      //visible faces list so we don't have to find it later.
      FaceList::Unlink(f);
      mVisibleFaces.PushBack(f);
    }
  }
  return isVisible;
}

void Hull3D::CreateNewConeFace(Edge* incidentEdge, Vertex* newVertex)
{
  //store locally the vertices on the edge of the incident edge
  Vertex* edgeVertices[2] = {nullptr, nullptr};
  //store locally the edges that form the face when combined with the incident edge.
  //Edge[0] is from edgeVertex[0] to newVertex. Likewise for edge 1.
  Edge* faceEdges[2] = {nullptr, nullptr};
  for(uint i = 0; i < 2; ++i)
  {
    //grab the end point and it's edge to the new vertex
    edgeVertices[i] = incidentEdge->EndPoints[i];
    faceEdges[i] = edgeVertices[i]->mNewEdge;
    //if there was already an edge to the new vertex then we're all good
    if(faceEdges[i] != nullptr)
      continue;

    //otherwise create that new edge and save it in our local array.
    faceEdges[i] = MakeNewEdge(edgeVertices[i],newVertex);
    edgeVertices[i]->mNewEdge = faceEdges[i];
  }

  //find out which face was visible on the incident edge, also save its index.
  //The index is used so we can set the correct adjacent face pointer on the
  //incident edge to the newly created face.
  uint visibleFaceIndex = 0;
  Face* visibleFace = nullptr;
  for(; visibleFaceIndex < 2; ++visibleFaceIndex)
  {
    visibleFace = incidentEdge->AdjFaces[visibleFaceIndex];
    if(visibleFace->IsVisible)
      break;
  }

  //we want to make sure that the new face we create has the correct winding order.
  //The visible face should already be in the correct order so we need to make sure
  //we follow the same edge order as that face. To do this find the index of the first
  //point of the incident edge, if the next index is not the other vertex of the edge
  //then we have to flip the winding order.

  //find what index on the face is the first vertex on the edge
  uint index0 = 0;
  for(; index0 < 3; ++index0)
  {
    if(visibleFace->Vertices[index0] == edgeVertices[0])
      break;
  }
  //if the next vertex is not the other vertex of the edge then
  //swap v0 and v1 to guarantee counter-clockwise ordering.
  if(visibleFace->Vertices[(index0 + 1) % 3] != edgeVertices[1])
    Math::Swap(edgeVertices[0],edgeVertices[1]);

  //now make the new face with all of the vertices and edges
  Face* face = MakeNewFace(edgeVertices[0],edgeVertices[1],newVertex);
  face->SetEdges(faceEdges[0],faceEdges[1],incidentEdge);
  //override the old incident edge's visible face index with the newly created face
  incidentEdge->AdjFaces[visibleFaceIndex] = face;

  //for both of the (possibly) new face edges, we need to tell them the new face is
  //adjacent to them. To do this find one index that is null and set that
  //to the adjacent face. We only set one of the adjacent pointers even
  //if both are null. If both are null then the face on the other side of
  //this edge has not been created yet and should be fill out before the
  //end of this AddVertex call.
  for(uint i = 0; i < 2; ++i)
  {
    for(uint j = 0; j < 2; ++j)
    {
      if(faceEdges[i]->AdjFaces[j] == nullptr)
      {
        //only set 1 face, see the above comment
        faceEdges[i]->AdjFaces[j] = face;
        break;
      }
    }
  }
}

void Hull3D::Cleanup()
{
  //delete the old edges and faces
  DeleteList(mToDeleteEdges);
  DeleteList(mVisibleFaces);

  //if there are any new edges, move them to the main edge list
  if(!mNewEdges.Empty())
    mEdges.Splice(mEdges.End(),mNewEdges.All());
  //same for new faces
  if(!mNewFaces.Empty())
    mFaces.Splice(mFaces.End(),mNewFaces.All());

  //we need to find what internal vertices need to be removed now.
  //for every valid edge, move the endpoints to a valid list, anything left
  //over is inside the hull and will be moved to the internal list
  VertexList temp;
  EdgeList::range edges = mEdges.All();
  for(; !edges.Empty(); edges.PopFront())
  {
    Edge& edge = edges.Front();
    VertexList::Unlink(edge.EndPoints[0]);
    VertexList::Unlink(edge.EndPoints[1]);
    temp.PushBack(edge.EndPoints[0]);
    temp.PushBack(edge.EndPoints[1]);
  }

  //move everything left to the internal vertices list
  if(!mHullVertices.Empty())
    mInternalVertices.Splice(mInternalVertices.End(),mHullVertices.All());
  //move all of the points that survived back to being on the hull
  if(!temp.Empty())
    mHullVertices.Splice(mHullVertices.End(),temp.All());

  //now iterate through every remaining vertex and
  //clear out it's pointer to the new edge
  VertexList::range vertices = mHullVertices.All();
  for(; !vertices.Empty(); vertices.PopFront())
  {
    Vertex& vertex = vertices.Front();
    vertex.mNewEdge = nullptr;
  }
}

void Hull3D::FixEdgeIndices()
{
  mEdgeCount = 0;
  //both count how many edges we have and sort the indices back down to the
  //smallest set of numbers. Currently these indices are used for the
  //mesh generation so we need the smallest set of indices.
  EdgeList::range edges = mEdges.All();
  for(; !edges.Empty(); edges.PopFront())
  {
    Edge& edge = edges.Front();
    edge.Id = mEdgeCount;
    ++mEdgeCount;
  }
}

void Hull3D::FixFaceIndices()
{
  mFaceCount = 0;
  //same as FixEdgeIndices, just with the faces
  FaceList::range faces = mFaces.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Face& face = faces.Front();
    face.Id = mFaceCount;
    ++mFaceCount;
;
  }
}

Hull3D::Edge* Hull3D::MakeNewEdge(Vertex* v0, Vertex* v1)
{
  //create the new edge with its id and put it on the new edge list
  Edge* edge = new Edge(mEdgeCount,v0,v1);
  mNewEdges.PushBack(edge);
  ++mEdgeCount;
  return edge;
}

Hull3D::Face* Hull3D::MakeNewFace(Vertex* v0, Vertex* v1, Vertex* v2)
{
  //see MakeNewEdge, this is the same but for faces
  Face* face = new Face(mFaceCount,v0,v1,v2);
  mNewFaces.PushBack(face);
  ++mFaceCount;

  return face;
}

template <typename ListType>
void Hull3D::DeleteList(ListType& list)
{
  while(!list.Empty())
  {
    typename ListType::value_type* item = &list.Front();
    list.PopFront();
    delete item;
  }
}

void Hull3D::VerifyHull()
{
  VerifyWinding();
  VerifyGraph();
}

bool Hull3D::VerifyWinding()
{
  //make sure that every vertex is on the negative side of the face.
  bool allCorrect = true;
  FaceList::range faces = mFaces.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Face& face = faces.Front();

    VertexList::range vertices = mHullVertices.All();
    for(; !vertices.Empty(); vertices.PopFront())
    {
      Vertex& vertex = vertices.Front();
      if(face.VerifyVisible(&vertex))
      {
        ErrorIf(true,"Vertex[%d] is on the wrong side of Face[%d].",vertex.Id,face.Id);
        allCorrect = false;
      }
    }
  }
  faces = mNewFaces.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Face& face = faces.Front();

    VertexList::range vertices = mHullVertices.All();
    for(; !vertices.Empty(); vertices.PopFront())
    {
      Vertex& vertex = vertices.Front();
      if(face.VerifyVisible(&vertex))
      {
        ErrorIf(true,"Vertex[%d] is on the wrong side of Face[%d].",vertex.Id,face.Id);
        allCorrect = false;
      }
    }
  }
  return allCorrect;
}

bool Hull3D::VerifyGraph()
{
  bool facesValid = true;
  bool edgesValid = true;

  //make sure that every edge on the face thinks that face is adjacent to it
  FaceList::range faces = mFaces.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Face& face = faces.Front();
    for(uint i = 0; i < 3; ++i)
    {
      Edge* edge = face.Edges[i];
      if(edge->AdjFaces[0] != &face && edge->AdjFaces[1] != &face)
      {
        facesValid = false;
        ErrorIf(true,"Invalid face graph. Face[%d] has edges (%d,%d,%d) "
                     "but edge[%d] thinks faces (%d,%d) are adjacent to it.",
                     face.Id,face.Edges[0]->Id,face.Edges[1]->Id,face.Edges[2]->Id,
                     edge->Id,edge->AdjFaces[0]->Id,edge->AdjFaces[1]->Id);
      }
    }
  }

  //check the new faces for the same as above
  faces = mNewFaces.All();
  for(; !faces.Empty(); faces.PopFront())
  {
    Face& face = faces.Front();
    for(uint i = 0; i < 3; ++i)
    {
      Edge* edge = face.Edges[i];
      if(edge->AdjFaces[0] != &face && edge->AdjFaces[1] != &face)
      {
        facesValid = false;
        ErrorIf(true,"Invalid face graph. Face[%d] has edges (%d,%d,%d) "
          "but edge[%d] thinks faces (%d,%d) are adjacent to it.",
          face.Id,face.Edges[0]->Id,face.Edges[1]->Id,face.Edges[2]->Id,
          edge->Id,edge->AdjFaces[0]->Id,edge->AdjFaces[1]->Id);
      }
    }
  }

  //make sure that every face adjacent to an edge this that edge is a part of it
  EdgeList::range edges = mEdges.All();
  for(; !edges.Empty(); edges.PopFront())
  {
    Edge& edge = edges.Front();
    for(uint i = 0; i < 2; ++i)
    {
      Face* face = edge.AdjFaces[i];
      if(face->Edges[0] != &edge && face->Edges[1] != &edge && face->Edges[2] != &edge)
      {
        edgesValid = false;
        ErrorIf(true,"Invalid edge graph. Edge[%d] has faces (%d,%d)"
          "but face[%d] thinks edges (%d,%d,%d) are a part of it.",
          edge.Id,edge.AdjFaces[0]->Id,edge.AdjFaces[1]->Id,
          face->Id,face->Edges[0]->Id,face->Edges[1]->Id,face->Edges[2]->Id);
      }
    }
  }
  //check the new edges as well
  edges = mNewEdges.All();
  for(; !edges.Empty(); edges.PopFront())
  {
    Edge& edge = edges.Front();
    for(uint i = 0; i < 2; ++i)
    {
      Face* face = edge.AdjFaces[i];
      if(face->Edges[0] != &edge && face->Edges[1] != &edge && face->Edges[2] != &edge)
      {
        edgesValid = false;
        ErrorIf(true,"Invalid edge graph. Edge[%d] has faces (%d,%d)"
          "but face[%d] thinks edges (%d,%d,%d) are a part of it.",
          edge.Id,edge.AdjFaces[0]->Id,edge.AdjFaces[1]->Id,
          face->Id,face->Edges[0]->Id,face->Edges[1]->Id,face->Edges[2]->Id);
      }
    }
  }
  return edgesValid && facesValid;
}

}//namespace Geometry
