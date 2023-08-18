
import { startWatch } from "./compiler";
import { startServer } from "./service_demo";
import * as Toy from "toy";

console.log(process.cwd());
startWatch();
startServer();
