/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_SYNTAX_TREE_HELPERS_HPP
#define ZILCH_SYNTAX_TREE_HELPERS_HPP

namespace Zilch
{
  // This list is used to hold nodes in the tree
  template <typename ValueType>
  class ZeroSharedTemplate DoublePointerArray : public PodArray<ValueType**>
  {
  public:
    // Type-defines
    typedef PodArray<ValueType**> base;
    
    // Add a node, as long as the value is not null
    template <typename T>
    bool Add(T*& value) // where T : SyntaxNode*
    {
      // Check the value to see if it's null
      if (value != nullptr)
      {
        // It's not, so push it on and return success
        base::PushBack((ValueType**)&value);
        return true;
      }

      // We failed to add the node since it was null
      return false;
    }

  private:
    // Don't allow direct pushing back
    void PushBack(const ValueType**& item);
    ValueType**& PushBack();
  };


  // This list is used to hold nodes in the tree
  template <typename ValueType>
  class ZeroSharedTemplate PopulatingPointerArray : public PodBlockArray<ValueType*>
  {
  public:
    // Type-defines
    typedef PodBlockArray<ValueType*> base;
    
    // Add a node, as long as the value is not null
    ValueType* Add(ValueType* value)
    {
      // Check the value to see if it's null
      if (value != nullptr)
      {
        // It's not, so push it on and return success
        base::PushBack(value);
      }

      // Return whatever was added, so it can be checked if it was valid
      return value;
    }

    // Populates an external list with pointers to each syntax node
    template <typename T>
    void Populate(DoublePointerArray<T>& childrenOut)
    {
      // Reserve space for performance
      childrenOut.Reserve(childrenOut.Size() + this->Size());

      // Loop throuhg all the nodes
      for (size_t i = 0; i < this->Size(); ++i)
      {
        childrenOut.Add((*this)[i]);
      }
    }

  private:
    // Don't allow direct pushing back
    void PushBack(const ValueType*& item);
    ValueType*& PushBack();
  };

  // This list is used to hold nodes of any type in the tree
  template <typename T>
  class ZeroSharedTemplate NodeList : public PopulatingPointerArray<T>
  {
  };

  // Type-defines
  typedef DoublePointerArray<SyntaxNode> NodeChildren;
  typedef DoublePointerArray<SyntaxType> SyntaxTypes;
  typedef NodeList<SyntaxType> SyntaxTypeList;

  template <typename TreeOwnerType, typename ContextType>
  class BranchWalker;
  
  namespace WalkerFlags
  {
    enum Enum
    {
      None = 0,
      ChildrenNotHandled = 1,
      PreventOtherWalkers = 2,
      Error = 4
    };
    typedef size_t Type;
  }

  template <typename TreeOwnerType, typename ContextType>
  class ZeroSharedTemplate WalkerContext
  {
  public:
    // Store a pointer back to the walker
    BranchWalker<TreeOwnerType, ContextType>* Walker;

    // These flags get reset with every walk
    WalkerFlags::Type Flags;
  };

  template <typename TreeOwnerType, typename ContextType>
  class ZeroSharedTemplate BranchWalker
  {
  public:

    // Constructor
    BranchWalker(CompilationErrors* errors = nullptr) :
      WasError(false),
      Errors(errors)
    {
    }

    // The member function type that we'd like to be able to traverse our trees
    typedef void (TreeOwnerType::*MemberFn)(SyntaxNode*& node, ContextType* context);

    // Register a visitor, and the condition is implied to be 
    template <typename ChildType>
    void Register(void (TreeOwnerType::*visitor)(ChildType*& node, ContextType* context))
    {
      RegisterInternal((MemberFn) visitor, ZilchTypeId(ChildType), false);
    }

    // Register a visitor, for a more derived child type
    template <typename DerivedChildType, typename ChildType>
    void RegisterDerived(void (TreeOwnerType::*visitor)(ChildType*& node, ContextType* context))
    {
      RegisterInternal((MemberFn) visitor, ZilchTypeId(DerivedChildType), false);
    }

    // Register a visitor that will visit any node of this base type, even if it derives from it
    template <typename ChildType>
    void RegisterNonLeafBase(void (TreeOwnerType::*visitor)(ChildType*& node, ContextType* context))
    {
      RegisterInternal((MemberFn) visitor, ZilchTypeId(ChildType), true);
    }

    template <typename NodeType>
    void GenericWalkChildren(TreeOwnerType* owner, NodeType*& node, ContextType* context)
    {
      // Now we need to go through every child that wasn't visited
      NodeChildren children;
      node->PopulateChildren(children);

      // Loop through all of the children that matched that type (including derived types)
      for (size_t i = 0; i < children.Size(); ++i)
      {
        // Get the current child
        SyntaxNode*& child = *children[i];

        // Walk down to the children
        Walk(owner, child, context);

        // If the error handler is available and an error ocurred...
        if (this->Errors != nullptr && this->Errors->WasError)
          return;
      }
    }

    // Using the registered visitors, visit all of the direct child nodes
    template <typename NodeType>
    void Walk(TreeOwnerType* owner, NodeList<NodeType>& nodes, ContextType* context)
    {
      // Loop through all the nodes in the list
      for (size_t i = 0; i < nodes.Size(); ++i)
      {
        // Get the current node
        NodeType*& node = nodes[i];

        // Walk this specific node
        this->Walk(owner, node, context);
      }
    }

    // Checks to see if any errors have occurred
    bool HasErrorOccurred()
    {
      return this->WasError || (this->Errors != nullptr && this->Errors->WasError);
    }

    // Using the registered visitors, visit all of the direct child nodes
    template <typename NodeType>
    void Walk(TreeOwnerType* owner, NodeType*& node, ContextType* context)
    {
      // Early out if an error occurred
      if (this->HasErrorOccurred())
        return;

      // Set the walker on the context
      context->Walker = this;

      // Error checking
      ErrorIf(node == nullptr, "You should never attempt to traverse a null node");

      // Get the node type
      BoundType* nodeType = ZilchVirtualTypeId(node);

      // Were the children walked over (or explicitly ignored)?
      bool childrenWereHandled = false;

      // Loop through all the visitors
      for (size_t i = 0; i < this->Visitors.Size(); ++i)
      {
        // Get the current visitor
        VisitorInfo& visitor = this->Visitors[i];

        // As long as the node we're visiting is somehow derived from the node visitor type
        if (nodeType == visitor.NodeType || (visitor.IsNonLeafBase && Type::BoundIsA(nodeType, visitor.NodeType)))
        {
          // Clear any flags before visiting this node
          context->Flags = WalkerFlags::None;

          // Invoke the visitor on that child
          (owner->*(visitor.Visitor))((SyntaxNode*&)node, context);

          // Store and reset the flags again
          WalkerFlags::Type flags = context->Flags;
          context->Flags = WalkerFlags::None;

          // If any kind of error occurred, early out
          if (flags & WalkerFlags::Error || this->HasErrorOccurred())
          {
            // Set an error flag so we won't do any more visiting
            this->WasError = true;
            return;
          }

          // If the children were walked by this visitor, then mark it so
          // This just means we will not generically walk the tree later
          if ((flags & WalkerFlags::ChildrenNotHandled) == 0)
            childrenWereHandled = true;

          // If we don't want anyone else to visit this node after us...
          // Note: We do not 'return' because we may still want to generically walk it's children
          if (flags & WalkerFlags::PreventOtherWalkers)
            break;

          // If the node was ever cleared...
          if (node == nullptr)
            return;

          // If the error handler is available and an error ocurred...
          if (this->Errors != nullptr && this->Errors->WasError)
            return;
        }
      }

      // Check if the children were not visited...
      if (childrenWereHandled == false)
      {
        // Generically walk all the children since nobody visited this poor old node
        this->GenericWalkChildren(owner, node, context);
      }
    }

  private:

    // Register a visitor
    void RegisterInternal(MemberFn visitor, BoundType* childTypeToVisit, bool isNonLeafBase)
    {
      VisitorInfo info;
      info.Visitor = visitor;
      info.NodeType = childTypeToVisit;
      info.IsNonLeafBase = isNonLeafBase;

      // Add the node to the children
      this->Visitors.PushBack(info);
    }

  public:

    // If an error occurred, this will be set
    // This must be cleared before reusing a walker
    bool WasError;

    // Store a reference to the error handler (may be null)
    CompilationErrors* Errors;

  private:

    // Information about the visitors
    class VisitorInfo
    {
    public:
      // Constructor
      VisitorInfo() :
        Visitor(nullptr),
        NodeType(nullptr),
        IsNonLeafBase(false)
      {
      }

      MemberFn Visitor;
      BoundType* NodeType;
      bool IsNonLeafBase;
    };

    // The visitors that have been registered
    typedef Array<VisitorInfo> VisitorArray;

    // Store all the visitors
    VisitorArray Visitors;

    // Not copyable
    ZilchNoCopy(BranchWalker);
  };
}

#endif