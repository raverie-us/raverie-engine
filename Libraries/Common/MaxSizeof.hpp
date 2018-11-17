#pragma once

namespace Zero
{

template <uint a, uint b>
class ConstMax
{
public:
  static const uint Result = (a > b) ? a : b;
};

#define MaxSizeof2(e1,e2) ConstMax< sizeof(e1),sizeof(e2) >::Result
#define MaxSizeof3(e1,e2,e3) ConstMax< sizeof(e1),MaxSizeof2(e2,e3) >::Result
#define MaxSizeof4(e1,e2,e3,e4) ConstMax< sizeof(e1),MaxSizeof3(e2,e3,e4) >::Result
#define MaxSizeof5(e1,e2,e3,e4,e5) ConstMax< sizeof(e1),MaxSizeof4(e2,e3,e4,e5) >::Result
#define MaxSizeof6(e1,e2,e3,e4,e5,e6) ConstMax< sizeof(e1),MaxSizeof5(e2,e3,e4,e5,e6) >::Result
#define MaxSizeof7(e1,e2,e3,e4,e5,e6,e7) ConstMax< sizeof(e1),MaxSizeof6(e2,e3,e4,e5,e6,e7) >::Result
#define MaxSizeof8(e1,e2,e3,e4,e5,e6,e7,e8) ConstMax< sizeof(e1),MaxSizeof7(e2,e3,e4,e5,e6,e7,e8) >::Result
#define MaxSizeof9(e1,e2,e3,e4,e5,e6,e7,e8,e9) ConstMax< sizeof(e1),MaxSizeof8(e2,e3,e4,e5,e6,e7,e8,e9) >::Result
#define MaxSizeof10(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10) ConstMax< sizeof(e1),MaxSizeof9(e2,e3,e4,e5,e6,e7,e8,e9,e10) >::Result
#define MaxSizeof11(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11) ConstMax< sizeof(e1),MaxSizeof10(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11) >::Result
#define MaxSizeof12(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12) ConstMax< sizeof(e1),MaxSizeof11(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12) >::Result
#define MaxSizeof13(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13) ConstMax< sizeof(e1),MaxSizeof12(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13) >::Result
#define MaxSizeof14(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14) ConstMax< sizeof(e1),MaxSizeof13(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14) >::Result
#define MaxSizeof15(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15) ConstMax< sizeof(e1),MaxSizeof14(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15) >::Result
#define MaxSizeof16(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16) ConstMax< sizeof(e1),MaxSizeof15(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16) >::Result
#define MaxSizeof17(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17) ConstMax< sizeof(e1),MaxSizeof16(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17) >::Result
#define MaxSizeof18(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18) ConstMax< sizeof(e1),MaxSizeof17(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18) >::Result
#define MaxSizeof19(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18,e19) ConstMax< sizeof(e1),MaxSizeof18(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18,e19) >::Result
#define MaxSizeof20(e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18,e19,e20) ConstMax< sizeof(e1),MaxSizeof19(e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16,e17,e18,e19,e20) >::Result

}
