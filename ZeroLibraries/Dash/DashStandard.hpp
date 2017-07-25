///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Standard Library Dependencies
#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Meta/MetaStandard.hpp"       // TODO: Investigate Dash dependency on Notifications
#include "Support/SupportStandard.hpp" // TODO: Investigate Dash dependency on Profiler

// Using Directives
namespace Zero
{
  // Basic Math Types
  using Math::BoolVector2;
  using Math::BoolVector3;
  using Math::BoolVector4;
  using Math::IntVector2;
  using Math::IntVector3;
  using Math::IntVector4;
  using Math::Vector2;
  using Math::Vector3;
  using Math::Vector4;
  using Math::Quaternion;
  using Math::Matrix2;
  using Math::Matrix3;
  using Math::Matrix4;
}

// Peer Forward Declarations
namespace Zero
{
  class Peer;
  class PeerPlugin;
  class PeerLink;
  class LinkPlugin;
  class LinkInbox;
  class LinkOutbox;
  class InPacket;
  class OutPacket;
  class OutMessage;
  class FragmentedMessage;
  class InMessageChannel;
}

// Peer Includes
#include "Enums.hpp"
#include "BandwidthStats.hpp"
#include "MessageConfig.hpp"
#include "Message.hpp"
#include "PacketConfig.hpp"
#include "Packet.hpp"
#include "MessageChannel.hpp"
#include "LinkInbox.hpp"
#include "LinkOutbox.hpp"
#include "ProtocolMessageData.hpp"
#include "PeerLink.hpp"
#include "Peer.hpp"

// Replicator Forward Declarations
namespace Zero
{
  class Replicator;
  class ReplicatorLink;
  class ReplicaStream;
  class Replica;
  class ReplicaChannel;
  class ReplicaChannelIndex;
  class ReplicaChannelType;
  class ReplicaProperty;
  class ReplicaPropertyType;
  class Route;
}

// Replicator Includes
#include "ReplicaConfig.hpp"
#include "Route.hpp"
#include "ReplicaProperty.hpp"
#include "ReplicaChannel.hpp"
#include "Replica.hpp"
#include "ReplicaStream.hpp"
#include "ReplicatorLink.hpp"
#include "Replicator.hpp"
