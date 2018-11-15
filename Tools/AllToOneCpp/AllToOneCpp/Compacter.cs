using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace AllToOneCpp
{
	enum IncludeStatus
	{
		None,
		PragmaOnce,
	}

	enum CodeFileType
	{
		HppFile,
		CppFile,
	}

	public delegate String HppNotFoundFn(String exactIncludeString, String relativeDir);

	public class CompactorOutput
	{
		public String CppCode;
		public String HppCode;
	}

	public class Compactor
	{
		/// <summary>
		/// Where we scan for *.hpp files. Alternatively if a file cannot be found, the HppNotFound event is called.
		/// </summary>
		public ListHashSet<String> HppDirectories = new ListHashSet<String>();

		/// <summary>
		/// If we ever attempt to pull an hpp from these directories (or any subdirectory) we will issue a warning.
		/// </summary>
		public ListHashSet<String> HppWarningDirectories = new ListHashSet<String>();

		/// <summary>
		/// Occurs when an hpp file cannot be found in one of the given hpp directories. This callback can return
		/// the path of the hpp if it resolves it itself, or return null if it's unknown. If an hpp cannot be found
		/// it will be left in the cpp file as a #include.
		/// </summary>
		public HppNotFoundFn HppNotFound;

		/// <summary>
		/// All of the files we want to build from.
		/// Hpp files should not explicitly be added unless they are unreferenced by any cpp, or their order must be controlled.
		/// </summary>
		public FileList FilesToProcess = new FileList();

		/// <summary>
		/// All files under these directories will have these directives applied.
		/// </summary>
		public Dictionary<String, CompacterDirectives> DirectoryDirectives = new Dictionary<String, CompacterDirectives>();

		/// <summary>
		/// Whether we attempt to reduce output code size (strips comments, redundant newlines, etc). Makes it human unreadable.
		/// </summary>
		public Boolean Minify = false;

		/// <summary>
		/// Whether we emit #line directives to remap compacted lines to the original files. Not valid with Minify.
		/// </summary>
		public Boolean EmitLineDirectives = true;

		/// <summary>
		/// Whether an warning/error occurred while compacting.
		/// </summary>
		public Boolean WasError = false;

		static Regex IncludeRegex = new Regex(@"^.*#include ""(.+)""(.*)$", RegexOptions.Compiled | RegexOptions.Multiline);
		static Regex PragmaOnceRegex = new Regex(@"#pragma once", RegexOptions.Compiled);
		static Regex CompacterDirectiveRegex = new Regex(@"//#compact ([ a-zA-Z0-9-_]+)", RegexOptions.Compiled);

		static Regex SingleLineComment = new Regex(@"//.*", RegexOptions.Compiled);
		static Regex MultiLineComment = new Regex(@"/\*.*?\*/", RegexOptions.Compiled | RegexOptions.Singleline);
		static Regex EmptyLine = new Regex(@"^\s+$", RegexOptions.Compiled | RegexOptions.Multiline);
		
		static Regex SingleLineFeed = new Regex(@"([^\r])\n", RegexOptions.Compiled);
		static Regex SingleCarriageReturn = new Regex(@"\r([^\n])", RegexOptions.Compiled);

		// These aren't just Cpp files, they could be *.inl, *.h, *.hpp, even *.cpp, etc
		HashSet<String> ProcessedOnceIncludeFiles = new HashSet<String>();

		StringBuilder SingleCpp = new StringBuilder();
		StringBuilder SingleHpp = new StringBuilder();
		String OutputDirectory = null;

		public static String NormalizeCombine(params String[] paths)
		{
			return NormalizePath(Path.Combine(paths));
		}

		public static String NormalizePath(String path)
		{
			return Path.GetFullPath(new Uri(Path.GetFullPath(path)).LocalPath)
					   .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
		}

		public void Compact(String cppPath, String hppPath, String outputDirectory)
		{
			if (outputDirectory != null)
			{
				Directory.CreateDirectory(outputDirectory);

				foreach (var file in Directory.GetFiles(outputDirectory))
				{
					File.Delete(file);
				}
			}
			this.OutputDirectory = outputDirectory;

			var result = this.Compact(Path.GetFileName(hppPath));

			if (String.IsNullOrWhiteSpace(cppPath) == false)
				File.WriteAllText(cppPath, result.CppCode);
			if (String.IsNullOrWhiteSpace(hppPath) == false)
				File.WriteAllText(hppPath, result.HppCode);
		}

		public CompactorOutput Compact(String hppFileName)
		{
			this.WasError = false;

			if (this.Minify && this.EmitLineDirectives)
				throw new Exception("Cannot minify code and emit line directives");

			this.SingleCpp.AppendLine("#include \"" + Path.GetFileName(hppFileName) + "\"");

			foreach (var file in this.FilesToProcess)
			{
				var debugFileStack = new Stack<String>();
				if (Path.GetExtension(file) == ".hpp" || Path.GetExtension(file) == ".h")
					this.ProcessIncludeCodeFile(debugFileStack, file, CodeFileType.HppFile);
				else
					this.ProcessCppCodeFile(debugFileStack, file);
			}

			var singleCppCode = this.SingleCpp.ToString();
			var singleHppCode = this.SingleHpp.ToString();

			if (this.Minify)
			{
				singleCppCode = this.Cleanup(singleCppCode);
				singleHppCode = this.Cleanup(singleHppCode);
			}

			this.SingleCpp.Clear();
			this.SingleHpp.Clear();

			return new CompactorOutput()
			{
				CppCode = singleCppCode,
				HppCode = singleHppCode,
			};
		}

		String Cleanup(String code)
		{
			String lastCode = null;

			for (var i = 0; i < 50; ++i)
			{
				lastCode = code;
				code = SingleLineFeed.Replace(code, "$1\r\n");
				code = SingleCarriageReturn.Replace(code, "\r\n$1");

				if (lastCode == code)
					break;
			}

			for (var i = 0; i < 50; ++i)
			{
				lastCode = code;
				code = code.Replace("\r\n\r\n\r\n", "\r\n\r\n");
				code = code.Replace("\n\n", "\n");
				code = code.Replace("\r\r", "\r");

				if (lastCode == code)
					break;
			}

			return code;
		}

		String ScanForInclude(String exactIncludeString, String relativeDir, Stack<String> debugFileStack)
		{
			var result = this.ScanForIncludeNoWarning(exactIncludeString, relativeDir);

			if (result == null)
				return null;

			foreach (var hppWarnDirectory in this.HppWarningDirectories)
			{
				if (result.StartsWith(hppWarnDirectory))
				{
					this.EmitWarning(debugFileStack, "Include from directory '" + hppWarnDirectory + "'");
				}
			}

			return result;
		}

		String ScanForIncludeNoWarning(String exactIncludeString, String relativeDir)
		{
			var relativePath = NormalizeCombine(relativeDir, exactIncludeString);
			if (File.Exists(relativePath))
			{
				return relativePath;
			}

			foreach (var hppDirectory in this.HppDirectories)
			{
				var hppPath = NormalizeCombine(hppDirectory, exactIncludeString);

				if (File.Exists(hppPath))
				{
					return hppPath;
				}
			}

			if (this.HppNotFound != null)
			{
				return NormalizePath(this.HppNotFound(exactIncludeString, relativeDir));
			}

			return null;
		}

		void AttemptEmitLineDirective(CodeFileType type, int line, String fileName)
		{
			if (this.EmitLineDirectives)
				this.AppendCodeSeparateLine(type, "#line " + line + " \"" + Path.GetFileName(fileName) + "\"");
		}

		void AppendCodeSeparateLine(CodeFileType type, String code)
		{
			this.AppendCode(type, Environment.NewLine + code + Environment.NewLine);
		}

		void AppendCode(CodeFileType type, String code, Int32 start = 0, Int32 length = -1)
		{
			if (length == -1)
				length = code.Length;

			if (type == CodeFileType.HppFile)
			{
				this.SingleHpp.Append(code, start, length);
			}
			else
			{
				this.SingleCpp.Append(code, start, length);
			}
		}

		int CountLines(String code, int start, int length)
		{
			int lines = 0;

			bool wasCarriageReturn = false;
			int end = start + length;
			for (var i = start; i < end; ++i)
			{
				char c = code[i];

				if (c == '\n' || c == '\r')
				{
					if ((wasCarriageReturn && c == '\n') == false)
					{
						++lines;
					}

					wasCarriageReturn = (c == '\r');
				}
			}

			return lines;
		}

		void ProcessCodeFile(Stack<String> debugFileStack, String codeFile, String code, CodeFileType type)
		{
			if (debugFileStack.Contains(codeFile))
			{
				var builder = new StringBuilder();
				foreach (var file in debugFileStack)
				{
					builder.AppendLine("  " + file);
				}
				throw new Exception("Cycle of inclusion detected: \n" + builder.ToString());
			}

			debugFileStack.Push(codeFile);
			if (debugFileStack.Count == 50)
				throw new Exception("Possible cycle of inclusion! Recursion depth was too deep");

			var directoryDirective = new CompacterDirectives();
			var directoryPath = NormalizePath(Path.GetDirectoryName(codeFile));
			if (this.DirectoryDirectives.ContainsKey(directoryPath))
			{
				directoryDirective = this.DirectoryDirectives[directoryPath];
			}
			var directives = this.ParseDirectives(debugFileStack, code, directoryDirective);

			if (this.OutputDirectory != null)
			{
				var pathToOutput = Path.Combine(this.OutputDirectory, Path.GetFileName(codeFile));
				//if (File.Exists(pathToOutput) == false)
				//{
				//    File.WriteAllText(pathToOutput, code);
				//}
				//else if (File.ReadAllText(pathToOutput) != code)
				//{
				//    String error = "The file '" + Path.GetFileName(pathToOutput) + "' has the same name as another file";
				//    error = error;
				//    //throw new Exception("The file '" + Path.GetFileName(pathToOutput) + "' has the same name as another file");
				//}
			}

			// If this is meant to only be included in a cpp file, then just set the type to cpp
			if (directives.CppOnly)
			{
				type = CodeFileType.CppFile;

				// Remove all pragma once declarations
				code = PragmaOnceRegex.Replace(code, String.Empty);
			}


			if (directives.PreprocessorCondition != null)
			{
				this.AppendCodeSeparateLine(type, "#if " + directives.PreprocessorCondition);
			}

			int line = 1;

			// When we compile, we want to make sure we get errors in the correct places
			this.AttemptEmitLineDirective(type, line, codeFile);

			var fullName = codeFile;

			var strIndex = 0;

			// Scan for includes
			var matches = IncludeRegex.Matches(code);
			foreach (Match match in matches)
			{
				// Skip any includes that are commented out
				// This does NOT catch multi-line comments!
				if (SingleLineComment.IsMatch(match.Value))
					continue;
				
				// Cpps include both cpp and header files
				int prevCodeLength = match.Index - strIndex;
				this.AppendCode(type, code, strIndex, prevCodeLength);
				line += this.CountLines(code, strIndex, prevCodeLength);

				strIndex = match.Index + match.Length;

				var includeFile = match.Groups[1].Value;

				var fullPathToIncludeFile = this.ScanForInclude(includeFile, Path.GetDirectoryName(codeFile), debugFileStack);

				if (fullPathToIncludeFile != null)
				{
					if (this.ProcessedOnceIncludeFiles.Contains(fullPathToIncludeFile) == false)
					{
						var includeStatus = this.ProcessIncludeCodeFile(debugFileStack, fullPathToIncludeFile, type);

						// We only ever add the include to the 'all includes' if it contains a pragma-once
						// Otherwise, we may want to reprocess that include
						if (includeStatus == IncludeStatus.PragmaOnce)
						{
							this.ProcessedOnceIncludeFiles.Add(fullPathToIncludeFile);
						}

						// Since we just entered another file (and probably emitted) and now we're back in our own file, emit a line directive again)
						this.AttemptEmitLineDirective(type, line, codeFile);
					}
				}
				else
				{
					var afterInclude = match.Groups[2].Value;
					if (afterInclude.Contains("@ignore") == false)
						this.EmitWarning(debugFileStack, "Unable to find '" + includeFile + "'");

					this.AppendCode(type, match.Value, 0, match.Value.Length);
				}
			}

			this.AppendCode(type, code, strIndex, code.Length - strIndex);

			if (directives.PreprocessorCondition != null)
			{
				this.AppendCode(type, Environment.NewLine + "#endif" + Environment.NewLine);
			}

			debugFileStack.Pop();
		}

		void AppendDebugFileStack(Stack<String> debugFileStack, StringBuilder builder)
		{
			foreach (var entry in debugFileStack.Reverse())
			{
				builder.AppendLine("  in '" + entry + "'");
			}
		}

		private void EmitWarning(Stack<String> debugFileStack, String message)
		{
			var contextBuilder = new StringBuilder();
			contextBuilder.AppendLine("Warning: " + message);
			this.AppendDebugFileStack(debugFileStack, contextBuilder);
			Console.Error.WriteLine(contextBuilder.ToString());
			this.WasError = true;
		}

		CompacterDirectives ParseDirectives(Stack<String> debugFileStack, String code, CompacterDirectives parentDirectives)
		{
			CompacterDirectives directives = parentDirectives.Clone();

			foreach (Match match in CompacterDirectiveRegex.Matches(code))
			{
				String directive = match.Groups[1].Value;
				switch (directive)
				{
					case "cpp-only":
						directives.CppOnly = true;
						break;

					default:
						this.EmitWarning(debugFileStack, "Unknown 'compacter' directive '" + match.Groups[1].Value + "'");
						break;
				}
			}

			return directives;
		}

		IncludeStatus ProcessIncludeCodeFile(Stack<String> debugFileStack, String codeFile, CodeFileType parentFile)
		{
			var status = IncludeStatus.None;
			var code = File.ReadAllText(codeFile);

			
			// If the file is a pragma once file, it should always be treated as a normal include and go into the include file
			if (PragmaOnceRegex.IsMatch(code))
			{
				status = IncludeStatus.PragmaOnce;
				this.ProcessCodeFile(debugFileStack, codeFile, code, CodeFileType.HppFile);
			}
			else
			{
				// Otherwise, it's basically be included like an 'inl' file, which is always included into its parent
				this.ProcessCodeFile(debugFileStack, codeFile, code, parentFile);
			}

			return status;
		}

		void ProcessCppCodeFile(Stack<String> debugFileStack, String codeFile)
		{
			var code = File.ReadAllText(codeFile);
			this.ProcessCodeFile(debugFileStack, codeFile, code, CodeFileType.CppFile);
		}
	}

	public class CompacterDirectives
	{
		/// <summary>
		/// Files will only be included into the cpp file (not the header file).
		/// </summary>
		public Boolean CppOnly;

		/// <summary>
		/// The condition used to include these files, such as: defined(PLATFORM_LINUX)
		/// A value of null means that the files will not be wrapped in a condition.
		/// </summary>
		public String PreprocessorCondition = null;

		public CompacterDirectives Clone()
		{
			return new CompacterDirectives()
			{
				CppOnly = this.CppOnly,
				PreprocessorCondition = this.PreprocessorCondition,
			};
		}
	}
}
