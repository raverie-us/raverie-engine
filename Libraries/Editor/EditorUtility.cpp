///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorUtility.cpp
/// Declaration of the Editor support classes EditorSpace and Selection.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Get the text drawing position for a given object
Vec3 GetObjectTextPosition(Cog* cog)
{
  // Offset the position upward to handle text being centered
  const Vec3 OffsetUp(0.0f, 1.0f, 0.0f);

  // If the object is valid...
  if (cog != NULL)
  {
    //// Does the object have a model?
    //Model* model = object->has(Model);

    //// If we have a model...
    //if (model != NULL)
    //{
    //  // Get the Aabb from the model
    //  Aabb aabb = model->GetAabb();

    //  // Use it's upper position as the offset
    //  return aabb.GetCenter() + Vec3(0.0f, aabb.GetHalfExtents().y, 0.0f) + OffsetUp;
    //}

    // Does the object have a collider?
    Collider* collider = cog->has(Collider);

    // If we have a collider...
    if (collider != NULL)
    {
      // Get the Aabb from the collider
      Aabb aabb = collider->mAabb;

      // Use it's upper position as the offset
      return aabb.GetCenter() + Vec3(0.0f, aabb.GetHalfExtents().y, 0.0f) + OffsetUp;
    }

    // Attempt to get the object's transform component
    Transform* transform = cog->has(Transform);

    // If the object has a transform...
    if (transform)
    {
      // Get the position of the transform
      return transform->GetTranslation();
    }
  }

  // Return the origin
  return Vec3::cZero;
}


float GetViewDistance(Aabb& aabb)
{
  return aabb.GetHalfExtents().Length() * 3;
}

void DisplayCodeDefinition(CodeDefinition& definition)
{
  // Check that the code definition is valid otherwise return
  if (!definition.NameLocation.IsValid() && !definition.NameLocation.CodeUserData)
    return;

  Resource* resource = (Resource*)definition.NameLocation.CodeUserData;
  DocumentResource* documentResource = Type::DynamicCast<DocumentResource*>(resource);

  Editor* editor = Z::gEditor;

  // If we have a resource but it's not a document, then select it and early out
    // This allows us to 'Go To Definition' on extension properties such as Mesh.Cube
  if (resource != nullptr && documentResource == nullptr)
  {
    MetaSelection* selection = editor->GetSelection();
    selection->SelectOnly(resource);
    selection->FinalSelectionChanged();
    return;
  }

  DocumentEditor* definitionDocument = nullptr;

  // If we have a resource where the definition lives
  if (DocumentResource* documentResource = Type::DynamicCast<DocumentResource*>(resource))
    definitionDocument = editor->OpenDocumentResource(documentResource);
  // This might be a generated stub code / document
  else if (resource == nullptr && definition.NameLocation.IsNative)
  {
    String extension = FileExtensionManager::GetZilchScriptTypeEntry()->GetDefaultExtensionNoDot();
    definitionDocument = editor->OpenTextString(definition.NameLocation.Origin, definition.NameLocation.Code, extension);
  }

  if (definitionDocument)
  {
    // Center the entire function, class, or element into view, then select just the name
    definitionDocument->GoToPosition(definition.ElementLocation.EndPosition);
    definitionDocument->GoToPosition(definition.ElementLocation.StartPosition);
    definitionDocument->GotoAndSelect(definition.NameLocation.StartPosition, definition.NameLocation.EndPosition);
  }
}

Aabb GetAabb(Cog* cog, IncludeMode::Type includeMode, bool world)
{
  if(cog == NULL)
    return Aabb(Vec3::cZero, Vec3::cZero);

  Vec3 center = Vec3::cZero;

  if(Transform* tx = cog->has(Transform))
    center = tx->GetWorldTranslation();

  ObjectLink* link = cog->has(ObjectLink);
  if(link != NULL)
    center = link->GetWorldPosition();

  Aabb aabb(center, Vec3::cZero);
  ExpandAabb(cog, aabb, includeMode, world);
  return aabb;
}

Aabb GetAabb(HandleParam instance, IncludeMode::Type includeMode, bool world)
{
  if(instance.IsNull())
    return Aabb(Vec3::cZero, Vec3::cZero);

  Vec3 center = Vec3::cZero;

  if(MetaTransform* metaTransform = instance.StoredType->HasInherited<MetaTransform>())
  {
    MetaTransformInstance transformInstance = metaTransform->GetInstance(instance);
    center = transformInstance.GetWorldTranslation();
  }

  if(Cog* cog = instance.Get<Cog*>())
  {
    ObjectLink* link = cog->has(ObjectLink);
    if(link != nullptr)
      center = link->GetWorldPosition();

    if (SelectionIcon* icon = cog->has(SelectionIcon))
      return Aabb(center, Vec3(1.0f, 1.0f, 1.0f));
  }


  Aabb aabb(center, Vec3::cZero);
  ExpandAabb(instance, aabb, includeMode, world);
  return aabb;
}

Aabb GetAabb(MetaSelection* selection, IncludeMode::Type includeMode)
{
  return GetAabbFromObjects(selection->All(), includeMode);
}

void ExpandAabb(Cog* cog, Aabb& aabb, IncludeMode::Type includeMode, bool world, bool toParent, bool expandTransform)
{
  Transform* tx = cog->has(Transform);
  if(tx == nullptr)
    return;

  bool invalid = true;
  Mat4 m = tx->GetLocalMatrix();

  if(Graphical* graphical = cog->has(Graphical))
  {
    Aabb subAabb;
    if(world)
    {
      subAabb = graphical->GetWorldAabb();
    }
    else
    {
      subAabb = graphical->GetLocalAabb();

      if(toParent)
        subAabb = subAabb.TransformAabb(m);
    }

    invalid &= !subAabb.Valid();
    aabb.Combine(subAabb);
  }

  if(Area* area = cog->has(Area))
  {
    Aabb subAabb;
    if(world)
    {
      subAabb = area->GetAabb();
    }
    else
    {
      subAabb = area->GetLocalAabb();

      if(toParent)
        subAabb = subAabb.TransformAabb(m);
    }

    invalid &= !subAabb.Valid();
    aabb.Combine(subAabb);
  }

  if(Collider* collider = cog->has(Collider))
  {
    // Colliders do not have a way to obtain a local aabb.
    if(world)
    {
      Aabb subAabb = collider->GetWorldAabb();
      invalid &= !subAabb.Valid();

      aabb.Combine(subAabb);
    }
  }

  if(invalid && expandTransform)
  {
    if(world)
      aabb.Expand(tx->GetWorldTranslation());
    else if(toParent && tx->TransformParent != nullptr)
      aabb.Expand(Math::TransformPoint(tx->TransformParent->GetLocalMatrix(), tx->GetLocalTranslation()));
    else
      aabb.Expand(tx->GetLocalTranslation());
  }

  if(includeMode != IncludeMode::Children)
    return;

  forRange(Cog& child, cog->GetChildren())
    ExpandAabb(&child, aabb, includeMode, world, !world);
}

void ExpandAabb(HandleParam instance, Aabb& aabb, IncludeMode::Type includeMode, bool world, bool toParent, bool expandTransform)
{
  Cog* cog = instance.Get<Cog*>();

  // Try the MetaTransform if no cog is available.  And, since MetaTransfrom
  // initializes its aabb to invalid - expanding won't affect the input aabb.
  if(cog == nullptr)
  {
    MetaTransform* mt = instance.StoredType->HasInherited<MetaTransform>();
    if(mt == nullptr)
      return;

    MetaTransformInstance transform = mt->GetInstance(instance);
    aabb.Combine(transform.mAabb);

    if(!transform.mAabb.Valid() && expandTransform)
    {
      if(world)
        aabb.Expand(transform.GetWorldTranslation());
      else if(toParent)
        aabb.Expand(transform.ToParent(transform.GetLocalTranslation()));
      else
        aabb.Expand(transform.GetLocalTranslation());
    }

    return;
  }

  ExpandAabb(cog, aabb, includeMode, world, toParent, expandTransform);
}

void ExpandAabbChildrenOnly(HandleParam instance, Aabb& aabb, bool world, bool expandTransform)
{
  Cog* cog = instance.Get<Cog*>();
  if(cog == nullptr)
    return;

  forRange(Cog& child, cog->GetChildren())
    ExpandAabb(&child, aabb, IncludeMode::Children, world, !world, expandTransform);
}

}//namespace Zero
