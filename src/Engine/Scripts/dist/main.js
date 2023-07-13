"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Main = void 0;
var function_1 = require("./function");
var Main = (function () {
    function Main() {
        this._func = new function_1.Function();
    }
    Main.prototype.write = function () {
        console.log('wwwwww');
    };
    return Main;
}());
exports.Main = Main;
//# sourceMappingURL=main.js.map