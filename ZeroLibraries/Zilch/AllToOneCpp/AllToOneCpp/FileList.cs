using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Text.RegularExpressions;

namespace AllToOneCpp
{
	public class FileList : ListHashSet<String>
	{
		public void AddFilesFromDirectory(String directory, String searchPattern = "*", Boolean includeSubDirectories = true)
		{
			var dir = new DirectoryInfo(directory);

			foreach (var cppFile in dir.EnumerateFiles(searchPattern))
			{
				this.Add(cppFile.FullName);
			}

			if (includeSubDirectories)
			{
				foreach (var subDir in dir.EnumerateDirectories())
				{
					// Recursively scan all sub directories (we need to make sure we note relative paths)
					this.AddFilesFromDirectory(subDir.FullName, searchPattern, includeSubDirectories);
				}
			}
		}

		public void AddFilesFromDirectory(String directory, Regex match, Boolean includeSubDirectories = true)
		{
			var dir = new DirectoryInfo(directory);

			foreach (var cppFile in dir.EnumerateFiles())
			{
				// If the regex matched successfully
				if (match.IsMatch(cppFile.Name))
				{
					this.Add(cppFile.FullName);
				}
			}

			if (includeSubDirectories)
			{
				foreach (var subDir in dir.EnumerateDirectories())
				{
					// Recursively scan all sub directories (we need to make sure we note relative paths)
					this.AddFilesFromDirectory(subDir.FullName, match, includeSubDirectories);
				}
			}
		}

		public void RemoveFilesByName(String nonFullPathName)
		{
			var toBeRemoved = new List<String>();

			foreach (var cppFile in this)
			{
				if (Path.GetFileName(cppFile) == nonFullPathName)
				{
					toBeRemoved.Add(cppFile);
				}
			}

			foreach (var toRemove in toBeRemoved)
			{
				this.Remove(toRemove);
			}
		}

		public void RemoveFilesByName(Regex match)
		{
			var toBeRemoved = new List<String>();

			foreach (var file in this)
			{
				if (match.IsMatch(Path.GetFileName(file)))
				{
					toBeRemoved.Add(file);
				}
			}

			foreach (var toRemove in toBeRemoved)
			{
				this.Remove(toRemove);
			}
		}

		public void RemoveCppFilesByFullName(Regex match)
		{
			var toBeRemoved = new List<String>();

			foreach (var file in this)
			{
				if (match.IsMatch(file))
				{
					toBeRemoved.Add(file);
				}
			}

			foreach (var toRemove in toBeRemoved)
			{
				this.Remove(toRemove);
			}
		}
	}
}
