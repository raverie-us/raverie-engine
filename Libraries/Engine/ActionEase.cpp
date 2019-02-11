// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// The Warp ease contains a vertical asymptote. We shouldn't
// get too close to it, or we'll get outrageous numbers.
// Experimentation reveals that this is about as close as we
// can get and keep the absolute value of the output within
// one hundred thousand for the In and Out eases, and within
// fifty thousand for the InOut ease
const float Ease::Warp::MinDifferenceFromAsymptote = 7.9579e-12f;

} // namespace Zero
