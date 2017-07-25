///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

#define InitializePropertyFilterForType(typeName) \
  ZilchInitializeType(PropertyFilter##typeName)

// Ranges
ZilchDefineRange(EventRange);
ZilchDefineRange(NetUserRange);
ZilchDefineRange(NetHostRange);

// Enums
ZilchDefineEnum(TcpSocketBind);
ZilchDefineEnum(NetUserAddResponse);
ZilchDefineEnum(Network);
ZilchDefineEnum(NetRefreshResult);
ZilchDefineEnum(Role);
ZilchDefineEnum(Authority);
ZilchDefineEnum(AuthorityMode);
ZilchDefineEnum(DetectionMode);
ZilchDefineEnum(ReliabilityMode);
ZilchDefineEnum(SerializationMode);
ZilchDefineEnum(RouteMode);
ZilchDefineEnum(ReplicationPhase);
ZilchDefineEnum(ConvergenceState);
ZilchDefineEnum(BasicNetType);
ZilchDefineEnum(TransportProtocol);
ZilchDefineEnum(ConnectResponseMode);
ZilchDefineEnum(TransmissionDirection);
ZilchDefineEnum(LinkStatus);
ZilchDefineEnum(LinkState);
ZilchDefineEnum(ConnectResponse);
ZilchDefineEnum(DisconnectReason);
ZilchDefineEnum(UserConnectResponse);
ZilchDefineEnum(TransferMode);
ZilchDefineEnum(Receipt);

// Arrays
ZeroDefineArrayType(NetPropertyInfoArray);

//**************************************************************************************************
ZilchDefineStaticLibrary(NetworkingLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRange(EventRange);
  ZilchInitializeRange(NetUserRange);
  ZilchInitializeRange(NetHostRange);

  // Enums
  ZilchInitializeEnum(TcpSocketBind);
  ZilchInitializeEnum(NetUserAddResponse);
  ZilchInitializeEnum(Network);
  ZilchInitializeEnum(NetRefreshResult);
  ZilchInitializeEnumAs(Role, "NetRole");
  ZilchInitializeEnum(Authority);
  ZilchInitializeEnum(AuthorityMode);
  ZilchInitializeEnum(DetectionMode);
  ZilchInitializeEnum(ReliabilityMode);
  ZilchInitializeEnum(SerializationMode);
  ZilchInitializeEnum(RouteMode);
  ZilchInitializeEnum(ReplicationPhase);
  ZilchInitializeEnum(ConvergenceState);
  ZilchInitializeEnum(BasicNetType);
  ZilchInitializeEnum(TransportProtocol);
  ZilchInitializeEnum(ConnectResponseMode);
  ZilchInitializeEnum(TransmissionDirection);
  ZilchInitializeEnum(LinkStatus);
  ZilchInitializeEnum(LinkState);
  ZilchInitializeEnum(ConnectResponse);
  ZilchInitializeEnum(DisconnectReason);
  ZilchInitializeEnum(UserConnectResponse);
  ZilchInitializeEnum(TransferMode);
  ZilchInitializeEnum(Receipt);

  // Meta Arrays
  ZeroInitializeArrayTypeAs(NetPropertyInfoArray, "NetPropertyInfos");

  // Events
  ZilchInitializeType(ConnectionEvent);
  ZilchInitializeType(ReceivedDataEvent);
  ZilchInitializeType(SendableEvent);
  ZilchInitializeType(WebResponseEvent);
  ZilchInitializeType(AcquireNetHostInfo);
  ZilchInitializeType(NetHostUpdate);
  ZilchInitializeType(NetHostListUpdate);
  ZilchInitializeType(NetPeerOpened);
  ZilchInitializeType(NetPeerClosed);
  ZilchInitializeType(NetGameStarted);
  ZilchInitializeType(NetPeerSentConnectRequest);
  ZilchInitializeType(NetPeerReceivedConnectRequest);
  ZilchInitializeType(NetPeerSentConnectResponse);
  ZilchInitializeType(NetPeerReceivedConnectResponse);
  ZilchInitializeType(NetLinkConnected);
  ZilchInitializeType(NetLinkDisconnected);
  ZilchInitializeType(NetLevelStarted);
  ZilchInitializeType(NetPeerSentUserAddRequest);
  ZilchInitializeType(NetPeerReceivedUserAddRequest);
  ZilchInitializeType(NetPeerSentUserAddResponse);
  ZilchInitializeType(NetPeerReceivedUserAddResponse);
  ZilchInitializeType(NetUserLostObjectOwnership);
  ZilchInitializeType(NetUserAcquiredObjectOwnership);
  ZilchInitializeType(RegisterCppNetProperties);
  ZilchInitializeType(NetObjectOnline);
  ZilchInitializeType(NetObjectOffline);
  ZilchInitializeType(NetUserOwnerChanged);
  ZilchInitializeType(NetChannelPropertyChange);
  ZilchInitializeType(NetEventSent);
  ZilchInitializeType(NetEventReceived);
  ZilchInitializeType(NetHostRecordEvent);

  // Meta Components
  ZilchInitializeType(EventBundleMetaComposition);
  ZilchInitializeType(PropertyFilterMultiPrimitiveTypes);
  ZilchInitializeType(PropertyFilterFloatingPointTypes);
  ZilchInitializeType(PropertyFilterArithmeticTypes);

  // Net property filters by type
  InitializePropertyFilterForType(Other);
  InitializePropertyFilterForType(Boolean);
  InitializePropertyFilterForType(Integer);
  InitializePropertyFilterForType(DoubleInteger);
  InitializePropertyFilterForType(Integer2);
  InitializePropertyFilterForType(Integer3);
  InitializePropertyFilterForType(Integer4);
  InitializePropertyFilterForType(Real);
  InitializePropertyFilterForType(DoubleReal);
  InitializePropertyFilterForType(Real2);
  InitializePropertyFilterForType(Real3);
  InitializePropertyFilterForType(Real4);
  InitializePropertyFilterForType(Quaternion);
  InitializePropertyFilterForType(String);

  // Other Networking Type Initialization
  ZilchInitializeType(TcpSocket);
  ZilchInitializeType(ThreadedWebRequest);
  ZilchInitializeType(BlockingWebRequest);
  ZilchInitializeType(ConnectionData);

  // NetPeer Type Initialization
  ZilchInitializeTypeAs(BitStreamExtended, "BitStream");
  ZilchInitializeType(EventBundle);
  ZilchInitializeType(NetPropertyInfo);
  ZilchInitializeType(NetPropertyConfig);
  ZilchInitializeType(NetPropertyType);
  ZilchInitializeType(NetProperty);
  ZilchInitializeType(NetChannelConfig);
  ZilchInitializeType(NetChannelType);
  ZilchInitializeType(NetChannel);
  ZilchInitializeType(NetHost);
  ZilchInitializeType(NetHostRecord);
  ZilchInitializeType(NetObject);
  ZilchInitializeType(NetSpace);
  ZilchInitializeType(NetUser);
  ZilchInitializeType(NetPeer);
  ZilchInitializeType(SimpleSocket);
  ZilchInitializeType(WebRequester);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void NetworkingLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  // Resource Managers
  InitializeResourceManager(NetChannelConfigManager);
  InitializeResourceManager(NetPropertyConfigManager);
}

//**************************************************************************************************
void NetworkingLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
