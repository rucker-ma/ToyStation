import * as fs from "node:fs";
import * as ts from "typescript";

let current_dir = process.cwd();
function watch(rootFileNames: string[], options: ts.CompilerOptions) {
  const files: ts.MapLike<{ version: number }> = {};

  // initialize the list of files
  rootFileNames.forEach(fileName => {
    files[fileName] = { version: 0 };
  });

  // Create the language service host to allow the LS to communicate with the host
  const servicesHost: ts.LanguageServiceHost = {
    getScriptFileNames: () => rootFileNames,
    getScriptVersion: (fileName) => {
      // 检查文件的版本变更，`watchFile`中已经变更了版本，ts会和内部的缓存比较，
      // 有了变更后才会读取文本进行编译
      if (fileName in files) {
        return files[fileName].version.toString()
      } else {
        return "0";
      }
    },
    getScriptSnapshot: fileName => {
      let path = fileName;
      if(fileName in files){
        path = options.baseUrl+fileName;
      }
      if (!fs.existsSync(path)) {
        console.log(path +" is not exist");
        return undefined;
      }

      return ts.ScriptSnapshot.fromString(fs.readFileSync(path).toString());
    },
     getCurrentDirectory: () => current_dir,
    //getCurrentDirectory: () => process.cwd(),
    getCompilationSettings: () => options,
    getDefaultLibFileName: options => ts.getDefaultLibFilePath(options),
    fileExists: ts.sys.fileExists,
    readFile: ts.sys.readFile,
    readDirectory: ts.sys.readDirectory,
    directoryExists: ts.sys.directoryExists,
    getDirectories: ts.sys.getDirectories,
  };

  // Create the language service files
  const services = ts.createLanguageService(servicesHost, ts.createDocumentRegistry());

  // Now let's watch the files
  rootFileNames.forEach(fileName => { 
    // First time around, emit all files
    emitFile(fileName);

    // Add a watch on the file to handle next change
    let filePath = options.baseUrl+fileName; 
    console.log(process.cwd());
    console.log("watch path: " + filePath);
  

    fs.watchFile(filePath, { persistent: true, interval: 250 }, (curr, prev) => {
      // Check timestamp
      console.log(filePath + " changed!");
      if (+curr.mtime <= +prev.mtime) {
        return;
      }
      // Update the version to signal a change in the file
      files[fileName].version++;
      // write the changes to disk
      emitFile(fileName);

    });
  });
  // fs.watch(options.baseUrl,{ persistent: true, recursive: true }, (event:fs.WatchEventType, file) => {
  //   // Check timestamp
   
  //   if(event == 'change'){
  //     console.log(file + " changed");
  //   // if (+curr.mtime <= +prev.mtime) {
  //   //   return;
  //   // }

  //   // Update the version to signal a change in the file
  //   files[file].version++;

  //   // const program = services.getProgram()

  //   // write the changes to disk
  //   emitFile(file);

  //   }
  // });
  function emitFile(fileName: string) {

    let output = services.getEmitOutput(fileName);

    if (!output.emitSkipped) {
      console.log(`Emitting ${fileName}`);
    } else {
      console.log(`Emitting ${fileName} failed`);
      logErrors(fileName);
    }

    output.outputFiles.forEach(o => {
      console.log("  write file: "+o.name);
      fs.writeFileSync(o.name, o.text, "utf8");
    });
  }

  function logErrors(fileName: string) {
    let allDiagnostics = services
      .getCompilerOptionsDiagnostics()
      .concat(services.getSyntacticDiagnostics(fileName))
      .concat(services.getSemanticDiagnostics(fileName));

    allDiagnostics.forEach(diagnostic => {
      let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
      if (diagnostic.file) {
        let { line, character } = diagnostic.file.getLineAndCharacterOfPosition(
          diagnostic.start!
        );
        console.log(`  Error ${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
      } else {
        console.log(`  Error: ${message}`);
      }
    });
  }
}

console.log(current_dir);

const options: ts.CompilerOptions = {
  module: ts.ModuleKind.CommonJS,
  sourceMap: true,
  incremental: true,
  outDir:"./dist/",
  baseUrl:"./content/",
  removeComments:true
  
}

const currentDirectoryFiles = fs
.readdirSync(options.baseUrl)
.filter(fileName => fileName.length >= 3 && fileName.substr(fileName.length - 3, 3) === ".ts");

currentDirectoryFiles.forEach((value: string, index: number, array: string[]) => {
  array[index] = value
});
// Start the watcher

watch(currentDirectoryFiles, options);

// process.chdir("dist");


