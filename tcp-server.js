const net = require("net");

const LOCAL_PORT = 16379;

const server = net.createServer((socket) => {
    socket.on("data", (data) => {
        console.log(data.length, data.toString("utf-8"));
    });
});

server.on("error", (err) => {
    console.log(err);
});

server.listen(LOCAL_PORT, (err) => {
    if (err) {
        console.log(err);
    } else {
        console.log(`server listen on ${LOCAL_PORT}`);
    }
});
