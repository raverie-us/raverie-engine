///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyTrack.cpp
/// 
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
Quat Slerp2(const Quat& q1, const Quat& q2, float param)
{
  //
  // Quaternion Interpolation With Extra Spins, pp. 96f, 461f
  // Jack Morrison, Graphics Gems III, AP Professional
  //

  Quat qt;
  float slerp_epsilon = 0.00001f;

  float cosom = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w; 
  bool flip = cosom < 0;
  if(flip)
  {
    cosom = -cosom;
  }

  float startVal, endVal;
  if((1.0f - cosom) > slerp_epsilon)
  {
    float omega = Math::ArcCos(cosom);
    float sinom = Math::Sin(omega);
    startVal = (float)(Math::Sin((1.0f - param) * omega) / sinom);
    endVal = (float)(Math::Sin(param * omega) / sinom);
  }
  else
  {
    startVal = (float)(1.0) - param;
    endVal = (float)param;
  }

  if(flip)
  {
    endVal = -endVal;
  }

  qt.x = (float) (startVal*q1.x + endVal*q2.x);
  qt.y = (float) (startVal*q1.y + endVal*q2.y);
  qt.z = (float) (startVal*q1.z + endVal*q2.z);
  qt.w = (float) (startVal*q1.w + endVal*q2.w);

  return qt;
}

//******************************************************************************
template<typename type>
type LerpValue(type& a, type& b, float t)
{
  return Interpolation::Lerp<type>::Interpolate(a, b, t);
}

//******************************************************************************
Resource* LerpValue(Resource*& a, Resource*& b, float t)
{
  if(t == 1.0f)
    return b;
  return a;
}

//******************************************************************************
Quat LerpValue(Quat& a, Quat& b, float t)
{
  return Slerp2(a, b, t);
}

//******************************************************************************
bool LerpValue(bool& a, bool& b, float t)
{
  if(t == 1.0f)
    return b;
  return a;
}

//******************************************************************************
template<typename KeyFrame>
void LerpKeyFrame(KeyFrame& out, KeyFrame& keyOne, KeyFrame& keyTwo, float t)
{
  out.KeyValue = LerpValue(keyOne.KeyValue, keyTwo.KeyValue, t);
}

//******************************************************************************
template<typename keyFrames, typename keyFrameType>
void InterpolateKeyFrame(float time, uint& keyFrameIndex, keyFrames& mKeyFrames,
                         keyFrameType& keyFrame)
{
  uint CurKey = keyFrameIndex;
  float animTime = time;

  // Since keys are not spaced at regular intervals we need to search
  // for the keyframes that will be interpolated between.  The track data is
  // used to store what the last keyframe was to prevent searching the entire
  // track.

  if(mKeyFrames.Size() == 0)
    return;

  if(CurKey >= mKeyFrames.Size())
    CurKey = 0;

  // If it's to the left of the first frame, use the key value of the first frame
  if(mKeyFrames.Front().Time > time)
  {
    keyFrame = mKeyFrames.Front();
    return;
  }

  // Search Forward in the keyframes for the interval
  while(CurKey!= mKeyFrames.Size() - 1 && 
    mKeyFrames[ CurKey + 1 ].Time < animTime )
    ++CurKey;

  // Search Backward in the keyframes for the interval
  while(CurKey!= 0 && mKeyFrames[ CurKey ].Time > animTime )
    --CurKey;

  if(CurKey == mKeyFrames.Size()  - 1)
  {
    // Past the last keyframe for this path so use the last frame and the
    // transform data so the animation is clamped to the last frame
    keyFrame = mKeyFrames[CurKey];
  }
  else
  {
    // Generate value by interpolating between the two keyframes
    keyFrameType& KeyOne = mKeyFrames[ CurKey ];
    keyFrameType& KeyTwo = mKeyFrames[ CurKey + 1 ];

    float t1 = KeyOne.Time;
    float t2 = KeyTwo.Time;

    // Normalize the distance between the two keyframes
    float segLen = t2 - t1;
    float segStart = animTime - t1;
    float segNormalizedT = segStart/segLen;

    LerpKeyFrame(keyFrame, KeyOne, KeyTwo, segNormalizedT);
  }

  // Remember the last keyframe
  keyFrameIndex = CurKey;
}

//******************************************************************************
BlendTrack* GetBlendTrack(StringParam name, BlendTracks& tracks, HandleParam instance, Property* prop)
{
  BlendTrack* blendTrack = tracks.FindValue(name, nullptr);

  if(blendTrack == nullptr)
  {
    blendTrack = new BlendTrack();
    blendTrack->Index = tracks.Size();
    blendTrack->Object = instance;
    blendTrack->Property = prop;
    tracks.Insert(name, blendTrack);
  }

  return blendTrack;
}

//******************************************************************************
bool ValidPropertyTrack(Property* property)
{
  // Cannot animate read only properties
  if(property->Set == nullptr)
    return false;

  // First check to see if it's a value type
  Type* propertyType = property->PropertyType;
  
#define DoVariantType(type)                   \
  if (propertyType == ZilchTypeId(type))      \
    return true;
  #include "Meta/VariantTypes.inl"
#undef DoVariantType

  // Resources are also valid types
  //MetaType* type = MetaDatabase::GetInstance()->FindType(property->TypeId);
  //if(type && type->IsA(MetaTypeOf(Resource)))
    //return true;

  return false;
}

//******************************************************************************
PropertyTrack* MakePropertyTrack(StringParam componentName,
                                 StringParam propertyName,
                                 StringParam propertyTypeName)
{
  // Get the type id from the given property type name
  BoundType* propertyType = MetaDatabase::GetInstance()->FindType(propertyTypeName);

  // Can't create the property track if the property type doesn't exist
  if(propertyType == nullptr)
    return nullptr;

  return MakePropertyTrack(componentName, propertyName, propertyType);
}

//******************************************************************************
PropertyTrack* MakePropertyTrack(StringParam componentName,
                                 StringParam propertyName,
                                 BoundType* propertyTypeId)
{
    // First check to see if it's a value type
#define DoVariantType(type)                                                    \
  if (propertyTypeId == ZilchTypeId(type))                                     \
      return new AnimatePropertyValueType<type>(componentName, propertyName);
#include "Meta/VariantTypes.inl"
#undef DoVariantType

  // If it's a Resource, create a ref type track
  //MetaType* type = MetaDatabase::GetInstance()->FindType(propertyTypeId);
  //if(type && type->IsA(MetaTypeOf(Resource)))
    //return new AnimatePropertyRefType<Resource>(componentName, propertyName);

  return nullptr;
}

//******************************************************************************
PropertyTrack* MakePropertyTrack(BoundType* componentType, Property* property)
{
  String componentName = componentType->Name;
  String propertyName = property->Name;
  String propertyTypeName = property->PropertyType->ToString();
  return MakePropertyTrack(componentName, propertyName, propertyTypeName);
}

}//namespace Zero
