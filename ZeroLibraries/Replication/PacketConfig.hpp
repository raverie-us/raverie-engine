///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//---------------------------------------------------------------------------------//
//                            Packet Configuration                                 //
//---------------------------------------------------------------------------------//

/// Protocol ID bits
/// Determines the unique protocol identifier length used to deflect unknown packets
#define PROTOCOL_ID_BITS 31

/// Packet sequence ID bits
/// Determines the maximum number of packet sequence IDs before wrap-around occurs
#define PACKET_SEQUENCE_ID_BITS 32
StaticAssertWithinRange(Range0, PACKET_SEQUENCE_ID_BITS, 1, UINTMAX_BITS);

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                Protocol ID                                      //
//---------------------------------------------------------------------------------//

/// Protocol ID bits
static const Bits ProtocolIdBits = PROTOCOL_ID_BITS;

/// Protocol ID
typedef UintN<ProtocolIdBits, false> ProtocolId;

//---------------------------------------------------------------------------------//
//                             Packet Sequence ID                                  //
//---------------------------------------------------------------------------------//

/// Packet sequence ID bits
static const Bits PacketSequenceIdBits = PACKET_SEQUENCE_ID_BITS;

/// Packet sequence ID
typedef UintN<PacketSequenceIdBits, true> PacketSequenceId;

//---------------------------------------------------------------------------------//
//                             Packet Header Size                                  //
//---------------------------------------------------------------------------------//

/// Minimum packet header size
static const Bits MinPacketHeaderBits = ProtocolIdBits /// Packet protocol ID
                                      + 1;             /// Packet 'Is standalone?' flag

/// Maximum packet header size
static const Bits MaxPacketHeaderBits = MinPacketHeaderBits
                                      + PacketSequenceIdBits; /// Packet sequence ID
} // namespace Zero
