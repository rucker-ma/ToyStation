"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
class Engine {
    start() {
        setInterval(() => this.tick(), 100);
    }
    tick() {
        console.log("tick...");
    }
}
exports.default = Engine;
//# sourceMappingURL=engine.js.map