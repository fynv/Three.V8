import { createRequire } from "module";
import { EventDispatcher } from "./EventDispatcher.js";

const require = createRequire(import.meta.url);
const { Server } = require('ws');
const { v4: uuidv4 } = require('uuid');

///// Start //////

class MsgDispatcher extends EventDispatcher
{
    constructor(ws)
    {
        super();
        this.ws = ws;
        this.stage = 0;
        this.key = "";
        ws.on('message', (msg) => this.onMsg(msg));
    }
    
    send(key, value)
    {
        this.ws.send(key);
        this.ws.send(value);
    }
    
    onMsg(msg)
    {
        if (this.stage == 0)
        {
            this.key = msg;
            this.stage = 1;
        }
        else if (this.stage == 1)
        {
            const event = {
                type: this.key,
                data: msg
            };            
            this.dispatchEvent(event);
            this.key = "";
            this.stage = 0;
        }
    }
    
    sendObj(key, obj)
    {
        this.ws.send(key);
        this.ws.send(JSON.stringify(obj));
    }
    
    addEventListenerObj(key, callback)
    {
        this.addEventListener(key, (event)=>{
            let obj = JSON.parse(event.data);
            callback(obj);                        
        });
    } 
}

const ws_server = new Server({ port: 8888 });

class Room
{
    constructor()
    {
        this.users = {};
        this.messengers = {};        
    }
    
    add_user(user, messenger)
    {
        this.users[user.username] = user;
        this.messengers[user.username] = messenger;
    }
    
    remove_user(user)
    {
        delete this.users[user.username];
        delete this.messengers[user.username];
    }
    
    broadcast(key, value, sender = null)
    {
        for (let username in this.messengers)
        {
            if (sender===null || username != sender.username)
            {
                this.messengers[username].send(key, value);
            }
        }
    }
    
    broadcastObj(key, obj, sender = null)
    {
        for (let username in this.messengers)
        {
            if (sender===null || username != sender.username)
            {
                this.messengers[username].sendObj(key, obj);
            }
        }
    }
    
    broadcastUsers()
    {
        this.broadcastObj("users", this.users);
    }
    
}

const rooms = {};

ws_server.on('connection', (ws, req) => {
    const ip = req.headers['x-forwarded-for'] || req.socket.remoteAddress.split(":")[3];
    console.log('New client connected: '+ip);
    
    let user = null;
    let room = null;

    const messenger = new MsgDispatcher(ws);

    messenger.addEventListener("join_room", (event)=>{
        let room_id = "" + event.data;
        user = {};
        user.username = uuidv4();        
        messenger.sendObj("identity", user);
        
        if (!(room_id in rooms))
        {
            rooms[room_id] = new Room();
        }
        
        room = rooms[room_id];        
        room.add_user(user, messenger);
        room.broadcastUsers();
    });
    
    messenger.addEventListener("audio", (event)=>{        
        room.broadcast(user.username+'_audio', event.data, user);
    });


    ws.on('close', () => {
        room.remove_user(user);
        user = null;
        room.broadcastUsers();
        console.log('Client has disconnected: '+ip);
    });
});




