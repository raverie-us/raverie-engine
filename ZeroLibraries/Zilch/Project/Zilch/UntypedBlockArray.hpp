/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_UNTYPED_BLOCK_ARRAY_HPP
#define ZILCH_UNTYPED_BLOCK_ARRAY_HPP

namespace Zilch
{
  // A block array (except it is untyped, and therefore works mostly with copyable structs)
  template <size_t BlockSize>
  class ZeroSharedTemplate UntypedBlockArray
  {
  public:

    // Constructor
    UntypedBlockArray()
    {
      this->CompactedSize = 0;
    }

    // Copy constructor
    UntypedBlockArray(const UntypedBlockArray& from) :
      CompactedSize(from.CompactedSize)
    {
      // Loop through all the blocks allocated by the source
      for (size_t i = 0; i < from.Blocks.Size(); ++i)
      {
        // Grab the current source block
        const Block& fromBlock = from.Blocks[i];

        // Add the block to ourselves and copy all the information over
        Block& toBlock = this->CreateBlock();
        toBlock.LengthWritten = fromBlock.LengthWritten;
        memcpy(toBlock.Data, fromBlock.Data, BlockSize);
      }
    }

    // Destructor
    ~UntypedBlockArray()
    {
      // Clear out the block array
      Clear();
    }

    // Efficiently compact all the data into one output buffer
    // This will wipe out any empty space between the blocks
    void RelativeCompact(byte* output)
    {
      // Loop through all the blocks
      for (size_t i = 0; i < this->Blocks.Size(); ++i)
      {
        // Get the current block
        Block& block = this->Blocks[i];

        // Copy all the block's data to the output
        memcpy(output, block.Data, block.LengthWritten);

        // Move the pointer forward so we can copy the next block
        output += block.LengthWritten;
      }
    }

    // Gets an element in a block given an exact location
    byte* GetAbsoluteElement(size_t index)
    {
      // Determine the block index and offset into that block
      size_t blockIndex = index / BlockSize;
      size_t blockOffset = index % BlockSize;

      // Get the current block
      Block& block = this->Blocks[blockIndex];

      // Return a pointer to that location
      return block.Data + blockOffset;
    }

    // Get an element by index (note that this is not a constant time operation)
    byte* GetRelativeElement(size_t index)
    {
      // Note: This could potentially be optimized using a few key facts
      // We know that we will probably mostly fill up each block, so block sizes
      // very close to the LengthWritten for any given full block
      // Each block could then store all the previous size up to that point
      // We could use the index to guess a block (index / BlockSize) and we know
      // that the element is either in that block or in a previous one close by

      // More than likely, it's not a problem since our block array
      // will be so big that worst case scenario, we're probably 
      // indexing into the second or third block (near constant time)

      // Loop through all the blocks
      for (size_t i = 0; i < this->Blocks.Size(); ++i)
      {
        // Get the current block
        Block& block = this->Blocks[i];

        // If the index is within this block
        if (index < block.LengthWritten)
        {
          // Return the element
          return block.Data + index;
        }
        else
        {
          // Move the index back by the amount written to this block
          index -= block.LengthWritten;
        }
      }

      // We didn't find it?
      Error("Unable to find a given element by index");
      return nullptr;
    }
    
    // Get the total size
    size_t RelativeSize()
    {
      return this->CompactedSize;
    }
    
    // Get the total size
    // Note: Do NOT use this as an absolute index, as it's possible that the next element
    // that's added to the block array will get pushed into another block (wrong index...)
    size_t AbsoluteSize()
    {
      if (this->Blocks.Empty())
      {
        return 0;
      }
      else
      {
        return (this->Blocks.Size() - 1) * BlockSize + this->Blocks.Back().LengthWritten;
      }
    }

    // Clear all the of blocks out
    void Clear()
    {
      // Reset the size since we now store nothing
      this->CompactedSize = 0;

      // Loop through all the blocks
      for (size_t i = 0; i < this->Blocks.Size(); ++i)
      {
        // Delete the block memory
        delete[] this->Blocks[i].Data;
      }

      // Clear the blocks out too
      this->Blocks.Clear();
    }

    // Request a chunk of memory of a given size, and return a pointer to the beginning of it
    byte* RequestElementOfSize(size_t size, size_t* absoluteIndexOut = nullptr)
    {
      // Error checking and handling
      ReturnIf(size > BlockSize, nullptr, "The element that was being allocated was larger than the block size");
      
      // We know that we'll add that size below (we have to!)
      this->CompactedSize += size;

      // If we have no blocks...
      if (this->Blocks.Empty())
      {
        // Create our first block since we have none
        this->CreateBlock();
      }

      // Call the recursive version
      return this->RequestElementOfSizeRecursive(size, absoluteIndexOut);
    }

  private:

    // A block of opcodes
    class Block
    {
    public:
      // A pointer to the data that each block stores
      byte* Data;
      
      // The length that we've written into the data block
      // Note that the size of the memory pointed at by 'Data' is the BlockSize, however,
      // we may not have written all the way up to the end due to different opcode sizes
      size_t LengthWritten;
    };

    // Recursive version of requesting an element (we only recurse once)
    byte* RequestElementOfSizeRecursive(size_t size, size_t* absoluteIndexOut = nullptr)
    {
      // Get the last block (the only one we should be writing to)
      Block& last = this->Blocks.Back();

      // Check if the last block has enough space for this element
      if (BlockSize - last.LengthWritten > size)
      {
        // Get a pointer to the element data
        byte* element = last.Data + last.LengthWritten;

        // If the user wants an absolute index back...
        if (absoluteIndexOut != nullptr)
        {
          // Return the index where we wrote the data
          *absoluteIndexOut = (this->Blocks.Size() - 1) * BlockSize + last.LengthWritten;
        }

        // We've now 'written' more to the last block
        last.LengthWritten += size;

        // Return the element
        return element;
      }
      else
      {
        // Otherwise, we need to create a fresh new block
        this->CreateBlock();

        // Invoke the function again
        return this->RequestElementOfSizeRecursive(size, absoluteIndexOut);
      }
    }

    // Create another block at the end of the block array
    Block& CreateBlock()
    {
      // Create a new block
      Block& block = this->Blocks.PushBack();

      // Create the block of data
      block.Data = new byte[BlockSize];

      // We haven't written anything yet
      block.LengthWritten = 0;
      return block;
    }

  private:

    // Store an array of all the blocks that we use
    PodArray<Block> Blocks;

    // Store the total size overall
    size_t CompactedSize;
  };
}

#endif
