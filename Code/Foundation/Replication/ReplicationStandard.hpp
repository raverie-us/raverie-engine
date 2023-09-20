// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"       // TODO: Investigate Replication dependency on Notifications
#include "Foundation/Support/SupportStandard.hpp" // TODO: Investigate Replication dependency on Profiler

// Using Directives
namespace Raverie
{
// Basic Math Types
using Math::BoolVector2;
using Math::BoolVector3;
using Math::BoolVector4;
using Math::IntVector2;
using Math::IntVector3;
using Math::IntVector4;
using Math::Matrix2;
using Math::Matrix3;
using Math::Matrix4;
using Math::Quaternion;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;
} // namespace Raverie

// Peer Forward Declarations
namespace Raverie
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
} // namespace Raverie

// Peer Includes
#include "Enums.hpp"
#include "BandwidthStats.hpp"
#include "MessageConfig.hpp"
#include "Message.hpp"
#include "PacketConfig.hpp"
#include "Packet.hpp"
#include "MessageChannel.hpp"
#include "ProtocolMessageData.hpp"
#include "LinkInbox.hpp"
#include "LinkOutbox.hpp"
#include "PeerLink.hpp"
#include "Peer.hpp"

// Replicator Forward Declarations
namespace Raverie
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
} // namespace Raverie

// Replicator Includes
#include "ReplicaConfig.hpp"
#include "Route.hpp"
#include "ReplicaProperty.hpp"
#include "ReplicaChannel.hpp"
#include "Replica.hpp"
#include "ReplicaStream.hpp"
#include "ReplicatorLink.hpp"
#include "Replicator.hpp"
