import * as Toy from "toy";

// class FirstObject extends Toy.TObject{

//     Tick(): void {
//         console.log("First Object tick...");
//     }
// }


class FirstObject {
    constructor(){
        console.log("create object");
    }
    Tick(): void {
        console.log("First Object tick...");
    }
}
export default FirstObject;