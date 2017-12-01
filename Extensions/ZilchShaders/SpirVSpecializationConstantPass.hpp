///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class SpirVSpecializationConstantPass;

namespace Events
{
ZilchDeclareEvent(CollectSpecializationConstants, SpecializationConstantEvent);
}//namespace Events

//-------------------------------------------------------------------SpecializationConstantEvent
class SpecializationConstantEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  SpecializationConstantEvent();
  
  /// Finds the first id for a specialization constant by name (non-scalar types
  /// consume n contiguous ids based upon the number of scalars contained).
  /// If the given name isn't a valid specialization constant then -1 is returned.
  /// For fragment properties the name is generated using GenerateSpirVPropertyName.
  int GetFirstId(StringParam name);

  /// The pass that invoked this event.
  SpirVSpecializationConstantPass* mSpecializationConstantPass;
  /// The reflection data given to the pass. Can be used to look up any current
  /// information about the shader, the most common of which is the type name.
  ShaderStageInterfaceReflection* mInputReflectionData;
  /// A map of a specialization constant's id to the sting value to override it with.
  /// The id should be looked up using GetFirstId. If a constant consists of multiple 
  /// scalar values then each individual scalar must be independently set (per spir-v spec).
  HashMap<int, String> mSpecializationOverridesById;
};

//-------------------------------------------------------------------SpirVSpecializationConstantPass
/// Sets and freezes specialization constants. Typically run before the backend to lock-down
/// values once the target language is known. The optimizer should typically be run after this again.
class SpirVSpecializationConstantPass : public BaseSpirVOptimizerPass
{
public:
  SpirVSpecializationConstantPass();
  bool RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData) override;

  /// Helper that sets all constant values on the optimizer.
  void SetSpecializationValues(spvtools::Optimizer& optimizer, ShaderStageInterfaceReflection& stageReflection, HashMap<int, String>& overrides);

  /// Should all specialization constants be frozen after setting values?
  /// Needed in-order for optimizations to bake-out constants in all further passes.
  bool mFreezeAllConstants;
};

}//namespace Zero
