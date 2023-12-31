// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"
#include "Foundation/Support/SupportStandard.hpp"

// Raverie Library Dependencies
#include "Foundation/RaverieLanguage/Precompiled.hpp"
using namespace Raverie;

namespace Raverie
{

// Networking library
class NetworkingLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(NetworkingLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

// Extension Library Dependencies
#include "Foundation/Replication/ReplicationStandard.hpp"

// Core Library Dependencies
#include "Systems/Engine/EngineStandard.hpp"
#include "Systems/Physics/PhysicsStandard.hpp"

// Other Networking Includes
#include "WebRequester.hpp"

// NetPeer Forward Declarations
namespace Raverie
{
class BitStreamExtended;
class EventBundle;
class NetHostRecord;
class FamilyTree;
class NetHost;
class NetProperty;
class NetPropertyType;
class NetPropertyConfig;
class NetPropertyConfigManager;
class NetPropertyInfo;
class NetChannel;
class NetChannelType;
class NetChannelConfig;
class NetChannelConfigManager;
class NetDiscoveryInterface;
class NetObject;
class NetUser;
struct PendingNetUser;
class NetSpace;
class NetPeer;
} // namespace Raverie

// NetPeer Includes
#include "BitStreamExtended.hpp"
#include "EventBundle.hpp"
#include "NetHostRecord.hpp"
#include "NetTypes.hpp"
#include "NetEvents.hpp"
#include "NetHost.hpp"
#include "NetProperty.hpp"
#include "NetChannel.hpp"
#include "NetObject.hpp"
#include "NetUser.hpp"
#include "NetSpace.hpp"
#include "NetPeerConnectionInterface.hpp"
#include "NetPeerMessageInterface.hpp"
#include "PendingHostPing.hpp"
#include "PingManager.hpp"
#include "NetDiscoveryInterface.hpp"
#include "InternetHostDiscovery.hpp"
#include "LanHostDiscovery.hpp"
#include "NetPeer.hpp"
#include "NetworkingBindingExtensions.hpp"
