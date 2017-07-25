///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Standard Library Dependencies
#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "Support/SupportStandard.hpp"

// Zilch Library Dependencies
#include "Zilch/Zilch.hpp"
using namespace Zilch;

namespace Zero
{

// Networking library
class ZeroNoImportExport NetworkingLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(NetworkingLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

// Extension Library Dependencies
#include "Dash/DashStandard.hpp"

// Core Library Dependencies
#include "Engine/EngineStandard.hpp"
#include "Physics/PhysicsStandard.hpp"

// Other Networking Includes
#include "SendableEvent.hpp"
#include "TcpSocket.hpp"
#include "SimpleSocket.hpp"
#include "IrcClient.hpp"
#include "WebRequest.hpp"

// NetPeer Forward Declarations
namespace Zero
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
}

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
