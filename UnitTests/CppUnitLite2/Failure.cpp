#include "Failure.h"

#include <string>
#include <iostream>

namespace CppUnitLite
{
Failure::Failure(const char* condition, const char* testName, 
                 const char* fileName, int lineNumber) 
	: m_condition(condition), m_testName(testName), m_fileName(fileName), 
    m_lineNumber(lineNumber)
{
}

const char* Failure::Condition() const
{
	return m_condition;
}

std::ostream& operator<< (std::ostream& stream, const Failure& failure)
{
  if(failure.m_condition != NULL)
  {
	  stream << failure.m_fileName
           << "(" << failure.m_lineNumber << "): "
           << "Failure in " << failure.m_testName 
           << ": \"" << failure.m_condition << "\" " 
           << std::endl;
  }
  else
  {
    stream << "Unimplemented Test: \"" << failure.m_testName << "\"" 
           << std::endl
           << "                    "
           << failure.m_fileName
           << "(" << failure.m_lineNumber << ")"
           << std::endl << std::endl;
  }
	return stream;
}
}
