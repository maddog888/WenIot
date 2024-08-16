//Web socket 库
const WebSocket = require('ws');
const http = require('http');
//Mqtt 库
const aedes = require('aedes')({
  persistence: null, // 禁用持久性存储
});
const server = require('net').createServer(aedes.handle);
//Mqtt 客户端库
const mqtt = require('mqtt');

/*
//Mysql 数据库库
const mysql = require('mysql2');
//配置连接信息
const sqlHost = ""; //数据库地址
const sqlPort = 3306; //数据库端口
const sqlUserName = ""; //数据库用户名
const sqlPassWord = ""; //数据库密码
const sqlDatabase = ""; //数据库名
//在此之前，请先将数据库文件导入数据库中
*/

//端口配置
const wport = 8083; // Mqtt端口
const port = 10080; // Mqtt端口

// 创建 HTTP 服务器
const aserver = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('WebSocket Server is running');
});

// 创建 WebSocket 服务器并将其附加到 HTTP 服务器
const wss = new WebSocket.Server({ server: aserver }); // 使用 "server" 选项

// WebSocket 连接建立时的处理函数
wss.on('connection', (ws, request) => {
  // 从连接请求的 URL 中提取用户名和密码
  const url = new URL(request.url, 'mqtt://localhost');
  const username = url.searchParams.get('u');
  const password = url.searchParams.get('p');
  const clientId = url.searchParams.get('cid');
  const subscribe = url.searchParams.get('s');
  const sendsubscribe = url.searchParams.get('ss');

  //连接当前Mqtt服务器进行监听
  const client = mqtt.connect("mqtt://localhost:"+port, {
    username: username,
    password: password,
    clientId: clientId ? clientId : "",
    subscribe: subscribe,
    sendsubscribe: sendsubscribe
  });

  // 监听连接成功后执行一次的事件
  client.on('connect', () => {
    client.subscribe(subscribe);
    //client.unsubscribe('myTopic'); // 取消订阅 'myTopic' 主题
    // 发送消息给客户端
    //ws.send('欢迎连接到 WebSocket 服务器！');
    //console.log("客户端信息:"+username+"|"+clientId+"连接！");
  });

  // MQTT 监听错误事件 持续监听
  client.on('error', (error) => {
    client.end();//MQTT 终止连接
    ws.send('MQTT端通信终止，错误：' + error);  //发送最后的错误代码
    ws.close();//终止连接
  });

  // 监听WebSocket客户端的断开连接事件
  ws.on('close', (code, reason) => {
    //console.log("客户端信息:"+username+"|"+clientId+"主动断开！");
    client.end();//MQTT 终止连接
  });

  // MQTT 监听消息 持续监听
  client.on('message', (topic, message) => {
    //console.log(`Received message on topic "${topic}": ${message.toString()}`);
    //转发到WebSocket
    ws.send(message.toString());
  });

  // WebSocket 监听客户端消息 持续监听
  ws.on('message', (message) => {
    //转发到MQTT
    //console.log(username+"|"+clientId+"发送了："+message);
    client.publish(sendsubscribe, message);
  });

});

// 开启WebSocket 服务端服务
aserver.listen(wport, () => {
  console.log(`WebSocket 服务器运行在 ${wport}`);
});

// 开启Mqtt 服务端服务
server.listen(port, function () {
  console.log(`MQTT 服务器运行在 ${port}`);
});

/*
// 查询用户名和密码等信息权限结果 单次逐个连接数据库处理
function selectUser(username, spassword) {
  // 创建数据库连接
  const connection = mysql.createConnection({
    host: sqlHost,
    port: sqlPort,
    user: sqlUserName,
    password: sqlPassWord,
    database: sqlDatabase
  });

  const query = 'SELECT * FROM users WHERE username = ?';

  connection.query(query, [username], (err, results) => {
    if (err) {
      console.error('Error executing query:', err);
      return;
    }

    console.log('Query results:', results);
  });

  // 关闭数据库连接
  connection.end();
}

aedes.authenticate = function (client, username, password, callback) {
  if (username === 'mqttwen' && password.toString() === 'mqtt888') {
    // 验证成功
    callback(null, true);
  } else {
    // 验证失败，拒绝连接
    callback(null, false);
  }
};

aedes.authenticate = function (client, username, password, callback) {
    if (username === 'mqttwen' && password.toString() === 'mqtt888') {
      // 验证成功
      callback(null, true);
    } else {
      // 验证失败，拒绝连接
      callback(null, false);
    }
};

aedes.on('client', function (client) {
  console.log('Client connected: ' + client.id);
  client.on('error', function (err) {
    console.error('Client error: ' + err.message);
  });
});

aedes.on('clientDisconnect', function (client) {
  console.log('Client disconnected: ' + client.id);
});

aedes.on('subscribe', function (subscriptions, client) {
  console.log('Client subscribed: ' + client.id);
});

aedes.on('unsubscribe', function (subscriptions, client) {
  console.log('Client unsubscribed: ' + client.id);
});

aedes.on('publish', function (packet, client) {
    if (client) {
      const clientInfo = client.id || 'UNKNOWN';
      const topic = packet.topic || 'UNKNOWN_TOPIC';
      const message = packet.payload ? packet.payload.toString() : '';
  
      console.log(`Received message from client ${clientInfo} on topic ${topic}: ${message}`);
    }
  });
*/