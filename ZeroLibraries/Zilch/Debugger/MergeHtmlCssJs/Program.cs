using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace MergeHtmlCssJs
{
	class Program
	{
		static void Main(string[] args)
		{
			var merger = new Merger();
			var text = merger.Merge(@"..\..\..\debugger.html");
			//var literal = Merger.ToLiteral(text);
			//literal = literal.Replace(" +\r\n", "\r\n");

			File.WriteAllText(@"..\..\..\..\..\Tools\ZilchDebugger.html", text);

		}
	}
}
