#!/usr/bin/env node

/**
 * Module dependencies.
 */

var app = require('../app');
var debug = require('debug')('iotserver:server');
var http = require('http');

/**
 * Get port from environment and store in Express.
 */
var port = normalizePort(process.env.PORT || '3000');
app.set('port', port);
/**
 * Create HTTP server.
 */
var server = http.createServer(app);

//mongoDB 접속 (mqtt보다 선행)
var mongoDB=require("mongodb").MongoClient;
var url = "mongodb://127.0.0.1:27017/iot";
var dbObj = null; 
//db접속정보

mongoDB.connect(url, function(err, db){
	dbObj=db;
	console.log("DB Connect success");
	//mongodb 연결 후 err, db정보 받고, db를 dbObj에 넘김.
});



//MQTT 서버 접속, DHT11 센서 데이터 읽기
var mqtt=require("mqtt");
var client=mqtt.connect("mqtt://165.132.45.240");
client.on("connect" , function(){
	client.subscribe("dht11");	
});

client.on("message", function(topic, message){
	   var obj=JSON.parse(message);
	   obj.created_at=new Date();
	   console.log(obj);
	  //mongodb에 dht11센서 정보를 저장하는 파트
	   var dht11=dbObj.collection("dht11");
	   //dbObj에 dht11의 정보를 collection해서 넣고,
	   //
	   dht11.save(obj, function(err, result){
		   //result -> 성공여부
		   if(err) console.log(err);
		   else console.log(JSON.stringify(result));
		   //{"n":1, "ok":1} 이런형태로 값이 넘어옴.
		   //에러가 넘어오면 에러송출, 에러안뜨면 result를 그대로 스트링화해서 출력
	   });//db연결부분까지 완성
});

/**
 * Listen on provided port, on all network interfaces.
 */

server.listen(port);
server.on('error', onError);
server.on('listening', onListening);

/**
 * Normalize a port into a number, string, or false.
 */

function normalizePort(val) {
  var port = parseInt(val, 10);

  if (isNaN(port)) {
    // named pipe
    return val;
  }

  if (port >= 0) {
    // port number
    return port;
  }

  return false;
}

/**
 * Event listener for HTTP server "error" event.
 */

function onError(error) {
  if (error.syscall !== 'listen') {
    throw error;
  }

  var bind = typeof port === 'string'
    ? 'Pipe ' + port
    : 'Port ' + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
    case 'EACCES':
      console.error(bind + ' requires elevated privileges');
      process.exit(1);
      break;
    case 'EADDRINUSE':
      console.error(bind + ' is already in use');
      process.exit(1);
      break;
    default:
      throw error;
  }
}

/**
 * Event listener for HTTP server "listening" event.
 */

function onListening() {
  var addr = server.address();
  var bind = typeof addr === 'string'
    ? 'pipe ' + addr
    : 'port ' + addr.port;
  debug('Listening on ' + bind);
}
