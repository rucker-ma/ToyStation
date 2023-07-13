"use strict";
console.log(process.cwd());

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};


Object.defineProperty(exports, "__esModule", { value: true });
const engine_1 = __importDefault(require("./engine"));
let engine = new engine_1.default();
console.log(process.cwd());
engine.start();
//# sourceMappingURL=entry.js.map