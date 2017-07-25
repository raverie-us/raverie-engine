using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RegexBuilder
{
    class State
    {
        public Dictionary<char, List<State>> mTransitions = new Dictionary<char, List<State>>();

        public HashSet<State> mNFAStates;

        public int mStateID;

        public static int mStateIDCounter = 0;

        public bool mAcceptingState = false;

        public int mTokenID = -1;

        public String mDebugTokenName = String.Empty;

        public bool mMarked = false;

        public String mChain;

        public int mChainTokenID = -1;

        public String mChainDebugTokenName = String.Empty;

        public State()
        {
            mStateID = mStateIDCounter;
            ++mStateIDCounter;
        }

        public State(IEnumerable<State> nfaStates) : this()
        {
            mNFAStates = new HashSet<State>(nfaStates);

            foreach (State state in mNFAStates)
            {
                if (state.mAcceptingState)
                {
                    mAcceptingState = true;
                    mTokenID = state.mTokenID;
                    mDebugTokenName = state.mDebugTokenName;
                }
            }
        }

        public void RemoveTransition(State toState)
        {
            foreach (List<State> states in mTransitions.Values)
            {
                states.Remove(toState);
            }
        }

        public bool IsDeadEnd()
        {
            if (mAcceptingState)
                return false;

            if (mTransitions.Count == 0)
                return true;
            
            foreach (List<State> states in mTransitions.Values)
            {
                foreach (State state in states)
                {
                    // It's not pointing back at ourself...
                    if (state != this)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        public void CleanTransitions()
        {
            List<char> toClean = new List<char>();

            foreach (char c in mTransitions.Keys)
            {
                if (mTransitions[c].Count == 0)
                {
                    toClean.Add(c);
                }
            }

            foreach (char c in toClean)
            {
                mTransitions.Remove(c);
            }
        }

        public void GetTransitions(char input, out List<State> outStates)
        {
            if (mTransitions.TryGetValue(input, out outStates) == false)
            {
                outStates = new List<State>();
            }
        }

        public void AddTransition(char input, State nextState)
        {
            List<State> states;

            if (mTransitions.TryGetValue(input, out states) == false)
            {
                states = new List<State>();
                mTransitions.Add(input, states);
            }

            states.Add(nextState);
        }
    }
}
