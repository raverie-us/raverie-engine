using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace RegexBuilder
{
	class RegexBuilder
	{
		private List<FSATable> mOperandStack = new List<FSATable>();

		private HashSet<char> mInputSet = new HashSet<char>();

		private FSATable mNFATable;

		private FSATable mDFATable;

		private List<char> mOperatorStack = new List<char>();

		private void Push(char input)
		{
			State s0 = new State();
			State s1 = new State();

			s0.AddTransition(input, s1);

			FSATable nfaTable = new FSATable();

			nfaTable.AppendState(s0);
			nfaTable.AppendState(s1);

			mOperandStack.Add(nfaTable);

			mInputSet.Add(input);
		}

		private bool Pop(out FSATable tableOut)
		{
			if (mOperandStack.Count != 0)
			{
				tableOut = mOperandStack[mOperandStack.Count - 1];
				mOperandStack.RemoveAt(mOperandStack.Count - 1);
				return true;
			}

			tableOut = null;
			return false;
		}

		private bool Concat()
		{
			FSATable A, B;

			if (!Pop(out B) || !Pop(out A))
			{
				return false;
			}

			A.GetLastState().AddTransition(Constants.Epsilon, B.GetFirstState());

			A.AppendFSATable(B);

			mOperandStack.Add(A);

			return true;
		}

		private bool Star()
		{
			FSATable A;

			if (!Pop(out A))
			{
				return false;
			}

			State start = new State();
			State end = new State();

			start.AddTransition(Constants.Epsilon, end);

			start.AddTransition(Constants.Epsilon, A.GetFirstState());

			A.GetLastState().AddTransition(Constants.Epsilon, end);

			A.GetLastState().AddTransition(Constants.Epsilon, A.GetFirstState());

			A.PrependState(start);
			A.AppendState(end);

			mOperandStack.Add(A);

			return true;
		}

		private bool Union()
		{
			FSATable A, B;

			if (!Pop(out B) || !Pop(out A))
			{
				return false;
			}

			State start = new State();
			State end = new State();

			start.AddTransition(Constants.Epsilon, A.GetFirstState());
			start.AddTransition(Constants.Epsilon, B.GetFirstState());

			A.GetLastState().AddTransition(Constants.Epsilon, end);
			B.GetLastState().AddTransition(Constants.Epsilon, end);

			A.PrependState(start);
			A.AppendFSATable(B);
			A.AppendState(end);

			mOperandStack.Add(A);

			return true;
		}

		private static bool IsInput(char input)
		{
			return input != '(' && input != ')' && input != '*' && input != Constants.Concat && input != '|';
		}

		private String ConcatExpand(String input)
		{
			StringBuilder result = new StringBuilder();

			for (int i = 0; i < input.Length - 1; ++i)
			{
				char left = input[i];
				char right = input[i + 1];

				result.Append(left);

				if (IsInput(left) || left == ')' || left == '*')
				{
					if (IsInput(right) || right == '(')
					{
						result.Append(Constants.Concat);
					}
				}
			}

			result.Append(input[input.Length - 1]);

			return result.ToString();
		}

		private bool Presedence(char opLeft, char opRight)
		{
			if (opLeft == opRight)
				return true;

			if (opLeft == '*')
				return false;

			if (opRight == '*')
				return true;

			if (opLeft == Constants.Concat)
				return false;

			if (opRight == Constants.Concat)
				return true;

			if (opLeft == '|')
				return false;
		
			return true;
		}

		bool Eval()
		{
			if (mOperatorStack.Count > 0)
			{
				char chOperator = mOperatorStack[mOperatorStack.Count - 1];
				mOperatorStack.RemoveAt(mOperatorStack.Count - 1);

				// Check which operator it is
				switch(chOperator)
				{
				case '*':
					return Star();
				case '|':
					return Union();
				case Constants.Concat:
					return Concat();
				}

				return false;
			}

			return false;
		}

		public bool DoRegex(Tokens tokens)
		{
			if (CreateNFA(tokens))
			{
				ConvertNFAtoDFA();
				ReduceDFA();
				CollapseChains();
				return true;
			}

			return false;
		}

		public bool DoRegex(String regex)
		{
			if (CreateNFA(regex))
			{
				ConvertNFAtoDFA();
				ReduceDFA();
				CollapseChains();
				return true;
			}

			return false;
		}

		private void CollapseChains()
		{
			CollapseChain(mDFATable.GetFirstState());
		}

		private String CollapseChain(State state)
		{
			if (state.mTransitions.Count == 1)
			{
				State childState = state.mTransitions.Values.First()[0];

				String result = CollapseChain(childState);

				if (state.mAcceptingState == false)
				{
					if (result != null)
					{
						mDFATable.GetStates().Remove(childState);
						state.mChain = state.mTransitions.Keys.First() + result;

						state.mChainTokenID = childState.mTokenID;

						if (childState.mChainDebugTokenName != null && childState.mChainDebugTokenName != String.Empty)
						{
							state.mChainDebugTokenName = childState.mChainDebugTokenName;
						}
						else
						{
							state.mChainDebugTokenName = childState.mDebugTokenName;
						}
						//state.mAcceptingState = childState.mAcceptingState;

						state.mTransitions.Clear();
						return state.mChain;
					}
				}
			}
			else if (state.mTransitions.Count == 0)
			{
				return String.Empty;
			}
			else
			{
				foreach (List<State> states in state.mTransitions.Values)
				{
					CollapseChain(states[0]);
				}
			}

			return null;
		}

		private void ReduceDFA()
		{
			// Get the set of all dead end states in DFA
			HashSet<State> deadEndSet = new HashSet<State>();
			foreach (State state in mDFATable.GetStates())
			{
				if (state.IsDeadEnd())
				{
					deadEndSet.Add(state);
				}
			}

			// If there are no dead ends then there is nothing to reduce
			if (deadEndSet.Count == 0)
				return;

			// Remove all transitions to these states
			foreach (State deadState in deadEndSet)
			{
				// Remove all transitions to this state
				foreach (State state in mDFATable.GetStates())
				{
					state.RemoveTransition(deadState);
				}

				mDFATable.GetStates().Remove(deadState);
			}

			foreach (State state in mDFATable.GetStates())
			{
				state.CleanTransitions();
			}
		}

		private void ConvertNFAtoDFA()
		{
			mDFATable = new FSATable();

			if (mNFATable.GetStates().Count == 0)
				return;

			State.mStateIDCounter = 0;

			HashSet<State> dfaStartStates;
			EpsilonClosure(out dfaStartStates, mNFATable.GetFirstState());

			State dfaStartState = new State(dfaStartStates);

			mDFATable.AppendState(dfaStartState);
			
			List<State> unmarkedDFAStates = new List<State>();
			unmarkedDFAStates.Add(dfaStartState);

			while (unmarkedDFAStates.Count != 0)
			{
				State processingDFAState = unmarkedDFAStates[unmarkedDFAStates.Count - 1];
				unmarkedDFAStates.RemoveAt(unmarkedDFAStates.Count - 1);

				foreach (char c in mInputSet)
				{
					HashSet<State> moveResult, epsilonClosureResult;
					Move(c, out moveResult, new HashSet<State>(processingDFAState.mNFAStates));
					EpsilonClosure(out epsilonClosureResult, moveResult);

					bool found = false;
					State s = null;

					for (int i = 0; i < mDFATable.GetStates().Count; ++i)
					{
						s = mDFATable.GetState(i);

						if (s.mNFAStates.SetEquals(epsilonClosureResult))
						{
							found = true;
							break;
						}
					}

					if (found == false)
					{
						State u = new State(epsilonClosureResult);
						unmarkedDFAStates.Add(u);
						mDFATable.AppendState(u);

						processingDFAState.AddTransition(c, u);
					}
					else
					{
						processingDFAState.AddTransition(c, s);
					}
				}
			}
		}

		private bool CreateNFA(Tokens tokens)
		{
			bool firstToken = true;

			foreach (Token token in tokens.mTokens)
			{
				bool firstChar = true;

				foreach (char c in token.mRegex)
				{
					Push(c);

					if (firstChar == false)
					{
						mOperatorStack.Add(Constants.Concat);
						Eval();
					}

					firstChar = false;
				}

				State acceptingStateForThisToken = mOperandStack[mOperandStack.Count - 1].GetLastState();
				acceptingStateForThisToken.mAcceptingState = true;
				acceptingStateForThisToken.mTokenID = token.mID;
				acceptingStateForThisToken.mDebugTokenName = tokens.mTokens[token.mID].mNames[0] + " /* " + String.Join(", ", tokens.mTokens[token.mID].mNames.ToArray()) + " */";

				if (firstToken == false)
				{
					mOperatorStack.Add('|');
					Eval();
				}

				firstToken = false;
			}


			if (!Pop(out mNFATable))
			{
				return false;
			}

			//mNFATable.GetLastState().mAcceptingState = true;
			return true;
		}

		private bool CreateNFA(String regex)
		{
			regex = ConcatExpand(regex);

			for (int i = 0; i < regex.Length; ++i)
			{
				char c = regex[i];

				if (IsInput(c))
				{
					Push(c);
				}
				else if (mOperatorStack.Count == 0 || c == '(')
				{
					mOperatorStack.Add(c);
				}
				else if (c == ')')
				{
					while (mOperatorStack[mOperatorStack.Count - 1] != '(')
					{
						if (!Eval())
						{
							return false;
						}
					}

					mOperatorStack.RemoveAt(mOperatorStack.Count - 1);
				}
				else
				{
					while (mOperatorStack.Count != 0 && Presedence(c, mOperatorStack[mOperatorStack.Count - 1]))
					{
						if (!Eval())
						{
							return false;
						}
					}

					mOperatorStack.Add(c);
				}
			}

			while (mOperatorStack.Count != 0)
			{
				if (!Eval())
				{
					return false;
				}
			}

			if (!Pop(out mNFATable))
			{
				return false;
			}

			mNFATable.GetLastState().mAcceptingState = true;
			return true;
		}

		void Move(char input, out HashSet<State> outputs, params State[] inputs)
		{
			HashSet<State> inputSet = new HashSet<State>(inputs);
			Move(input, out outputs, inputSet);
		}

		void Move(char input, out HashSet<State> outputs, HashSet<State> inputs)
		{
			outputs = new HashSet<State>();

			foreach (State state in inputs)
			{
				List<State> transitionStates;
				state.GetTransitions(input, out transitionStates);

				foreach (State tstate in transitionStates)
				{
					outputs.Add(tstate);
				}
			}
		}

		void EpsilonClosure(out HashSet<State> outputs, params State[] inputs)
		{
			HashSet<State> inputSet = new HashSet<State>(inputs);
			EpsilonClosure(out outputs, inputSet);
		}

		void EpsilonClosure(out HashSet<State> outputs, HashSet<State> inputs)
		{
			outputs = new HashSet<State>(inputs);

			List<State> stack = new List<State>(inputs);

			while (stack.Count != 0)
			{
				State state = stack[stack.Count - 1];
				stack.RemoveAt(stack.Count - 1);

				List<State> epsilonStates;
				state.GetTransitions(Constants.Epsilon, out epsilonStates);

				foreach (State epsilonState in epsilonStates)
				{
					if (outputs.Contains(epsilonState) == false)
					{
						outputs.Add(epsilonState);
						stack.Add(epsilonState);
					}
				}
			}

			return;
		}

		public void SaveDFAGraph()
		{
			SaveGraph(mDFATable, "DFA");
		}

		public void SaveDFACode()
		{
			StringBuilder builder = new StringBuilder();

			SaveStateCode(mDFATable.GetFirstState(), builder, 0);

			String code = builder.ToString();

			File.WriteAllText(@"..\..\..\..\Project\Zilch\TokenReader.inl", code);
		}

		private String Tab(int depth)
		{
			return new String(' ', depth * 2);
		}

		private void SaveStateCode(State state, StringBuilder builder, int depth)
		{
			if (state.mTransitions.Count != 0)
			{
				builder.AppendFormat("{0}character = ReadCharacter();\n\n", Tab(depth));

				builder.AppendFormat("{0}switch (character)\n", Tab(depth));
				builder.AppendFormat("{0}{{\n", Tab(depth));

				foreach (KeyValuePair<char, List<State>> transitions in state.mTransitions)
				{
					State childState = transitions.Value[0];
					char toChild = transitions.Key;

					builder.AppendFormat("{0}case '{1}':\n", Tab(depth + 1), toChild);
					builder.AppendFormat("{0}{{\n", Tab(depth + 1));

					// If we're at the root level, examine the transition character to see if this is a symbol or a keyword
					if (depth == 0)
					{
						if (char.IsLetter(toChild))
						{
							builder.AppendFormat("{0}tokenType = TokenCategory::Keyword;\n\n", Tab(depth + 2));
						}
						else
						{
							builder.AppendFormat("{0}tokenType = TokenCategory::Symbol;\n\n", Tab(depth + 2));
						}
					}

					if (childState.mAcceptingState == true)
					{
						builder.AppendFormat("{0}lastAcceptedPos = this->Position;\n", Tab(depth + 2));
						builder.AppendFormat("{0}outToken->TokenId = Grammar::{1};\n", Tab(depth + 2), childState.mDebugTokenName);
						builder.AppendFormat("{0}acceptedToken = true;\n\n", Tab(depth + 2));
					}
					else
					{
						builder.AppendFormat("{0}acceptedToken = false;\n\n", Tab(depth + 2));
					}


					if (childState.mChain != null)
					{
						builder.AppendFormat("{0}if (DiffString(\"{1}\"))\n", Tab(depth + 2), childState.mChain);
						builder.AppendFormat("{0}{{\n", Tab(depth + 2), childState.mChain);
						builder.AppendFormat("{0}lastAcceptedPos = this->Position;\n", Tab(depth + 3));
						builder.AppendFormat("{0}outToken->TokenId = Grammar::{1};\n", Tab(depth + 3), childState.mChainDebugTokenName);
						builder.AppendFormat("{0}acceptedToken = true;\n", Tab(depth + 3));
						builder.AppendFormat("{0}}}\n", Tab(depth + 2));
					}

					SaveStateCode(childState, builder, depth + 2);

					builder.AppendFormat("{0}break;\n", Tab(depth + 2));
					builder.AppendFormat("{0}}}\n\n", Tab(depth + 1));
				}

				//builder.AppendFormat("{0}default:;\n", Tab(depth + 1));
				//builder.AppendFormat("{0}{{\n", Tab(depth + 1));
				//builder.AppendFormat("{0}return false;\n", Tab(depth + 2));
				//builder.AppendFormat("{0}}}\n", Tab(depth + 1));

				builder.AppendFormat("{0}}}\n", Tab(depth));
			}
			else
			{
				builder.AppendFormat("{0}character = ReadCharacter();\n\n", Tab(depth));
			}
		}

		public void SaveNFAGraph() 
		{
			SaveGraph(mNFATable, "NFA");
		}


		public void SaveGraph(FSATable table, String name)
		{
			StringBuilder strGraph = new StringBuilder("digraph{\n");
			List<State> faStates = table.GetStates();

			for (int i = 0; i < faStates.Count; ++i)
			{
				State state = faStates[i];
				String strStateID = state.mStateID.ToString();
				strGraph.Append("\t" + strStateID + "\t[");

				if (state.mAcceptingState)
				{
					strGraph.Append("shape=doubleoctagon, ");
				}

				String label = null;
			   
				if (state.mChain != null)
				{
					strGraph.Append("color=blue, ");
					label = state.mChain;
				}

				if (state.mTokenID != -1)
				{
					if (label == null)
					{
						label = "(" + state.mDebugTokenName + ")";
					}
					else
					{
						label += " : (" + state.mDebugTokenName + ")";
					}
				}

				if (label != null)
				{
					strGraph.Append("label=\"" + label + "\", ");
				}

				strGraph.Append("];\n");
			}

			strGraph.Append("\n");

			// Record transitions
			for (int i = 0; i < faStates.Count; ++i)
			{
				State state = faStates[i];
				List<State> states;

				state.GetTransitions(Constants.Epsilon, out states);

				for (int j = 0; j < states.Count; ++j)
				{
					// Record transition
					String strStateID1 = state.mStateID.ToString();
					String strStateID2 = states[j].mStateID.ToString();
					strGraph.Append("\t" + strStateID1 + " -> " + strStateID2);
					strGraph.Append("\t[label=\"epsilon\"];\n");
				}

				foreach (char input in mInputSet)
				{
					state.GetTransitions(input, out states);

					for (int j = 0; j < states.Count; ++j)
					{
						// Record transition
						String strStateID1 = state.mStateID.ToString();
						String strStateID2 = states[j].mStateID.ToString();
						strGraph.Append("\t" + strStateID1 + " -> " + strStateID2);
						strGraph.Append("\t[label=\"" + input.ToString() + "\"];\n");
					}
				}
			}

			strGraph.Append("}");

			File.WriteAllText(@"C:\Temp\" + name + @".gv", strGraph.ToString());

			System.Diagnostics.Process.Start(@"C:\Program Files (x86)\Graphviz2.28\bin\dot.exe", @"-Tpng -o""C:\Temp\" + name + @".png"" ""C:\Temp\" + name + @".gv""").Close();
		}
	}
}
