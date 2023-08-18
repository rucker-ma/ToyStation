"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const compiler_1 = require("./compiler");
const service_demo_1 = require("./service_demo");
console.log(process.cwd());
(0, compiler_1.startWatch)();
(0, service_demo_1.startServer)();
//# sourceMappingURL=entry.js.map