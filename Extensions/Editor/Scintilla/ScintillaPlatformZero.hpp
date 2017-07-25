///////////////////////////////////////////////////////////////////////////////
///
/// \file TextEditor.cpp
/// Implementation of the Scintilla Platform on Zero Engine
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

class SurfaceImpl : public Surface
{
public:
  SurfaceImpl(Zero::Widget* widget);

  void Init(WindowID wid) override;
  void Init(SurfaceID sid, WindowID wid) override;
  void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;
  void Release() override;
  bool Initialised() override;
  void PenColour(ColourDesired fore) override;
  int LogPixelsY() override;
  int DeviceHeightFont(int points) override;
  void MoveTo(int x_, int y_) override;
  void LineTo(int x_, int y_) override;
  void Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) override;
  void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void FillRectangle(PRectangle rc, ColourDesired back) override;
  void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
  void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill, ColourDesired outline, int alphaOutline, int flags) override;
  void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
  void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;
  void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
  void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override;
  void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore) override;
  void MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) override;
  XYPOSITION WidthText(Font &font_, const char *s, int len) override;
  XYPOSITION WidthChar(Font &font_, char ch) override;
  XYPOSITION Ascent(Font &font_) override;
  XYPOSITION Descent(Font &font_) override;
  XYPOSITION InternalLeading(Font &font_) override;
  XYPOSITION ExternalLeading(Font &font_) override;
  XYPOSITION Height(Font &font_) override;
  XYPOSITION AverageCharWidth(Font &font_) override;
  void SetClip(PRectangle rc) override;
  void FlushCachedState() override;
  void SetUnicodeMode(bool unicodeMode_) override;
  void SetDBCSMode(int codePage) override;

  enum DrawType { Lines, Quads, Text };

  Zero::Widget* mWidget;

  Zero::ViewBlock* mViewBlock;
  Zero::FrameBlock* mFrameBlock;
  Zero::ViewNode* mViewNode;
  Zero::Vec4 mColor;
  Zero::Rect mClipRect;
  Zero::Rect mBaseRect;

  float mLastX;
  float mLastY;
  DrawType mDrawType;
};

#ifdef SCI_NAMESPACE
} //namespace Scintilla
#endif
