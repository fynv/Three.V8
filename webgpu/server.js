import { createRequire } from "module";
import { fileURLToPath } from 'url';
import path from 'path';

const require = createRequire(import.meta.url);

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const express = require('express');
const http = require('http');

////////////// Start ///////////////////////////////

const app = express();
const server = http.createServer(app);

app.use(express.static(path.join(__dirname, "client")));


server.listen(8888, () => {
    console.log('listening on *:8888');
});



