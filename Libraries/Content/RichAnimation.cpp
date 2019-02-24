// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(KeyFrameAdded);
DefineEvent(KeyFrameModified);
DefineEvent(KeyFrameDeleted);
DefineEvent(TrackAdded);
DefineEvent(TrackDeleted);
DefineEvent(AnimationModified);
} // namespace Events

ZilchDefineType(KeyFrameEvent, builder, type)
{
}

ZilchDefineType(TrackEvent, builder, type)
{
}

String ComponentNameFromPath(StringParam propertyPath)
{
  StringTokenRange r = StringTokenRange(propertyPath, '.');
  return r.Front();
}

String PropertyNameFromPath(StringParam propertyPath)
{
  StringTokenRange r = StringTokenRange(propertyPath, '.');
  r.PopFront();
  return r.Front();
}

String GetPropertyPath(HandleParam object, Property* property)
{
  // Find the property track
  String componentName = object.StoredType->Name;
  String propertyName = property->Name;

  return BuildString(componentName, ".", propertyName);
}

String GetObjectPath(Cog* object, Cog* root)
{
  if (object == nullptr)
    return "INVALIDPATH";

  if (object == root)
    return "/";

  String name = object->GetName();
  String path = GetObjectPath(object->GetParent(), root);

  if (path == "/")
    path = BuildString(path, name);
  else
    path = BuildString(path, cAnimationPathDelimiterStr, name);

  return path;
}

KeyFrame::KeyFrame()
{
  mParent = nullptr;
  mEditorFlags = 0;
}

KeyFrame::KeyFrame(float time, AnyParam value, KeyFrameId id, TrackNode* parent) : Id(id)
{
  mTime = time;
  mValue = value;
  mParent = parent;
  mEditorFlags = 0;
}

void KeyFrame::Serialize(Serializer& stream)
{
  SerializeName(mTime);
  SerializeName(mValue);
  SerializeName(mTangentIn);
  SerializeName(mTangentOut);
  SerializeNameDefault(mEditorFlags, (uint)0);
}

KeyFrame* KeyFrame::Duplicate(float newTime)
{
  KeyFrameId id = (KeyFrameId)GenerateUniqueId64();
  KeyFrame* duplicate = new KeyFrame(newTime, mValue, id, mParent);
  duplicate->mTangentIn = mTangentIn;
  duplicate->mTangentOut = mTangentOut;

  mParent->mKeyFrames.Insert(newTime, duplicate);

  // Dispatch events to the track and to the rich animation that a key
  // frame was added
  KeyFrameEvent e;
  e.mKeyFrame = duplicate;
  mParent->DispatchBubble(Events::KeyFrameAdded, &e);

  // The animation has been modified
  // mParent->mRichAnimation->SendModifiedEvent();

  // Update the total duration for the animation
  mParent->mRichAnimation->UpdateDuration();

  Modified();

  return duplicate;
}

void KeyFrame::SetTime(float time)
{
  mParent->mKeyFrames.EraseEqualValues(this);
  mParent->mKeyFrames.Insert(time, this);
  mTime = time;

  Modified();
}

float KeyFrame::GetTime()
{
  return mTime;
}

void KeyFrame::SetValue(AnyParam value)
{
  // Set the value
  mValue = value;

  Modified();
}

Any KeyFrame::GetValue()
{
  return mValue;
}

void KeyFrame::SetTangentIn(Vec2Param tangent)
{
  mTangentIn = tangent;
  Modified();
}

Vec2 KeyFrame::GetTangentIn()
{
  return mTangentIn;
}

void KeyFrame::SetTangentOut(Vec2Param tangent)
{
  mTangentOut = tangent;
  Modified();
}

Vec2 KeyFrame::GetTangentOut()
{
  return mTangentOut;
}

void KeyFrame::SetTangents(Vec2Param tangentIn, Vec2Param tangentOut)
{
  mTangentIn = tangentIn;
  mTangentOut = tangentOut;
  Modified();
}

Vec2 KeyFrame::GetGraphPosition()
{
  Type* keyType = mValue.StoredType;
  float y = 0.0f;
  if (keyType->IsA(ZilchTypeId(float)))
    y = mValue.Get<float>();
  else if (keyType->IsA(ZilchTypeId(int)))
    y = (float)mValue.Get<int>();
  else if (keyType->IsA(ZilchTypeId(bool)))
    y = (float)mValue.Get<bool>();
  return Vec2(mTime, y);
}

TrackNode* KeyFrame::GetParentTrack()
{
  return mParent;
}

void KeyFrame::Destroy()
{
  mParent->KeyFrameDeleted(this);
  delete this;
}

void KeyFrame::DispatchBubble(StringParam eventName, Event* e)
{
  // Dispatch on ourself then to our parent
  GetDispatcher()->Dispatch(eventName, e);
  mParent->DispatchBubble(eventName, e);
}

void KeyFrame::Modified()
{
  // Set the dirty bit
  mParent->Modified(this);
}

ZilchDefineType(TrackNode, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

TrackNode::TrackNode()
{
  Parent = nullptr;
  mRichAnimation = nullptr;
  Id = cInvalidTrackId;
  mTargetMeta = nullptr;
  mDisplayColor = Vec4::cZero;
  mDisabled = false;
  mPropertyType = ZilchTypeId(void);
  Type = TrackType::Invalid;
}

void CreateSubTracks(TrackNode* parent, cstr names, RichAnimation* richAnim)
{
  // The amount of SubProperty tracks to create
  uint elements = strlen(names);

  // Create each SubProperty track. It's important that the tracks are in the
  // same order as they are in the vector type, as they will be indexed
  // in that order
  for (uint i = 0; i < elements; ++i)
  {
    String name = String(names[i]);
    String path = BuildString(parent->Path, ".", name);

    // Create and register the track
    TrackNode* subTrack = new TrackNode(name, path, TrackType::SubProperty, nullptr, parent, richAnim);
  }
}

TrackNode::TrackNode(StringParam name,
                     StringParam path,
                     TrackType::Enum type,
                     BoundType* targetMeta,
                     TrackNode* parent,
                     RichAnimation* richAnim)
{
  Name = name;
  Path = path;
  Type = type;
  mTargetMeta = targetMeta;
  Id = cInvalidTrackId;
  mDisabled = false;
  mPropertyType = ZilchTypeId(void);

  Parent = parent;
  mRichAnimation = richAnim;

  mDisplayColor = Vec4::cZero;

  if (Parent)
    Parent->AddChild(this);

  mRichAnimation->RegisterTrack(this);

  // Look up the property type id if we're a property type
  if (type == TrackType::Property)
  {
    Property* prop = targetMeta->GetProperty(name);
    BoundType* boundType = Type::GetBoundType(prop->PropertyType);

    if (boundType != nullptr)
    {
      mPropertyType = boundType;
    }
    else
    {
      Error("The property we tried to animate was not a BoundType (may be "
            "because of the meta refactor)");
      mPropertyType = ZilchTypeId(void);
    }
  }
  // Sub-property types are always floats
  else if (type == TrackType::SubProperty)
  {
    BoundType* boundType = ZilchTypeId(float);
    mPropertyType = boundType;
  }

  /// Add sub tracks if we're a vector type
  if (mPropertyType == ZilchTypeId(Vec2))
    CreateSubTracks(this, "XY", richAnim);
  else if (mPropertyType == ZilchTypeId(Vec3))
    CreateSubTracks(this, "XYZ", richAnim);
  else if (mPropertyType == ZilchTypeId(Vec4))
    CreateSubTracks(this, "RGBA", richAnim);
  else if (mPropertyType == ZilchTypeId(Quat))
    // Quaternions will be edited with Euler angle curves, so we only need XYZ
    CreateSubTracks(this, "XYZ", richAnim);
}

void TrackNode::Serialize(Serializer& stream)
{
  SerializeEnumName(TrackType, Type);
  SerializeName(Name);
  SerializeName(Path);
  SerializeNameDefault(mDisabled, false);
  // Serialized to support old versions. The PropertyTypeName will always
  // get priority over the type id
  stream.SerializeFieldDefault("PropertyTypeName", mPropertyType.mName, cInvalidTypeName);

  // Update from legacy types
  if (stream.GetMode() == SerializerMode::Loading)
  {
    if (mPropertyType.mName == "float")
      mPropertyType = ZilchTypeId(float);
    else if (mPropertyType.mName == "bool")
      mPropertyType = ZilchTypeId(bool);
    else if (mPropertyType.mName == "uint")
      mPropertyType = ZilchTypeId(int);
    else if (mPropertyType.mName == "Vec2")
      mPropertyType = ZilchTypeId(Vec2);
    else if (mPropertyType.mName == "Vec3")
      mPropertyType = ZilchTypeId(Vec3);
    else if (mPropertyType.mName == "Vec4")
      mPropertyType = ZilchTypeId(Vec4);
    else if (mPropertyType.mName == "Quat")
      mPropertyType = ZilchTypeId(Quat);
  }

  SerializeName(mKeyFrames);
  SerializeName(Children);
}

void TrackNode::Initialize(RichAnimation* richAnimation, TrackNode* parent)
{
  Parent = parent;
  mRichAnimation = richAnimation;

  mRichAnimation->RegisterTrack(this);

  // For now, we only need the target meta for property tracks.
  // The parent of a property track will always be the component, so look
  // up the components meta
  if (Type == TrackType::Property)
  {
    mTargetMeta = MetaDatabase::GetInstance()->FindType(parent->Name);

    // Look up the type id only if we were given an invalid from the
    // data file. This should be for old rich animation files
    if (mPropertyType == ZilchTypeId(void) && mTargetMeta)
    {
      if (Property* prop = mTargetMeta->GetProperty(Name))
      {
        mPropertyType = Type::GetBoundType(prop->PropertyType);

        if (mPropertyType == nullptr)
        {
          Error("The property we tried to animate was not a BoundType (may be "
                "because of the meta refactor)");
          mPropertyType = ZilchTypeId(void);
        }
      }
    }
  }
  else if (Type == TrackType::Component)
  {
    mTargetMeta = MetaDatabase::GetInstance()->FindType(Name);
  }
  else if (Type == TrackType::SubProperty)
  {
    mPropertyType = ZilchTypeId(float);
  }

  // Initialize all children tracks
  forRange (TrackNode* child, Children.All())
  {
    child->Initialize(richAnimation, this);
  }

  // Generate a unique id for all key frames
  forRange (KeyFrames::value_type entry, mKeyFrames.All())
  {
    KeyFrame* keyFrame = entry.second;

    keyFrame->Id = (KeyFrameId)GenerateUniqueId64();
    keyFrame->mParent = this;
  }
}

void TrackNode::Modified()
{
  mRichAnimation->mModified = true;
}

String GetTrackPath(TrackNode* track)
{
  if (track == nullptr || track->IsRoot())
    return "INVALIDPATH";

  if (track->Parent->IsRoot())
    return "/";

  String name = track->Name;

  // Start with our parents path
  String path = track->Parent->Path;

  if (path == "/")
    path = BuildString(path, name);
  else
    path = BuildString(path, cAnimationPathDelimiterStr, name);

  return path;
}

void RebuildPath(TrackNode* track)
{
  if (track->Type != TrackType::Object)
    return;

  track->Path = GetTrackPath(track);

  forRange (TrackNode* child, track->Children.All())
  {
    RebuildPath(child);
  }
}

void TrackNode::Rename(StringParam newName, Status& status)
{
  // No need to do anything if it's the same name
  if (Name == newName)
    return;

  // Cannot rename the root track
  if (Parent == nullptr || Parent->IsRoot())
  {
    status.SetFailed("Cannot rename root track.");
    return;
  }

  // Check to see if there are any siblings with the same name
  forRange (TrackNode* sibling, Parent->Children.All())
  {
    if (sibling->Name == newName)
    {
      status.SetFailed("Sibling track has same name. Tracks must have unique paths.");
      return;
    }
  }

  // Set the name
  Name = newName;

  RebuildPath(this);

  Modified();
}

bool TrackNode::IsRoot()
{
  return Parent == nullptr;
}

template <typename VectorType, uint Elements>
Any SubPropertySample(TrackNode* vectorNode, float t)
{
  VectorType sampledVector;

  // Sample each track and Assign it to the final vector
  for (uint i = 0; i < Elements; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subTrack = vectorNode->Children[i];

    // Sample the track for the current element
    float sampledValue = subTrack->SampleTrack(t).Get<float>(GetOptions::AssertOnNull);
    sampledVector[i] = sampledValue;
  }

  return sampledVector;
}

template <>
Any SubPropertySample<Quat, 3>(TrackNode* vectorNode, float t)
{
  // The SubProperty tracks of a Quaternion track are stored in euler angles,
  // so we need to sample them as euler angles and convert it back to a quat
  Math::EulerAngles eulerAngles(Math::EulerOrders::XYZs);

  // Sample each track and Assign it to the euler angles
  for (uint i = 0; i < 3; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subTrack = vectorNode->Children[i];

    // Sample the track for the current element
    float rotation = subTrack->SampleTrack(t).Get<float>(GetOptions::AssertOnNull);
    eulerAngles[i] = Math::DegToRad(rotation);
  }

  // Convert back to a quaternion
  return Math::ToQuaternion(eulerAngles);
}

Any TrackNode::SampleTrack(float t)
{
  ErrorIf(Type == TrackType::Object || Type == TrackType::Component,
          "Only Property and SubProperty tracks can be sampled.");

  /// Float types can sample the baked curve directly, but vector types
  /// need to sample each SubProperty track to build a final sampled vector type
  if (mPropertyType == ZilchTypeId(float))
  {
    BakePiecewiseFunction();
    return mBakedCurve.Sample(t);
  }
  else if (mPropertyType == ZilchTypeId(Vec2))
    return SubPropertySample<Vec2, 2>(this, t);
  else if (mPropertyType == ZilchTypeId(Vec3))
    return SubPropertySample<Vec3, 3>(this, t);
  else if (mPropertyType == ZilchTypeId(Vec4))
    return SubPropertySample<Vec4, 4>(this, t);
  else if (mPropertyType == ZilchTypeId(Quat))
    return SubPropertySample<Quat, 3>(this, t);

  // Sampling is currently only supported for float and vector types
  Error("Cannot sample track.");
  return Any();
}

template <typename VectorType>
Any SubSample(AnyParam sample, uint element)
{
  VectorType vector = sample.Get<VectorType>(GetOptions::AssertOnNull);
  return vector[element];
}

template <>
Any SubSample<Quat>(AnyParam sample, uint element)
{
  // We could just grab the index directly from the sampled value,
  // but for Quaternions, we need to convert it to Euler Angles first
  Quat rotation = sample.Get<Quat>(GetOptions::AssertOnNull);
  Math::EulerAngles eulerAngles(rotation, Math::EulerOrders::XYZs);
  return eulerAngles[element];
}

Any TrackNode::SampleObject(Cog* animGraphObject)
{
  ReturnIf(Type == TrackType::Object || Type == TrackType::Component,
           Any(),
           "Cannot sample object or component track directly.");

  // If we're a sub property track, we want to sample only the element
  // of the vector type
  if (Type == TrackType::SubProperty)
  {
    // Sample our parent for the vector type
    Any sample = Parent->SampleObject(animGraphObject);

    // If it's invalid, the property likely no longer exists
    if (!sample.IsHoldingValue())
      return Any();

    // Find which element we are
    uint element = Parent->Children.FindIndex(this);

    // Sample the value at the index in the vector type
    BoundType* parentType = Parent->mPropertyType;
    if (parentType == ZilchTypeId(Vec2))
      return SubSample<Vec2>(sample, element);
    else if (parentType == ZilchTypeId(Vec3))
      return SubSample<Vec3>(sample, element);
    else if (parentType == ZilchTypeId(Vec4))
      return SubSample<Vec4>(sample, element);
    else if (parentType == ZilchTypeId(Quat))
      return SubSample<Quat>(sample, element);
    else
      // Likely the parent track type was changed on a script component
      return Any();
  }

  // We want to query the meta property
  Property* property = GetProperty(animGraphObject);

  // If the property no longer exists, we cannot sample it
  if (property == nullptr)
    return Any();

  // Get the object instance
  Component* instance = GetComponent(animGraphObject);

  ReturnIf(!instance, Any(), "Could not find object instance to create key frame.");

  // Query the meta property
  return property->GetValue(instance);
}

TrackNode* TrackNode::IsValid(Cog* animGraphObject, Status& status)
{
  // The root is always "valid"
  if (IsRoot())
    return nullptr;

  // Check to see if our parent is valid and return it if it isn't
  TrackNode* invalidNode = Parent->IsValid(animGraphObject, status);
  if (invalidNode)
    return invalidNode;

  switch (Type)
  {
  case TrackType::Object:
  {
    // Test for the validity of the object
    if (GetCog(animGraphObject) == nullptr)
      status.SetFailed(String::Format("Failed to find object at '%s'", Path.c_str()));
    break;
  }
  case TrackType::Component:
  {
    if (GetComponent(animGraphObject) == nullptr)
      status.SetFailed(String::Format("Failed to find Component '%s'", Name.c_str()));
    break;
  }
  case TrackType::Property:
  {
    // Check to see if the property exists (could be gone from script
    // components)
    Property* prop = GetProperty(animGraphObject);
    if (prop == nullptr)
    {
      String componentName = Parent->Name;
      String message =
          String::Format("Failed to find Property '%s' on Component '%s'", Name.c_str(), componentName.c_str());
      status.SetFailed(message);
    }
    // If the type has changed from the initial type the track was created with
    else if (prop->PropertyType != mPropertyType)
    {
      // The stored type could no longer exist (such as a removed custom
      // resource)
      BoundType* storedType = mPropertyType;

      // The current type should exist as it's currently on the object
      BoundType* currType = Type::GetBoundType(prop->PropertyType);
      ErrorIf(currType == nullptr, "Invalid type id.");

      cstr currTypeName = currType->Name.c_str();
      cstr storedTypeName = "Unknown Type";

      // If the stored type still exists, we can display the old name difference
      if (storedType)
        storedTypeName = storedType->Name.c_str();

      String message = String::Format("Property type has changed from '%s' to '%s'.", storedTypeName, currTypeName);
      status.SetFailed(message);
    }
    break;
  }
  // Already valid if we get to this point because our parent property is valid
  case TrackType::SubProperty:
    break;
  case TrackType::Invalid:
    break;
  }

  // If we're invalid, return ourself as the blocking node
  if (status.Failed())
    return this;

  // We're valid, so there's no blocking node
  return nullptr;
}

bool TrackNode::IsDisabled()
{
  if (mDisabled)
    return true;

  if (Parent)
    return Parent->IsDisabled();

  // Root is never disabled
  return false;
}

ObjPtr TrackNode::GetObject(Cog* animGraphObject)
{
  if (animGraphObject == nullptr)
    return nullptr;

  switch (Type)
  {
  case TrackType::Object:
    return GetCog(animGraphObject);
  case TrackType::Component:
    return GetComponent(animGraphObject);
  case TrackType::Property:
  case TrackType::SubProperty:
    return GetProperty(animGraphObject);
  case TrackType::Invalid:
    break;
  }

  Error("Invalid track type.");
  return nullptr;
}

Cog* TrackNode::GetCog(Cog* animGraphObject)
{
  if (animGraphObject == nullptr)
    return nullptr;

  // Get the object track for this track
  TrackNode* objectTrack = GetObjectTrack();

  // Start from the time line object
  Cog* cog = animGraphObject;

  // Object names are separated by '/'
  StringTokenRange r = StringTokenRange(objectTrack->Path, cAnimationPathDelimiter);

  // We can skip the root object as we're starting from it
  r.PopFront();

  // Walk until there's nothing left
  forRange (String childName, r)
  {
    // Find the next child
    cog = cog->FindChildByName(childName);

    // The path is not valid for the given time line object
    if (cog == nullptr)
      return nullptr;
  }

  return cog;
}

Component* TrackNode::GetComponent(Cog* animGraphObject)
{
  if (animGraphObject == nullptr)
    return nullptr;

  // Get the component track from this track
  TrackNode* componentTrack = GetComponentTrack();

  // The object track will always be the component tracks parent
  TrackNode* objectTrack = componentTrack->Parent;

  // Get the Cog on the object track
  Cog* owner = objectTrack->GetCog(animGraphObject);

  // If we found a valid object, query and return the component
  if (owner)
    return owner->QueryComponentType(componentTrack->mTargetMeta);
  return nullptr;
}

Property* TrackNode::GetProperty(Cog* animGraphObject)
{
  if (animGraphObject == nullptr)
    return nullptr;

  // Only grab it if the component exists
  if (Component* component = GetComponent(animGraphObject))
  {
    // Get the property track from this track
    TrackNode* propertyTrack = GetPropertyTrack();

    // This will properly handle proxy metas
    BoundType* componentMeta = ZilchVirtualTypeId(component);

    // Find the property on the component track of the properties name
    return componentMeta->GetProperty(propertyTrack->Name);
  }

  return nullptr;
}

TrackNode* TrackNode::GetObjectTrack()
{
  TrackNode* currentTrack = this;

  // Continue walking until we hit an object track
  // This will never go passed the root node because the root is of type Object
  while (currentTrack->Type != TrackType::Object)
    currentTrack = currentTrack->Parent;

  return currentTrack;
}

TrackNode* TrackNode::GetComponentTrack()
{
  ErrorIf(Type == TrackType::Object, "Cannot get a component track from an object track.");

  TrackNode* currentTrack = this;

  // Continue walking until we hit a component track
  while (currentTrack->Type != TrackType::Component)
    currentTrack = currentTrack->Parent;

  return currentTrack;
}

TrackNode* TrackNode::GetPropertyTrack()
{
  ErrorIf(Type == TrackType::Object || Type == TrackType::Component,
          "Cannot get a property track from an object or component track.");

  // If we're the property track, simply return ourselves
  if (Type == TrackType::Property)
    return this;

  // Otherwise, we must be a sub property track, so return our parent
  return Parent;
}

TrackNode::KeyFrames::valueRange TrackNode::FindKeyFrames(float time)
{
  return mKeyFrames.FindAll(time);
}

template <typename VectorType, uint Elements>
void CreateSubKeyFrames(TrackNode* parent, float time, AnyParam value)
{
  VectorType vector = value.Get<VectorType>(GetOptions::AssertOnNull);

  // Create a key frame on each SubProperty track with the value
  // of that element in the vector type
  for (uint i = 0; i < Elements; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subProperty = parent->Children[i];

    // Create the key frame with the element in the vector
    Any element = vector[i];
    subProperty->CreateKeyFrame(time, element);
  }
}

template <>
void CreateSubKeyFrames<Quat, 3>(TrackNode* parent, float time, AnyParam value)
{
  // The SubProperty tracks of a Quaternion track are stored in euler angles,
  // so we need to convert the quaternion to euler angles and create the key
  // frame with the euler angle for each axis
  Quat quaternion = value.Get<Quat>(GetOptions::AssertOnNull);
  Math::EulerAngles eulerAngles(quaternion, Math::EulerOrders::XYZs);

  // Create a key frame for each element of the euler angles
  for (uint i = 0; i < 3; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subProperty = parent->Children[i];
    Any element = Math::RadToDeg(eulerAngles[i]);

    // Create the key frame
    subProperty->CreateKeyFrame(time, element);
  }
}

KeyFrame* TrackNode::CreateKeyFrame(float time, AnyParam value)
{
  ErrorIf(Type == TrackType::Object || Type == TrackType::Component,
          "Key frames can only be added to Property and SubProperty tracks.");

  // If this property node is of a vector type, we want to create key frames
  // on the SubProperty tracks
  if (mPropertyType == ZilchTypeId(Vec2))
    CreateSubKeyFrames<Vec2, 2>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Vec3))
    CreateSubKeyFrames<Vec3, 3>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Vec4))
    CreateSubKeyFrames<Vec4, 4>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Quat))
    CreateSubKeyFrames<Quat, 3>(this, time, value);
  else
  {
    // Generate a unique id for this key frame
    KeyFrameId id = (KeyFrameId)GenerateUniqueId64();

    // Create the new key frame
    KeyFrame* keyFrame = new KeyFrame(time, value, id, this);

    // Add the key frame
    mKeyFrames.Insert(time, keyFrame);

    // The baked curve is no longer valid
    mBakedCurve.Clear();

    // Default tangents for now
    keyFrame->mTangentIn = Vec2(-0.05f, 0);
    keyFrame->mTangentOut = Vec2(0.05f, 0);

    // Dispatch events to the track and to the rich animation that a key
    // frame was added
    KeyFrameEvent e;
    e.mKeyFrame = keyFrame;
    this->DispatchBubble(Events::KeyFrameAdded, &e);

    // The animation has been modified
    // mRichAnimation->SendModifiedEvent();
    mRichAnimation->mModified = true;

    // Update the total duration for the animation
    mRichAnimation->UpdateDuration();

    return keyFrame;
  }

  return nullptr;
}

template <typename VectorType, uint Elements>
void UpdateSubKeyAtTime(TrackNode* propertyNode, float time, AnyParam value)
{
  VectorType vectorValue = value.Get<VectorType>(GetOptions::AssertOnNull);

  // Update the key for each element
  for (uint i = 0; i < Elements; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subPropertyTrack = propertyNode->Children[i];

    // Update or add the key frame
    Any elementValue = vectorValue[i];
    subPropertyTrack->UpdateOrCreateKeyAtTime(time, elementValue);
  }
}

template <>
void UpdateSubKeyAtTime<Quat, 3>(TrackNode* propertyNode, float time, AnyParam value)
{
  // The SubProperty tracks of a Quaternion track are stored in euler angles,
  // so we need to convert the quaternion to euler angles and update the key
  // frames with the euler angle for each axis
  Quat rotation = value.Get<Quat>(GetOptions::ReturnDefaultOrNull);
  Math::EulerAngles eulerAngles(rotation, Math::EulerOrders::XYZs);

  for (uint i = 0; i < 3; ++i)
  {
    // The child tracks are in the same order as the elements in the vector
    TrackNode* subPropertyTrack = propertyNode->Children[i];

    // Update or add the key frame
    Any elementValue = Math::RadToDeg(eulerAngles[i]);
    subPropertyTrack->UpdateOrCreateKeyAtTime(time, elementValue);
  }
}

void TrackNode::UpdateOrCreateKeyAtTime(float time, AnyParam value)
{
  // If it's of a vector type, we want to call the same function
  // on the SubProperty tracks with the corresponding element of the vector
  if (mPropertyType == ZilchTypeId(Vec2))
    UpdateSubKeyAtTime<Vec2, 2>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Vec3))
    UpdateSubKeyAtTime<Vec3, 3>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Vec4))
    UpdateSubKeyAtTime<Vec4, 4>(this, time, value);
  else if (mPropertyType == ZilchTypeId(Quat))
    UpdateSubKeyAtTime<Quat, 3>(this, time, value);
  else
  {
    // Attempt to find the key frames at the given time
    KeyFrames::valueRange keys = FindKeyFrames(time);

    // If there are none, create a new one
    if (keys.Empty())
    {
      CreateKeyFrame(time, value);
    }
    else
    {
      // Used to store the ones we're going to delete
      Array<KeyFrame*> toDestroy;
      bool first = true;

      // We want to collapse all the key frames at this time to the new value,
      // so set the first key frame to the new value and destroy the rest
      forRange (KeyFrame* frame, keys)
      {
        if (first)
          frame->SetValue(value);
        else
          toDestroy.PushBack(frame);
        first = false;
      }

      // Destroy the remaining key frames
      forRange (KeyFrame* frame, toDestroy.All())
      {
        frame->Destroy();
      }
    }
  }
}

void TrackNode::ClearKeyFrames()
{
  // Destroy all key frames
  while (!mKeyFrames.Empty())
  {
    KeyFrame* keyFrame = mKeyFrames.Front().second;

    // Destroying it will remove it from mKeyFrames
    keyFrame->Destroy();
  }

  // The baked curve is no longer valid
  mBakedCurve.Clear();
}

void TrackNode::GetKeyTimes(Array<float>& keyTimes)
{
  for (uint i = 0; i < mKeyFrames.Size(); ++i)
    keyTimes.PushBack(mKeyFrames[i].first);
}

struct TrackSort
{
  bool operator()(const TrackNode* left, const TrackNode* right)
  {
    return left->Type > right->Type;
  }
};

void TrackNode::AddChild(TrackNode* child)
{
  // Store the child track
  child->Parent = this;
  Children.PushBack(child);

  // We want the object tracks to show up after all component tracks
  Sort(Children.All(), TrackSort());
}

void TrackNode::DispatchBubble(StringParam eventName, Event* e)
{
  // Dispatch the event on ourself and bubble it to the rich animation
  GetDispatcher()->Dispatch(eventName, e);
  mRichAnimation->GetDispatcher()->Dispatch(eventName, e);
}

bool GetStartAndEndTimes(TrackNode* vectorTrack, float* startTime, float* endTime, uint elements)
{
  *startTime = Math::cInfinite;
  *endTime = -Math::cInfinite;

  bool validAxis = false;

  // Find the start and end times of the tracks
  for (uint i = 0; i < elements; ++i)
  {
    TrackNode* subTrack = vectorTrack->Children[i];
    if (subTrack->mKeyFrames.Empty())
      continue;
    validAxis = true;
    float start = subTrack->mKeyFrames.Front().second->GetTime();
    float end = subTrack->mKeyFrames.Back().second->GetTime();
    *startTime = Math::Min(*startTime, start);
    *endTime = Math::Max(*endTime, end);
  }

  return validAxis;
}

template <typename VectorType, uint Elements>
void BakeSubPropertyKeyFrames(TrackNode* vectorTrack, Array<TrackNode::KeyEntry>& keyFrames)
{
  // Get the start and end time of the track based on our sub property tracks
  float startTime, endTime;
  bool valid = GetStartAndEndTimes(vectorTrack, &startTime, &endTime, Elements);

  if (!valid)
    return;

  float range = endTime - startTime;
  const float cStep = 0.003f;

  // Step through and sample at the given times. This will stop before the
  // endTime
  for (float t = startTime; t < endTime; t += cStep)
  {
    // Sampling the property track of a vector type will sample each
    // sub-property track (one for each element) and build a final vector
    // based on that track
    Any sampledVector = vectorTrack->SampleTrack(t);
    keyFrames.PushBack(TrackNode::KeyEntry(t, sampledVector));
  }

  // Sample one more time at the end time
  // NOTE* this method could cause two samples very close to each other at the
  // end. This could be improved later
  Any sampledVector = vectorTrack->SampleTrack(endTime);
  keyFrames.PushBack(TrackNode::KeyEntry(endTime, sampledVector));
}

void TrackNode::BakeKeyFrames(Array<KeyEntry>& keyFrames)
{
  // If we get a void type, it's likely that the property no longer exists
  // on the component
  if (mPropertyType == ZilchTypeId(void))
    return;
  // For float types, we want to use the baked points on an adaptive curve
  else if (mPropertyType == ZilchTypeId(float))
  {
    BakePiecewiseFunction();

    forRange (Vec3 pos, mBakedCurve.GetBakedCurve())
    {
      float time = pos.x;
      float value = pos.y;
      keyFrames.PushBack(KeyEntry(time, value));
    }
  }
  // For vector types, we want to sample each curve of each element
  // to build a final set of key frames
  else if (mPropertyType == ZilchTypeId(Vec2))
    BakeSubPropertyKeyFrames<Vec2, 2>(this, keyFrames);
  else if (mPropertyType == ZilchTypeId(Vec3))
    BakeSubPropertyKeyFrames<Vec3, 3>(this, keyFrames);
  else if (mPropertyType == ZilchTypeId(Vec4))
    BakeSubPropertyKeyFrames<Vec4, 4>(this, keyFrames);
  else if (mPropertyType == ZilchTypeId(Quat))
    BakeSubPropertyKeyFrames<Quat, 3>(this, keyFrames);
  // All other types cannot be represented as curves, so the key frames
  // are already in their final format
  else
  {
    keyFrames.Reserve(mKeyFrames.Size());

    // Add each key frame
    forRange (KeyFrame* keyFrame, mKeyFrames.AllValues())
    {
      float time = keyFrame->GetTime();
      Any value = keyFrame->GetValue();

      keyFrames.PushBack(KeyEntry(time, value));
    }
  }
}

void TrackNode::InvalidateBakedCurve()
{
  mBakedCurve.Clear();

  forRange (TrackNode* child, Children.All())
  {
    child->InvalidateBakedCurve();
  }
}

TrackNode::~TrackNode()
{
}

void TrackNode::Destroy()
{
  // Remove ourselves from our parent
  if (Parent)
    Parent->Children.EraseValueError(this);

  // Destroy all of our child nodes
  while (!Children.Empty())
  {
    Children.Front()->Destroy();
  }

  mRichAnimation->DestroyTrack(this);

  // Delete ourself
  delete this;
}

void TrackNode::KeyFrameDeleted(KeyFrame* keyFrame)
{
  // Erase the key frame from our list
  mKeyFrames.EraseEqualValues(keyFrame);

  // Remove it from the dirty bit list if it's in there
  mRichAnimation->mDirtyKeyFrames.Erase(keyFrame);

  // The baked curve is no longer valid
  mBakedCurve.Clear();

  // Dispatch an event saying that we're being deleted
  KeyFrameEvent e;
  e.mKeyFrame = keyFrame;
  DispatchBubble(Events::KeyFrameDeleted, &e);

  mRichAnimation->mModified = true;
}

void TrackNode::Modified(KeyFrame* keyFrame)
{
  mRichAnimation->Modified(keyFrame);

  // The baked curve is no longer valid
  mBakedCurve.Clear();

  // If we're a sub property node, our parents baked curve is no longer valid
  if (Type == TrackType::SubProperty)
  {
    Parent->mBakedCurve.Clear();
  }
}

void TrackNode::BakePiecewiseFunction()
{
  mBakedCurve.mError = mRichAnimation->mSampleTolerance;

  // If it's already baked, no need to do anything
  if (mBakedCurve.IsBaked())
    return;

  mBakedCurve.Clear();

  forRange (KeyFrame* keyFrame, mKeyFrames.AllValues())
  {
    Vec2 pos = keyFrame->GetGraphPosition();
    Vec2 tanIn = keyFrame->GetTangentIn();
    Vec2 tanOut = keyFrame->GetTangentOut();

    mBakedCurve.AddControlPoint(pos, tanIn, tanOut);
  }

  mBakedCurve.Bake();
}

ZilchDefineType(RichAnimation, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindGetterSetterProperty(SampleTolerance);
}

RichAnimation::RichAnimation()
{
  mRoot = nullptr;
  mCurrTrackId = 0;
  mDuration = 0.0f;
  mSampleTolerance = 0.05f;
  mModified = false;
}

RichAnimation::~RichAnimation()
{
  DestroyRecursive(mRoot);
}

void RichAnimation::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSampleTolerance, 0.05f);
  SerializeNameDefault(mDuration, 0.0f);

  if (mRoot == nullptr)
  {
    mRoot = new TrackNode("Root", String(), TrackType::Object, nullptr, nullptr, this);
  }
  stream.SerializeField("Root", *mRoot);
}

void RichAnimation::Initialize()
{
  mRoot->Initialize(this, nullptr);
}

void PushToAnimationRecursive(TrackNode* objectTrackInfo, Animation* animation, uint& objectTrackCount, float& duration)
{
  ErrorIf(objectTrackInfo->Type != TrackType::Object, "Track type must be object.");

  // Create the Object Track on the animation
  ObjectTrack* objectTrack = new ObjectTrack();
  objectTrack->ObjectTrackId = objectTrackCount++;
  objectTrack->SetFullPath(objectTrackInfo->Path);
  animation->ObjectTracks.PushBack(objectTrack);

  forRange (TrackNode* currTrack, objectTrackInfo->Children.All())
  {
    // Ignore disabled tracks
    if (currTrack->IsDisabled())
      continue;

    // If it's an object track, continue recursing
    if (currTrack->Type == TrackType::Object)
    {
      PushToAnimationRecursive(currTrack, animation, objectTrackCount, duration);
      continue;
    }

    // The only valid children of an object track are Objects and Components,
    // therefor this is a component track

    // The children of component tracks MUST be property tracks, so
    // iterate through all of them and add them to the object track
    forRange (TrackNode* propertyTrackNode, currTrack->Children.All())
    {
      // Ignore disabled tracks
      if (propertyTrackNode->IsDisabled())
        continue;

      String componentName = currTrack->Name;
      String propertyName = propertyTrackNode->Name;

      // Bake the key frames
      Array<TrackNode::KeyEntry> bakedKeyFrames;
      propertyTrackNode->BakeKeyFrames(bakedKeyFrames);

      // Don't create the track if there's no key frames
      if (bakedKeyFrames.Empty())
        continue;

      // Build the property track
      PropertyTrack* propertyTrack = MakePropertyTrack(componentName, propertyName, propertyTrackNode->mPropertyType);

      // The property track could fail to create if the property type doesn't
      // exist anymore
      if (propertyTrack)
      {
        propertyTrack->Name = BuildString(componentName, ".", propertyName);

        // Insert all baked key values into the property track
        forRange (TrackNode::KeyEntry& entry, bakedKeyFrames.All())
        {
          float time = entry.first;
          Any& value = entry.second;
          propertyTrack->AddKey(value, time);

          // Update the duration for the animation
          duration = Math::Max(time, duration);
        }

        propertyTrack->ResortKeyFrames();

        // Add it to the object track
        objectTrack->AddPropertyTrack(propertyTrack);
      }
    }
  }
}

void RichAnimation::BakeToAnimation(Animation* animation)
{
  // Clear the animation of any data
  //   forRange(ObjectTrack& objectTrack, animation->ObjectTracks.All())
  //   {
  //     forRange(PropertyTrack& propertyTrack,
  //     objectTrack.PropertyTracks.All())
  //     {
  //       delete &propertyTrack;
  //     }
  //     delete &objectTrack;
  //   }

  animation->ObjectTracks.Clear();
  animation->mDuration = 0.0f;
  animation->mNumberOfTracks = 0;

  for (uint i = 0; i < mRoot->Children.Size(); ++i)
  {
    TrackNode* track = mRoot->Children[i];
    PushToAnimationRecursive(track, animation, animation->mNumberOfTracks, animation->mDuration);
  }
}

TrackNode* RichAnimation::GetObjectTrack(TrackNode* parent, StringTokenRange pathRange, String currPath, bool createNew)
{
  // We want to find the object track with this name.  If we can't find it,
  // we will create the track and continue searching down if there is more
  // in the path
  String trackName = pathRange.Front();
  pathRange.PopFront();

  // If trackName is empty, we're at the first element of the path. It's
  // empty because the first character of all paths is a '/'
  if (currPath.Empty())
  {
    currPath = "/";
    trackName = "/";
  }
  // Otherwise, build the path with the previous current path
  else
  {
    if (currPath == "/")
      currPath = BuildString(currPath, trackName);
    else
      currPath = BuildString(currPath, cAnimationPathDelimiterStr, trackName);
  }

  // Search all the direct children of the current parent for the track
  // with the name we're searching for
  TrackNode* objectTrack = nullptr;
  for (uint i = 0; i < parent->Children.Size(); ++i)
  {
    // Get the track
    TrackNode* currTrack = parent->Children[i];

    // If it's the correct track, set it and break out of the loop
    if (currTrack->Name == trackName)
    {
      objectTrack = currTrack;
      break;
    }
  }

  // If we didn't find it, we have to create the object track
  if (objectTrack == nullptr)
  {
    if (createNew == false)
      return nullptr;

    objectTrack = new TrackNode(trackName, currPath, TrackType::Object, nullptr, parent, this);
  }

  // If this is the last object in the path, we have found the track
  if (pathRange.Empty())
    return objectTrack;

  // Continue recursing to the end of the path
  return GetObjectTrack(objectTrack, pathRange, currPath, createNew);
}

TrackNode* RichAnimation::GetObjectTrack(StringRange path, bool createNew)
{
  StringTokenRange pathRange = StringTokenRange(path, cAnimationPathDelimiter);
  return GetObjectTrack(mRoot, pathRange, String(), createNew);
}

TrackNode*
RichAnimation::GetPropertyTrack(TrackNode* objectTrack, StringParam path, BoundType* targetMeta, bool createNew)
{
  // First we have to get the component track
  String componentName = ComponentNameFromPath(path);
  TrackNode* componentTrack = GetComponentTrack(objectTrack, componentName, targetMeta, createNew);

  if (componentTrack == nullptr && !createNew)
    return nullptr;

  // Attempt to find the property track on the component track
  TrackNode* propertyTrack = GetDirectChildTrack(componentTrack, path);

  if (propertyTrack == nullptr && createNew)
  {
    // Create the track
    String propertyName = PropertyNameFromPath(path);
    propertyTrack = new TrackNode(propertyName, path, TrackType::Property, targetMeta, componentTrack, this);
  }

  return propertyTrack;
}

TrackNode* RichAnimation::GetDirectChildTrack(TrackNode* parent, StringParam path)
{
  // Search all the direct children of the root
  for (uint i = 0; i < parent->Children.Size(); ++i)
  {
    // If it has the correct path, return it
    TrackNode* currTrack = parent->Children[i];
    if (currTrack->Path == path)
      return currTrack;
  }

  return nullptr;
}

TrackNode*
RichAnimation::GetComponentTrack(TrackNode* objectTrack, StringParam component, BoundType* targetMeta, bool createNew)
{
  // Recurse down from the object track to find the property track
  TrackNode* componentTrack = GetDirectChildTrack(objectTrack, component);

  // If we didn't find it, we have to create a new one
  if (componentTrack == nullptr && createNew)
  {
    componentTrack = new TrackNode(component, component, TrackType::Component, targetMeta, objectTrack, this);
  }

  return componentTrack;
}

TrackNode* RichAnimation::GetPropertyTrack(
    Cog* object, Cog* animGraphObject, BoundType* componentType, StringParam propertyName, bool createNew)
{
  // Find the object track (it will be created if it doesn't exist)
  String objectPath = GetObjectPath(object, animGraphObject);
  TrackNode* objectTrack = GetObjectTrack(objectPath, createNew);

  if (objectTrack == nullptr)
    return nullptr;

  // Grab the component
  Component* component = object->QueryComponentType(componentType);
  BoundType* virtualComponentType = ZilchVirtualTypeId(component);
  Property* prop = virtualComponentType->GetProperty(propertyName);

  // Find the property track (it will be created if it doesn't exist)
  String propertyPath = GetPropertyPath(component, prop);
  TrackNode* propertyTrack = GetPropertyTrack(objectTrack, propertyPath, virtualComponentType, createNew);
  return propertyTrack;
}

void GetMaxDuration(TrackNode* root, float& maxDuration)
{
  // Update the max duration from the last key
  if (!root->mKeyFrames.Empty())
  {
    TrackNode::KeyFrames::value_type& key = root->mKeyFrames.Back();
    maxDuration = Math::Max(maxDuration, key.first);
  }

  // Continue recursing down the tree
  for (uint i = 0; i < root->Children.Size(); ++i)
    GetMaxDuration(root->Children[i], maxDuration);
}

void RichAnimation::UpdateDuration()
{
  mDuration = 0.0f;
  GetMaxDuration(mRoot, mDuration);
}

void RichAnimation::RegisterTrack(TrackNode* track)
{
  // Protect from double registration. Should this be an error?
  if (mTrackMap.Count(track->Id) == 0)
  {
    track->Id = mCurrTrackId++;
    mTrackMap.Insert(track->Id, track);

    TrackEvent eventToSend;
    eventToSend.mTrack = track;
    GetDispatcher()->Dispatch(Events::TrackAdded, &eventToSend);
  }
}

void RichAnimation::SetSampleTolerance(float tolerance)
{
  mSampleTolerance = Math::Clamp(tolerance, 0.001f, 100.0f);
  mRoot->InvalidateBakedCurve();
  SendModifiedEvent();
}

float RichAnimation::GetSampleTolerance()
{
  return mSampleTolerance;
}

void RichAnimation::Flush()
{
  // We need to store the key frames in the dirty list in case the dirty list
  // is modified while we're iterating over it
  Array<KeyFrame*> modifiedKeyFrames;
  modifiedKeyFrames.Reserve(mDirtyKeyFrames.Size());

  forRange (KeyFrame* keyFrame, mDirtyKeyFrames.All())
  {
    modifiedKeyFrames.PushBack(keyFrame);
  }

  mDirtyKeyFrames.Clear();

  // Update the duration if anything was modified
  if (!modifiedKeyFrames.Empty())
    UpdateDuration();

  forRange (KeyFrame* modifiedKeyFrame, modifiedKeyFrames.All())
  {
    KeyFrameEvent e;
    e.mKeyFrame = modifiedKeyFrame;

    modifiedKeyFrame->DispatchBubble(Events::KeyFrameModified, &e);
  }

  if (!modifiedKeyFrames.Empty() || mModified)
    SendModifiedEvent();

  mModified = false;
}

RichAnimation::range RichAnimation::allTracks()
{
  return range(mRoot);
}

void RichAnimation::DestroyRecursive(TrackNode* root)
{
  for (uint i = 0; i < root->Children.Size(); ++i)
    DestroyRecursive(root->Children[i]);

  SafeDelete(root);
}

void RichAnimation::DestroyTrack(TrackNode* track)
{
  // Erase it from the map
  mTrackMap.Erase(track->Id);

  // Send an event saying that the track is has been deleted
  TrackEvent e;
  e.mTrack = track;
  track->DispatchBubble(Events::TrackDeleted, &e);

  // Make sure none of the tracks are in the dirty list
  forRange (KeyFrame* keyFrame, track->mKeyFrames.AllValues())
  {
    mDirtyKeyFrames.Erase(keyFrame);
  }

  mModified = true;

  UpdateDuration();
}

void RichAnimation::Modified(KeyFrame* keyFrame)
{
  mDirtyKeyFrames.Insert(keyFrame);
}

void RichAnimation::SendModifiedEvent()
{
  Event eventToSend;
  GetDispatcher()->Dispatch(Events::AnimationModified, &eventToSend);
}

ZilchDefineType(RichAnimationBuilder, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindGetterSetterProperty(PreviewArchetype)->Add(new MetaEditorResource(false, true));
}

RichAnimationBuilder::RichAnimationBuilder() : DirectBuilderComponent(0, ".data", "Animation")
{
}

void RichAnimationBuilder::Initialize(ContentComposition* item)
{
  DirectBuilderComponent::Initialize(item);
  item->EditMode = ContentEditMode::ContentItem;
}

void RichAnimationBuilder::Serialize(Serializer& stream)
{
  DirectBuilderComponent::Serialize(stream);
  SerializeNameDefault(mPreviewArchetype, String());
}

Archetype* RichAnimationBuilder::GetPreviewArchetype()
{
  return ArchetypeManager::GetInstance()->FindOrNull(mPreviewArchetype);
}

void RichAnimationBuilder::SetPreviewArchetype(Archetype* archetype)
{
  if (archetype)
    mPreviewArchetype = archetype->ResourceIdName;
  else
    mPreviewArchetype = "";
}

void RichAnimationBuilder::BuildContent(BuildOptions& buildOptions)
{
  // Default behavior for direct builder is to just copy the content file.
  String destFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(buildOptions.SourcePath, mOwner->Filename);

  // Load the rich animation file
  RichAnimation richAnimation;
  LoadFromDataFile(richAnimation, sourceFile);
  richAnimation.Initialize();

  // Bake the rich animation to a normal animation file
  HandleOf<Animation> animationHandle = Animation::CreateRuntime();
  Animation* animation = animationHandle;
  richAnimation.BakeToAnimation(animation);

  // Save the normal animation
  bool success = SaveToDataFile(*animation, destFile);

  if (!success)
  {
    buildOptions.Failure = true;
    buildOptions.Message =
        String::Format("Failed to bake rich animation file at %s to %s", sourceFile.c_str(), destFile.c_str());
  }

  // Update the file time so that NeedsBuilding works.
  SetFileToCurrentTime(destFile);
}

RichAnimation::range::range(TrackNode* root) : mCurrent(root)
{
}
TrackNode* RichAnimation::range::Front()
{
  return mCurrent;
}

void RichAnimation::range::PopFront()
{
  mCurrent = NextTrack(mCurrent);
}

bool RichAnimation::range::Empty()
{
  return mCurrent == nullptr;
}

TrackNode* RichAnimation::range::NextTrack(TrackNode* track)
{
  // Always walk down the left
  if (!track->Children.Empty())
    return track->Children.Front();

  // Find a valid sibling, if none, walk up and find valid siblings of parents
  while (track->Parent != nullptr)
  {
    // Accept the next sibling if it exists
    TrackNode* nextSibling = NextSibling(track);
    if (nextSibling)
      return nextSibling;

    // If we were the last sibling, go up to our parent and continue
    track = track->Parent;
  }

  // If we went back to the root,
  return nullptr;
}

TrackNode* RichAnimation::range::NextSibling(TrackNode* track)
{
  Array<TrackNode*>& parentChildren = track->Parent->Children;

  /// One passed our index is the next sibling
  uint trackIndex = parentChildren.FindIndex(track);
  uint nextSiblingIndex = trackIndex + 1;

  // If there is a next sibling, return it
  if (nextSiblingIndex < parentChildren.Size())
    return parentChildren[nextSiblingIndex];

  // Otherwise we were the last (or only) sibling
  return nullptr;
}

} // namespace Zero
