import * as Toy from "toy";

export function LoadCompiledModule(module) {
  var type = module.default;
  let name = type.name;
  console.log("get object type: ",name);
  //将对象类型保存下来
  Toy.scriptTypeCache[type.name] = type;  
}