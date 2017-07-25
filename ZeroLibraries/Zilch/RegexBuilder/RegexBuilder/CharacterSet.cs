using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RegexBuilder
{
    struct CharacterRange
    {
        public char mMin;
        public char mMax;
    }

    class CharacterSet
    {
        SortedSet<char> mAllCharacters = new SortedSet<char>();

        List<CharacterRange> mCharacterRanges = new List<CharacterRange>();

        public static CharacterSet Intersect(CharacterSet c1, CharacterSet c2)
        {
            CharacterSet newSet = new CharacterSet();

            foreach (char c in c1.mAllCharacters.Intersect(c2.mAllCharacters))
            {
                newSet.mAllCharacters.Add(c);
            }

            newSet.ComputeRanges();

            return newSet;
        }

        public static CharacterSet Union(CharacterSet c1, CharacterSet c2)
        {
            CharacterSet newSet = new CharacterSet();

            foreach (char c in c1.mAllCharacters.Union(c2.mAllCharacters))
            {
                newSet.mAllCharacters.Add(c);
            }

            newSet.ComputeRanges();

            return newSet;
        }

        public static CharacterSet Subtract(CharacterSet c1, CharacterSet c2)
        {
            CharacterSet newSet = new CharacterSet();

            foreach (char c in c1.mAllCharacters)
            {
                if (c2.mAllCharacters.Contains(c) == false)
                {
                    newSet.mAllCharacters.Add(c);
                }
            }

            newSet.ComputeRanges();

            return newSet;
        }

        public void AddCharacterRange(char min, char max)
        {
            System.Diagnostics.Debug.Assert(min < max, "The minimum character range must be less than the maximum!");

            for (int i = min; i <= max; ++i)
            {
                mAllCharacters.Add((char)i);
            }

            ComputeRanges();
        }

        public void RemoveCharacterRange(char min, char max)
        {
            System.Diagnostics.Debug.Assert(min < max, "The minimum character range must be less than the maximum!");

            for (int i = min; i <= max; ++i)
            {
                mAllCharacters.Remove((char)i);
            }

            ComputeRanges();
        }

        private void ComputeRanges()
        {
            mCharacterRanges.Clear();

            char lastChar = (char)0;

            char localMin = lastChar;
            char localMax = lastChar;

            foreach (char c in mAllCharacters)
            {
                // If we're just starting out, initialize the local min and max
                if (lastChar == 0)
                {
                    localMin = c;
                    localMax = c;
                }
                // If the difference between the current and last character is only 1, then it's part of the current range
                else if (c - lastChar == 1)
                {
                    localMax = c;
                }
                // It's not the beginning, and we just found a difference of more than 1 between the current and last character (this range is done)
                else
                {
                    mCharacterRanges.Add(new CharacterRange() { mMin = localMin, mMax = localMax });

                    localMin = c;
                    localMax = c;
                }

                lastChar = c;
            }

            if (lastChar != 0)
            {
                // Finally, add the local min and max
                mCharacterRanges.Add(new CharacterRange() { mMin = localMin, mMax = localMax });
            }
        }
    }
}
