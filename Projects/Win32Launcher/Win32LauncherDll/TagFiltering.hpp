///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// TagPolicy for ZeroBuilds
struct ZeroBuildTagPolicy
{
  bool mShowExperimentalBranches;
  bool mShowOnlyPreferredPlatform;
  String mPreferredPlatform;

  ZeroBuildTagPolicy()
  {
    mShowExperimentalBranches = false;
    mShowOnlyPreferredPlatform = true;
    mPreferredPlatform = GetPlatformString();
  }

  ZeroBuildTagPolicy(LauncherConfig* launcherConfig)
  {
    mShowExperimentalBranches = launcherConfig->mShowExperimentalBranches;
    mShowOnlyPreferredPlatform = launcherConfig->mDisplayOnlyPreferredPlatform;
    mPreferredPlatform = GetPlatformString();
  }

  TagSet& GetTagSet(ZeroBuild* build)
  {
    return build->GetBuildContent(true)->mTagSet;
  }

  String GetName(ZeroBuild* build)
  {
    return build->GetDisplayString();
  }

  bool ShouldInclude(ZeroBuild* build)
  {
    BuildId buildId = build->GetBuildId();
    if(!mShowExperimentalBranches && !buildId.mExperimentalBranchName.Empty())
      return false;
    if(mShowOnlyPreferredPlatform && buildId.mPlatform != mPreferredPlatform)
      return false;
    return true;
  }
};

/// TagPolicy for TemplateProject
struct TemplatePackageTagPolicy
{
  TagSet& GetTagSet(TemplateProject* project)
  {
    return project->GetZeroTemplate(true)->mTagSet;
  }

  String GetName(TemplateProject* project)
  {
    return project->GetDisplayName();
  }

  bool ShouldInclude(TemplateProject* project)
  {
    return project->ContainsVersion(mBuildId);
  }

  BuildId mBuildId;
};

// Small helper struct to make filtering of projects easier. This is needed because the
// TagSet must be returned by reference but the project has 2 sets that need to be merged
// (and the sets might not exist because they're on the ProjectDescription component)
struct ProjectInformation
{
  CachedProject* mCachedProject;
  TagSet mTags;
};

struct ProjectPolicy
{
  TagSet& GetTagSet(ProjectInformation& project)
  {
    return project.mTags;
  }

  String GetName(ProjectInformation& project)
  {
    return project.mCachedProject->GetProjectName();
  }

  bool ShouldInclude(ProjectInformation& project)
  {
    return true;
  }
};

// Requires a policy that Contains 3 functions (GetTagSet, GetName, and ShouldInclude)
template <typename DataType, typename Policy>
void FilterDataSetWithTags(TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch,
                           Array<DataType>& input, Array<DataType>& results, 
                           TagSet& resultTags, Policy& policy)
{
  String searchText = activeSearch.ToLower();
  for(size_t i = 0; i < input.Size(); ++i)
  {
    DataType& item = input[i];
    TagSet& tagSet = policy.GetTagSet(item);

    //see if this standalone Contains all of the active tags
    bool containsAllTags = true;
    bool containsRejection = false;
    if(!ContainsRequiredTagsCaseInsensitive(activeTags, tagSet))
      containsAllTags = false;

    // Check for the wild-card tag (overrides required tags but not rejection)
    if(tagSet.Contains("*"))
      containsAllTags = true;

    for(HashSet<String>::range range = rejectionTags.All(); !range.Empty(); range.PopFront())
    {
      String rejectionTag = range.Front();
      if(tagSet.Contains(rejectionTag) == true)
      {
        containsRejection = true;
        break;
      }
    }

    //if the standalone Contains all of the necessary tags then we need to check if it Contains
    //the current search string and we need to populate the new list of available tags
    if(containsAllTags && !containsRejection)
    {
      //check the active search string and see if this version matches
      if(searchText.Empty() || policy.GetName(item).ToLower().Contains(searchText))
      {
        if(policy.ShouldInclude(item))
          results.PushBack(item);
      }
      else
      {
        //check to see if there's a partial match with any tag (aka add any item with reasonable tags)
        for(TagSet::range range = tagSet.All(); !range.Empty(); range.PopFront())
        {
          StringRange tag = range.Front();
          if(searchText.Empty() || tag.ToLower().Contains(searchText))
          {
            if(policy.ShouldInclude(item))
            {
              results.PushBack(item);
              break;
            }
          }
        }
      }

      //check all of the tags of this standalone to see if we should add them
      for(TagSet::range range = tagSet.All(); !range.Empty(); range.PopFront())
      {
        StringRange tag = range.Front();
        //if the active tags already Contains this tag then don't add it again
        if(activeTags.Contains(tag))
          continue;
        //match this tag against the active search
        if(searchText.Empty() || tag.Contains(searchText))
          resultTags.Insert(tag);
      }
    }
  }
}

}//namespace Zero
