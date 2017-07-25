///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorCommands.hpp
/// Various editor commands.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

void BindEditorCommands(Cog* configCog, CommandManager* commands);

DeclareEnum2(SelectComponentMode, WithComponent, WithOutComponent);

class MetaSelection;
class Editor;
struct Aabb;

void DeleteSelectedObjects(Editor* editor, Space* space);
void AddToSelection(Space* space, MetaSelection* selection, BoundType* boundType, SelectComponentMode::Enum mode);
void SaveSelectionToClipboard(Editor* editor, Space* space);
void LoadObjectFromClipboard(Editor* editor, Space* space);
Cog* SelectTopOfTree(Editor* editor, Space* cog);
Cog* GetParentOfSelection(Editor* editor, Space* cog);
void SelectAllInTree(Editor* editor, Space* cog);
void ClearSelection(Editor* editor, Space* cog);
void DuplicateSelection(Editor* editor, Space* space);

/// Parents all selected objects to the primary selected object
/// (highlighted in orange)
void ParentToPrimary(Editor* editor, Space* space);
/// Detaches all selected objects from their parents
void DetachSelected(Editor* editor, Space* space);
/// Detaches all child objects of the selected object
void FlattenTree(Editor* editor, Space* space);

void FilterChildrenAndProtected(Array<Cog*>& cog, MetaSelection* selection);
void FilterChildrenAndProtected(const Array<CogId>& cogsIn, Array<Cog*>& cogsOut);

void ClearObjectStore();

void EnableFpuExceptions();

void FocusOnSelectedObjects();

void CameraFocusSpace(Space* space);

DeclareEnum3(EditFocusMode, AutoTime, Center, Frame);

void CameraFocusSpace(Space* space, Cog* camera, EditFocusMode::Enum mode);
void CameraFocusSpace(Space* space, Cog* camera, const Aabb& focusAabb, EditFocusMode::Enum mode);

// Space to run shortcuts, Viewport for short cuts (null for default)
bool ExecuteShortCuts(Space* space, Viewport* viewport, KeyboardEvent* event);
bool CogHierarchyIndexCompareFn(Cog* lhs, Cog* rhs);

}//namespace Zero
