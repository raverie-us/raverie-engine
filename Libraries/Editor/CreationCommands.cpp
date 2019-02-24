// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Widget/Command.hpp"
#include "Editor.hpp"
#include "EditorCameraController.hpp"
#include "Engine/CogOperations.hpp"
#include "Widget/CommandBinding.hpp"
#include "Engine/Configuration.hpp"

namespace Zero
{

class EditorCreateObjectCommand : public CommandExecuter
{
public:
  String ArchetypeName;
  EditorCreateObjectCommand(StringParam archetypeName) : ArchetypeName(archetypeName)
  {
  }

  String GetDescription()
  {
    return BuildString("Create ", ArchetypeName);
  }

  void Execute(Command* command, CommandManager* commandManager) override;

  bool IsEnabled(Command* command, CommandManager* commandManager) override
  {
    return commandManager->GetContext()->Get<Space>() != nullptr;
  }
};

void EditorCreateObjectCommand::Execute(Command* command, CommandManager* manager)
{
  Space* space = manager->GetContext()->Get<Space>();
  if (space == nullptr)
    return CommandFailed(command, ZilchTypeId(Space));

  Editor* editor = Z::gEditor;

  Cog* editorCameraObject = space->FindObjectByName(SpecialCogNames::EditorCamera);
  if (editorCameraObject == nullptr)
    return;

  // Create the object
  Archetype* archetype = ArchetypeManager::Find(ArchetypeName);
  Cog* cog = space->Create(archetype);
  if (cog == nullptr)
    return;

  // We don't want it to be associated with the Archetype as they're all core
  // Resources
  cog->ClearArchetype();
  cog->SetName(ArchetypeName);

  // Create it at the origin
  Transform* transform = cog->has(Transform);
  if (transform)
    transform->SetLocalTranslation(Vec3::cZero);

  // If a cog command context has been set attach the new cog as a child
  CommandManager* commandManager = CommandManager::GetInstance();
  if (Cog* selectedCog = commandManager->GetContext()->Get<Cog>())
  {
    // Preserve local so that it is created under the parent
    cog->AttachToPreserveLocal(selectedCog);
  }
  else
  {
    EditorCameraController* editorCameraController = editorCameraObject->has(EditorCameraController);
    if (editorCameraController != nullptr)
    {
      if (transform)
        transform->SetLocalTranslation(editorCameraController->GetLookTarget());
    }
  }

  // Queue the creation of this object
  OperationQueue* opQueue = editor->GetOperationQueue();
  ObjectCreated(opQueue, cog);

  MetaSelection* selection = editor->GetSelection();
  selection->SelectOnly(cog);
  selection->FinalSelectionChanged();
}

typedef void (*StackCreateFunc)(Space* space, Vec3Param pos, Vec3Param size, uint levels, uint width);

class StackCreateCommand : public CommandExecuter
{
public:
  Vec3 Size;
  uint Height;
  uint Width;

  StackCreateFunc Function;
  StackCreateCommand(StackCreateFunc func, Vec3Param size, uint height, uint width) : Function(func)
  {
    Size = size;
    Height = height;
    Width = width;
  }

  bool IsEnabled(Command* command, CommandManager* commandManager) override
  {
    return commandManager->GetContext()->Get<Space>() != nullptr;
  }

  void Execute(Command* command, CommandManager* manager)
  {
    Space* space = manager->GetContext()->Get<Space>();
    if (space == nullptr)
      return CommandFailed(command, ZilchTypeId(Space));

    (*Function)(space, Vec3(0, 0, 0), Size, Height, Width);
  }
};

void CreatePyramid(Space* space, Vec3Param pos, Vec3Param scale, uint levels, uint width)
{
  // Start a batched operation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("CreatePyramid");

  // width unused
  Quat orientation = Quat::cIdentity;

  Vec3 boxSize = Vec3(2.0f, 2.0f, 2.0f);

  Cog* root = CreateFromArchetype(queue, space, CoreArchetypes::Transform, Vec3::cZero);
  root->SetName("PyramidRoot");
  root->ClearArchetype();

  for (uint level = 0; level < levels; ++level)
  {
    float y = level * boxSize.y + boxSize.y * 0.5f;
    float width = (((levels - 1) - level) / 2.0f) * boxSize.x;
    for (float x = -width; x <= width; x += boxSize.x)
    {
      for (float z = -width; z <= width; z += boxSize.x)
      {
        Cog* cog = CreateFromArchetype(queue, space, CoreArchetypes::Cube, Vec3(x, y, z), orientation, boxSize);
        cog->ClearArchetype();
        cog->AttachTo(root);
      }
    }
  }

  queue->EndBatch();
}

void CreateTeeter(Space* space, Vec3Param pos, Vec3Param size, uint levels, uint width)
{
  // Start a batched operation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("CreateTeeter");

  Cog* root = CreateFromArchetype(queue, space, CoreArchetypes::Transform, Vec3::cZero);
  root->SetName("TeeterRoot");
  root->ClearArchetype();

  Quat orientation = Quat::cIdentity;

  // Create base
  Vec3 halfSize = size * 0.5f;
  Cog* cog = CreateFromArchetype(
      queue, space, CoreArchetypes::Cube, Vec3(0, halfSize.y, 0) + pos, orientation, Vec3(size.x, size.y, size.x));
  cog->ClearArchetype();
  cog->AttachTo(root);

  cog = CreateFromArchetype(queue,
                            space,
                            CoreArchetypes::Cube,
                            Vec3(0, size.y + halfSize.x, 0) + pos,
                            orientation,
                            Vec3(size.y * 2, size.x, size.x));
  cog->ClearArchetype();
  cog->AttachTo(root);

  float newGround = size.y + size.x;

  for (uint i = 0; i < levels; ++i)
  {
    cog = CreateFromArchetype(queue,
                              space,
                              CoreArchetypes::Cube,
                              Vec3(-size.y + halfSize.z, newGround + halfSize.z + size.z * float(i), 0) + pos,
                              orientation,
                              Vec3(size.z, size.z, size.z));
    cog->ClearArchetype();
    cog->AttachTo(root);
  }

  float big = size.z * float(levels);

  cog = CreateFromArchetype(queue,
                            space,
                            CoreArchetypes::Cube,
                            Vec3(size.y + -halfSize.z, newGround + big * 0.5f, 0) + pos,
                            orientation,
                            Vec3(size.z, big, size.z));
  cog->ClearArchetype();
  cog->AttachTo(root);

  queue->EndBatch();
}

void CreateTower(Space* space, Vec3Param pos, Vec3Param size, uint levels, uint width)
{
  // Start a batched operation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("CreateTower");

  Cog* root = CreateFromArchetype(queue, space, CoreArchetypes::Transform, Vec3::cZero);
  root->SetName("TowerRoot");
  root->ClearArchetype();

  Quat orientation = Quat::cIdentity;

  for (uint height = 0; height < levels; ++height)
  {
    float y = (float)height * size.x + 0.5f * size.x;
    for (uint w = 0; w < width; ++w)
    {
      float x = (float)w * size.x;
      for (uint depth = 0; depth < width; ++depth)
      {
        float z = (float)depth * size.x;
        Cog* cog = CreateFromArchetype(
            queue, space, CoreArchetypes::Cube, Vec3(x, y, z) + pos, orientation, Vec3(size.x, size.x, size.x));
        cog->ClearArchetype();
        cog->AttachTo(root);
      }
    }
  }

  queue->EndBatch();
}

void CreateWall(Space* space, Vec3Param pos, Vec3Param size, uint height, uint width)
{
  // Start a batched operation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("CreateWall");

  Cog* root = CreateFromArchetype(queue, space, CoreArchetypes::Transform, Vec3::cZero);
  root->SetName("WallRoot");
  root->ClearArchetype();

  Quat orientation = Quat::cIdentity;

  Vec3 halfScale = size / 2.0f;
  Vec3 boxSize = Vec3(1.0f, 1.0f, 2.0f);

  for (uint h = 0; h < height; ++h)
  {
    float y = boxSize.y * ((float)h * size.y + halfScale.y);

    for (uint w = 0; w < width; ++w)
    {
      float x = (w - width / 2.0f) * 1.05f * size.x * boxSize.x + boxSize.x * halfScale.x * (h % 2);
      Cog* cog = CreateFromArchetype(queue, space, CoreArchetypes::Cube, Vec3(x, y, 0) + pos, orientation, size);
      cog->ClearArchetype();
      cog->AttachTo(root);
    }
  }

  queue->EndBatch();
}

void CreateBlockTower(Space* space, Vec3Param pos, Vec3Param size, uint height, uint width)
{
  // Start a batched operation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("CreateBlockTower");

  Cog* root = CreateFromArchetype(queue, space, CoreArchetypes::Transform, Vec3::cZero);
  root->SetName("BlockTowerRoot");
  root->ClearArchetype();

  Quat orientation = Quat::cIdentity;

  Vec3 scale;
  scale[0] = size.x / width;
  scale[1] = size.x / width;
  scale[2] = size.x; // / width;
  Vec3 halfScale = scale * 0.5f;

  for (uint h = 0; h < height; ++h)
  {
    float y = ((float)h * scale[1] + halfScale[1]);

    if (h % 2)
    {
      orientation.Set(real(0.0), real(0.0), real(0.0), real(1.0));
    }
    else
    {
      Math::ToQuaternion(Vec3(0, 1, 0), Math::cPi * 0.5f, &orientation);
    }

    float xStart = pos[0] - width * 1.0f * halfScale[0] + halfScale[0] * 1.0f;
    for (uint w = 0; w < width; ++w)
    {
      float x = xStart + w * scale[0] * 1.0f;

      if (h % 2)
      {
        Cog* cog = CreateFromArchetype(queue, space, CoreArchetypes::Cube, Vec3(x, y, 0), orientation, scale);
        cog->ClearArchetype();
        cog->AttachTo(root);
      }
      else
      {
        Cog* cog = CreateFromArchetype(queue, space, CoreArchetypes::Cube, Vec3(0, y, x), orientation, scale);
        cog->ClearArchetype();
        cog->AttachTo(root);
      }
    }
  }
  queue->EndBatch();
}

void BindCreationCommands(Cog* configCog, CommandManager* commands)
{
  // Creation commands
  commands->AddCommand("CreateTransform", new EditorCreateObjectCommand(CoreArchetypes::Transform));
  commands->AddCommand("CreateCamera", new EditorCreateObjectCommand(CoreArchetypes::Camera));
  commands->AddCommand("CreateDirectionalLight", new EditorCreateObjectCommand(CoreArchetypes::DirectionalLight));
  commands->AddCommand("CreateDirectionalLightShadows",
                       new EditorCreateObjectCommand(CoreArchetypes::DirectionalLightShadows));
  commands->AddCommand("CreatePointLight", new EditorCreateObjectCommand(CoreArchetypes::PointLight));
  commands->AddCommand("CreateSpotLight", new EditorCreateObjectCommand(CoreArchetypes::SpotLight));
  commands->AddCommand("CreateSpotLightShadows", new EditorCreateObjectCommand(CoreArchetypes::SpotLightShadows));

  commands->AddCommand("CreateCube", new EditorCreateObjectCommand(CoreArchetypes::Cube));
  commands->AddCommand("CreateSphere", new EditorCreateObjectCommand(CoreArchetypes::Sphere));
  commands->AddCommand("CreateCylinder", new EditorCreateObjectCommand(CoreArchetypes::Cylinder));
  commands->AddCommand("CreateWedge", new EditorCreateObjectCommand(CoreArchetypes::Wedge));
  commands->AddCommand("CreateSprite", new EditorCreateObjectCommand(CoreArchetypes::Sprite));
  commands->AddCommand("CreateSpriteText", new EditorCreateObjectCommand(CoreArchetypes::SpriteText));
  commands->AddCommand("CreateSpriteParticles", new EditorCreateObjectCommand(CoreArchetypes::SpriteParticles));
  commands->AddCommand("CreateSpline", new EditorCreateObjectCommand(CoreArchetypes::Spline));
  commands->AddCommand("CreateSplineParticleSystem",
                       new EditorCreateObjectCommand(CoreArchetypes::SplineParticleSystem));

  commands->AddCommand("CreateGrid", new EditorCreateObjectCommand(CoreArchetypes::Grid));

  bool devConfig = Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig) != nullptr;
  if (devConfig)
  {
    commands->AddCommand("CreatePyramid", new StackCreateCommand(CreatePyramid, Vec3(1, 1, 1), 7, 0));
    commands->AddCommand("CreateTower", new StackCreateCommand(CreateTower, Vec3(1, 1, 1), 10, 1));
    commands->AddCommand("CreateWall", new StackCreateCommand(CreateWall, Vec3(2, 0.5f, 1), 12, 6));
    commands->AddCommand("CreateBlockTower", new StackCreateCommand(CreateBlockTower, Vec3(3, 3, 3), 18, 3));
    commands->AddCommand("CreateTeeter", new StackCreateCommand(CreateTeeter, Vec3(1, 4, 1), 4, 3));
  }
}

} // namespace Zero
