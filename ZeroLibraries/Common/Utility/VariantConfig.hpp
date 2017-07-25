///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//---------------------------------------------------------------------------------//
//                            Variant Configuration                                //
//---------------------------------------------------------------------------------//

//
// These Variant configuration options allow us to easily decide what missing stored type functionality we consider acceptable.
// None of the following options alter Variant's actual behavior in any way, they only toggle asserts notifying the developer that
// potentially undesirable fallback behavior was performed during a Variant operation (because the requested stored type functionality was unavailable).
//

/// Error On Missing Default Constructor?
/// During variant default assignment, stored types without a default constructor will be zero-initialized instead.
/// Note: Zero-initialized is likely not a valid state for complex types! Highly recommended to keep this assert enabled.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_DEFAULT_CONSTRUCTOR 1

/// Error On Missing Move Constructor?
/// During variant move assignment, stored types without a move constructor will be copied instead.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_MOVE_CONSTRUCTOR 0

/// Error On Missing ComparePolicy?
/// During variant equality comparison, stored types without a valid ComparePolicy will return false instead.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_COMPARE_POLICY 1

/// Error On Missing HashPolicy?
/// During variant hash operation, stored types without a valid HashPolicy will return 0 instead.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_HASH_POLICY 1

/// Error On Missing ToString Function?
/// During variant value to string conversion, stored types without a global ToString function will return String() instead.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_TO_STRING_FUNCTION 1

/// Error On Missing ToValue Function?
/// During variant string to value conversion, stored types without a global ToValue function will assign a default constructed value instead.
/// If enabled, variant will assert immediately before this occurs.
/// If disabled, variant will remain silent when this occurs.
#define VARIANT_ERROR_ON_MISSING_TO_VALUE_FUNCTION 1
