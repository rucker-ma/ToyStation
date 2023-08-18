import * as http from "node:http";
import * as url from "url";
import * as Toy from "toy";

var server: http.Server = http.createServer((req, res) => {
    res.setHeader("Access-Control-Allow-Origin","*");
    res.setHeader("Access-Control-Allow-Headers","GET,PUT,POST,DELETE,OPTIONS");
    res.setHeader("Access-Control-Allow-Headers","Content-Type,request-origin");
    if (req.url != "./favicon.ico") {
        let client_url = url.parse(req.url).query;

    }
});

export function startServer() {
    if (!server.listening) {
        server.listen(6666, '127.0.0.1', () => {
            console.log("服务已启动...");
        })
    }
};

