import { createRequire } from "module";
import { fileURLToPath } from 'url';
import path from 'path';

const require = createRequire(import.meta.url);

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const express = require('express');
const http = require('http');
const { Server } = require("socket.io");
const { v4: uuidv4 } = require('uuid');

////////////// Start ///////////////////////////////

const app = express();
const server = http.createServer(app);

app.use(express.static(path.join(__dirname, "client")));

const io = new Server(server);

const avatars = [
    'assets/models/CyberbotGold.glb',
    'assets/models/CyberbotGreen.glb',
    'assets/models/CyberbotPink.glb',
    'assets/models/CyberbotRed.glb',
    'assets/models/CyberbotSilver.glb',
    'assets/models/CyberbotYellow2.glb',
    'assets/models/character.glb'
];    

let users = {};
function emit_fullstatus()
{
    io.emit('full_status', users);
}

io.on("connection", (socket) => {
    const ip = socket.handshake.headers['x-forwarded-for'] || socket.conn.remoteAddress.split(":")[3];
    console.log("New client connected: "+ip);

    let user = null;

    socket.on('identity', () => {
        user = {};
        user.username = uuidv4();

        let model_idx = Math.floor(Math.random() * 6);
        user.src_avatar = avatars[model_idx];

        user.url_anim = "assets/models/Animations.glb";
        user.name_idle = "idle";
        user.name_forward = "walk_forward";
        user.name_backward = "walk_backward";
        socket.emit("identity", user);

        users[user.username] = user;
        emit_fullstatus();
    });

    socket.on('avatar_status', (avatar_status)=>{
        if (user)
        {
            user.state = avatar_status.state;
            user.position = avatar_status.position;
            user.quaternion = avatar_status.quaternion;
            socket.broadcast.emit('avatar_status', avatar_status);
        }

    });


    socket.on('disconnect',  () => {
        delete users[user.username];
        user = null;
        emit_fullstatus();
        console.log('Client has disconnected: '+ip);
    });
});


server.listen(8888, () => {    
    console.log('listening on *:8888');
});



