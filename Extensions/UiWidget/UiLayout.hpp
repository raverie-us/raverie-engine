///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations.
class UiWidget;

//----------------------------------------------------------------------- Layout
/// Layouts are in charge of calling UpdateTransform on all children, regardless
/// of whether or not they ignore layouts.
class UiLayout : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Returns the minimum size that this widget needs to be based on the children.
  virtual Vec2 Measure(Rect& rect) = 0;
  
  /// Update the translation and sizes of all children objects.
  virtual Vec2 DoLayout(Rect& rect, UiTransformUpdateEvent* e) = 0;

  /// Padding getter / setters for binding until we have Thickness binding.
  float GetPaddingLeft();
  float GetPaddingTop();
  float GetPaddingRight();
  float GetPaddingBottom();
  void SetPaddingLeft(float val);
  void SetPaddingTop(float val);
  void SetPaddingRight(float val);
  void SetPaddingBottom(float val);

  /// Calling this will set a breakpoint before the layout is done.
  void Debug();
  
  /// We're dependent having a Widget.
  UiWidget* mWidget;

  /// Adds a border around all child objects. For more information, see the
  /// CSS Box Model.
  Thickness mPadding;

  /// Used to debug the layout in the editor.
  bool mDebug;

  //-------------------------------------------------------------------- Helpers
protected:
  /// Updates all child widgets that are not in a layout.
  void UpdateNotInLayout(UiTransformUpdateEvent* e);

  /// Helper for shifting widgets based on their alignment in a layout.
  void CalculateAlignment(Axis::Type axis, uint alignment, Vec2Param areaSize,
              Vec2Param areaPos, Vec2Param childSize, Vec2Ref childTranslation);

  /// Finds the maximum of all the minimum sizes of all child widgets.
  Vec2 MaxMeasure(Rect& rect);

  void ApplyPadding(Thickness& padding, Rect& area);
  void RemovePadding(Thickness& padding, Rect& area);

  //---------------------------------------------------------- Filtered Children
  /// Range for walking through all children that are in the layout.
  struct UiFilteredChildren
  {
    UiFilteredChildren(UiWidget* widget);

    UiWidget* Front();
    bool Empty();
    void PopFront();
    void SkipInvalid();

    UiWidget::ChildList::range mRange;
  };

  UiFilteredChildren AllWidgetsInLayout();
};

}//namespace Zero
