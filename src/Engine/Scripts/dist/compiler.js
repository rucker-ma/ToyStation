"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.startWatch = void 0;
const fs = __importStar(require("node:fs"));
const node_path_1 = __importDefault(require("node:path"));
const ts = __importStar(require("typescript"));
const loader_1 = require("./loader");
function watch(rootFileNames, options) {
    const files = {};
    rootFileNames.forEach(fileName => {
        files[fileName] = { version: 0 };
    });
    const servicesHost = {
        getScriptFileNames: () => rootFileNames,
        getScriptVersion: (fileName) => {
            if (fileName in files) {
                return files[fileName].version.toString();
            }
            else {
                return "0";
            }
        },
        getScriptSnapshot: fileName => {
            let path = fileName;
            if (fileName in files) {
                path = options.baseUrl + fileName;
            }
            if (!fs.existsSync(path)) {
                console.log(path + " is not exist");
                return undefined;
            }
            return ts.ScriptSnapshot.fromString(fs.readFileSync(path).toString());
        },
        getCurrentDirectory: () => process.cwd(),
        getCompilationSettings: () => options,
        getDefaultLibFileName: options => ts.getDefaultLibFilePath(options),
        fileExists: ts.sys.fileExists,
        readFile: ts.sys.readFile,
        readDirectory: ts.sys.readDirectory,
        directoryExists: ts.sys.directoryExists,
        getDirectories: ts.sys.getDirectories,
    };
    const services = ts.createLanguageService(servicesHost, ts.createDocumentRegistry());
    rootFileNames.forEach(fileName => {
        emitFile(fileName);
        let filePath = options.baseUrl + fileName;
        console.log(process.cwd());
        console.log("watch path: " + filePath);
        fs.watchFile(filePath, { persistent: true, interval: 250 }, (curr, prev) => {
            console.log(filePath + " changed!");
            if (+curr.mtime <= +prev.mtime) {
                return;
            }
            files[fileName].version++;
            emitFile(fileName);
        });
    });
    function emitFile(fileName) {
        let output = services.getEmitOutput(fileName);
        if (!output.emitSkipped) {
            console.log(`Emitting ${fileName}`);
        }
        else {
            console.log(`Emitting ${fileName} failed`);
            logErrors(fileName);
        }
        output.outputFiles.forEach(o => {
            console.log("  write file: " + o.name);
            fs.writeFileSync(o.name, o.text, "utf8");
            if (o.name.endsWith(".js.map")) {
                return;
            }
            let relative_path = "./" + node_path_1.default.relative(options.outDir, o.name);
            console.log(process.cwd());
            Promise.resolve(`${relative_path}`).then(s => __importStar(require(s))).then(loader_1.LoadCompiledModule)
                .catch(err => {
                console.log(err);
            });
        });
    }
    function logErrors(fileName) {
        let allDiagnostics = services
            .getCompilerOptionsDiagnostics()
            .concat(services.getSyntacticDiagnostics(fileName))
            .concat(services.getSemanticDiagnostics(fileName));
        allDiagnostics.forEach(diagnostic => {
            let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
            if (diagnostic.file) {
                let { line, character } = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
                console.log(`  Error ${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
            }
            else {
                console.log(`  Error: ${message}`);
            }
        });
    }
}
function startWatch() {
    const options = {
        module: ts.ModuleKind.CommonJS,
        sourceMap: true,
        incremental: true,
        outDir: "./dist/",
        baseUrl: "./content/",
        removeComments: true
    };
    const currentDirectoryFiles = fs
        .readdirSync(options.baseUrl)
        .filter(fileName => fileName.length >= 3 && fileName.substr(fileName.length - 3, 3) === ".ts");
    currentDirectoryFiles.forEach((value, index, array) => {
        array[index] = value;
    });
    watch(currentDirectoryFiles, options);
}
exports.startWatch = startWatch;
//# sourceMappingURL=compiler.js.map