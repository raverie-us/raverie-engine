using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace ValueUnitTestGeneration
{
	class Program
	{
		[STAThread]
		static void Main(string[] args)
		{
			var program = new Program();
			program.ProgramMain(args);
		}

		void ProgramMain(string[] args)
		{
			var members = new List<ValueCode>()
			{
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "$.Y",
				},
				new ValueCode()
				{
					Initialization = "$.X = 12345; $.Z = 12345; $.X = 9863321; $.Z = 88371435;",
					AccessValue = "$.X + $.Z"
				},
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "$.PY",
				},
				new ValueCode()
				{
					Initialization = "$.PX = 12345; $.PZ = 12345; $.PX = 9863321; $.PZ = 88371435;",
					AccessValue = "$.PX + $.PZ"
				},
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "$.Function()"
				},
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "$.PassThrough(98234756)"
				},
			};

			var typesBeingAccessed = new List<ValueCode>()
			{
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "StructStatic",
				},
				new ValueCode()
				{
					Initialization = "var structNoConstructor_ = StructNoConstructor();",
					AccessValue = "structNoConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structDefaultConstructor_ = StructDefaultConstructor();",
					AccessValue = "structDefaultConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structParameterConstructor_ = StructParameterConstructor(9863321, 98234756, 88371435);",
					AccessValue = "structParameterConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structInheritedNoConstructor_ = StructInheritedNoConstructor();",
					AccessValue = "structInheritedNoConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structInheritedDefaultConstructor_ = StructInheritedDefaultConstructor();",
					AccessValue = "structInheritedDefaultConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structInheritedParameterConstructor_ = StructInheritedParameterConstructor(9863321, 98234756, 88371435);",
					AccessValue = "structInheritedParameterConstructor_",
				},
				new ValueCode()
				{
					Initialization = "var structInheritedOverwrittenConstructor_ = StructInheritedOverwrittenConstructor();",
					AccessValue = "structInheritedOverwrittenConstructor_",
				},
			};
			
			// Add all the class versions exactly the same as the above structs
			// Make sure to clone the array so we don't modify while walking through it
			foreach (var structAccess in typesBeingAccessed.ToArray())
			{
				Func<String, String> replaceStructWithClass = (input) => Regex.Replace(Regex.Replace(input, @"\bstruct", "class"), @"\bStruct", "Class");
				typesBeingAccessed.Add(new ValueCode()
				{
					Initialization = replaceStructWithClass(structAccess.Initialization),
					AccessValue = replaceStructWithClass(structAccess.AccessValue),
				});
			}

			// How many types apprea in the original 'typesBeingAccessed' array, not including the static
			// For each of these types, our composition composes all of them again in different ways
			var typesComposedOnComposition = 7;

			// The class has all the same struct members, and then has more extra class members
			// The letter at the beginning denotes whether its on both 'b', or only the class 'c'
			var compositionMembers = new List<String>()
			{
				"bSDC", // StructDefaultConstructed
				"bSPC", // StructPreConstructed
				"bSPG", // StructPropertyGetter
				"bSDS", // StructDefaultConstructedBySetters

				"cRDC", // ReferenceDefaultConstructed
				"cRPC", // ReferencePreConstructed
				"bRPG", // ReferencePropertyGetter

				"cCDC", // ClassDefaultConstructed
				"cCPC", // ClassPreConstructed
				"cCPG", // ClassPropertyGetter
				"cCDS", // ClassDefaultConsturctedBySetters
			};

			for (var i = 0; i < typesComposedOnComposition; ++i)
			{
				foreach (var compositionMember in compositionMembers)
				{
					var member = compositionMember.Substring(1);

					// All members exist on the class composition version
					typesBeingAccessed.Add(new ValueCode()
					{
						Initialization = "var classComposition_ = ClassComposition();",
						AccessValue = "classComposition_." + member + i,
					});
					
					typesBeingAccessed.Add(new ValueCode()
					{
						Initialization = "var superComposition_ = SuperComposition();",
						AccessValue = "superComposition_.ClassComposition." + member + i,
					});

					typesBeingAccessed.Add(new ValueCode()
					{
						Initialization = "",
						AccessValue = "SuperComposition.ClassComposition." + member + i,
					});


					if (compositionMember[0] == 'b')
					{
						typesBeingAccessed.Add(new ValueCode()
						{
							Initialization = "var structComposition_ = StructComposition();",
							AccessValue = "structComposition_." + member + i,
						});

						typesBeingAccessed.Add(new ValueCode()
						{
							Initialization = "var superComposition_ = SuperComposition();",
							AccessValue = "superComposition_.StructComposition." + member + i,
						});

						typesBeingAccessed.Add(new ValueCode()
						{
							Initialization = "",
							AccessValue = "SuperComposition.StructComposition." + member + i,
						});
					}
				}
			}

			var singleAndCompoundFinalList = new List<ValueCode>()
			{
				new ValueCode()
				{
					Initialization = "var stackLocal_ = 98234756;",
					AccessValue = "stackLocal_",
				},
				new ValueCode()
				{
					Initialization = "",
					AccessValue = "98234756",
				},
			};

			foreach (var typeBeingAccessed in typesBeingAccessed)
			{
				foreach (var member in members)
				{
					// We first initialize the type being accessed, and then the member (some members have init code)
					// Members use a $ to denote where the type being accessed should go within 'AccessValue'
					singleAndCompoundFinalList.Add(new ValueCode()
					{
						Initialization = typeBeingAccessed.Initialization + member.Initialization.Replace("$", typeBeingAccessed.AccessValue),
						AccessValue = member.AccessValue.Replace("$", typeBeingAccessed.AccessValue),
					});
				}
			}

			var builder = new StringBuilder();

			var counter = 0;
			foreach (var finalValueCode in singleAndCompoundFinalList)
			{
				builder.AppendLine("  [Static] function ValueUnitTest" + counter + "() : Integer");
				builder.AppendLine("  {");
				builder.AppendLine("    " + finalValueCode.Initialization);
				builder.AppendLine("    return " + finalValueCode.AccessValue + ";");
				builder.AppendLine("  }");
				++counter;
			}

			var allTests = builder.ToString();
			Clipboard.SetText(allTests);
		}
	}

	class ValueCode
	{
		public String Initialization;
		public String AccessValue;
	}
}
