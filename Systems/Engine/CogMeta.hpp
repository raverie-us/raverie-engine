////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------- Cog Serialization Filter
class CogSerializationFilter : public SerializationFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool ShouldSerialize(Object* object) override;
};

//------------------------------------------------------------------------------ Cog Meta Operations
class CogMetaOperations : public MetaOperations
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  u64 GetUndoHandleId(HandleParam object) override;

  // Used to restore the space modified state when any operations are done to the Cog.
  Any GetUndoData(HandleParam object) override;
  void ObjectModified(HandleParam object) override;
  void RestoreUndoData(HandleParam object, AnyParam undoData) override;
  ObjectRestoreState* GetRestoreState(HandleParam object) override;
};

//------------------------------------------------------------------------ Cog Meta Data Inheritance
class CogMetaDataInheritance : public MetaDataInheritanceRoot
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  CogMetaDataInheritance() {}

  /// MetaDataInheritanceRoot Interface.
  String GetInheritId(HandleParam object, InheritIdContext::Enum context) override;
  void SetInheritId(HandleParam object, StringParam inheritId) override;

  /// MetaDataInheritance Interface.
  Guid GetUniqueId(HandleParam object) override;
  void Revert(HandleParam object) override;
  bool CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath) override;
  void RevertProperty(HandleParam object, PropertyPathParam propertyPath) override;
  void SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state) override;
  void RebuildObject(HandleParam object) override;
};

//----------------------------------------------------------- Cog Meta Transform
// Gizmos use this interface to transform objects.
class CogMetaTransform : public MetaTransform
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaTransformInstance GetInstance(HandleParam object) override;
};

//------------------------------------------------------ Cog Archetype Extension
// Used to give a custom editor for Archetype on Cog instead of the usual resource selector.
class CogArchetypeExtension : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//------------------------------------------------------------- Cog Meta Display
class CogMetaDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

}//namespace Zero
