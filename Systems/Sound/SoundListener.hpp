///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//----------------------------------------------------------------------------------- Sound Listener

class SoundSpace;

/// Uses the object's position to "hear" all SoundEmitters in the SoundSpace
class SoundListener : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundListener();
  ~SoundListener();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  /// If this property is set to false the SoundListener will not produce any sound. 
  /// All audio in the SoundSpace will continue to be processed, so this is not the same as pausing the sounds.
  bool GetActive();
  void SetActive(bool value);
  /// The SoundNode associated with this SoundListener.
  HandleOf<SoundNode> GetSoundNode();

//internals
  Link<SoundListener> link;
  void Update(float invDt);

private:
  HandleOf<SoundNode> mSoundNode;
  bool mActive;
  SoundSpace* mSpace;
  Transform* mTransform;
  Vec3 mPrevPosition;

  friend class SoundSpace;
  friend class SoundSystem;
  friend class SoundNode;
};

}//namespace Zero
