Handles
=======

A handle is the internal primitive Zilch uses to represent all allocated classes within the language. A handle's usage is similar to that of a *pointer* or *smart_ptr* in C++. Handles have a 'Data' section which can store any data that the user wants (a HandleManager should know how to interpret this data). Handles may:

- Reference count upon copy constructing, assignment, and destruction
- Safely delete an object and null out all other references
- Directly point at objects in an unsafe manner (like classic C++ pointers)
- Change any of these behaviours based upon the HandleManager they store
- Store the type of the object they're pointing at (used for virtual behavior)

.. note::

  The technique we currently use for HeapManager this is by giving each allocated object a UniqueId (the handle also stores the UniqueId of the object it points at).

Handle Managers
---------------
The HandleManager is an interface that can be implemented to provide custom behavior for handles (such as for custom reference counting)

There are a few built-in handle managers:

- *HeapManager*:

  - One exists per ExecutableState
  - Reference counted
  - Allows deleting
  - Safely nulls out all other handles to the same object
  - Used any time we use the ``new`` keyword in Zilch (unless the type is a C++ type that overrides the HandleManager)
  - Or ``state->AllocateHeapObject``
  - Or ``state->AllocateDefaultConstructedHeapObject``

- *StackManager*:

  - One exists per ExecutableState (because each state has its own stack)
  - Does not allow deleting (can't delete stack memory)
  - Safely nulls out all other handles to the same object when the stack returns
  - Used any time a struct is created on the stack (``local`` keyword)

- *PointerManager*:

  - Global manager (does not require an ExecutableState)
  - Does not allow deleting (may be allowed in the future)
  - Does not null out other handles to the same object (unsafe)
  - Typically used when a C++ object is passed into Zilch by pointer

- *StringManager*:

  - Global manager (does not require an ExecutableState)
  - Strings are internally reference counted and are C++ objects
  - Strings are immutable and not deletable, therefore there is no need to null out instances

Constructing a Handle
---------------------

It's not often that a handle needs to be constructed manually, however the situation may arise

.. code-block:: cpp

  Handle handle;
  handle.Type = YourBoundType;
  handle.Manager = state->GetHandleManager<ManagerType>();
  handle.Manager->ObjectToHandle(valueMemory, someHandle);

