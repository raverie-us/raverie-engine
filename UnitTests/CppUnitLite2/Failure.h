#pragma once
#include <iosfwd>

namespace CppUnitLite
{
class Failure
{
public:
	Failure(const char* condition, const char* testName, const char* fileName, 
          int lineNumber);

	const char* Condition() const;

private:
	friend std::ostream& operator<<(std::ostream& stream, const Failure& failure);

	const char* m_condition;
	const char* m_testName;
	const char* m_fileName;
	int m_lineNumber;
};
}
