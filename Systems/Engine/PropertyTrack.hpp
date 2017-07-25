///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyTrack.hpp
/// 
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct TrackParams
{
  enum UpdateType
  {
    Scrub,
    Game
  };

  enum UpdatingDirection
  {
    Forward,
    Reverse
  };

  float Time;
  UpdatingDirection Direction;
  UpdateType Type;
};

//--------------------------------------------------------------- Property Track
/// Base class for all time line track that animate properties.
class PropertyTrack
{
public:
  PropertyTrack(){TrackIndex = uint(-1); TypeId = ZilchTypeId(void);}
  
  virtual void Serialize(Serializer& stream){}
  
  virtual ~PropertyTrack(){};
  virtual void LinkInstance(PropertyTrackPlayData& data, BlendTracks& tracks,
                            StringParam objectPath, Cog* object) = 0;
  virtual void UpdateFrame(PropertyTrackPlayData& data, TrackParams& params,
                           AnimationFrame& animationFrame){};
    
  // Editing
  virtual void InsertKey(PropertyTrackPlayData& data, float time) {}
  virtual void InsertKey(AnyParam value, float time){}
  virtual void AddKey(AnyParam value, float time){}
  virtual void ResortKeyFrames(){};
  virtual void GetKeyTimes(Array<float>& times) {}
  virtual void GetKeyValues(Array<Any>& values) {}
  
  /// Will be "ComponentName.PropertyName" e.g. "Transform.Translation"
  String Name;
  BoundType* TypeId;
  String TypeName;
  uint TrackIndex;
  
  IntrusiveLink(PropertyTrack, link);
};

typedef InList<PropertyTrack> PropertyTrackList;

//-------------------------------------------------------- Animate Property Type
template <typename propertyType>
class AnimatePropertyType : public PropertyTrack
{
public:
  AnimatePropertyType(StringParam componentName, StringParam propertyName);

  /// Property tracks need to support both value and reference type
  /// properties. We have this function because only the derived type
  /// knows if it should pull out the type from the variant as value or as ref.
  virtual propertyType VariantToType(AnyParam variant) = 0;

  /// PropertyTrack Interface.
  void Serialize(Serializer& stream) override;
  void LinkInstance(PropertyTrackPlayData& data, BlendTracks& tracks,
                    StringParam objectPath, Cog* object) override;
  void UpdateFrame(PropertyTrackPlayData& data, TrackParams& params,
                   AnimationFrame& animationFrame) override;
  void GetKeyTimes(Array<float>& times) override;
  void GetKeyValues(Array<Any>& values) override;
  void InsertKey(PropertyTrackPlayData& data, float time) override;
  void InsertKey(AnyParam value, float time) override;
  void AddKey(AnyParam value, float time) override;
  void ResortKeyFrames() override;

  struct KeyFrameT
  {
    float Time;
    propertyType KeyValue;
    void Serialize(Serializer& stream);

    bool operator<(const KeyFrameT& right) const { return Time < right.Time; }
  };

  Array<KeyFrameT> mKeyFrames;
  String mComponentName;
  String mPropertyName;
};

//--------------------------------------------------- Animate Property Value Type
template <typename propertyType>
class AnimatePropertyValueType : public AnimatePropertyType<propertyType>
{
public:
  typedef AnimatePropertyType<propertyType> BaseType;
  AnimatePropertyValueType(StringParam componentType, StringParam propertyName);

  /// AnimatePropertyType Interface.
  propertyType VariantToType(AnyParam variant) override;
};

//---------------------------------------------------- Animate Property Ref Type
template <typename propertyType>
class AnimatePropertyRefType : public AnimatePropertyType<propertyType*>
{
public:
  typedef AnimatePropertyType<propertyType*> BaseType;
  AnimatePropertyRefType(StringParam componentName, StringParam propertyName);
  
  /// AnimatePropertyType Interface.
  propertyType* VariantToType(AnyParam variant) override;
};

BlendTrack* GetBlendTrack(StringParam name, BlendTracks& tracks, HandleParam instance, Property* prop);
/// Returns whether or not the given property can be animated.
bool ValidPropertyTrack(Property* property);

/// Creates a track that animates the property of the given component.
PropertyTrack* MakePropertyTrack(StringParam componentName,
                                 StringParam propertyName,
                                 StringParam propertyTypeName);
PropertyTrack* MakePropertyTrack(StringParam componentName,
                                 StringParam propertyName,
                                 BoundType* propertyTypeId);
PropertyTrack* MakePropertyTrack(BoundType* componentName, Property* property);

#include "PropertyTrack.inl"

}//namespace Zero
