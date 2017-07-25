///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations.
class UiWidget;
class UiRootWidget;

DeclareEnum4(StencilDrawMode, None, Add, Remove, Test);

//------------------------------------------------------------------ Ui Renderer
class UiRenderer : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UiRenderer();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Renders the Ui to the given color render target. The depth render target must have stencil.
  void Render(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth, MaterialBlock* renderPass);

private:
  void RenderWidgets(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth,
                     MaterialBlock* renderPass, UiWidget* widget, Vec4Param colorTransform);

  void AddGraphical(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth,
                    MaterialBlock* renderPass, Cog* widgetCog, StencilDrawMode::Enum stencilMode,
                    uint stencilIncrement);

  /// Clears the GraphicalRangeInterface and 
  void FlushGraphicals(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth,
                       MaterialBlock* renderPass);

  /// All objects to be rendered will be added to this list.
  GraphicalRangeInterface mGraphicals;

  uint mStencilCount;
  StencilDrawMode::Enum mStencilDrawMode;

  RenderSettings mStencilAddSettings;
  RenderSettings mStencilRemoveSettings;
  RenderSettings mStencilTestSettings;

  /// Dependencies.
  UiWidget* mWidget;
  UiRootWidget* mRootWidget;
};

}//namespace Zero
