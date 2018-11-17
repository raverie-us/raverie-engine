///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyTrack.inl
/// 
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

//******************************************************************************
template <typename propertyType>
AnimatePropertyType<propertyType>::AnimatePropertyType(StringParam componentName,
                                                       StringParam propertyName)
{
  mComponentName = componentName;
  mPropertyName = propertyName;
  TypeId = ZilchTypeId(propertyType);

  // Look up the type name
  TypeName = TypeId->Name;
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::Serialize(Serializer& stream)
{
  PropertyTrack::Serialize(stream);
  SerializeName(mKeyFrames);
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::LinkInstance(PropertyTrackPlayData& data,
                         BlendTracks& tracks, StringParam objectPath, Cog* object)
{
  BoundType* componentMeta = MetaDatabase::GetInstance()->FindType(mComponentName);

  if(componentMeta == nullptr)
    return;

  data.mComponent = object->QueryComponentType(componentMeta);
  if(data.mComponent == nullptr)
    return;

  BoundType* componentType = ZilchVirtualTypeId(data.mComponent);
  Property* property = componentType->GetProperty(mPropertyName);
  ReturnIf(property == nullptr, , "Failed to find property.");

  // The property type has changed, so do nothing
  if(property->PropertyType != TypeId)
    return;

  String propertyName = BuildString(componentType->Name, ".", mPropertyName);
  String fullName = BuildString(objectPath, ".", propertyName);
  
  data.mBlend = GetBlendTrack(fullName, tracks, data.mComponent, property);
  data.mKeyframeIndex = 0;
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::UpdateFrame(PropertyTrackPlayData& data,
                            TrackParams& params, AnimationFrame& animationFrame)
{
  if(data.mBlend == NULL)
    return;

  KeyFrameT keyFrame;
  InterpolateKeyFrame(params.Time, data.mKeyframeIndex,
                      this->mKeyFrames, keyFrame);

  animationFrame.Tracks[ data.mBlend->Index ].Active = true;
  animationFrame.Tracks[ data.mBlend->Index ].Value = keyFrame.KeyValue;
};

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::GetKeyTimes(Array<float>& times)
{
  forRange(KeyFrameT& key, mKeyFrames.All())
  {
    times.PushBack(key.Time);
  }
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::GetKeyValues(Array<Any>& values)
{
  forRange(KeyFrameT& key, mKeyFrames.All())
  {
    values.PushBack(Any(key.KeyValue));
  }
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::InsertKey(PropertyTrackPlayData& data,
                                                  float time)
{
  BoundType* componentType = ZilchVirtualTypeId(data.mComponent);
  Property* property = componentType->GetProperty(mPropertyName);
  ReturnIf(property == nullptr, , "Failed to find property.");

  Any var = property->GetValue(data.mComponent);
  KeyFrameT keyFrame = { time , *(propertyType*)var.GetData() };
  mKeyFrames.PushBack(keyFrame);
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::InsertKey(AnyParam value, float time)
{
  propertyType var = VariantToType(value);

  for(uint i = 0; i < mKeyFrames.Size(); ++i)
  {
    if(mKeyFrames[i].Time == time)
    {
      mKeyFrames[i].KeyValue = var;
      return;
    }
  }

  KeyFrameT keyFrame = { time , var };
  mKeyFrames.PushBack(keyFrame);
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::AddKey(AnyParam value, float time)
{
  propertyType var = VariantToType(value);

  KeyFrameT keyFrame = { time , var };
  mKeyFrames.PushBack(keyFrame);
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::ResortKeyFrames()
{
  Sort(mKeyFrames.All());
}

//******************************************************************************
template <typename propertyType>
void AnimatePropertyType<propertyType>::KeyFrameT::Serialize(Serializer& stream)
{
  SerializeName(Time);
  SerializeName(KeyValue);
}

//******************************************************************************
template <typename propertyType>
AnimatePropertyValueType<propertyType>::AnimatePropertyValueType(
                            StringParam componentName, StringParam propertyName)
  : BaseType(componentName, propertyName)
{

}

//******************************************************************************
template <typename propertyType>
propertyType AnimatePropertyValueType<propertyType>::VariantToType(AnyParam variant)
{
  return variant.Get<propertyType>(GetOptions::AssertOnNull);
}

//******************************************************************************
template <typename propertyType>
AnimatePropertyRefType<propertyType>::AnimatePropertyRefType(
                            StringParam componentName, StringParam propertyName)
  : BaseType(componentName, propertyName)
{

}

//******************************************************************************
template <typename propertyType>
propertyType* AnimatePropertyRefType<propertyType>::VariantToType(AnyParam variant)
{
  return variant.Get<propertyType*>(GetOptions::AssertOnNull);
}
