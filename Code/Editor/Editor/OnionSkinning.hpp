// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class AnimationEditor;

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

} // namespace Zero
