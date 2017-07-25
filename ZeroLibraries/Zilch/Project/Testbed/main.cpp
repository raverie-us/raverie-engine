/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2015, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"
using namespace Zilch;
#include <vld.h>


class Animal
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  String mName;
};

class Horse : public Animal
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

ZilchDefineType(Animal, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindConstructor(const Animal&);
  ZilchBindField(mName);
}

ZilchDefineType(Horse, builder, type)
{
}

class Farm
{
public:
  Array<Animal*> mAnimals;

  ~Farm()
  {
    ZilchForEach(Animal* animal, mAnimals)
    {
      delete animal;
    }
  }

  ZilchDeclareType(TypeCopyMode::ReferenceType);
  Animal GetPig()
  {
    Animal pig;
    pig.mName = "Piggy";
    return pig;
  }

  void AddPig(StringParam name)
  {
    mAnimals.PushBack(new Animal());
    mAnimals.Back()->mName = name;
  }

  void AddPiggy(const HandleOf<Animal>& animal)
  {
    Animal* bob = animal;
    //mAnimals.PushBack(animal);
  }

  Array<Animal*>::range GetPigs()
  {
    return mAnimals.All();
  }
};

ZilchDeclareRange(Array<Animal*>::range);
ZilchDefineRange(Array<Animal*>::range);

ZilchDefineType(Farm, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchBindMethod(GetPig);
  ZilchBindMethod(AddPig);
  ZilchBindMethod(AddPiggy);
  ZilchBindMethod(GetPigs);

}



ZilchDeclareStaticLibrary(TestLibrary, ZeroNoImportExport);

ZilchDefineStaticLibrary(TestLibrary)
{
  ZilchInitializeType(Animal);
  ZilchInitializeType(Horse);
  ZilchInitializeType(Farm);
  ZilchInitializeRangeAs(Array<Animal*>::range, "AnimalRange");
}

void PrintName(Animal* animal)
{
  printf("%s\n", animal->mName.c_str());
}

#include "Platform/Process.hpp"
#include "ProcessClass.h"

//***************************************************************************
int main(int argc, char* argv[])
{
  ZilchSetup setup(SetupFlags::None);

  TestLibrary::GetInstance().BuildLibrary();



  Project project;
  EventConnect(&project, Events::CompilationError, DefaultErrorCallback);

  Module dependencies;
  dependencies.PushBack(TestLibrary::GetInstance().GetLibrary());
  //dependencies.push_back(Io::GetInstance().GetLibrary());

  //String html = dependencies.BuildDocumentationHtml();

  EventConnect(&Console::Events, Events::ConsoleWrite, DefaultWriteText);
  EventConnect(&Console::Events, Events::ConsoleRead, DefaultReadText);

  project.AddCodeFromFile("Test.z", nullptr);
  //project.AddCodeFromString("scope { Math.Sin(5); } ");

  LibraryRef lib = project.Compile("Test", dependencies, EvaluationMode::Project);

  if (lib)
  {
    dependencies.PushBack(lib);
    ExecutableState* state = dependencies.Link();
    state->SetTimeout(5);
    EventConnect(state, Events::UnhandledException, DefaultExceptionCallback);

    state->SetTimeout(5);
    if (state)
    {
      ExceptionReport report;
      BoundType* programType = state->Dependencies.FindType(ExpressionProgram);
      Handle program = state->AllocateDefaultConstructedHeapObject(programType, report, HeapFlags::ReferenceCounted);

      Function* function = programType->FindFunction(ExpressionMain, Array<Type*>(), ZilchTypeId(Any), FindMemberOptions::None);

      auto start = clock();
      Call call(function, state);
      call.Set(Call::This, program);
      call.Invoke(report);
      auto end = clock();
      auto val = call.Get<Any>(Call::Return);
      
      printf("Return Value: %s\n", val.ToString().c_str());
      printf("Time: %d\n", end - start);
      printf("Time Seconds: %g\n", (end - start) / (double)CLOCKS_PER_SEC);
    }
    delete state;
  }

  system("pause");
  return 0;
}
