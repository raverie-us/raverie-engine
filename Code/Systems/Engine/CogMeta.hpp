// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Serialization Filter
class CogSerializationFilter : public SerializationFilter
{
public:
  RaverieDeclareType(CogSerializationFilter, TypeCopyMode::ReferenceType);
  bool ShouldSerialize(Object* object) override;
};

// Cog Meta Operations
class CogMetaOperations : public MetaOperations
{
public:
  RaverieDeclareType(CogMetaOperations, TypeCopyMode::ReferenceType);

  u64 GetUndoHandleId(HandleParam object) override;

  // Used to restore the space modified state when any operations are done to
  // the Cog.
  Any GetUndoData(HandleParam object) override;
  void ObjectModified(HandleParam object, bool intermediateChange) override;
  void RestoreUndoData(HandleParam object, AnyParam undoData) override;
  ObjectRestoreState* GetRestoreState(HandleParam object) override;
};

// Meta Data Inheritance
class CogMetaDataInheritance : public MetaDataInheritanceRoot
{
public:
  RaverieDeclareType(CogMetaDataInheritance, TypeCopyMode::ReferenceType);
  CogMetaDataInheritance()
  {
  }

  /// MetaDataInheritanceRoot Interface.
  String GetInheritId(HandleParam object, InheritIdContext::Enum context) override;
  void SetInheritId(HandleParam object, StringParam inheritId) override;

  /// MetaDataInheritance Interface.
  Guid GetUniqueId(HandleParam object) override;
  void Revert(HandleParam object) override;
  bool CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath) override;
  void RevertProperty(HandleParam object, PropertyPathParam propertyPath) override;
  void RestoreRemovedChild(HandleParam parent, ObjectState::ChildId childId) override;
  void SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state) override;
  void RebuildObject(HandleParam object) override;
};

// Cog Meta Transform
/// Gizmos use this interface to transform objects.
class CogMetaTransform : public MetaTransform
{
public:
  RaverieDeclareType(CogMetaTransform, TypeCopyMode::ReferenceType);

  MetaTransformInstance GetInstance(HandleParam object) override;
};

// Archetype Extension
/// Used to give a custom editor for Archetype on Cog instead of the usual
/// resource selector.
class CogArchetypeExtension : public EditorPropertyExtension
{
public:
  RaverieDeclareType(CogArchetypeExtension, TypeCopyMode::ReferenceType);
};

// Cog Meta Display
class CogMetaDisplay : public MetaDisplay
{
public:
  RaverieDeclareType(CogMetaDisplay, TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

// Cog Meta Display
/// When saving a Cog as a property (such as in CogPath), we want to save out
/// the CogId, not the full definition of the Cog.
class CogMetaSerialization : public MetaSerialization
{
public:
  RaverieDeclareType(CogMetaSerialization, TypeCopyMode::ReferenceType);

  bool SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer) override;
  void AddCustomAttributes(HandleParam object, TextSaver* saver) override;

  /// ContextIds are currently only for levels, so when saving out an Archetype
  /// definition, this will be set to false, and context id's won't be assigned.
  /// This means that CogPaths must have a unique path within an Archetype.
  static bool sSaveContextIds;
};

class CogArchetypePropertyFilter : public MetaPropertyFilter
{
public:
  RaverieDeclareType(CogArchetypePropertyFilter, TypeCopyMode::ReferenceType);

  bool Filter(Member* prop, HandleParam instance) override;
};

} // namespace Raverie
