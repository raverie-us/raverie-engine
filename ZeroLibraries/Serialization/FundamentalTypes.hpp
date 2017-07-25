///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

//This file is used to generate code that serializes the fundamental types.
//Fundamental types are serialized directly using the virtual serializer interface.
//All other types are serialized using templated serialization policies. Which 
//recursively break them down into fundamental types.
FUNDAMENTAL(bool);
FUNDAMENTAL(int);
FUNDAMENTAL(unsigned int);
FUNDAMENTAL(float);
FUNDAMENTAL(double);
FUNDAMENTAL(u64);
FUNDAMENTAL(s64);
FUNDAMENTAL(Guid);