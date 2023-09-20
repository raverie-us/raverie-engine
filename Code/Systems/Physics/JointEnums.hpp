// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace JointEnums
{

// Define an enum that constraints all of the joint types and the total joint
// count.
#define JointType(type) type##Type,

enum JointTypes
{
#include "JointList.hpp"
  JointCount
};

#undef JointType

} // namespace JointEnums

// Additionally define JointTypes of the same enum specification zero needs for
// binding
namespace JointTypes
{

// Define an enum that constraints all of the joint types and the total joint
// count.
#define JointType(type) type,

// Declare the actual Enum
enum Enum
{
#include "JointList.hpp"
  Size
};
#undef JointType

// Define the Names for binding
#define JointType(type) #type,
static const cstr Names[] = {
#include "JointList.hpp"
    "Size"};
#undef JointType

} // namespace JointTypes

} // namespace Raverie
