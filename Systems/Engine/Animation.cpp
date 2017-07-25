///////////////////////////////////////////////////////////////////////////////
///
/// \file Animation.cpp
/// Implementation of the Animation resource and support classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2011-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ Object Track
ZilchDefineType(ObjectTrack, builder, type)
{
}

ObjectTrack::ObjectTrack()
{
  mTrackCount = 0;
}

ObjectTrack::~ObjectTrack()
{
  DeleteObjectsIn(PropertyTracks);
}

void ObjectTrack::Serialize(Serializer& stream)
{
  SerializeName(FullPath);
  SerializeName(ObjectTrackId);

  if(stream.GetMode() == SerializerMode::Saving)
  {
    forRange(PropertyTrack& track, PropertyTracks.All())
    {
      stream.StartPolymorphic("PropertyTrack");

      // The reason we're saving the name of the track outside of the track
      // object is because to create the PropertyTrack class, we need the
      // MetaProperty from the component type. We can get that information
      // from the tracks name
      stream.SerializeField("Name", track.Name);
      stream.SerializeField("TypeName", track.TypeName);

      track.Serialize(stream);
      stream.EndPolymorphic();
    }
  }
  else
  {
    // Walk all property tracks
    PolymorphicNode propertyTrack;
    while(stream.GetPolymorphic(propertyTrack))
    {
      // Read the name of the track
      String trackName;
      stream.SerializeField("Name", trackName);

      // If the track name is empty (likely because we failed to serialize
      // the name due to an old format), check to see if the name is in the
      // polymorphic node, which is how it used to be serialized
      if(trackName.Empty())
      {
        trackName = propertyTrack.Name;

        ErrorIf(trackName == "PropertyTrack", "Old animation file cannot be read.");
      }

      // Pull out the component and property names
      StringTokenRange r = StringTokenRange(trackName, '.');
      String componentName = r.Front();
      r.PopFront();
      String propertyName = r.Front();

      // Get the type name
      String typeName;
      stream.SerializeFieldDefault("TypeName", typeName, String(cInvalidTypeName));
      
      // If it's an invalid type id (likely from an old animation file), lets
      // attempt to look up the type id from meta.
      if(typeName == cInvalidTypeName)
      {
        // Attempt to grab the component
        BoundType* componentType = MetaDatabase::GetInstance()->FindType(componentName);
        if(componentType)
        {
          // Grab the type id from the property
          if(Property* prop = componentType->GetProperty(propertyName))
            typeName = prop->PropertyType->ToString();
        }

        // If we still don't have a valid type id, we cannot create the
        // correct property type
        if(typeName == cInvalidTypeName)
        {
          String format = "Failed to load animation track of type: %s.";
          String error = BuildString(format, trackName.c_str());
          DoNotifyWarning("Animation Error", error);
          continue;
        }
      }

      // Create the property track
      PropertyTrack* newPropertyTrack = MakePropertyTrack(componentName, propertyName, typeName);
      if(newPropertyTrack)
      {
        AddPropertyTrack(newPropertyTrack);
        newPropertyTrack->Serialize(stream);
        newPropertyTrack->Name = trackName;
      }

      stream.EndPolymorphic();
    }
  }
}

void ObjectTrack::UpdateFrame(PlayData& playData, TrackParams& params, AnimationFrame& frame)
{
  //Get the per instance data for this object track
  ObjectTrackPlayData& thisData = playData[ObjectTrackId];

  //Did the object get destroyed?
  Cog* object = thisData.ObjectHandle;
  if(object)
  {
    //Update all sub tracks
    PropertyTrackList::range r = PropertyTracks.All();
    for(;!r.Empty();r.PopFront())
    {
      PropertyTrack& track = r.Front();

      //Update track with its per instance data
      PropertyTrackPlayData& instanceData = thisData.mSubTrackPlayData[track.TrackIndex];

      // Don't do anything if this specific object doesn't have that component
      // Should this be a warning?
      if(instanceData.mComponent)
        track.UpdateFrame(instanceData, params, frame);
    }
  }
}

PropertyTrack* ObjectTrack::GetPropertyTrack(StringParam name)
{
  forRange(PropertyTrack& propertyTrack, PropertyTracks.All())
  {
    if(propertyTrack.Name == name)
      return &propertyTrack;
  }

  return nullptr;
}

void ObjectTrack::AddPropertyTrack(PropertyTrack* track)
{
  ErrorIf(track->TrackIndex != uint(-1), "Track has already been assigned an index.");

  track->TrackIndex = mTrackCount;
  ++mTrackCount;
  
  PropertyTracks.PushBack(track);
}

//------------------------------------------------------------ Animation
ZilchDefineType(Animation, builder, type)
{
  ZilchBindGetterProperty(Duration);

  ZeroBindDocumented();
}

Animation::Animation()
{
  mDuration = 0.0f;
  mNumberOfTracks = 0;
}

Animation::~Animation()
{
  Unload();
}

void Animation::Save(StringParam filename)
{
  SaveToDataFile(*this, filename, DataFileFormat::Text);
}

void Animation::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    SerializeName(mDuration);

    forRange(ObjectTrack& track, ObjectTracks.All())
    {
      stream.SerializePolymorphic(track);
    }
  }
  else
  {
    SerializeName(mDuration);

    // Walk through each object track
    PolymorphicNode objectNode;
    while(stream.GetPolymorphic(objectNode))
    {
      // Create and serialize the new object track
      ObjectTrack* objectTrack = new ObjectTrack();
      objectTrack->Serialize(stream);

      // Add it
      ObjectTracks.PushBack(objectTrack);
      ++mNumberOfTracks;
        
      // End the object node
      stream.EndPolymorphic();
    }
  }
}

void Animation::Unload()
{
  DeleteObjectsIn(ObjectTracks);
}

void Animation::UpdateFrame(PlayData& playData, TrackParams& params, AnimationFrame& frame)
{
  ObjectTrackList::range r = ObjectTracks.All();
  for(;!r.Empty();r.PopFront())
  {
    r.Front().UpdateFrame(playData, params, frame);
  }
}

//----------------------------------------------------------- Animation Loaders


class AnimationLoaderData : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    //Just load an empty animation for now
    Animation* animation = new Animation();
    AnimationManager::GetInstance()->AddResource(entry, animation);
    return animation;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    return LoadFromFile(entry);
  }
};

struct AnimationLoadPattern
{
  template<typename readerType>
  static void LoadObjectTrack(Animation& animation, uint trackId, readerType& reader)
  {
    ObjectTrackHeader trackHeader;
    reader.Read(trackHeader);

    ObjectTrack* track = new ObjectTrack();
    reader.ReadString(track->FullPath);

    // read the position key frames
    PropertyTrack* translationTrack = MakePropertyTrack("Transform", "Translation", ZilchTypeId(Vec3));
    Array<PositionKey> positionKeys;
    positionKeys.Resize(trackHeader.mNumPositionKeys);
    reader.ReadArray(positionKeys.Data(), trackHeader.mNumPositionKeys);

    for (size_t i = 0; i < trackHeader.mNumPositionKeys; ++i)
      translationTrack->InsertKey(positionKeys[i].Position, positionKeys[i].Keytime);

    // read the rotation key frames
    PropertyTrack* rotationTrack = MakePropertyTrack("Transform", "Rotation", ZilchTypeId(Quat));
    Array<RotationKey> rotationKeys;
    rotationKeys.Resize(trackHeader.mNumRotationKeys);
    reader.ReadArray(rotationKeys.Data(), trackHeader.mNumRotationKeys);

    for (size_t i = 0; i < trackHeader.mNumRotationKeys; ++i)
      rotationTrack->InsertKey(rotationKeys[i].Rotation, rotationKeys[i].Keytime);

    // finally read the scale key frames
    PropertyTrack* scaleTrack = MakePropertyTrack("Transform", "Scale", ZilchTypeId(Vec3));
    Array<ScalingKey> scalingKeys;
    scalingKeys.Resize(trackHeader.mNumScalingKeys);
    reader.ReadArray(scalingKeys.Data(), trackHeader.mNumScalingKeys);

    for (size_t i = 0; i < trackHeader.mNumScalingKeys; ++i)
      scaleTrack->InsertKey(scalingKeys[i].Scale, scalingKeys[i].Keytime);

    // sort all the inserted tracks by their key frame time
    translationTrack->ResortKeyFrames();
    rotationTrack->ResortKeyFrames();
    scaleTrack->ResortKeyFrames();

    // add all the data to our object track
    track->AddPropertyTrack(translationTrack);
    track->AddPropertyTrack(rotationTrack);
    track->AddPropertyTrack(scaleTrack);

    // add the loaded object track to our animation
    track->ObjectTrackId = trackId;
    animation.ObjectTracks.PushBack(track);
  }

  template<typename readerType>
  static void Load(Animation* animation, readerType& reader)
  {
    AnimationHeader animHeader;
    reader.Read(animHeader);

    animation->mDuration = animHeader.mAnimationDuration;
    animation->mNumberOfTracks = animHeader.mNumTracks;

    uint trackId = 0;
    while (true)
    {
      FileChunk chunk = reader.ReadChunkHeader();
      switch (chunk.Type)
      {
        case 0:
          return;
        case ObjectTrackChunk:
          LoadObjectTrack(*animation, trackId, reader);
          break;
        default:
          ErrorIf(true, "Incorrect animation data format\n");
          break;
      }
      ++trackId;
    }
  }
};

ImplementResourceManager(AnimationManager, Animation);

AnimationManager::AnimationManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("AnimationBin", new ChunkFileLoader<AnimationManager, AnimationLoadPattern>());
  AddLoader("Animation", new TextDataFileLoader<AnimationManager>());
  DefaultResourceName = "DefaultAnimation";
  mCanAddFile = true;
  AddGeometryFileFilters(this);
  mOpenFileFilters.InsertAt(1, FileDialogFilter("Rich Animation (*.Animation.data)", "*.Animation.data"));

  // Append the rich animation filter to the "All Animations"
  FileDialogFilter& allFilter = mOpenFileFilters[0];
  allFilter.mDescription = "All Animations";
  allFilter.mFilter = BuildString(allFilter.mFilter, ";*.Animation.data");

  mCanReload = true;
  mCanCreateNew = true;
  mExtension = DataResourceExtension;
  //mTemplates.PushBack(ResourceTemplate("RichAnimation", String()));
}

}//namespace Zero
