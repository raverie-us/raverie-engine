///////////////////////////////////////////////////////////////////////////////
///
/// \file Tool.cpp
/// Implementation of the Tool classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ToolActivate);
  DefineEvent(ToolDeactivate);
  DefineEvent(ToolDraw);
}

namespace Tags
{
  DefineTag(Tool);
}

ZilchDefineType(Tool, builder, type)
{
  ZilchBindMethod(BeginDrag);
}

ZilchDefineType(ViewportTextWidget, builder, type)
{
}

//-------------------------------------------------------------- Tool Mouse Drag
class ToolMouseDrag : public MouseManipulation
{
public:
  Tool* mTool;
  HandleOf<Viewport> mViewport;

  ToolMouseDrag(Composite* owner, Mouse* mouse, Tool* tool, Viewport* viewport)
    : MouseManipulation(mouse, owner)
  {
    mTool = tool;
    mViewport = viewport;
    mTool->StartDrag(viewport, mouse->GetClientPosition());
  }

  ~ToolMouseDrag()
  {
    Viewport* viewport = mViewport;
    if(viewport == nullptr)
      return;

    mTool->EndDrag(viewport);
    viewport->TakeFocus();
  }

  void OnMouseMove(MouseEvent* event) override
  {
    Viewport* viewport = mViewport;
    if(viewport == nullptr)
      return;

    bool valid = mTool->MouseDragMovement(viewport, mMouseStartPosition, event->Position);
    if(!valid)
      this->Destroy();
  }

  void OnMouseUpdate(MouseEvent* event) override
  {
    Viewport* viewport = mViewport;
    if(viewport == nullptr)
      return;

    mTool->MouseDragUpdate(viewport, mMouseStartPosition, event->Position);
  }

  void OnKeyDown(KeyboardEvent* event) override
  {
    Viewport* viewport = mViewport;
    if(viewport == nullptr)
      return;

    viewport->DispatchEvent(Events::KeyDown, event);
    mTool->KeyDown(viewport,event);
  }

  void OnKeyUp(KeyboardEvent* event) override
  {
    Viewport* viewport = mViewport;
    if(viewport == nullptr)
      return;

    viewport->DispatchEvent(Events::KeyUp, event);
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }

  void OnRightMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }

  void OnTargetDestroy(MouseEvent* event) override
  {
    this->Destroy();
  }
};

//------------------------------------------------------------------------- Tool
Component* Tool::GetOrCreateEditComponent(BoundType* componentType, StringParam defaultName, StringParam defaultArchetype, CogId& lastEdited, bool canCreate)
{
  // Get the current selection
  MetaSelection* selection = Z::gEditor->GetSelection();

  // Try to get the component from the primary selection first...
  Cog* primaryCog = selection->GetPrimaryAs<Cog>();
  Component* component = nullptr;
  if (primaryCog)
    component = primaryCog->QueryComponentType(componentType);

  // If the primary has the component...
  if (component != nullptr)
  {
    // Store the selected cog and return the component
    lastEdited = selection->GetPrimaryAs<Cog>();
    return component;
  }

  // Check if our last stored selection had the component...
  component = lastEdited.QueryComponentId(componentType);

  // If the component is valid...
  if (component != nullptr)
  {
    // Return the height map component since it still exists
    return component;
  }

  // Otherwise, we need to try and see if any selected object has it
  auto range = selection->AllOfType<Cog>();

  // Loop through all the selected objects
  while (range.Empty() == false)
  {
    // Get the currently selected object and iterate forward
    Cog* selected = range.Front();
    range.PopFront();

    // If the object is valid...
    if (selected != nullptr)
    {
      // Grab the component from the current selected object
      component = selected->QueryComponentType(componentType);

      // If the component was valid
      if (component != nullptr)
      {
        // Store the selected cog and return the component
        lastEdited = selected;
        return component;
      }
    }
  }

  // Attempt to find the object by component
  auto space = Z::gEditor->GetEditSpace();

  // As long as we have a target space...
  if (space != nullptr)
  {
    Cog* found = space->FindObjectByName(defaultName);

    // Did we find an object with the default name?
    if (found != nullptr)
    {
      // Try to grab the component from the found object
      component = found->QueryComponentType(componentType);

      // If the component is not null
      if (component != nullptr)
      {
        // Store the found cog and return the component
        lastEdited = found;
        return component;
      }
    }

    auto range = space->AllObjects();

    // Loop through all objects
    while (range.Empty() == false)
    {
      // Get the object and iterate forward
      Cog* selected = &range.Front();
      range.PopFront();

      // If the object is valid...
      if (selected != nullptr)
      {
        // Grab the component from the current selected object
        component = selected->QueryComponentType(componentType);

        // If the component was valid
        if (component != nullptr)
        {
          // Store the selected cog and return the component
          lastEdited = selected;
          return component;
        }
      }
    }

    if (canCreate)
    {
      // Finally, as a last ditch effort, we should create the archetype...
      Cog* created = space->CreateNamed(defaultArchetype);

      // If we actually created the archetype...
      if (created != nullptr)
      {
        // Tell the user that the object was created
        DoNotify
        (
          "Object Created",
          "A tool editable object was created since none could "
          "be found (either in the selection or space)",
          "Warning"
        );

        // Clear the archetype from the object (we don't want them modifying it)
        created->ClearArchetype();

        // Try to grab the component from the created object
        component = created->QueryComponentType(componentType);

        // Clear the current selection and select the created object
        selection->Clear();
        selection->SetPrimary(created);

        // If the component is not null
        if (component != nullptr)
        {
          // Set the name
          created->SetName(defaultName);

          // Store the created cog and return the component
          lastEdited = created;
          return component;
        }
      }
    }
  }

  // Otherwise, we got nothing!
  return nullptr;
}

ViewportTextWidget* Tool::CreateViewportTextWidget(StringParam text)
{
  EditorViewport* viewport = Z::gEditor->mEditorViewport;
  if (viewport == nullptr)
    return nullptr;

  ViewportTextWidget* widget = new ViewportTextWidget(viewport);
  widget->SetText(text);
  widget->SizeToContents();
  viewport->UpdateTransformExternal();

  return widget;
}

void Tool::BeginDrag(Viewport* viewport)
{
  Mouse* mouse = Mouse::GetInstance();

  new ToolMouseDrag(viewport->GetParent(), mouse, this, viewport);
}

}//namespace Zero
