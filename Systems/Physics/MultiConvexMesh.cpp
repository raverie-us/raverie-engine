///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{


//-------------------------------------------------------------------Array Defines
PhysicsDefineArrayType(MultiConvexMeshVertexData);
PhysicsDefineArrayType(MultiConvexMeshIndexData);

//-------------------------------------------------------------------MultiConvexMeshSubMeshData
ZilchDefineType(MultiConvexMeshSubMeshData, builder, type)
{
  ZeroBindDocumented();

  // Explicitly bind the derived type versions of these functions
  // (auto-binding can do weird things with the base class overloads)
  ZilchFullBindGetterSetter(builder, type, &ZilchSelf::All, ZilchInstanceOverload(RangeType), nullptr, ZilchNoOverload, "All");
  ZilchFullBindMethod(builder, type, &ZilchSelf::Add, ZilchInstanceOverload(SubConvexMesh*), "Add", ZilchNoNames);
  ZilchFullBindMethod(builder, type, &ZilchSelf::RemoveAt, ZilchInstanceOverload(void, int), "RemoveAt", "arrayIndex");
  ZilchBindMethod(Get);
  ZilchBindMethod(Clear);
  ZilchBindGetterProperty(Count);
}

SubConvexMesh* MultiConvexMeshSubMeshData::Add()
{
  // Add a sub-mesh and mark the resource as modified
  SubConvexMesh* newMesh = mOwner->AddSubMesh();
  mOwner->ResourceModified();
  return newMesh;
}

void MultiConvexMeshSubMeshData::RemoveAt(int arrayIndex)
{
  if(!ValidateArrayIndex(arrayIndex))
    return;

  delete mOwner->mMeshes[arrayIndex];
  mOwner->mMeshes.EraseAt(arrayIndex);
  mOwner->ResourceModified();
}

void MultiConvexMeshSubMeshData::Clear()
{
  mOwner->ClearSubMeshes();
  mOwner->ResourceModified();
}

MultiConvexMeshSubMeshData::RangeType MultiConvexMeshSubMeshData::All()
{
  return RangeType(this);
}

//-------------------------------------------------------------------SubConvexMesh
ZilchDefineType(SubConvexMesh, builder, type)
{
  ZeroBindDocumented();

  ZeroBindTag(Tags::Physics);
  ZilchBindGetter(Indices);
  ZilchBindGetter(TriangleIndices);

  ZilchBindField(mMesh);
  ZilchBindFieldGetter(mValid);
}

SubConvexMesh::SubConvexMesh()
{
  mMesh = nullptr;
  mValid = false;
  mIndexData.mBoundArray = &mIndices;
  mTriangleIndexData.mBoundArray = &mTriangleIndices;
}

void SubConvexMesh::Serialize(Serializer& stream)
{
  SerializeNameDefault(mIndices, IndexArray());
  SerializeNameDefault(mTriangleIndices, IndexArray());
}

void SubConvexMesh::SetOwner(MultiConvexMesh* owner)
{
  mMesh = owner;
  mIndexData.mOwner = owner;
  mTriangleIndexData.mOwner = owner;
}

bool SubConvexMesh::Validate(VertexArrayParam verts)
{
  mValid = ValidateInternal(verts);
  // If we're invalid then default configure the center of mass and volume
  // to some dummy values to prevent crashes from using this.
  if(!mValid)
  {
    mCenterOfMass = Vec3::cZero;
    mVolume = 1.0;
  }
  return mValid;
}

bool SubConvexMesh::ValidateInternal(VertexArrayParam verts)
{
  // If we find an index outside the range of our vertices then we're invalid
  for(size_t i = 0; i < mIndices.Size(); ++i)
  {
    uint index = mIndices[i];
    if(index >= verts.Size())
      return false;
  }

  // Make sure we don't have dangling triangle indices
  if(mTriangleIndices.Size() % 3 != 0)
    return false;

  // Validate that all triangle indices are withing the valid range
  for(size_t i = 0; i < mTriangleIndices.Size(); ++i)
  {
    uint index = mTriangleIndices[i];
    if(index >= verts.Size())
      return false;
  }

  return true;
}

void SubConvexMesh::ComputeUniqueIndices()
{
  // Find all unique triangle indices
  HashSet<int> uniqueIndices;
  uniqueIndices.Append(mTriangleIndices.All());
  mIndices.Insert(mIndices.End(), uniqueIndices.All());
}

void SubConvexMesh::ComputeCenterOfMassAndVolume(VertexArrayParam verts)
{
  // Validated that we have data to compute from
  if(verts.Empty() || mTriangleIndices.Empty())
    return;

  Geometry::CalculateTriMeshCenterOfMassAndVolume(verts, mTriangleIndices, mCenterOfMass, mVolume);
}

Mat3 SubConvexMesh::ComputeInertiaTensor(VertexArrayParam verts, Vec3Param centerOfMass, Vec3Param scale)
{
  Mat3 inertiaTensor = Mat3::cIdentity;

  // Validated that we have data to compute from
  if(verts.Empty() || mTriangleIndices.Empty())
    return inertiaTensor;

  Geometry::CalculateTriMeshInertiaTensor(verts, mTriangleIndices, centerOfMass, &inertiaTensor, scale);
  return inertiaTensor;
}

Aabb SubConvexMesh::ComputeAabb(VertexArrayParam verts)
{
  mAabb.SetInvalid();
  // If we're invalid we could have indices into vertices that
  // don't exist so just return a bad aabb
  if(!mValid)
    return mAabb;

  for(uint i = 0; i < mIndices.Size(); ++i)
    mAabb.Expand(verts[mIndices[i]]);
  return mAabb;
}

void SubConvexMesh::Support(VertexArrayParam verts, Vec3Param direction, Vec3Ptr support)
{
  // Support should never be called on an invalid sub-mesh so no error checking should be needed

  // N version
  real longestDistance = -Math::PositiveMax();
  for(uint i = 0; i < mIndices.Size(); ++i)
  {
    Vec3Param curr = verts[mIndices[i]];
    real dist = Math::Dot(direction, curr);
    if(dist > longestDistance)
    {
      longestDistance = dist;
      *support = curr;
    }
  }
}

Vec3 SubConvexMesh::GetCenter()
{
  return mCenterOfMass;
}

bool SubConvexMesh::CastRay(const Ray& localRay, MultiConvexMesh* mesh, ProxyResult& result, BaseCastFilter& filter)
{
  // If the ray doesn't hit our aabb then just skip it
  if(Overlap(localRay, mAabb) == false)
    return false;

  bool triangleWasHit = false;
  for(size_t i = 0; i < GetTriangleCount(); ++i)
  {
    Triangle tri = GetTriangle(mesh->mVertices, i);

    Intersection::IntersectionPoint pointData;
    //  the ray for intersection with the triangle
    Intersection::IntersectionType tResult = Intersection::RayTriangle(localRay.Start, localRay.Direction, tri[0], tri[1], tri[2], &pointData);

    if(tResult <= Intersection::None)
      continue;

    if(pointData.T < result.mTime)
    {
      result.mTime = pointData.T;
      triangleWasHit = true;
      result.ShapeIndex = i;
      result.mPoints[0] = pointData.Points[0];
      result.mPoints[1] = pointData.Points[1];
      result.mContactNormal = tri.GetRawNormal();
    }
  }
  return triangleWasHit;
}

Triangle SubConvexMesh::GetTriangleIndexed(VertexArrayParam verts, uint index)
{
  Vec3 p0 = verts[mTriangleIndices[index]];
  Vec3 p1 = verts[mTriangleIndices[index + 1]];
  Vec3 p2 = verts[mTriangleIndices[index + 2]];
  return Triangle(p0, p1, p2);
}

Triangle SubConvexMesh::GetTriangle(VertexArrayParam verts, uint index)
{
  return GetTriangleIndexed(verts, index * 3);
}

uint SubConvexMesh::GetTriangleCount()
{
  return mTriangleIndices.Size() / 3;
}

void SubConvexMesh::Draw(VertexArrayParam verts, Mat4Param transform, bool drawEdges, bool drawFaces)
{
  if(!mValid)
    return;
  
  if(drawFaces)
    DrawFaces(verts, transform, Color::Lime);
  if(drawEdges)
    DrawEdges(verts, transform, Color::Lime);
}

void SubConvexMesh::Draw2d(VertexArrayParam verts, Mat4Param transform, bool drawEdges, bool drawFaces)
{
  if(!mValid)
    return;

  if(drawFaces)
    DrawFaces(verts, transform, Color::Lime);
  if(drawEdges)
    DrawEdges2d(verts, transform, Color::Lime);
}

void SubConvexMesh::DrawFaces(VertexArrayParam verts, Mat4Param transform, ByteColor color)
{
  uint triCount = GetTriangleCount();
  for(uint i = 0; i < triCount; ++i)
  {
    Triangle tri = GetTriangle(verts, i);
    tri.p0 = Math::TransformPoint(transform, tri.p0);
    tri.p1 = Math::TransformPoint(transform, tri.p1);
    tri.p2 = Math::TransformPoint(transform, tri.p2);
  
    ByteColor alphaColor = color;
    SetAlphaByte(alphaColor, 50);
    gDebugDraw->Add(Debug::Triangle(tri).Color(alphaColor));
  }
}

void SubConvexMesh::DrawEdges(VertexArrayParam verts, Mat4Param transform, ByteColor color)
{
  uint triCount = GetTriangleCount();
  for(uint i = 0; i < triCount; ++i)
  {
    Triangle tri = GetTriangle(verts, i);
    tri.p0 = Math::TransformPoint(transform, tri.p0);
    tri.p1 = Math::TransformPoint(transform, tri.p1);
    tri.p2 = Math::TransformPoint(transform, tri.p2);
    
    gDebugDraw->Add(Debug::Line(tri.p0, tri.p1).Color(color));
    gDebugDraw->Add(Debug::Line(tri.p1, tri.p2).Color(color));
    gDebugDraw->Add(Debug::Line(tri.p2, tri.p0).Color(color));
  }
}

void SubConvexMesh::DrawEdges2d(VertexArrayParam verts, Mat4Param transform, ByteColor color)
{
  uint faceIndexCount = mIndices.Size() / 2;
  for(uint i = 0; i < faceIndexCount; ++i)
  {
    uint currIndex = i;
    uint nextIndex = (i + 1) % faceIndexCount;

    Vec3 p0 = verts[mIndices[currIndex]];
    Vec3 p1 = verts[mIndices[nextIndex]];
    p0 = Math::TransformPoint(transform, p0);
    p1 = Math::TransformPoint(transform, p1);

    gDebugDraw->Add(Debug::Line(p0, p1).Color(color));

    currIndex = i + faceIndexCount;
    nextIndex = (i + 1) % faceIndexCount + faceIndexCount;

    Vec3 p2 = verts[mIndices[currIndex]];
    Vec3 p3 = verts[mIndices[nextIndex]];
    p2 = Math::TransformPoint(transform, p2);
    p3 = Math::TransformPoint(transform, p3);

    gDebugDraw->Add(Debug::Line(p2, p3).Color(color));

    gDebugDraw->Add(Debug::Line(p0, p2).Color(color));
    gDebugDraw->Add(Debug::Line(p1, p3).Color(color));
  }
}

MultiConvexMeshIndexData* SubConvexMesh::GetIndices()
{
  mIndexData.mOwner = mMesh;
  return &mIndexData;
}

MultiConvexMeshIndexData* SubConvexMesh::GetTriangleIndices()
{
  mTriangleIndexData.mOwner = mMesh;
  return &mTriangleIndexData;
}

//-------------------------------------------------------------------MultiConvexMesh
DefinePhysicsRuntimeClone(MultiConvexMesh);

ZilchDefineType(MultiConvexMesh, builder, type)
{
  ZeroBindDocumented();

  ZeroBindTag(Tags::Physics);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);

  ZilchBindGetter(Modified);
  ZilchBindGetter(Valid);
  ZilchBindMethod(Validate);
  ZilchBindMethod(UpdateAndNotifyIfModified);
  ZilchBindGetter(Vertices);
  ZilchBindGetter(SubMeshes);
  // Expose volume/center of mass/inertia tensor later
}

MultiConvexMesh::MultiConvexMesh()
{
  mModified = false;
  mIsValid = true;
  mVertexData.mOwner = this;
  mVertexData.mBoundArray = &mVertices;
  mSubMeshData.mOwner = this;
  mSubMeshData.mBoundArray = &mMeshes;

  mFlags.Clear();
  mFlags.SetFlag(MultiConvexMeshFlags::EditType2D);
}

MultiConvexMesh::~MultiConvexMesh()
{
  ClearSubMeshes();
}

void MultiConvexMesh::Save(StringParam filename)
{
  SaveToDataFile(*this, filename, DataFileFormat::Binary);
}

void MultiConvexMesh::Serialize(Serializer& stream)
{
  SerializeNameDefault(mVertices, VertexArray());
  SerializeNameDefault(mMeshes, SubMeshArray());

  // If we're loading then set the mesh owner on all of the sub-mesh bound arrays to ourself
  if(stream.GetMode() == SerializerMode::Loading)
  {
    for(size_t i = 0; i < mMeshes.Size(); ++i)
    {
      mMeshes[i]->mIndexData.mOwner = this;
      mMeshes[i]->mTriangleIndexData.mOwner = this;
    }
  }
}

void MultiConvexMesh::Initialize()
{
  Validate(false);
  RebuildCachedInfo();
}

void MultiConvexMesh::ResourceModified()
{
  // If we're already modified we don't need to do any extra logic
  if(GetModified())
    return;

  mModified = true;

  MultiConvexMeshManager* manager = (MultiConvexMeshManager*)GetManager();
  manager->mModifiedMeshes.PushBack(MultiConvexMeshManager::MeshReference(this));
}

HandleOf<MultiConvexMesh> MultiConvexMesh::CreateRuntime()
{
  return MultiConvexMeshManager::CreateRuntime();
}

void MultiConvexMesh::CopyTo(MultiConvexMesh* destination)
{
  // Destruct the destination mesh
  destination->ClearSubMeshes();
  // Copy all vertices over
  destination->mVertices = mVertices;
  // Copy all sub-meshes
  for(size_t i = 0; i < mMeshes.Size(); ++i)
  {
    SubConvexMesh* mySubMesh = mMeshes[i];
    // Construct a new sub-mesh on the destination and copy over the indices
    SubConvexMesh* otherSubMesh = destination->AddSubMesh();
    otherSubMesh->mIndices = mySubMesh->mIndices;
    otherSubMesh->mTriangleIndices = mySubMesh->mTriangleIndices;
  }
  // There's no need to copy any other data (aabb, volume, etc...) because we mark the mesh as
  // modified. When the next frame happens or the user tries to access any data the new mesh will
  // be computed and all those values will be filled out. We could potentially save calculations 
  // by copying those values over, but if someone is cloning a mesh it's likely they will be
  // modifying it shortly after anyways. Maybe optimize later...
  destination->ResourceModified();
}

void MultiConvexMesh::Draw(Mat4Param transform, bool drawEdges, bool drawFaces)
{
  if(mFlags.IsSet(MultiConvexMeshFlags::EditType2D))
  {
    for(uint i = 0; i < mMeshes.Size(); ++i)
      mMeshes[i]->Draw2d(mVertices, transform, drawEdges, drawFaces);
  }
  else
  {
    for(uint i = 0; i < mMeshes.Size(); ++i)
      mMeshes[i]->Draw(mVertices, transform, drawEdges, drawFaces);
  }
}

SubConvexMesh* MultiConvexMesh::AddSubMesh()
{
  SubConvexMesh* newMesh = new SubConvexMesh();
  newMesh->SetOwner(this);
  mMeshes.PushBack(newMesh);
  mModified = true;
  return newMesh;
}

MultiConvexMeshVertexData* MultiConvexMesh::GetVertices()
{
  return &mVertexData;
}

MultiConvexMeshSubMeshData* MultiConvexMesh::GetSubMeshes()
{
  return &mSubMeshData;
}

void MultiConvexMesh::ClearSubMeshes()
{
  for(size_t i = 0; i < mMeshes.Size(); ++i)
    delete mMeshes[i];
  mMeshes.Clear();
}

bool MultiConvexMesh::GetModified()
{
  return mModified;
}

bool MultiConvexMesh::GetValid()
{
  return mIsValid;
}

void MultiConvexMesh::FillEmptyIndices()
{
  // If any sub-mesh doesn't have any indices set then compute
  // them from the sub-meshes triangle indices
  for(size_t i = 0; i < mMeshes.Size(); ++i)
  {
    SubConvexMesh* subMesh = mMeshes[i];
    if(subMesh->mIndices.Size() == 0)
      subMesh->ComputeUniqueIndices();
  }
}

bool MultiConvexMesh::Validate(bool throwExceptionIfInvalid)
{
  mIsValid = true;

  // Validate each sub-mesh. We don't early out so that all sub-meshes know
  // if they're invalid so a user can check this flag.
  for(size_t i = 0; i < mMeshes.Size(); ++i)
    mIsValid &= mMeshes[i]->Validate(mVertices);

  if(!mIsValid && throwExceptionIfInvalid)
  {
    DoNotifyException("Invalid Mesh", "Physics Mesh contains invalid indices");
    mAabb.SetInvalid();
  }

  return mIsValid;
}

void MultiConvexMesh::UpdateAndNotifyIfModified()
{
  if(!GetModified())
    return;

  mModified = false;
  // Deal with the user not setting mIndices (temporary until major refactor)
  FillEmptyIndices();
  Validate(false);
  RebuildCachedInfo();

  // As we have just changed values like center of mass and aabb, we need to notify any collider
  // who used this mesh that we've changed so they can update world-space values.
  ResourceEvent toSend;
  toSend.EventResource = this;
  DispatchEvent(Events::ResourceModified, &toSend);
}

real MultiConvexMesh::GetVolume()
{
  return mVolume;
}

real MultiConvexMesh::GetWorldVolume(Vec3Param worldScale)
{
  return mVolume * worldScale.x * worldScale.y * worldScale.z;
}

Vec3 MultiConvexMesh::GetCenterOfMass()
{
  return mCenterOfMass;
}

Vec3 MultiConvexMesh::GetWorldCenterOfMass(Vec3Param worldScale)
{
  return mCenterOfMass * worldScale;
}

Mat3 MultiConvexMesh::ComputeInvInertiaTensor(Vec3Param worldScale, real totalMass)
{
  // If we aren't valid then return a default inertia tensor to prevent crashes.
  // Arbitrarily choose zero instead of identity.
  if(!GetValid())
    return Mat3(0, 0, 0, 0, 0, 0, 0, 0, 0);

  Vec3 worldCenterOfMass = GetWorldCenterOfMass(worldScale);

  Mat3 totalInertiaTensor;
  totalInertiaTensor.ZeroOut();
  // Combine all of the inertia tensors that were computed about their local center
  // of masses to one inertia tensor about the total center of mass
  for(size_t i = 0; i < mMeshes.Size(); ++i)
  {
    SubConvexMesh* subMesh = mMeshes[i];
    Vec3 subCenterOfMass = subMesh->mCenterOfMass;
    real subMass = subMesh->mVolume;
    Mat3 subInertiaTensor = subMesh->ComputeInertiaTensor(mVertices, subCenterOfMass, worldScale);
    
    Geometry::CombineInertiaTensor(totalInertiaTensor, worldCenterOfMass, subInertiaTensor, subCenterOfMass, subMass);
  }
  totalInertiaTensor *= totalMass;

  return totalInertiaTensor.Inverted();
}

bool MultiConvexMesh::CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  result.mTime = Math::PositiveMax();
  bool subMeshHit = false;
  for(size_t i = 0; i < mMeshes.Size(); ++i)
  {
    SubConvexMesh* subMesh = mMeshes[i];
    subMeshHit |= subMesh->CastRay(localRay, this, result, filter);
  }
  return subMeshHit;
}

void MultiConvexMesh::RebuildCachedInfo()
{
  ComputeAabb();
  ComputeCenterOfMassAndVolume();
}

void MultiConvexMesh::ComputeCenterOfMassAndVolume()
{
  // If we're not valid then arbitrarily set the volume and center of mass to prevent crashes.
  if(!GetValid())
  {
    mVolume = 1;
    mCenterOfMass = Vec3::cZero;
    return;
  }

  mCenterOfMass = Vec3::cZero;
  mVolume = real(0.0);
  // Accumulate the volume and weighted center of mass of each sub shape
  for(uint i = 0; i < mMeshes.Size(); ++i)
  {
    SubConvexMesh* subMesh = mMeshes[i];
    subMesh->ComputeCenterOfMassAndVolume(mVertices);
    real volume = subMesh->mVolume;
    Vec3 centerOfMass = subMesh->mCenterOfMass;

    mCenterOfMass += volume * centerOfMass;
    mVolume += volume;
  }

  if(mVolume != real(0.0))
    mCenterOfMass /= mVolume;
}

Aabb MultiConvexMesh::ComputeAabb()
{ 
  mAabb.SetInvalid();

  for(uint i = 0; i < mMeshes.Size(); ++i)
    mAabb.Combine(mMeshes[i]->ComputeAabb(mVertices));
  return mAabb;
}

//---------------------------------------------------------- MultiConvexMeshManager
ImplementResourceManager(MultiConvexMeshManager, MultiConvexMesh);

MultiConvexMeshManager::MultiConvexMeshManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("MultiConvexMesh", new BinaryDataFileLoader<MultiConvexMeshManager>());
  DefaultResourceName = "DefaultMultiConvexMesh";
  
  mCategory = "Physics";
  mExtension = "multiconvexmesh";
  mCanCreateNew = true;
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.MultiConvexMesh.data"));
  mCanDuplicate = true;
}

MultiConvexMesh* MultiConvexMeshManager::CreateNewResourceInternal(StringParam name)
{
  MultiConvexMesh* mesh = new MultiConvexMesh();
  mesh->mFlags.SetFlag(MultiConvexMeshFlags::NewlyCreatedInEditor);
  mesh->mManager = this;
  mesh->ResourceModified();
  return mesh;
}

void MultiConvexMeshManager::UpdateAndNotifyModifiedResources()
{
  for(size_t i = 0; i < mModifiedMeshes.Size(); ++i)
    mModifiedMeshes[i]->UpdateAndNotifyIfModified();
  mModifiedMeshes.Clear();
}

}//namespace Zero
