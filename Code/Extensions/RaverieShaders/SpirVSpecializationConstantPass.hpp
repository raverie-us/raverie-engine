// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class SpirVSpecializationConstantPass;

namespace Events
{
RaverieDeclareEvent(CollectSpecializationConstants, SpecializationConstantEvent);
} // namespace Events

class SpecializationConstantEvent : public Raverie::EventData
{
public:
  RaverieDeclareType(SpecializationConstantEvent, Raverie::TypeCopyMode::ReferenceType);

  SpecializationConstantEvent();

  /// Finds the first id for a specialization constant by name (non-scalar types
  /// consume n contiguous ids based upon the number of scalars contained).
  /// If the given name isn't a valid specialization constant then -1 is
  /// returned. For fragment properties the name is generated using
  /// GenerateSpirVPropertyName.
  int GetFirstId(StringParam name);

  /// The pass that invoked this event.
  SpirVSpecializationConstantPass* mSpecializationConstantPass;
  /// The reflection data given to the pass. Can be used to look up any current
  /// information about the shader, the most common of which is the type name.
  ShaderStageInterfaceReflection* mInputReflectionData;
  /// A map of a specialization constant's id to the sting value to override it
  /// with. The id should be looked up using GetFirstId. If a constant consists
  /// of multiple scalar values then each individual scalar must be
  /// independently set (per spir-v spec).
  HashMap<int, String> mSpecializationOverridesById;
};

/// Sets and freezes specialization constants. Typically run before the backend
/// to lock-down values once the target language is known. The optimizer should
/// typically be run after this again.
class SpirVSpecializationConstantPass : public BaseSpirVOptimizerPass
{
public:
  SpirVSpecializationConstantPass();
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;

  void GetSpecializationFlags(Array<String>& outFlags, ShaderStageInterfaceReflection& inputStageReflection, ShaderStageInterfaceReflection& outputStageReflection);
  /// Helper that filles out an array of flags for the optimizer.
  void SetSpecializationValues(Array<String>& outFlags, HashMap<int, String>& overrides);

  /// Should all specialization constants be frozen after setting values?
  /// Needed in-order for optimizations to bake-out constants in all further
  /// passes.
  bool mFreezeAllConstants;
};

} // namespace Raverie
