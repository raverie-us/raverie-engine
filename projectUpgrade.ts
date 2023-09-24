import fs from "fs";
import path from "path";

if (process.argv[2]) {
  const projectDir = process.argv[2];

  const updateMetaFile = (filePath: string) => {
    const text = fs.readFileSync(filePath, "utf8");
    let newText = text;
    newText = newText.replace(/ZilchScriptBuilder/gum, "RaverieScriptBuilder");

    if (newText !== text) {
      console.log("Updated meta", filePath);
      fs.writeFileSync(filePath, newText, "utf8");
    }
  }

  const updateScriptFile = (filePath: string) => {
    const text = fs.readFileSync(filePath, "utf8");
    let newText = text;
    newText = newText.replace(/\bZero\.\b/gum, "Raverie.");
    newText = newText.replace(/ZeroObject/gum, "RaverieObject");
    newText = newText.replace(/Zilch/gum, "Raverie");


    if (newText !== text) {
      console.log("Updated script", filePath);
      fs.writeFileSync(filePath, newText, "utf8");
    }
  }

  const applyRenames = (dir: string) => {
    for (const fileName of fs.readdirSync(dir)) {
      const oldPath = path.join(dir, fileName);

      if (fs.statSync(oldPath).isDirectory()) {
        applyRenames(oldPath);
        continue;
      }

      const fileNameBase = fileName.substring(0, fileName.indexOf("."));

      if (fileName.endsWith(".meta")) {
        updateMetaFile(oldPath);
      }
      if (fileName.endsWith(".raveriescript") || fileName.endsWith(".zilchscript") || fileName.endsWith(".z")) {
        updateScriptFile(oldPath);
      }

      let newFileName = fileName;
      if (fileName.endsWith(".zeroproj")) {
        newFileName = "Project.raverieproj";
      }
      if (fileName.endsWith(".zilchscript")) {
        newFileName = `${fileNameBase}.raveriescript`;
      }
      if (fileName.endsWith(".zilchscript.meta")) {
        newFileName = `${fileNameBase}.raveriescript.meta`;
      }
      if (fileName.endsWith(".z")) {
        newFileName = `${fileNameBase}.raveriescript`;
      }
      if (fileName.endsWith(".z.meta")) {
        newFileName = `${fileNameBase}.raveriescript.meta`;
      }
      if (fileName.endsWith(".zilchfrag")) {
        newFileName = `${fileNameBase}.raveriefrag`;
      }
      if (fileName.endsWith(".zilchfrag.meta")) {
        newFileName = `${fileNameBase}.raveriefrag.meta`;
      }
      if (fileName.endsWith(".zilchFrag")) {
        newFileName = `${fileNameBase}.raveriefrag`;
      }
      if (fileName.endsWith(".zilchFrag.meta")) {
        newFileName = `${fileNameBase}.raveriefrag.meta`;
      }

      if (newFileName !== fileName) {
        const newPath = path.join(dir, newFileName);
        console.log("RENAME", oldPath, newPath);
        fs.renameSync(oldPath, newPath);
      }
    }
  }

  applyRenames(projectDir);
}
