///////////////////////////////////////////////////////////////////////////////
///
/// \file OnionSkinning.hpp
/// Declaration of the OnionSkinning class.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class AnimationEditor;

//--------------------------------------------------------------- Onion Skinning
class OnionSkinning
{
public:
  OnionSkinning(AnimationEditor* editor);

  /// Updates the onion skinning.
  void Update();

  /// Clears all preview objects.
  void Clear();

private:
  void CreateObjects();
  bool ApplyTrack(BoundType* componentType, StringParam propertyName);
  float GetObjectSampleTime(uint objectIndex);

  /// The amount of objects to show.
  uint mObjectCount;

  /// The amount of time in seconds between each preview object.
  float mStepTime;

  /// All preview objects.
  Array<HandleOf<Cog>> mPreviewObjects;

  /// Access to the editor.
  AnimationEditor* mEditor;
};

}//namespace Zero
