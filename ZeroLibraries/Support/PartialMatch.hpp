///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------ Command Filtering
static const int cNoMatch = INT_MIN;

// Returns priority or cNoMatch if there was no match (higher priority means higher up)
template<typename RangeType, typename Compare>
int PartialMatchOnly(RangeType part, RangeType full, Compare compare)
{
  bool foundAtLeastOneMatch = false;
  int priority = 0;

  //Compare the words
  while (!full.Empty())
  {
    RangeType partCopy = part;
    RangeType fullCopy = full;

    bool match = true;

    //Find sub matches
    while (!partCopy.Empty())
    {
      if(compare(partCopy.Front(), fullCopy.Front()) == false)
      {
        match = false;
        break;
      }
      partCopy.PopFront();
      fullCopy.PopFront();
    }

    if(match)
    {
      priority -= fullCopy.SizeInBytes();
      foundAtLeastOneMatch = true;
      break;
    }

    full.PopFront();
    //later matchers are worth less
    priority -= 10;
  }

  if(foundAtLeastOneMatch)
  {
    return priority;
  }
  else
  {
    return cNoMatch;
  }
}

// Returns priority or cNoMatch if there was no match (higher priority means higher up)
template<typename RangeType, typename Compare>
int PartialMatch(RangeType part, RangeType full, Compare compare)
{
  if(part.Empty())
    return 0;

  RangeType copyPart = part;

  bool allUpper = false;

  RangeType copyUpper = part;

  //Determine if it's all uppercase
  while(!copyUpper.Empty())
  {
    if(copyUpper.Front() != ToUpper(copyUpper.Front()))
    {
      allUpper = false;
      break;
    }
    else
    {
      allUpper = true;
    }

    copyUpper.PopFront();
  }

  int priority = PartialMatchOnly(part, full, compare);

  //If it's all upper case...
  if(allUpper)
  {
    int upperPriority = 0;

    while(!part.Empty() && !full.Empty())
    {
      if(full.Front() == ToUpper(full.Front()))
      {
        if(part.Front() == full.Front())
        {
          ++upperPriority;
          part.PopFront();
        }
        else
        {
          upperPriority = 0;
          break;
        }
      }

      full.PopFront();
    }

    if(upperPriority > priority && upperPriority != 0 && part.Empty() == true)
    {
      priority = upperPriority;
    }
  }

  return priority;
}

// These are organized in order of lowest to highest priority
DeclareEnum6(FilterMatch, None, PartialCaseInsensitive, PartialCaseSensitive, Acronym, StartsWithCaseInsensitive, StartsWithCaseSensitive);

template <typename ValueType>
struct StringFilterResult
{
  // These are not sorted, and will remain in the order passed in
  Array<ValueType*> mFiltered;
  Array<size_t> mFilteredOriginalIndices;

  // The primary is the best selected element
  ValueType* mPrimary;
  size_t mPrimaryOriginalIndex;
  size_t mPrimaryFilteredIndex;
};

/// Default adapter from the filter (just returns the input)
String StringContainerAdapter(StringParam element);

template <typename ValueType>
struct PrioritizedEntry
{
  FilterMatch::Enum mMatch;
  int mPriority;
  ValueType* mValue;
  size_t mOriginalIndex;
  size_t mFilteredIndex;

  PrioritizedEntry() :
    mMatch(FilterMatch::None),
    mPriority(INT_MIN),
    mValue(nullptr),
    mOriginalIndex((size_t)-1),
    mFilteredIndex((size_t)-1)
  {
  }

  void ChooseHigherPriority(FilterMatch::Enum match, int priority, ValueType* value, size_t originalIndex, size_t filteredIndex)
  {
    // If we got a better major match, or the match is the same and we got a better minor match, choose that
    if (match > mMatch || (match == mMatch && priority > mPriority))
    {
      mMatch = match;
      mPriority = priority;
      mValue = value;
      mOriginalIndex = originalIndex;
      mFilteredIndex = filteredIndex;
      return;
    }
  }
};

/// Checks to see if the given text matches an acronym
/// Verifies that the acronym is actually an acronym
bool MatchesAcronym(StringRangeParam text, StringRangeParam acronym);

/// Checks to see if a partial bit of text is found within a full string
bool MatchesPartial(StringParam text, StringParam partial, RuneComparer compare = CaseSensitiveCompare);

/// The adapter pulls out the strings that we're filtering from any container
/// Usage: auto filtered = FilterStrings(stringArray.All(), "Hello", YourContainerAdapter);
template <typename RangeType, typename Adapter>
StringFilterResult<typename RangeType::value_type> FilterStrings(RangeType range, StringParam searchText, Adapter adapter)
{
  typedef typename RangeType::value_type ValueType;

  PrioritizedEntry<ValueType> primary;

  size_t indexCounter = 0;

  StringFilterResult<ValueType> results;

  while (!range.Empty())
  {
    ValueType& front = range.Front();
    String text = adapter(front);
    range.PopFront();

    size_t filteredIndex = results.mFiltered.Size();

    // Whether or not this current string matched anything
    bool matchedAny = false;

    // We prioritize shorter strings (and higher number is higher priority, hence negative)
    int priority = -(int)text.SizeInBytes();

    if (text.StartsWith(searchText, CaseSensitiveCompare))
    {
      primary.ChooseHigherPriority(FilterMatch::StartsWithCaseSensitive, priority, &front, indexCounter, filteredIndex);
      matchedAny = true;
    }
    else if (text.StartsWith(searchText, CaseInsensitiveCompare))
    {
      primary.ChooseHigherPriority(FilterMatch::StartsWithCaseInsensitive, priority, &front, indexCounter, filteredIndex);
      matchedAny = true;
    }
    else if (MatchesAcronym(text, searchText))
    {
      primary.ChooseHigherPriority(FilterMatch::Acronym, priority, &front, indexCounter, filteredIndex);
      matchedAny = true;
    }
    else if (MatchesPartial(text, searchText, CaseSensitiveCompare))
    {
      primary.ChooseHigherPriority(FilterMatch::PartialCaseSensitive, priority, &front, indexCounter, filteredIndex);
      matchedAny = true;
    }
    else if (MatchesPartial(text, searchText, CaseInsensitiveCompare))
    {
      primary.ChooseHigherPriority(FilterMatch::PartialCaseInsensitive, priority, &front, indexCounter, filteredIndex);
      matchedAny = true;
    }

    if (matchedAny)
    {
      results.mFiltered.PushBack(&front);
      results.mFilteredOriginalIndices.PushBack(indexCounter);
    }

    ++indexCounter;
  }

  // Finally, set the primary we found on the results (may be null) and return it
  results.mPrimary = primary.mValue;
  results.mPrimaryOriginalIndex = primary.mOriginalIndex;
  results.mPrimaryFilteredIndex = primary.mFilteredIndex;
  return results;
}

/// Helper function that works for any container of strings (see FilterStrings above)
/// Usage: auto filtered = FilterStrings(stringArray.All(), "Hello");
template <typename RangeType>
StringFilterResult<typename RangeType::value_type> FilterStrings(RangeType range, StringParam searchText)
{
  return FilterStrings(range, searchText, StringContainerAdapter);
}

}
