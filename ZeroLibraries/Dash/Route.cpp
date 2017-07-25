///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                    Route                                        //
//---------------------------------------------------------------------------------//

// Constants
const Route Route::All(RouteMode::Exclude);  // Exclude no links (Specifies all links)
const Route Route::None(RouteMode::Include); // Include no links (Specifies no links)

Route::Route(RouteMode::Enum mode, ReplicatorIdSet targets)
  : mMode(mode),
    mTargets(ZeroMove(targets))
{
}
Route::Route(RouteMode::Enum mode, ReplicatorId replicatorId)
  : mMode(mode),
    mTargets()
{
  mTargets.Insert(replicatorId);
}
Route::Route(ReplicatorId replicatorId)
  : mMode(RouteMode::Include),
    mTargets()
{
  mTargets.Insert(replicatorId);
}
Route::Route(ReplicatorLink* replicatorLink)
  : mMode(RouteMode::Include),
    mTargets()
{
  if(!replicatorLink)
    return;

  Assert(replicatorLink->GetReplicatorId() != 0);
  mTargets.Insert(replicatorLink->GetReplicatorId());
}
Route::Route(PeerLink* link)
  : mMode(RouteMode::Include),
    mTargets()
{
  ReplicatorLink* replicatorLink = link ? link->GetPlugin<ReplicatorLink>("ReplicatorLink") : nullptr;
  if(!replicatorLink)
    return;

  Assert(replicatorLink->GetReplicatorId() != 0);
  mTargets.Insert(replicatorLink->GetReplicatorId());
}

} // namespace Zero
