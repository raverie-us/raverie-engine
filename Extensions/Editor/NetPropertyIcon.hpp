///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Forward Declarations
namespace Zero
{
  class NetPropertyInfo;
  class SearchViewEvent;
}

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               NetPropertyIcon                                   //
//---------------------------------------------------------------------------------//

/// Network Property Icon
/// Used to toggle network replication for any supported property
class NetPropertyIcon : public Composite
{
public:
  /// Typedefs
  typedef NetPropertyIcon ZilchSelf;

  /// Constructor
  NetPropertyIcon(Composite* parent, HandleParam object,
                  Property* metaProperty);

  //
  // Composite Interface
  //

  void UpdateTransform() override;

  //
  // Event Handlers
  //

  void OnMouseEnter(Event* event);
  void OnMouseExit(Event* event);
  void OnLeftClick(Event* event);
  void OnRightClick(MouseEvent* event);
  void OnSearchCompleted(SearchViewEvent* event);

  //
  // Icon Interface
  //

  /// Returns true if the property is enabled for network replication, else false
  bool IsEnabled();

  /// Returns the net property info associated with the property (if enabled), else nullptr
  NetPropertyInfo* GetNetPropertyInfo();

  //
  // Data
  //

  /// Currently mousing over the icon?
  bool                         mMouseOver;
  /// Icon element
  Element*                     mIcon;
  /// Component where the property is defined
  Handle                       mComponentHandle;
  /// Property specified
  Property*                    mProperty;
  /// Active search view
  HandleOf<FloatingSearchView> mActiveSearch;
  /// Description tooltip
  HandleOf<ToolTip>            mTooltip;
};

/// Returns true if the selected object(s) should display net property icons next to all supported properties in the property grid
bool ShouldDisplayNetPropertyIcon(HandleParam selection);

/// Callback for adding custom icons to the property grid
Widget* CreateNetPropertyIcon(Composite* parent, HandleParam object,
                              Property* metaProperty, void* clientData);

}//namespace Zero
