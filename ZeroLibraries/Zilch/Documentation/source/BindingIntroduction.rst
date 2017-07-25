Binding Introduction
====================

To get started, download the latest `Zilch library <../downloads#http://>`_. For convenience, all of the Zilch C++ code
is compacted into one header and one cpp.

.. warning::
  Zilch uses its own containers, including Array, HashMap, String, etc. For compatibility with STL, the containers support ``.begin()`` and ``.end()`` iterators, as well as ``.size()``. The String class supports ``.c_str()``.

Compiling C++
-------------

Include the Zilch.cpp into your compilation process, and start by having your main include the Zilch.hpp

.. literalinclude:: ../../Project/Documentation/CompilingCpp.inl
  :language: cpp

So long as Zilch.cpp is built and linked in, this program should fully compile.

Compiling a Zilch script
------------------------

.. literalinclude:: ../../Project/Documentation/CompilingZilchScript.inl
  :language: cpp

Running a Zilch script
----------------------

As a language meant to be called and run from C++, Zilch has no 'main entrypoint' of its own.
Instead it is up to the user to instantiate types defined within Zilch scripts and call
functions on them (or invoke static methods).

Create a Zilch script with code that we want to test out.

``Player.zilch``

.. literalinclude:: ../../Project/Documentation/Player.zilch
  :language: as

From the C++ side change ``AddCodeFromString`` to ``AddCodeFromFile``.

.. code-block:: cpp

  project.AddCodeFromFile("Player.zilch");

.. warning::
  Make sure to put the ``Player.zilch`` file side by side with the executable, or make
  sure that the working directory is setup correctly so that the executable can find the script file.

We now need to compile the code into a ``Library``, and finally link that library into an ``ExecutableState``.
A ``Library`` stores all the compiled types and functions. The ``ExecutableState``
has its own stack and virtual machine, and is responsible for executing Zilch code.

.. literalinclude:: ../../Project/Documentation/RunningZilchScript.inl
  :language: cpp

