const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const WebSocket = require('ws');
const express = require('express');
const path = require('path');
const fs = require('fs');
const { exec } = require('child_process');

// ========== EXPRESS ==========
const app = express();
app.use(express.static('/home/brbqueue/brbqueue-nu'));

app.get('/keyboard/show', (req, res) => {
    exec('DISPLAY=:0 squeekboard &', (err) => { if (err) console.log('KB error:', err); });
    res.send('ok');
});
app.get('/keyboard/hide', (req, res) => {
    exec('pkill squeekboard', (err) => { if (err) console.log('KB hide error:', err); });
    res.send('ok');
});

app.listen(3000, () => console.log('Kiosk: http://localhost:3000'));

// ========== GPIO — PLATE WEIGHT SENSOR ONLY ==========
const PLATE_IR_PIN = 596;
let plateNormal = true;
let gpioReady = false;

const pinPath = `/sys/class/gpio/gpio${PLATE_IR_PIN}`;
if (!fs.existsSync(pinPath)) {
    try { fs.writeFileSync('/sys/class/gpio/export', PLATE_IR_PIN.toString()); }
    catch (e) { /* already exported */ }
}

setTimeout(() => {
    try { fs.writeFileSync(`/sys/class/gpio/gpio${PLATE_IR_PIN}/direction`, 'in'); }
    catch (e) { console.log('GPIO direction error:', e.message); }
    gpioReady = true;
    console.log('Plate sensor ready — polling every 100ms');
}, 500);

setInterval(() => {
    if (!gpioReady) return;
    try {
        const plateVal = fs.readFileSync(`/sys/class/gpio/gpio${PLATE_IR_PIN}/value`, 'utf8').trim();
        const newPlateNormal = (plateVal === '0');
        if (newPlateNormal !== plateNormal) {
            plateNormal = newPlateNormal;
            console.log(newPlateNormal ? 'PLATE NORMAL' : 'PLATE DROPPED');
            broadcast({ type: 'plateStatus', normal: plateNormal });
            sendToArduino(newPlateNormal ? 'PLATE_NORMAL' : 'PLATE_DROPPED');
        }
    } catch (e) {}
}, 100);

app.get('/api/plate-status', (req, res) => {
    res.json({ normal: plateNormal });
});

// ========== VARIABLES ==========
let arduinoPort = null;
let wsServer = null;
let clients = [];
let reconnectTimer = null;

function broadcast(message) {
    const data = JSON.stringify(message);
    clients.forEach((ws, index) => {
        if (ws.readyState === WebSocket.OPEN) ws.send(data);
        else clients.splice(index, 1);
    });
}

function findArduino() {
    if (reconnectTimer) clearTimeout(reconnectTimer);
    SerialPort.list().then(ports => {
        const arduino = ports.find(p =>
            (p.manufacturer && p.manufacturer?.toLowerCase().includes('arduino')) ||
            (p.manufacturer && p.manufacturer?.toLowerCase().includes('ch340')) ||
            p.path.includes('ttyUSB') || p.path.includes('ttyACM')
        );
        if (arduino) { connectArduino(arduino.path); }
        else { reconnectTimer = setTimeout(findArduino, 3000); }
    }).catch(() => { reconnectTimer = setTimeout(findArduino, 3000); });
}

function connectArduino(portPath) {
    if (arduinoPort?.isOpen) arduinoPort.close();
    arduinoPort = new SerialPort({ path: portPath, baudRate: 9600 });
    const parser = arduinoPort.pipe(new ReadlineParser({ delimiter: '\n' }));
    arduinoPort.on('open', () => { broadcast({ type: 'status', connected: true }); });
    arduinoPort.on('error', () => { arduinoPort = null; broadcast({ type: 'status', connected: false }); });
    arduinoPort.on('close', () => { arduinoPort = null; broadcast({ type: 'status', connected: false }); });
    parser.on('data', (data) => {
        const trimmed = data.trim();
        if (trimmed) broadcast({ type: 'arduino', data: trimmed });
    });
}

function sendToArduino(cmd) {
    if (arduinoPort?.isOpen) arduinoPort.write(cmd + '\n');
}

// ========== WEBSOCKET ==========
wsServer = new WebSocket.Server({ port: 8080 });
wsServer.on('connection', (ws) => {
    clients.push(ws);
    ws.send(JSON.stringify({ type: 'status', connected: arduinoPort?.isOpen || false }));
    ws.send(JSON.stringify({ type: 'plateStatus', normal: plateNormal }));
    ws.on('message', (message) => { sendToArduino(message.toString().trim()); });
    ws.on('close', () => { clients = clients.filter(c => c !== ws); });
});

findArduino();

process.on('SIGINT', () => {
    try { fs.writeFileSync('/sys/class/gpio/unexport', PLATE_IR_PIN.toString()); } catch(e) {}
    if (arduinoPort?.isOpen) arduinoPort.close();
    if (wsServer) wsServer.close();
    process.exit(0);
});