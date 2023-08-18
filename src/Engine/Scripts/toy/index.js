'use strict'
let typeCache = Object.create(null);
let scriptTypeCache = {};
var Toy = new Proxy(typeCache, {
    get: function (target, name,receiver) {
        if(name == "__esModule"){
            // return {};
            return null;
        }
        if(name == "scriptTypeCache"){
            return scriptTypeCache;
        }
        if (!(name in target)) {
            //for test
            target[name] = "name";

            // target[name] = _toy_findModule(name);
            console.log("wait to load");
        }
        return target[name];
    },
    defineProperty:function(target,property,attributes){
        Object.defineProperty(target,property,attributes);
        return true;
    }
});
// Object.defineProperty(Toy,"scriptTypeCache",scriptTypeCache);
Toy.scriptTypeCache = scriptTypeCache;
module.exports=Toy;
global.Toy=Toy;