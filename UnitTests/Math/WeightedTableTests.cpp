///////////////////////////////////////////////////////////////////////////////
///
/// \file WeightedTableTests.cpp
/// Unit tests for WeightedProbabilityTable.
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Common/CommonStandard.hpp"
#include "Math/WeightedProbabilityTable.hpp"
#include "Math/Random.hpp"
#include "Containers/HashMap.hpp"

//make sure that the probability that has been split into multiple buckets for
//aliasing is correct. That is make sure that when adding all of the
//probabilities together for each item we get the original probability of the item.
template <typename TableType>
void VerifyTotalProbability(CppUnitLite::TestResult& result_,const char * m_name, TableType& table, float epsilon = .01f)
{
  uint size = table.mItems.Size();
  Zero::Array<TableType::WeightType> computedWeights;
  computedWeights.Resize(size,0);

  //each item in the table has 2 probabilities: the owner of the table
  //and the alias in the able. These add up to 1.
  for(uint i = 0; i < size; ++i)
  {
    //add to our table of weights the owner's probability
    //and the remaining portion to the aliased item
    float weight = table.mItems[i].mWeightedProbability;
    computedWeights[i] += weight;
    computedWeights[table.mItems[i].mAlias] += 1.0f - weight;
  }

  //check the summed probabilities to make sure it is equal
  //to the item's total probability (have to rescale by the
  //total probability and size since they were re-normalized)
  for(uint i = 0; i < size; ++i)
  {
    float weightProb = size * table.mItems[i].mProbability / table.mTotalProbability;
    CHECK_CLOSE(weightProb,computedWeights[i],epsilon);
  }
}

template <typename TableType>
void VerifyDistributionByIndex(CppUnitLite::TestResult& result_,const char * m_name, TableType& table, uint samples = 1000000, float epsilon = .01f)
{
  Math::Random random;

  uint size = table.mItems.Size();
  Zero::Array<uint> computedWeights;
  computedWeights.Resize(size,0);

  //sample n times and count how many times each item was hit
  for(uint i = 0; i < samples; ++i)
  {
    uint index = table.SampleIndex(random);
    ++computedWeights[index];
  }

  //verify that the item was hit within a reasonably
  //probability of what it should have been
  for(uint i = 0; i < size; ++i)
  {
    float count = (float)computedWeights[i];
    float actualProb = count / samples;
    float expectedProb = table.mItems[i].mProbability / table.mTotalProbability;

    CHECK_CLOSE(actualProb,expectedProb,epsilon);
  }
}

template <typename TableType>
void VerifyDistribution(CppUnitLite::TestResult& result_,const char * m_name, TableType& table, uint samples = 1000000, float epsilon = .01f)
{
  Math::Random random;

  uint size = table.mItems.Size();
  Zero::HashMap<TableType::ValueType,uint> computedWeights;

  //sample n times and count how many times each item was hit
  for(uint i = 0; i < samples; ++i)
  {
    TableType::ValueType value = table.Sample(random);
    if(computedWeights.Find(value).Empty())
      computedWeights[value] = 0;

    ++computedWeights[value];
  }

  //verify that the item was hit within a reasonably
  //probability of what it should have been
  for(uint i = 0; i < size; ++i)
  {
    float count = (float)computedWeights[table.mItems[i].mValue];
    float actualProb = count / samples;
    float expectedProb = table.mItems[i].mProbability / table.mTotalProbability;

    CHECK_CLOSE(actualProb,expectedProb,epsilon);
  }
}

TEST(WeightedTableTest1)
{
  //build the table from the example
  Math::WeightedProbabilityTable<uint> table;
  table.AddItem(1,1.0f/2.0f);
  table.AddItem(2,1.0f/3.0f);
  table.AddItem(3,1.0f/12.0f);
  table.AddItem(4,1.0f/12.0f);
  table.BuildTable();

  VerifyTotalProbability(result_,m_name,table);
  VerifyDistributionByIndex(result_,m_name,table);
  VerifyDistribution(result_,m_name,table);
}

TEST(WeightedTableTest2)
{
  float allowedMax = 2.0f;
  float allowedMin = .01f;

  Math::Random random;

  Math::WeightedProbabilityTable<uint> table;
  uint size = random.IntRangeInEx(1,1000);
  //randomly add items to the table
  for(uint i = 0; i < size; ++i)
  {
    float val = random.FloatRange(allowedMin,allowedMax);
    table.AddItem(i,val);
  }
  //build it
  table.BuildTable();

  //make sure the probability with the aliases add up to the real probability
  VerifyTotalProbability(result_,m_name,table);

  //now sample the table many times and make sure we get the expected probability
  uint iterations = 1 << 20;
  float epsilon = .001f;
  VerifyDistributionByIndex(result_,m_name,table, iterations, epsilon);
  VerifyDistribution(result_,m_name,table, iterations, epsilon);
}

TEST(WeightedTableTest3)
{
  //testing with large allowed discrepancies between bucket sizes.
  float allowedMax = 50.0f;
  float allowedMin = .01f;

  Math::Random random;

  Math::WeightedProbabilityTable<uint> table;
  uint size = random.IntRangeInEx(1,1000);
  //randomly add items to the table
  for(uint i = 0; i < size; ++i)
  {
    float val = random.FloatRange(allowedMin,allowedMax);
    table.AddItem(i,val);
  }
  //build it
  table.BuildTable();

  //make sure the probability with the aliases add up to the real probability
  VerifyTotalProbability(result_,m_name,table);

  //now sample the table many times and make sure we get the expected probability
  uint iterations = 1 << 20;
  float epsilon = .001f;
  VerifyDistributionByIndex(result_,m_name,table, iterations, epsilon);
  VerifyDistribution(result_,m_name,table, iterations, epsilon);
}

TEST(WeightedTableTest_TestSize)
{
  //make sure size of 0 is right
  Math::WeightedProbabilityTable<uint> table;
  CHECK_EQUAL(0,table.Size());

  //just check resizing a few times
  float prob = 1.0;
  for(uint i = 0; i < 3; ++i)
    table.AddItem(i,prob);
  table.BuildTable();
  CHECK_EQUAL(3,table.Size());
  
  for(uint i = 0; i < 40; ++i)
    table.AddItem(i,prob);
  table.BuildTable();
  CHECK_EQUAL(43,table.Size());
}

TEST(WeightedTableTest_TestClear)
{
  Math::WeightedProbabilityTable<uint> table;

  //add items to the table and build it
  float prob = 1.0;
  for(uint i = 0; i < 10; ++i)
    table.AddItem(i,prob);
  table.BuildTable();

  //clear the table and make sure that the size and total probability is right
  table.Clear();
  CHECK_EQUAL(0,table.Size());
  CHECK_EQUAL(0.0f,table.mTotalProbability);

  //make sure that adding new items and building works now
  table.AddItem(1,1.0f/2.0f);
  table.AddItem(2,1.0f/3.0f);
  table.AddItem(3,1.0f/12.0f);
  table.AddItem(4,1.0f/12.0f);
  table.BuildTable();

  VerifyTotalProbability(result_,m_name,table);
  VerifyDistributionByIndex(result_,m_name,table);
  VerifyDistribution(result_,m_name,table);
}

TEST(WeightedTableTest_RemoveItemAt)
{
  //build the table from the example
  Math::WeightedProbabilityTable<uint> table;
  table.AddItem(1,1.0f/2.0f);
  table.AddItem(2,1.0f/3.0f);
  table.AddItem(3,1.0f/12.0f);
  table.AddItem(4,1.0f/12.0f);
  table.BuildTable();

  table.RemoveItemAt(2);
  table.BuildTable();

  VerifyTotalProbability(result_,m_name,table);
  VerifyDistributionByIndex(result_,m_name,table);
  VerifyDistribution(result_,m_name,table);
}

TEST(WeightedTableTest_BuildTwice1)
{
  //testing building the table once, adding more, then building the table again
  float allowedMax = 2.0f;
  float allowedMin = .01f;

  Math::Random random;

  Math::WeightedProbabilityTable<uint> table;
  uint size = random.IntRangeInEx(1,1000);

  //add n items of random values to the table and build it
  for(uint i = 0; i < size; ++i)
  {
    float val = random.FloatRange(allowedMin,allowedMax);
    table.AddItem(i,val);
  }
  table.BuildTable();
  //do that n times again
  for(uint i = 0; i < size; ++i)
  {
    float val = random.FloatRange(allowedMin,allowedMax);
    table.AddItem(i + size,val);
  }
  table.BuildTable();


  //make sure it is all right
  VerifyTotalProbability(result_,m_name,table);

  uint iterations = 1 << 20;
  float epsilon = .001f;
  VerifyDistributionByIndex(result_,m_name,table,iterations, epsilon);
  VerifyDistribution(result_,m_name,table,iterations, epsilon);
}