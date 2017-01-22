'use strict';

// includes
var express = require('express');
var app = express();
app.use(express.static('www'));
var server = require('http').Server(app);
var io = require('socket.io')(server);
var SerialPort = require("serialport");
var Gpio = require('onoff').Gpio;

// we will use output pins BCM17/18/27/22 (RPi Physical Pins 11/12/13/15)
const RELAY_PINS = [17, 18, 27, 22];
let relays = [];

// init gpio (defaulted to pulldown)
for (let i = 0; i < 4; i ++) {
  relays.push(new Gpio(RELAY_PINS[i], 'out'));
}

// init serialport
var port = new SerialPort("/dev/serial0", {
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
    const relayIndex = relayNum - 1;
    
    if (state === 'off') {
      relays[relayIndex].write(0, function (err) {
        if (err) {
          console.log('Relay ' + relayNum + ' err: ' + err);
        } else {
          console.log('Relay ' + relayNum + ' set low (off)');
        }
      });
    } else if (state === 'on') {
      relays[relayIndex].write(1, function (err) {
        if (err) {
          console.log('Relay ' + relayNum + ' err: ' + err);
        } else {
          console.log('Relay ' + relayNum + ' set high (on)');
        }
      });
    } else if (state === 'on5') {
      relays[relayIndex].write(1, function (err) {
        if (err) {
          console.log('Relay ' + relayNum + ' err: ' + err);
        } else {
          console.log('Relay ' + relayNum + ' set high (on)');
          console.log('Setting timer for 5 seconds to turn back off...');

          setTimeout(function () {
            relays[relayIndex].write(0, function (err) {
              if (err) {
                console.log('Relay ' + relayNum + ' err: ' + err);
              } else {
                console.log('Relay ' + relayNum + ' set low (off)');
              }
            });
          }, 5000);
        }
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



