///////////////////////////////////////////////////////////////////////////////
///
/// \file ConditionalRange.hpp
/// Range that uses a functor condition.
///
/// Authors: Trevor Sundberg
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
  struct ConditionalPolicy
  {
    template <typename RangeValue>
    bool operator()(const RangeValue& value)
    {
      return true;
    }
  };

  template <typename Range, typename ConditionPolicy = ConditionalPolicy>
  class ConditionalRange : public Range
  {
  public:
    ConditionalRange()
    {
    }

    // Copy constructor
    ConditionalRange(const Range& value, ConditionPolicy policy = ConditionPolicy())
      : Range(value), mPolicy(policy)
    {
      Advance();
    }

    void PopFront()
    {
      Range::PopFront();
      Advance();
    }

    ConditionalRange& All() { return *this; }
    const ConditionalRange& All() const { return *this; }

  private:

    // Advance forward to the next place where the condition is true
    void Advance()
    {
      while (!Range::Empty() && !mPolicy(Range::Front()))
      {
        Range::PopFront();
      }
    }

  private:

    // Store the conditional policy
    ConditionPolicy mPolicy;
  };

}
