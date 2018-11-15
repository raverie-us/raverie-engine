using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace AllToOneCpp
{
	public class ListHashSet<T> : ICollection<T>, IList<T>, IEnumerable<T>, IEnumerable
	{
		private List<T> mList = new List<T>();
		private HashSet<T> mSet = new HashSet<T>();

		public void Add(T item)
		{
			if (mSet.Contains(item))
				return;

			mSet.Add(item);
			mList.Add(item);
		}

		public void Clear()
		{
			mSet.Clear();
			mList.Clear();
		}

		public bool Contains(T item)
		{
			return mSet.Contains(item);
		}

		public void CopyTo(T[] array, int arrayIndex)
		{
			mList.CopyTo(array, arrayIndex);
		}

		public int Count
		{
			get { return mList.Count; }
		}

		public bool IsReadOnly
		{
			get { return false; }
		}

		public bool Remove(T item)
		{
			var resultList = mList.Remove(item);
			var resultSet = mSet.Remove(item);

			if (resultList != resultSet)
				throw new Exception("Internal error: Attempted to remove from both List and HashSet, inconsistent results!");

			return resultList;
		}

		public IEnumerator<T> GetEnumerator()
		{
			return mList.GetEnumerator();
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return mList.GetEnumerator();
		}

		public int IndexOf(T item)
		{
			return mList.IndexOf(item);
		}

		public void Insert(int index, T item)
		{
			if (mSet.Contains(item))
				return;

			mSet.Add(item);
			mList.Insert(index, item);
		}

		public void RemoveAt(int index)
		{
			T item = mList[index];
			mList.RemoveAt(index);
			var resultSet = mSet.Remove(item);

			if (resultSet == false)
				throw new Exception("Internal error: Item was in a List and not in the HashSet!");
		}

		public T this[int index]
		{
			get
			{
				return mList[index];
			}
			set
			{
				mList[index] = value;
			}
		}
	}
}
