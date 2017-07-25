using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AllToOneCpp;
using System.Diagnostics;
using System.IO;

namespace ZilchToOneCpp
{
	class Program
	{
		static int Main(string[] args)
		{
			var compactor = new Compactor();

			var returnValue = 0;

			compactor.EmitLineDirectives = false;
			compactor.Minify = false;

			if (args.Length == 3 || args.Length == 4)
			{
				var target = args[0];
				var directoryPath = args[1];

				var forZilch = (target == "-Zilch");
				var forZero = !forZilch;

				var zeroPath = directoryPath;
				var zilchPath = directoryPath;

				if (forZero)
					zilchPath = Path.Combine(directoryPath, @"ZeroLibraries\Zilch");

				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(zilchPath, @"Project\Zilch"), "*.cpp");

				var standardLibraries = Path.Combine(zilchPath, @"Project\StandardLibraries");

				if (forZero)
					standardLibraries = Path.Combine(zeroPath, @"ZeroLibraries");

				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Common"), "*.cpp");
				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Math"), "*.cpp");
				compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform"), "*.cpp", false);

				// These files get conditionally included into the 'Debugging.cpp'
				compactor.FilesToProcess.RemoveFilesByName("DebuggingWindows.cpp");
				compactor.FilesToProcess.RemoveFilesByName("DebuggingGeneric.cpp");

				// At the moment, we don't use regex, so just only include NoRegex.cpp
				compactor.FilesToProcess.RemoveFilesByName("Regex.cpp");

				// We explicitly need to process the platform selector header file first
				compactor.FilesToProcess.Add(Path.Combine(standardLibraries, @"Platform\PlatformSelector.hpp"));

				// Windows platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Windows")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_WINDOWS)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Windows"), "*.cpp");
				}

				// Posix platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Posix")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_POSIX)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Posix"), "*.cpp");
				}

				// Empty platform
				{
					compactor.DirectoryDirectives.Add(Compactor.NormalizePath(Path.Combine(standardLibraries, @"Platform\Empty")), new CompacterDirectives()
					{
						PreprocessorCondition = "defined(PLATFORM_EMSCRIPTEN)",
						CppOnly = true,
					});
					compactor.FilesToProcess.AddFilesFromDirectory(Path.Combine(standardLibraries, @"Platform\Empty"), "*.cpp");
				}

				// This file brings in dependencies upon the engine and other bad things!
				compactor.FilesToProcess.RemoveFilesByName("CrashHandler.cpp");
				compactor.FilesToProcess.RemoveFilesByName("StackWalker.cpp");

				compactor.HppDirectories.Add(Path.Combine(standardLibraries, @"Common"));
				compactor.HppDirectories.Add(standardLibraries);

				var hppPath = args[2];

				var cppPath = String.Empty;
				if (args.Length == 4)
					cppPath = args[3];

				//var output = Path.Combine(zilchPath, @"Project\ZilchAll\NonCompacted\");
				compactor.Compact(cppPath, hppPath, null);

				Console.WriteLine("Done compacting");
			}
			else
			{
				Console.Error.WriteLine("Expected: <-Zilch or -Zero> <Zilch or Zero directory> <output.hpp> [output.cpp]");
				Console.Error.WriteLine("Given: `" + Environment.CommandLine + "`");
				returnValue = -1;
			}

			if (Debugger.IsAttached)
			{
				Console.WriteLine("Press enter to continue...");
				Console.ReadLine();
			}

			if (compactor.WasError)
				returnValue = -1;

			return returnValue;
		}

		static void CompileLib(String vsVersion, String vsYear, String chipset, String configuration, String runtime, String arguments)
		{
			Process compiler = StartCmd(@"C:\ZilchBuildOutput\DistributionOut");

			String fileName = "Zilch_" + vsYear + "_" + chipset + "_" + runtime;

			String pathIf64 = "";
			String vcVarsVersion = "32";
			if (chipset == "x64")
			{
				pathIf64 = "x86_amd64\\";
				vcVarsVersion = "x86_amd64";
			}

			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"vcvars" + vcVarsVersion + @".bat""");
			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"cl""  /c /Zi /nologo /W4 /WX " + arguments + @" /" + runtime + @" /Gm- /GS /Gy /fp:fast /Gd /Fd""C:\ZilchBuildOutput\Distribution\" + fileName + @".pdb"" /Fa""C:\ZilchBuildOutput\DistributionOut\\"" /Fo""C:\ZilchBuildOutput\DistributionOut\" + fileName + @".obj"" ""C:\Sandbox\Zero2\Zilch\Project\ZilchAll\Zilch.cpp""");
			compiler.StandardInput.WriteLine(@"""C:\Program Files (x86)\Microsoft Visual Studio " + vsVersion + @".0\VC\bin\" + pathIf64 + @"lib"" /OUT:""C:\ZilchBuildOutput\Distribution\" + fileName + @".lib"" /NOLOGO /LTCG ""C:\ZilchBuildOutput\DistributionOut\" + fileName + @".obj""");
			EndCmd(compiler);

			// Capitolize the PDB (cl outputs pdb as all lowercase)
			String pdbName = @"C:\ZilchBuildOutput\Distribution\" + fileName + ".pdb";
			File.Move(pdbName, pdbName);
		}

		static Process StartCmd(String workingDirectory)
		{
			Process compiler = new Process();
			compiler.StartInfo.FileName = "cmd.exe";
			compiler.StartInfo.WorkingDirectory = @"C:\ZilchBuildOutput\DistributionOut";
			compiler.StartInfo.RedirectStandardInput = true;
			compiler.StartInfo.RedirectStandardOutput = true;
			compiler.StartInfo.UseShellExecute = false;
			compiler.Start();
			return compiler;
		}

		static void EndCmd(Process compiler)
		{
			compiler.StandardInput.WriteLine(@"exit");
			Debug.Write(compiler.StandardOutput.ReadToEnd());
			compiler.WaitForExit();
			compiler.Close();
		}
	}
}
