'use strict';

// includes
var express = require('express');
var app = express();
app.use(express.static('www'));
var server = require('http').Server(app);
var io = require('socket.io')(server);
var SerialPort = require("serialport");
var gpio = require("pi-gpio");

// we will use output pins 35, 36, 37, 38
const FIRST_RELAY_PIN = 35;

// init gpio
for (let i = 0; i < 4; i++) {
  gpio.open(FIRST_RELAY_PIN + i, 'output pullup', function(err) {     // Open pin for output (active low)
    gpio.write(FIRST_RELAY_PIN + i, 1, function() {          // Set pin high (1)
      console.log('Pin ' + (FIRST_RELAY_PIN + i) + ' initialized for output (active low)');
    });
  });
}

// init serialport
var port = new SerialPort("/dev/ttyS0", {
  baudRate: 115200,
  parser: SerialPort.parsers.byteDelimiter([13,10])
});

port.on('data', function (data) {
  console.log('Serial Data: ' + data);
  io.sockets.emit('console', data);
});

// open errors will be emitted as an error event
port.on('error', function(err) {
  console.log('Serial Error: ', err.message);
});


io.on('connection', function(socket){
  console.log('a socket connected @ ' + new Date());
  socket.emit('console', 'cnc log connected @ ' + new Date());

  socket.on('setrelay', (relayNum, state) => {
    console.log('RELAY: ' + relayNum + ': ' + state);
    
    if (state === 'off') {
      gpio.write(FIRST_RELAY_PIN + parseInt(relayNum, 10), 1, function() {
        console.log('Pin ' + FIRST_RELAY_PIN + parseInt(relayNum, 10) + 'set high (off)');
      });
    } else if (state === 'on') {
      gpio.write(FIRST_RELAY_PIN + parseInt(relayNum, 10), 0, function() {
        console.log('Pin ' + FIRST_RELAY_PIN + parseInt(relayNum, 10) + 'set low (on)');
      });
    } else if (state === 'on5') {
      gpio.write(FIRST_RELAY_PIN + parseInt(relayNum, 10), 0, function() {
        console.log('Pin ' + FIRST_RELAY_PIN + parseInt(relayNum, 10) + 'set low (on)');
        console.log('Setting timer for 5 seconds to turn back off...');

        setTimeout(function () {
          gpio.write(FIRST_RELAY_PIN + parseInt(relayNum, 10), 1, function() {
            console.log('Pin ' + FIRST_RELAY_PIN + parseInt(relayNum, 10) + 'set high (off)');
          });
        }, 5000);
      });
    }
  });

  socket.on('setcmd', (cmd) => {
    console.log('CMD: ' + cmd);

    if (port.isOpen()) {
      port.write(cmd + "\r\n", function(err) {
        if (err) {
          return console.log('Error on serial write: ', err.message);
        }
        console.log('Command written to serial port');
      });
    }
  });
});

// fire up the http server
server.listen(3000);



