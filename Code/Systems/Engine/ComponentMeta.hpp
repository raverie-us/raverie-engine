// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Meta Data Inheritance
class ComponentMetaDataInheritance : public MetaDataInheritance
{
public:
  RaverieDeclareType(ComponentMetaDataInheritance, TypeCopyMode::ReferenceType);

  /// MetaDataInheritance Interface.
  void Revert(HandleParam object) override;
  bool CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath) override;
  void RevertProperty(HandleParam object, PropertyPathParam propertyPath) override;
  void SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state) override;
  void RebuildObject(HandleParam object) override;
};

// Component Meta Operations
class ComponentMetaOperations : public MetaOperations
{
public:
  RaverieDeclareType(ComponentMetaOperations, TypeCopyMode::ReferenceType);

  u64 GetUndoHandleId(HandleParam object) override;

  // Used to restore the space modified state when any operations are done to
  // the Component.
  Any GetUndoData(HandleParam object) override;
  void ObjectModified(HandleParam object, bool intermediateChange) override;
  void RestoreUndoData(HandleParam object, AnyParam undoData) override;
  ObjectRestoreState* GetRestoreState(HandleParam object) override;
};

} // namespace Raverie
