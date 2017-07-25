///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef LockFreeQueue_H
#define LockFreeQueue_H

namespace Audio
{
  void* AtomicCompareExchangePointer(void** destination, void* exchange, void* comperand);
  void AtomicSetPointer(void** target, void* value);
  bool AtomicCheckEqualityPointer(void* first, void* second);

  //-------------------------------------------------------------------------------- Lock Free Queue

  template <typename T>
  class LockFreeQueue
  {
  private:
    struct Node
    {
      Node(T value) : Value(value), Next(nullptr) {}

      T Value;
      Node* Next;
    };

  public:
    // Create the dummy node
    LockFreeQueue() { First = Last = new Node(T()); }

    ~LockFreeQueue()
    {
      // Delete anything remaining on the queue
      while (First)
      {
        Node* temp = First;
        First = temp->Next;
        delete temp;
      }
    }

    void Write(const T& object)
    {
      // Create the new object
      Last->Next = new Node(object);
      // Set the Last pointer
      AtomicSetPointer((void**)&Last, Last->Next);
    }

    bool Read(T& result)
    {
      // Check if there is anything on the queue
      if (!AtomicCheckEqualityPointer(First, Last))
      {
        // Store the pointers
        Node* firstNode = First;
        Node* nextNode = First->Next;
        // Get the value from the next node
        result = nextNode->Value;
        // Move the First pointer forward
        First = First->Next;
        // Delete the old First node
        delete firstNode;

        return true;
      }

      return false;
    }

  private:
    // Only touched by reader
    Node* First;
    // Shared by writer and reader
    Node* Last;
  };

}

#endif