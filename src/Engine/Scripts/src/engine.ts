class Engine{

    start(){
        setInterval(()=>this.tick(),100);
    }
    tick(){
        console.log("tick...");
    }
}

export default Engine;