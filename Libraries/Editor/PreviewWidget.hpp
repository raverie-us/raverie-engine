// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(PreviewObjectChanged);
}

DeclareEnum3(PreviewImportance,
             None,   // No useful way to display element
             Simple, // Decorative or just a icon not very useful
             High);  // Needed to be able to edit object

struct PreviewWidgetInitializer
{
  PreviewWidgetInitializer() : Area(nullptr), Interactive(false){};
  Composite* Area;
  String Name;
  Handle Object;
  bool Interactive;
};

DeclareEnum3(PreviewAnimate, None, MouseOver, Always);

class PreviewWidget : public Composite
{
public:
  ZilchDeclareType(PreviewWidget, TypeCopyMode::ReferenceType);

  PreviewWidget(Composite* parent);
  PreviewWidget(PreviewWidgetInitializer& initializer);

  virtual void AnimatePreview(PreviewAnimate::Enum value)
  {
  }
  virtual Handle GetEditObject()
  {
    return Handle();
  }
  virtual void SetInteractive(bool interactive)
  {
    mInteractive = interactive;
  };

  /// Widget Interface.
  void UpdateTransform() override;
  virtual Vec2 GetHalfSize();

  TextureView* mBackground;
  String mName;
  Handle mObject;
  bool mInteractive;
};

class PreviewWidgetGroup : public PreviewWidget
{
public:
  PreviewWidgetGroup(Composite* parent);
  PreviewWidgetGroup(Composite* parent, StringParam name);

  PreviewWidget* AddPreviewWidget(StringParam name,
                                  HandleParam instance,
                                  PreviewImportance::Enum minImportance = PreviewImportance::None);
  void AnimatePreview(PreviewAnimate::Enum value) override;

  /// Widget Interface.
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  IntVec2 GetDimensions();
  Vec2 GetMinTileSize();
  Array<PreviewWidget*> mPreviewWidgets;
};

template <typename type>
PreviewWidget* CreatePreviewWidgetT(PreviewWidgetInitializer& initializer)
{
  return new type(initializer);
}

typedef PreviewWidget* (*CreatePreviewWidget)(PreviewWidgetInitializer& initializer);

// Creator
struct PreviewWidgetCreator
{
  PreviewWidgetCreator() : Importance(PreviewImportance::None), Creator(nullptr)
  {
  }
  PreviewWidgetCreator(PreviewImportance::Enum importance, CreatePreviewWidget creator) :
      Importance(importance),
      Creator(creator)
  {
  }
  PreviewImportance::Enum Importance;
  CreatePreviewWidget Creator;
};

/// PreviewWidgetFactory can make preview widgets
class PreviewWidgetFactory : public ExplicitSingleton<PreviewWidgetFactory, Object>
{
public:
  ZilchDeclareType(PreviewWidgetFactory, TypeCopyMode::ReferenceType);

  typedef HashMap<String, PreviewWidgetCreator> CellEditorMapType;
  CellEditorMapType Creators;
  CreatePreviewWidget GetCreator(StringParam type)
  {
    return Creators.FindValue(type, PreviewWidgetCreator()).Creator;
  }
};

class ResourcePreview
{
public:
  static PreviewWidget* CreatePreviewWidget(Composite* parent,
                                            StringParam name,
                                            HandleParam instance,
                                            PreviewImportance::Enum minImportance = PreviewImportance::None);

  static PreviewImportance::Enum GetPreviewImportance(BoundType* resourceType);
};

} // namespace Zero
