import sublime;
import sublime_plugin;
import inspect;
import pprint;
import subprocess;
import os;
import tempfile;
import json;
import time;
import cgi;


# TODO:
# Make the run command possibly output to its own window, also async?
# Descriptions on completions, and a real overload resolution list
# Since we have to generate a temporary file for compiling and auto-complete, add a feature that says 'treat this file as another name' - directly to Zilch

class RunZilchCommand(sublime_plugin.ApplicationCommand):
  def run(self):
    window = sublime.active_window();
    if (not window):
      print("RunZilch: A window must be active with Zilch code inside of it");

    view = window.active_view();
    if (not view):
      print("RunZilch: A view must be active with Zilch code inside of it");

    # Create a temporary file with ".zilch" at the end and fill it out with the code we grabbed from the buffer (keep the file around)
    tempFileName = Zilch.save_view_to_temporary_file(view);

    # The command line we're going to execute includes the Zilch executable and any command line parameters we need to query errors
    commandLine = [Zilch.get_executable_path()];

    # We need to tell the command line to compile this file (no command needed, just append the list of files)
    commandLine.append(tempFileName);

    commandLine.append("-Run");
    #commandLine.append("-WaitForDebugger");

    output_text = Zilch.execute_and_get_output(commandLine);

    # The sublime console shows CR as a block character, so remove it
    print("********************************************** Begin Zilch");
    print(output_text.replace("\r", ""));
    sublime.active_window().run_command("show_panel", {"panel": "console", "toggle": True})
    print("********************************************** End Zilch");

class Zilch(sublime_plugin.EventListener):

  #********** Static helper functions **********
  def view_show_instant(self, view, point):
    visibleRegion = view.visible_region();

    # If we can already see the point, then don't bother scrolling
    if (visibleRegion.contains(point)):
      return;

    # Ideally I would take the time to figure out the centering math, but its hurting my brain
    # Just scroll to the position and leave 10 lines of buffer space above it
    lineSpaceAbove = 10;
    layoutVector = view.text_to_layout(point);
    layoutVector = (layoutVector[0], layoutVector[1] - view.line_height() * lineSpaceAbove);
    view.set_viewport_position(layoutVector, False);

  def html_escape(plainText, trim=True):
    if (trim):
      plainText = plainText.strip();
    html = cgi.escape(plainText);
    html = html.replace("\n", "<br>");
    return html;

  def get_executable_path():
    # The executable is located in the same path as this file, then in either a Debug/Release folder, called ZilchMain
    pluginDir = os.path.dirname(os.path.realpath(__file__));
    #buildDir = "Debug";
    buildDir = "Release";
    zilchExecutable = os.path.join(pluginDir, buildDir, "ZilchMain");
    return zilchExecutable;

  def execute_and_get_output(commandLineArray):
    startPerfCounter = time.perf_counter();

    try:
      output = subprocess.check_output(commandLineArray, shell=True)
      output_text = ''.join(map(chr,output));
    except subprocess.CalledProcessError as e:
      output_text = e.output.decode("utf-8");

    endPerfCounter = time.perf_counter();
    elapsedPerformanceTime = endPerfCounter - startPerfCounter;
    print("Execution Time: {:.12f} seconds".format(elapsedPerformanceTime))

    return output_text;

  def is_zilch_view(view):
    syntax = view.settings().get('syntax');
    return syntax and syntax.endswith("Zilch.tmLanguage");

  def save_view_to_temporary_file(view):
    # Because the file is often not yet saved to the disk, we need to copy the buffer in memory to a file
    allCode = view.substr(sublime.Region(0, view.size()));
    
    # Create a temporary file with ".zilch" at the end and fill it out with the code we grabbed from the buffer (keep the file around)
    codeFile = tempfile.NamedTemporaryFile(suffix=".zilch", delete=False);
    tempFileName = codeFile.name;
    codeFile.write(allCode.encode('utf-8'));
    codeFile.close();
    return tempFileName;

  #*********************************************

  def on_post_save(self, view):
    # We only want to do error checking for files using the Zilch syntax
    if (not Zilch.is_zilch_view(view)):
      return;

    # Create a temporary file with ".zilch" at the end and fill it out with the code we grabbed from the buffer (keep the file around)
    tempFileName = Zilch.save_view_to_temporary_file(view);

    # The command line we're going to execute includes the Zilch executable and any command line parameters we need to query errors
    commandLine = [Zilch.get_executable_path()];

    # We need to tell the command line to compile this file (no command needed, just append the list of files)
    commandLine.append(tempFileName);

    commandLine.append("-CompileAndReport");
    #commandLine.append("-WaitForDebugger");

    output_text = Zilch.execute_and_get_output(commandLine);

    jsonObject = json.loads(output_text);

    if (jsonObject["IsError"]):
      #pprint.pprint(inspect.getargspec(view.show_popup));

      start   = view.text_point(jsonObject["StartLine"  ] - 1, jsonObject["StartCharacter"  ] - 1);
      primary = view.text_point(jsonObject["PrimaryLine"] - 1, jsonObject["PrimaryCharacter"] - 1);
      end     = view.text_point(jsonObject["EndLine"    ] - 1, jsonObject["EndCharacter"    ] - 1);

      errorRegion = sublime.Region(start, end);

      # We would like to call 'show', but that animates and there is no way to tell it not to
      # The problem with animation is that 'show_popup' will not actually show if its off screen, so we need instant animation
      self.view_show_instant(view, primary);
      #view.show(errorRegion);

      message = jsonObject["Message"];
      htmlMessage = Zilch.html_escape(message);
      view.show_popup("<style>body { margin: 1px 3px; } html { background-color: #333333; color: #FF0000; }</style><b>^</b> " + htmlMessage, location=primary, max_width=500);

      view.add_regions("ZilchError", [errorRegion], "invalid", "", sublime.DRAW_NO_FILL | sublime.DRAW_NO_OUTLINE | sublime.DRAW_SQUIGGLY_UNDERLINE);
      view.set_status("ZilchError", "File: Fudge, Line 33: Expected a ';' but instead we got a '++'");
      print(jsonObject["FormattedMessage"]);

    return;

  def on_modified(self, view):
    # We only want to clear status and popups for files using the Zilch syntax
    if (not Zilch.is_zilch_view(view)):
      return;

    # If the user types anything in the view, clear the Zilch error status
    view.erase_regions("ZilchError");
    view.erase_status("ZilchError");
    view.hide_popup();

  def on_query_completions(self, view, prefix, locations):
    # We only want to do auto-complete for files using the Zilch syntax
    if (not Zilch.is_zilch_view(view)):
      return;

    # If there is no selection or cursor, we can't do auto-complete
    if (not locations):
      return;

    # The prefix appears to be a string that preceeds the cursor, but when we do '.' this string is empty
    if (prefix != ''):
      return;

    # Only do auto-complete on the first selected cursor (later we can add multi-cursor auto-complete)
    location = locations[0];
    triggerCharacter = view.substr(location - 1);
    
    # Create a temporary file with ".zilch" at the end and fill it out with the code we grabbed from the buffer (keep the file around)
    tempFileName = Zilch.save_view_to_temporary_file(view);
    
    # The command line we're going to execute includes the Zilch executable and any command line parameters we need to query auto-complete
    commandLine = [Zilch.get_executable_path()];

    # We need to tell the command line to compile this file (no command needed, just append the list of files)
    # Note: This should ALWAYS come before any commands due to how the command parser works (any value following a command gets attached to it)
    commandLine.append(tempFileName);

    # Auto complete needs to know the cursor position and the file we're in (Origin means file)
    commandLine.extend(["-AutoCompleteCursor", str(location)]);
    commandLine.extend(["-AutoCompleteOrigin", tempFileName]);
    #commandLine.append("-WaitForDebugger");

    #view.show_popup('hello this is a test of the auto complete window', sublime.COOPERATE_WITH_AUTO_COMPLETE);
    output_text = Zilch.execute_and_get_output(commandLine);
    
    completions = [];
    jsonObject = json.loads(output_text);
    #print(output_text);

    if (triggerCharacter == '('):
      completionOverloadCount = len(jsonObject["CompletionOverloads"]);
      if completionOverloadCount != 0:
        # For now, just show the first description we find, until we can figure out how to get a proper overload list
        bestIndex = jsonObject["BestCompletionOverload"];
        bestCompletionOverload = jsonObject["CompletionOverloads"][bestIndex];

        overloadText = "";
        if (completionOverloadCount > 1):
          overloadText = "\n** Overload [" + str(bestIndex + 1) + " of " + str(completionOverloadCount) + "] **";

        plainPopupText = bestCompletionOverload["Signature"] + overloadText + "\n\n" + bestCompletionOverload["Description"];

        htmlPopupText = Zilch.html_escape(plainPopupText);
        view.show_popup(htmlPopupText, sublime.COOPERATE_WITH_AUTO_COMPLETE, max_width=800);

      # We pretty much never want to show completions here
      return ([], sublime.INHIBIT_WORD_COMPLETIONS);

    else:
      for completionEntry in jsonObject["CompletionEntries"]:
        completionName        = completionEntry["Name"];
        completionDescription = completionEntry["Description"];
        completionType        = completionEntry["Type"];
        completionShortType   = completionEntry["ShortType"];

        completions.append([completionName + "    \t    " + completionShortType, completionName]);

      if (jsonObject["Success"]):
        if (len(completions) == 0):
          completions.append(["(no members)", " "]);
        completions = (completions, sublime.INHIBIT_WORD_COMPLETIONS);

    return completions;
