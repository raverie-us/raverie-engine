// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

class FpsSampler : public DataSampler
{
  void Setup(RangeData& data, EntryLabel& label) override
  {
    label.Name = "Fps";
    data.AutoNormalized = true;
    data.MinValue = 0;
    data.MaxValue = 100.0f;
  }

  float Sample() override
  {
    using namespace Profile;
    Array<Record*>::range records = ProfileSystem::Instance->GetRecords();
    Record* record = records.Front();
    record->Update();
    float timeInS = ProfileSystem::Instance->GetTimeInSeconds((ProfileTime)record->Average());

    float fps = 0.0f;
    if (timeInS != 0.0f)
      fps = 1.0f / timeInS;
    // Return frame rate zero to one
    return fps;
  }
};

// Oscilloscope sampler
class OscilloscopeSampler : public DataSampler
{
  typedef OscilloscopeSampler ZilchSelf;

  void Setup(RangeData& data, EntryLabel& label) override
  {
    label.Name = "Input";
    data.AutoNormalized = true;
    data.MinValue = -1.0f;
    data.MaxValue = 1.0f;

    mIndex = 0;

    // ConnectThisTo(&mInput, Events::SoundInputArrived, OnSoundInput);
  }

  float Sample() override
  {
    if (mIndex < mLastData.Size())
    {
      float value = mLastData[mIndex];
      mIndex += 680;
      return value;
    }
    else
    {
      return -1.0f;
    }
  }

  size_t mIndex;
  Array<float> mLastData;
};

class MemorySampler : public DataSampler
{
public:
  void Setup(RangeData& data, EntryLabel& label) override
  {
    label.Name = "Memory MB";
    data.AutoNormalized = true;
    data.MinValue = 0;
    data.MaxValue = 10.0f;
  }

  float Sample() override
  {
    Memory::Graph* memoryNode = Memory::GetRoot();
    Memory::Stats stat;
    memoryNode->Compute(stat);
    float localKB = stat.BytesAllocated / 1024.0f;
    return localKB / 1024.0f;
  }
};

class ObjectSampler : public DataSampler
{
  void Setup(RangeData& data, EntryLabel& label) override
  {
    label.Name = "Objects";
    data.AutoNormalized = true;
  }

  float Sample() override
  {
    return (float)Z::gTracker->GetObjectCount();
  }
};

class CosSampler : public DataSampler
{
public:
  void Setup(RangeData& data, EntryLabel& label) override
  {
    label.Name = "Cos";
  }

  float p;
  CosSampler()
  {
    p = 0.0f;
  }
  float Sample() override
  {
    p += 0.1f;
    return Math::Cos(p) * 0.5f + 0.5f;
  }
};

void AddMemory(Editor* editor)
{
  MemoryGraphWidget* graphWidget = new MemoryGraphWidget(editor);
  graphWidget->SetName("Memory");
  graphWidget->SetSize(Pixels(400, 100));
  editor->AddManagedWidget(graphWidget, DockArea::Bottom, true);
}

void AddPerformance(Editor* editor)
{
  if (!editor->mManager->FindWidget("Performance"))
  {
    PerformanceGraphWidget* graphWidget = new PerformanceGraphWidget(editor);
    graphWidget->SetName("Performance");
    graphWidget->SetSize(Pixels(400, 200));
    editor->AddManagedWidget(graphWidget, DockArea::Bottom, true);
  }
}

void AddGraph(Editor* editor)
{
  GraphView* graph = new GraphView(editor);

  graph->SetName("Graph");
  graph->SetSize(Pixels(280, 400));
  graph->AddSampler(new FpsSampler());
  graph->AddSampler(new MemorySampler());
  graph->AddSampler(new ObjectSampler());
  editor->AddManagedWidget(graph, DockArea::Floating, true);
}

void BeginTracing(Editor* editor)
{
  Profile::ProfileSystem::Instance->BeginTracing();
}

void EndTracing(Editor* editor)
{
  Array<Profile::TraceEvent> traceEvents;
  Profile::ProfileSystem::Instance->EndTracing(traceEvents);

  static const String cCategory("cat");
  static const String cProcessId("pid");
  static const String cThreadId("tid");
  static const String cTimeStamp("ts");
  static const String cDuration("dur");
  static const String cName("name");
  static const String cArgs("args");
  static const String cPhase("ph");
  static const String cComplete("X");
  static const String cValue("value");

  Zilch::JsonBuilder builder;
  builder.IsCompactMode = true;
  builder.Begin(Zilch::JsonType::ArraySingleLine);
  forRange (Profile::TraceEvent& traceEvent, traceEvents)
  {
    builder.Begin(Zilch::JsonType::Object);
    builder.Key(cPhase);
    builder.Value(cComplete);

    builder.Key(cCategory);
    builder.Value(traceEvent.mCategory);
    builder.Key(cProcessId);
    builder.Value(0);
    builder.Key(cThreadId);
    builder.Value(traceEvent.mThreadId);
    builder.Key(cTimeStamp);
    builder.Value(traceEvent.mTimestamp);
    builder.Key(cDuration);
    builder.Value(traceEvent.mDuration);
    builder.Key(cName);
    builder.Value(traceEvent.mName);

    if (!traceEvent.mArgs.Empty())
    {
      builder.Key(cArgs);
      builder.Begin(Zilch::JsonType::Object);
      builder.Key(cValue);
      builder.Value(traceEvent.mArgs);
      builder.End();
    }
    builder.End();
  }
  builder.End();

  String json = builder.ToString();

  Archive archive(ArchiveMode::Compressing, CompressionLevel::MaxCompression);
  archive.AddFileBlock("trace.json", DataBlock((byte*)json.Data(), json.SizeInBytes()));
  ByteBufferBlock block(archive.ComputeZipSize());
  archive.WriteBuffer(block);

  String compressedData((cstr)block.GetBegin(), block.Size());

  ProjectSettings* project = Z::gEditor->mProject.has(ProjectSettings);
  String defaultFileName = BuildString(project->ProjectName, "-", GetTimeAndDateStamp(), ".zip");
  new SimpleSaveFileDialog(compressedData, "Save a trace", "Trace Zip File", "*.zip", "zip", defaultFileName);
}

void SetupGraphCommands(Cog* configCog, CommandManager* commands)
{
  commands->AddCommand("Performance", BindCommandFunction(AddPerformance), true);
  commands->AddCommand("Graph", BindCommandFunction(AddGraph), true);
  commands->AddCommand("BeginTracing", BindCommandFunction(BeginTracing), true);
  commands->AddCommand("EndTracing", BindCommandFunction(EndTracing), true);
}

} // namespace Zero
