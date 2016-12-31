'use strict';

// includes
var express = require('express');
var app = express();
app.use(express.static('www'));
var server = require('http').Server(app);
var io = require('socket.io')(server);



io.on('connection', function(socket){
  console.log('a socket connected');

  for (let i = 0; i < 100; i++) {
    socket.emit('console', 'cnc log @ ' + new Date());
  }

  socket.on('setrelay', (relayNum, state) => {
    console.log('RELAY: ' + relayNum + ': ' + state);
  });

  socket.on('setcmd', (cmd) => {
    console.log('CMD: ' + cmd);
  });
});




// import serialport
/*var SerialPort = require("serialport");
var port = new SerialPort("/dev/ttyS0", {
  baudRate: 115200
});

port.on('open', function() {
  port.write('main screen turn on', function(err) {
    if (err) {
      return console.log('Error on write: ', err.message);
    }
    console.log('message written');
  });
});

// open errors will be emitted as an error event
port.on('error', function(err) {
  console.log('Error: ', err.message);
});*/





// fire up the http server
server.listen(3000);



