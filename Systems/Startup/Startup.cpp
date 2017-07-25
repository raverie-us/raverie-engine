///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------------ Startup
//**************************************************************************************************
Engine* ZeroStartup::Initialize(ZeroStartupSettings& settings)
{
  InitializeLibraries(settings);
  return InitializeEngine();
}

//**************************************************************************************************
void ZeroStartup::InitializeLibraries(ZeroStartupSettings& settings)
{
  CommonLibrary::Initialize();

  // Temporary location for registering handle managers
  //ZilchRegisterSharedHandleManager(ReferenceCountedHandleManager);
  ZilchRegisterSharedHandleManager(CogHandleManager);
  ZilchRegisterSharedHandleManager(ComponentHandleManager);
  ZilchRegisterSharedHandleManager(ResourceHandleManager);
  ZilchRegisterSharedHandleManager(WidgetHandleManager);
  ZilchRegisterSharedHandleManager(ContentItemHandleManager);

  // Basic handles
  ZeroRegisterHandleManager(ReferenceCountedEmpty);
  ZeroRegisterHandleManager(SafeId32);
  ZeroRegisterHandleManager(SafeId64);
  ZeroRegisterHandleManager(ThreadSafeId32);
  ZeroRegisterHandleManager(ThreadSafeId64);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64);

  // Object handles
  ZeroRegisterHandleManager(ReferenceCountedObject);
  ZeroRegisterHandleManager(SafeId32Object);
  ZeroRegisterHandleManager(SafeId64Object);
  ZeroRegisterHandleManager(ThreadSafeId32Object);
  ZeroRegisterHandleManager(ThreadSafeId64Object);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32Object);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64Object);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32Object);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  ZeroRegisterHandleManager(ReferenceCountedEventObject);
  ZeroRegisterHandleManager(SafeId32EventObject);
  ZeroRegisterHandleManager(SafeId64EventObject);
  ZeroRegisterHandleManager(ThreadSafeId32EventObject);
  ZeroRegisterHandleManager(ThreadSafeId64EventObject);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32EventObject);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64EventObject);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32EventObject);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64EventObject);

  ZeroRegisterHandleManager(ContentComposition);

  ZeroRegisterThreadSafeReferenceCountedHandleManager(ThreadSafeReferenceCounted);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(BlendSettings);
  ZeroRegisterThreadSafeReferenceCountedHandleManager(DepthSettings);

  // Setup the core Zilch library
  mZilchSetup = new ZilchSetup();

  // We need the calling state to be set so we can create Handles for Meta Components
  Zilch::Module module;
  mState = module.Link();
  ExecutableState::CallingState = mState;

  MetaDatabase::Initialize();

  // Add the core library to the meta database
  MetaDatabase::GetInstance()->AddNativeLibrary(Core::GetInstance().GetLibrary());

  ZilchManager::Initialize();

  // Initialize Zero Libraries
  PlatformLibrary::Initialize();
  GeometryLibrary::Initialize();
  // Geometry doesn't know about the Meta Library, so it cannot add itself to the MetaDatabase
  MetaDatabase::GetInstance()->AddNativeLibrary(GeometryLibrary::GetLibrary());
  MetaLibrary::Initialize();
  SerializationLibrary::Initialize();
  ContentMetaLibrary::Initialize();
  SpatialPartitionLibrary::Initialize();

  EngineLibrary::Initialize(settings);
  WindowsShellSystemLibrary::Initialize();
  GraphicsLibrary::Initialize();
  PhysicsLibrary::Initialize();
  NetworkingLibrary::Initialize();
  SoundLibrary::Initialize();

  WidgetLibrary::Initialize();
  GameplayLibrary::Initialize();
  EditorLibrary::Initialize();
  UiWidgetLibrary::Initialize();

  ZilchScriptLibrary::Initialize();

  NativeBindingList::ValidateTypes();

  // Load documentation for all native libraries
  DocumentationLibrary::GetInstance()->LoadDocumentation(FilePath::Combine(Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory, "Documentation.data"));
}

//**************************************************************************************************
Engine* ZeroStartup::InitializeEngine()
{
  return Z::gEngine;
}

//**************************************************************************************************
void ZeroStartup::Shutdown()
{
  Zero::TimerBlock block("Shutting down Libraries.");

  Core::GetInstance().GetLibrary()->ClearComponents();

  // Shutdown in reverse order
  ZilchScriptLibrary::Shutdown();

  UiWidgetLibrary::Shutdown();
  EditorLibrary::Shutdown();
  GameplayLibrary::Shutdown();
  WidgetLibrary::Shutdown();

  SoundLibrary::Shutdown();
  NetworkingLibrary::Shutdown();
  PhysicsLibrary::Shutdown();
  GraphicsLibrary::Shutdown();
  WindowsShellSystemLibrary::Shutdown();
  EngineLibrary::Shutdown();

  SpatialPartitionLibrary::Shutdown();
  ContentMetaLibrary::Shutdown();
  SerializationLibrary::Shutdown();
  MetaLibrary::Shutdown();
  GeometryLibrary::Shutdown();
  PlatformLibrary::Shutdown();
  
  // ClearLibrary
  ZilchScriptLibrary::GetInstance().ClearLibrary();

  UiWidgetLibrary::GetInstance().ClearLibrary();
  EditorLibrary::GetInstance().ClearLibrary();
  GameplayLibrary::GetInstance().ClearLibrary();
  WidgetLibrary::GetInstance().ClearLibrary();

  SoundLibrary::GetInstance().ClearLibrary();
  NetworkingLibrary::GetInstance().ClearLibrary();
  PhysicsLibrary::GetInstance().ClearLibrary();
  GraphicsLibrary::GetInstance().ClearLibrary();
  WindowsShellSystemLibrary::GetInstance().ClearLibrary();
  EngineLibrary::GetInstance().ClearLibrary();

  SpatialPartitionLibrary::GetInstance().ClearLibrary();
  ContentMetaLibrary::GetInstance().ClearLibrary();
  SerializationLibrary::GetInstance().ClearLibrary();
  MetaLibrary::GetInstance().ClearLibrary();
  GeometryLibrary::GetInstance().ClearLibrary();

  // Destroy
  ZilchScriptLibrary::Destroy();

  UiWidgetLibrary::Destroy();
  EditorLibrary::Destroy();
  GameplayLibrary::Destroy();
  WidgetLibrary::Destroy();

  SoundLibrary::Destroy();
  NetworkingLibrary::Destroy();
  PhysicsLibrary::Destroy();
  GraphicsLibrary::Destroy();
  WindowsShellSystemLibrary::Destroy();
  EngineLibrary::Destroy();

  SpatialPartitionLibrary::Destroy();
  ContentMetaLibrary::Destroy();
  SerializationLibrary::Destroy();
  MetaLibrary::Destroy();
  GeometryLibrary::Destroy();

  ZilchManager::Destroy();
  MetaDatabase::Destroy();
  
  delete mState;
  delete mZilchSetup;

  CommonLibrary::Shutdown();
}

}//namespace Zero
