using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RegexBuilder
{
    class FSATable
    {
        private List<State> mStates = new List<State>();

        public void AppendFSATable(FSATable table)
        {
            mStates.AddRange(table.mStates);
        }

        public State GetFirstState()
        {
            return mStates[0];
        }

        public State GetLastState()
        {
            return mStates[mStates.Count - 1];
        }

        public void PrependState(State state)
        {
            mStates.Insert(0, state);
        }

        public void AppendState(State state)
        {
            mStates.Add(state);
        }

        public List<State> GetStates()
        {
            return mStates;
        }

        public State GetState(int index)
        {
            return mStates[index];
        }
    }
}
