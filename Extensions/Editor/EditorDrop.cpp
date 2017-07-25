///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorDrop.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Sound/SoundStandard.hpp"

namespace Zero
{

bool DropOnObject(MetaDropEvent* event, Cog* droppedOn)
{
  Handle instance = event->Instance;
  BoundType* metaObjectDrop = instance.StoredType;
  Space* space = droppedOn->GetSpace();
  OperationQueue* queue = Z::gEditor->GetOperationQueue();

  // Search for property
  {
    //Search for a valid property
    BoundType* cogType = ZilchVirtualTypeId(droppedOn);
    MetaComposition* composition = cogType->HasInherited<MetaComposition>();

    //Check all components on object for properties that can be dropped.
    uint componentCount = composition->GetComponentCount(droppedOn);
    for(uint index = 0; index < componentCount; ++index)
    {
      Handle component = composition->GetComponentAt(droppedOn, index);
      
      //Check each property on the component
      forRange(Property* property, component.StoredType->GetProperties())
      {
        // if the object type being dropped matches the property of a component on the cog
        // set that property to be the dropped item
        if(property->PropertyType == metaObjectDrop)
        {
          if(event->Testing)
          {
            // display function is used over ToString to avoid showing the resoureceId
            String displayString;
            if (MetaDisplay* display = metaObjectDrop->HasInherited<MetaDisplay>())
              displayString = display->GetName(instance);
            else
              displayString = metaObjectDrop->Name;

            event->Result = String::Format("Set %s.%s to %s", component.StoredType->Name.c_str(),
                                                    property->Name.c_str(), displayString.c_str());
          }
          else
          {
            ChangeAndQueueProperty(queue, component, property, instance);
          }
          return true;
        }
      }
    }
  }
  return false;
}

bool DropOnObjectViewport(MetaDropEvent* event, Viewport* viewport, Space* space)
{
  Handle instance = event->Instance;
  BoundType* metaObjectDrop = instance.StoredType;
  String typeName = metaObjectDrop->Name;
  Vec2 dropPosition = event->Position;

  // DocumentResources can define classes
  if(DocumentResource* script = instance.Get<DocumentResource*>())
  {
    if(event->Testing)
      event->Result = String::Format("Open document %s", script->Name.c_str());
    else
      Z::gEditor->OpenDocumentResource(script);
    return true;
  }

  if(Archetype* archetype = instance.Get<Archetype*>())
  {
    // Only drop normal game objects (not spaces)
    if(archetype->mStoredType != ZilchTypeId(Cog))
      return false;

    if(event->Testing)
    {
      event->Result = String::Format("Create instance of %s", archetype->Name.c_str());
    }
    else
    {
      Z::gEditor->Tools->mCreationTool->CreateWithViewport(viewport, dropPosition, archetype);
    }
    return true;
  }

  if(Level* level = instance.Get<Level*>())
  {
    if(event->Testing)
    {
      event->Result = String::Format("Edit %s", level->Name.c_str());
    }
    else
    {
      Z::gEditor->EditResource(level);
    }
    return true;
  }

  // Creating an object if it's a sprite source
  if(SpriteSource* spriteSource = instance.Get<SpriteSource*>())
  {
    if(event->Testing)
    {
      event->Result = String::Format("Create sprite object from %s", spriteSource->Name.c_str());
    }
    else
    {
      Cog* object = Z::gEditor->Tools->mCreationTool->CreateWithViewport(viewport, dropPosition, CoreArchetypes::Sprite);
      if(object)
      {
        Sprite* sprite = object->has(Sprite);
        sprite->SetSpriteSource(spriteSource);
        object->ClearArchetype();
        object->SetName(spriteSource->Name);
      }
    }
    return true;
  }

  // Creating an object if it's a mesh
  if(Mesh* mesh = instance.Get<Mesh*>())
  {
    if(event->Testing)
    {
      event->Result = String::Format("Create mesh object from %s", mesh->Name.c_str());
    }
    else
    {
      Cog* object = Z::gEditor->Tools->mCreationTool->CreateWithViewport(viewport, dropPosition, CoreArchetypes::Transform);
      if(object)
      {
        object->AddComponentByName("Model");
        Model* model = object->has(Model);
        model->SetMesh(mesh);
        object->ClearArchetype();
        object->SetName(mesh->Name);
      }
    }
    return true;
  }

  // Create a sound emitter from dropping a sound cue
  if(SoundCue* soundCue = instance.Get<SoundCue*>())
  {
    if (event->Testing)
    {
      event->Result = String::Format("Create sound emitter object from %s", soundCue->Name.c_str());
    }
    else
    {
      Cog* object = Z::gEditor->Tools->mCreationTool->CreateWithViewport(viewport, dropPosition, CoreArchetypes::Transform);
      if (object)
      {
        object->AddComponentByName("SoundEmitter");
        object->AddComponentByName("SimpleSound");
        SimpleSound* sound = object->has(SimpleSound);
        sound->SetCue(soundCue);
        object->ClearArchetype();
        object->SetName(soundCue->Name);
      }
    }
    return true;
  }

  // when dragging something from the library view show what you are dragging around
  if (MetaDisplay* display = metaObjectDrop->HasInherited<MetaDisplay>())
    event->Result = display->GetName(instance);

  return false;
}

bool EditorDrop(MetaDropEvent* event, Viewport* viewport, Space* space, Cog* droppedOn)
{
  BoundType* droppedType = event->Instance.StoredType;

  // It's useful to have dropping a Mesh or Sprite act like dropping an Archetype. Create the 
  // object, but use Shift to set a property like most other Resources
  if(droppedType->IsA(ZilchTypeId(Mesh)) || droppedType->IsA(ZilchTypeId(SpriteSource)))
  {
    if (Keyboard::Instance->KeyIsDown(Keys::Shift) == false)
      droppedOn = nullptr;
  }

  if(droppedOn)
  {
    bool dropSuccess =  DropOnObject(event, droppedOn);
    if(dropSuccess)
      return dropSuccess;
  }

  if(viewport)
    return DropOnObjectViewport(event, viewport, space);

  return false;
}

}