///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                    Route                                        //
//---------------------------------------------------------------------------------//

/// Network Route
/// Identifies a set of link targets
class Route
{
public:
  /// Constants
  static const Route All;
  static const Route None;

  /// Constructors
  Route(RouteMode::Enum mode = RouteMode::Exclude, ReplicatorIdSet targets = ReplicatorIdSet());
  Route(RouteMode::Enum mode, ReplicatorId replicatorId);
  Route(ReplicatorId replicatorId);
  Route(ReplicatorLink* replicatorLink);
  Route(PeerLink* link);

  /// Data
  RouteMode::Enum mMode;    /// Routing mode
  ReplicatorIdSet mTargets; /// Route link targets
};

} // namespace Zero
