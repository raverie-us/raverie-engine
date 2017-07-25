///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Allows a cog to ignore certain effect types (such as gravity or drag)
/// that are being applied to the entire space (effects on Space or LevelSettings).
class IgnoreSpaceEffects : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;

  // Check if this physics effect should be ignored
  bool IsIgnored(PhysicsEffect* effect);

  /// Whether or not to ignore drag effects.
  bool GetIgnoreDrag();
  void SetIgnoreDrag(bool ignore);
  /// Whether or not to ignore flow effects.
  bool GetIgnoreFlow();
  void SetIgnoreFlow(bool ignore);
  /// Whether or not to ignore force effects.
  bool GetIgnoreForce();
  void SetIgnoreForce(bool ignore);
  /// Whether or not to ignore gravity effects.
  bool GetIgnoreGravity();
  void SetIgnoreGravity(bool ignore);
  /// Whether or not to ignore point force effects.
  bool GetIgnorePointForce();
  void SetIgnorePointForce(bool ignore);
  /// Whether or not to ignore point gravity effects.
  bool GetIgnorePointGravity();
  void SetIgnorePointGravity(bool ignore);
  /// Whether or not to ignore thrust effects.
  bool GetIgnoreThrust();
  void SetIgnoreThrust(bool ignore);
  /// Whether or not to ignore vortex effects.
  bool GetIgnoreVortex();
  void SetIgnoreVortex(bool ignore);
  /// Whether or not to ignore wind effects.
  bool GetIgnoreWind();
  void SetIgnoreWind(bool ignore);
  /// Whether or not to ignore torque effects.
  bool GetIgnoreTorque();
  void SetIgnoreTorque(bool ignore);
  /// Whether or not to ignore buoyancy effects.
  bool GetIgnoreBuoyancy();
  void SetIgnoreBuoyancy(bool ignore);
  /// Whether or not to ignore custom effects.
  bool GetIgnoreCustom();
  void SetIgnoreCustom(bool ignore);

  /// Should the given effect type be ignored?
  bool GetIgnoreState(PhysicsEffectType::Enum effectType);
  /// Set if an effect type should be ignored.
  void SetIgnoreState(PhysicsEffectType::Enum effectType, bool ignore);

private:
  BitField<PhysicsEffectType::Enum> mFlags;
};

}//namespace Zero
