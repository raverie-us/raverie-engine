// MIT Licensed (see LICENSE.md).
#pragma once

#undef Assert
#include <map>
#include <assert.h>
#include <vector>

#include "include/Platform.h"
#include "include/SciLexer.h"
#include "include/Scintilla.h"
#include "include/ILexer.h"
#include "include/ScintillaWidget.h"

#include "lexlib/PropSetSimple.h"
#include "lexlib/WordList.h"
#include "lexlib/LexerBase.h"
#include "lexlib/LexerModule.h"
#include "lexlib/LexerNoExceptions.h"
#include "lexlib/LexerSimple.h"
#include "lexlib/OptionSet.h"
#include "lexlib/SparseState.h"
#include "lexlib/LexAccessor.h"
#include "lexlib/Accessor.h"
#include "lexlib/CharacterSet.h"
#include "lexlib/StyleContext.h"

#include "src/CallTip.h"
#include "src/Catalogue.h"
#include "src/SplitVector.h"
#include "src/Partitioning.h"
#include "src/CellBuffer.h"
#include "src/CharClassify.h"
#include "src/RunStyles.h"
#include "src/Decoration.h"
#include "src/Document.h"
#include "src/Selection.h"
#include "src/Style.h"
#include "src/XPM.h"
#include "src/LineMarker.h"
#include "src/Indicator.h"
#include "src/ViewStyle.h"
#include "src/PositionCache.h"
#include "src/AutoComplete.h"
#include "src/ContractionState.h"
#include "src/ExternalLexer.h"
#include "src/FontQuality.h"
#include "src/KeyMap.h"
#include "src/PerLine.h"
#include "src/RESearch.h"
#include "src/Editor.h"
#include "src/ScintillaBase.h"
#include "src/SVector.h"
#include "src/UniConversion.h"

namespace Scintilla
{

class SurfaceImpl : public Surface
{
public:
  SurfaceImpl(Zero::Widget* widget);

  void Init(WindowID wid) override;
  void Init(SurfaceID sid, WindowID wid) override;
  void InitPixMap(int width, int height, Surface* surface_, WindowID wid) override;
  void Release() override;
  bool Initialised() override;
  void PenColour(ColourDesired fore) override;
  int LogPixelsY() override;
  int DeviceHeightFont(int points) override;
  void MoveTo(int x_, int y_) override;
  void LineTo(int x_, int y_) override;
  void Polygon(Point* pts, int npts, ColourDesired fore, ColourDesired back) override;
  void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void FillRectangle(PRectangle rc, ColourDesired back) override;
  void FillRectangle(PRectangle rc, Surface& surfacePattern) override;
  void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void AlphaRectangle(PRectangle rc,
                      int cornerSize,
                      ColourDesired fill,
                      int alphaFill,
                      ColourDesired outline,
                      int alphaOutline,
                      int flags) override;
  void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char* pixelsImage) override;
  void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
  void Copy(PRectangle rc, Point from, Surface& surfaceSource) override;
  void DrawTextNoClip(PRectangle rc,
                      Font& font_,
                      XYPOSITION ybase,
                      const char* s,
                      int len,
                      ColourDesired fore,
                      ColourDesired back) override;
  void DrawTextClipped(PRectangle rc,
                       Font& font_,
                       XYPOSITION ybase,
                       const char* s,
                       int len,
                       ColourDesired fore,
                       ColourDesired back) override;
  void DrawTextTransparent(
      PRectangle rc, Font& font_, XYPOSITION ybase, const char* s, int len, ColourDesired fore) override;
  void MeasureWidths(Font& font_, const char* s, int len, XYPOSITION* positions) override;
  XYPOSITION WidthText(Font& font_, const char* s, int len) override;
  XYPOSITION WidthChar(Font& font_, char ch) override;
  XYPOSITION Ascent(Font& font_) override;
  XYPOSITION Descent(Font& font_) override;
  XYPOSITION InternalLeading(Font& font_) override;
  XYPOSITION ExternalLeading(Font& font_) override;
  XYPOSITION Height(Font& font_) override;
  XYPOSITION AverageCharWidth(Font& font_) override;
  void SetClip(PRectangle rc) override;
  void FlushCachedState() override;
  void SetUnicodeMode(bool unicodeMode_) override;
  void SetDBCSMode(int codePage) override;

  void RoundedLineRectHelper(PRectangle rc,
                             int cornerEmulation,
                             ColourDesired fill,
                             int alphaFill,
                             ColourDesired outline,
                             int alphaOutline,
                             int flags);

  enum DrawType
  {
    Lines,
    Quads,
    Text
  };

  void MakeViewNode(Zero::PrimitiveType::Enum primitive, StringParam textureName);
  void MakeViewNode(Zero::PrimitiveType::Enum primitive, Zero::Texture* texture);

  Zero::Widget* mWidget;

  Zero::ViewBlock* mViewBlock;
  Zero::FrameBlock* mFrameBlock;
  Zero::ViewNode* mViewNode;
  Zero::Vec4 mColor;
  Zero::WidgetRect mClipRect;
  Zero::WidgetRect mBaseRect;

  float mLastX;
  float mLastY;
};
} // namespace Scintilla
