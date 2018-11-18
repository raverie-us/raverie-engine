///////////////////////////////////////////////////////////////////////////////
///
/// \file SampleCurveEditor.hpp
/// Declaration of the SampleCurveEditor Composite.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations
class CurveEditor;
class TextBox;
class GraphWidget;
class MetaDropEvent;
class MultiSampleCurveEditor;

//---------------------------------------------------------- Sample Curve Editor
class SampleCurveEditor : public CurveEditor
{
public:
  /// Typedefs.
  typedef SampleCurveEditor ZilchSelf;

  /// Constructor.
  SampleCurveEditor(Composite* parent, GraphWidget* graph);

  /// CurveEditor Interface.
  Vec2 ToPixelPosition(Vec2 graphPos) override;
  Vec2 ToGraphPosition(Vec2 pixelPos) override;
  Vec2 ClampPixelPosition(Vec2 pixelPos) override;

  /// Used for converting between pixels and graph units.
  GraphWidget* mGraph;
};

//---------------------------------------------------------- Sample Curve Object
class SampleCurveObject : public CurveObject
{
public:
  typedef SampleCurveObject ZilchSelf;
  SampleCurveObject(SampleCurveEditor* editor, SampleCurve* curve);

  void GetCurve(Vec3Array& curve) override;
  void RemoveControlPoint(ControlPoint* controlPoint) override;

  /// Sets the curve being edited.
  void SetCurve(SampleCurve* curve);

  /// Returns the curve being edited.
  SampleCurve* GetSampleCurve();

  /// Loads the given SampleCurve into the curve editor.
  void LoadFromCurve(SampleCurve* curve);

  /// Bakes the current curve from the editor to the given SampleCurve.
  void BakeToCurve(SampleCurve* curve);

private:
  /// We need to update the SampleCurve when it's modified.
  void OnCurveModified(Event* e);

  void OnControlPointModified(CurveEvent* e);

  /// We need to push debug samples from the SampleCurve when this is called.
  void OnPushDebugSamples(Event* e);

  /// Called when the editor is saved.
  void OnSave(Event* e);

  bool mIgnoreCurveEditorEvents;

  /// The curve we're currently editing.
  HandleOf<SampleCurve> mSampleCurve;
};

//---------------------------------------------------- Multi Sample Curve Editor
class MultiSampleCurveEditor : public Composite
{
public:
  /// Typedefs.
  typedef MultiSampleCurveEditor ZilchSelf;

  /// Constructor.
  MultiSampleCurveEditor(Composite* parent);

  /// Composite Interface.
  void UpdateTransform();

  /// Loads the given SampleCurve into the curve editor.
  SampleCurveObject* LoadCurve(SampleCurve* curve);

  /// We want to add a curve if it's dropped on this view.
  void OnMetaDrop(MetaDropEvent* e);

private:
  friend class SampleCurveObject;

  /// Text box event response.
  void OnHeightChange(Event*);
  void OnWidthChange(Event*);

  Composite* mGraphArea;

  /// The text box for the height max of the curve.
  TextBox* mHeightTextBox;
  TextBox* mWidthTextBox;

  /// The graph drawn behind the curves.
  GraphWidget* mGraph;

  /// All curves currently being edited.
  SampleCurveEditor* mCurveEditor;
};

}// namespace Zero
