using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.CodeDom.Compiler;
using System.CodeDom;
using System.Text.RegularExpressions;
using SquishIt.Framework;

namespace MergeHtmlCssJs
{
	enum ExternalLinkType
	{
		Stylesheet = 1,
		Javascript = 2,
		StylesheetImport = 3,
	}

	public class Merger
	{
		Regex ExternalLink = new Regex("<link rel=\"stylesheet\" href=\"(.*?)\".*?/?>|<script src=\"(.*?)\".*?></script>|@import url\\(\"(.*?)\"\\)");

		public string Merge(string file)
		{
			string fileText = File.ReadAllText(file);

			var builder = new StringBuilder();

			this.RecursiveInclude(fileText, Path.GetDirectoryName(file), builder);

			return builder.ToString();
		}

		public void RecursiveInclude(string parentFileText, string parentDirectoryPath, StringBuilder builder)
		{
			var lastIndex = 0;
			var matches = this.ExternalLink.Matches(parentFileText);
			foreach (Match match in matches)
			{
				var externalType = ExternalLinkType.Stylesheet;
				String relativeFilePath = null;
				for (var i = 1; i <= 3; ++i)
				{
					var matchGroupText = match.Groups[i].Value;
					if (String.IsNullOrWhiteSpace(matchGroupText) == false)
					{
						relativeFilePath = matchGroupText;
						externalType = (ExternalLinkType)i;
						break;
					}
				}
				
				builder.Append(parentFileText.Substring(lastIndex, match.Index - lastIndex));

				var filePath = Path.Combine(parentDirectoryPath, relativeFilePath);
				var includedFileText = File.ReadAllText(filePath);

				if (externalType == ExternalLinkType.Javascript)
				{
					var minifier = new Yahoo.Yui.Compressor.JavaScriptCompressor();
					var minifiedJavascript = minifier.Compress(includedFileText);
					builder.Append("\n<script>\n");
					builder.Append(minifiedJavascript);
					builder.Append("\n</script>\n");
				}
				else if (externalType == ExternalLinkType.Stylesheet)
				{
					var minifier = new Yahoo.Yui.Compressor.CssCompressor();
					var minifiedCss = minifier.Compress(includedFileText);
					builder.Append("\n<style>\n");
					this.RecursiveInclude(minifiedCss, Path.GetDirectoryName(filePath), builder);
					builder.Append("\n</style>\n");
				}
				else
				{
					this.RecursiveInclude(includedFileText, Path.GetDirectoryName(filePath), builder);
				}



				lastIndex = match.Index + match.Length;
			}

			// Append the rest of the text
			// In the event that no matches were found, this will append the entire file
			builder.Append(parentFileText.Substring(lastIndex, parentFileText.Length - lastIndex));
		}

		public static string ToLiteral(string input)
		{
			using (var writer = new StringWriter())
			{
				using (var provider = CodeDomProvider.CreateProvider("CSharp"))
				{
					provider.GenerateCodeFromExpression(new CodePrimitiveExpression(input), writer, null);
					return writer.ToString();
				}
			}
		}
	}

}
