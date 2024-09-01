// Wen_iot.h
// 2024年9月1日
// 为ESP设计的通用物联网模块-快速实现联网-定时-倒计时及各种任务！
// 开源协议：MIT！！！亲们随便造！
// 使用方法简单！！！
// 开源项目地址：https://github.com/maddog888/WenIot
//所需模块
#include "wen_ap.h" //导入Wen_WiFi配网
#include <ESP8266WiFi.h>   //WiFi模块库文件
#include <WiFiUdp.h>  //UDP 协议
#include <NTPClient.h>  //NTP服务器库
#include <PubSubClient.h>  //Mqtt模块库文件
//#include <ArduinoJson.h>   //Json模块库文件-必须

// MQTT相关配置信息
const char *mqtt_broker_addr = "broker.emqx.io";  // 服务器地址
const uint16_t mqtt_broker_port = 1883;          // 服务端口号

const char *mqtt_username = "********";  // 账号（非必须）
const char *mqtt_password = "********";  // 密码（非必须）

const uint16_t mqtt_client_buff_size = 4096;  // 客户端缓存大小（非必须）
String mqtt_client_id = mqtt_title + "_weniot_client";       // 客户端ID

String mqtt_topic_pub = mqtt_title + "_weniot_pub";  // 需要发布到的主题
String mqtt_topic_sub = mqtt_title + "_weniot_sub";  // 需要订阅的主题

//Mqtt检测状态时间
unsigned long previousConnectMillis = 0;  // 毫秒时间记录
const long intervalConnectMillis = 5000;  // 时间间隔

//wifi client
WiFiClient tcpClient;

//Mqtt
PubSubClient mqttClient;

// NTP服务器信息
const char* ntpServer = "pool.ntp.org"; // NTP服务器地址
const long gmtOffset_sec = 28800;       // 中国北京时间的时差，以秒为单位
const int daylightOffset_sec = 0;      // 夏令时的时差，以秒为单位
bool timeRight = true;  //时间是否正确
//本地计时
unsigned long initialTime = 0;  //获取到的北京时间
unsigned long nowTime = 0;  //计算到的北京时间
unsigned long iotDayTime = 0;  //记录超天运行时间
// 创建 WiFiUDP 实例以与NTP服务器通信
WiFiUDP ntpUDP;
// 创建 NTPClient 实例
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

// 定义任务列表数据类型
struct CountdownTimer {
  unsigned long time; //时间
  int runInt; //执行操作
};
// 创建定时队列
CountdownTimer timers[TimerMax];
// 创建倒计时队列
CountdownTimer Dtimers[DTimerMax]; 
//创建任务列表
String tList[Tasks];

//毫秒转时间
// String getTime(unsigned long elapsedTime) {
//   unsigned long seconds = (elapsedTime / 1000) % 60;
//   unsigned long minutes = (elapsedTime / 60000) % 60;
//   unsigned long hours = (elapsedTime / 3600000);

//   // 将时间格式化为文本数据
//   String timeData = String(hours) + "时";
//   timeData += String(minutes) + "分";
//   timeData += String(seconds) + "秒";

//   return timeData;
// }

//添加定时任务
bool startTimer(unsigned long time, int runInt) {
  bool state = false; //添加状态，默认失败
  // 遍历倒计时器数组，更新每个倒计时器的状态
  for (int i = 0; i < TimerMax; i++) {
    //寻找到当前空闲的列
    if (timers[i].time==0) {
      timers[i].time = time;
      timers[i].runInt = runInt;
      state = true; //添加成功
      Serial.println("倒计时器 " + String(i) + " 启动，执行时间：" + getTime(time));
      break;
    }
  }
  return state;
}

//添加倒计时任务
bool startDTimer(unsigned long time, int runInt) {
  bool state = false; //添加状态，默认失败
  // 遍历倒计时器数组，更新每个倒计时器的状态
  for (int i = 0; i < DTimerMax; i++) {
    //寻找到当前空闲的列
    if (Dtimers[i].time==0) {
      Dtimers[i].time = (millis() + time);
      Dtimers[i].runInt = runInt;
      state = true; //添加成功
      break;
    }
  }
  return state;
}

//当控制端ping请求响应的数据
void sendUpdate(){
  // 创建一个 JSON 对象
  DynamicJsonDocument doc(1024);
  doc = returnData(); //  先把用户需要的数据拿到
  // 向 JSON 对象添加数据
  doc["iotTime"] = getTime(nowTime); //默认
  doc["runTime"] = getTime(millis()); //默认
  //定时任务
  for (int i = 0; i < TimerMax; i++) {
    if(timers[i].time>0){
      doc["taskLists"][i][0] = String(i);
      doc["taskLists"][i][1] = String(timers[i].time);
      doc["taskLists"][i][2] = tList[timers[i].runInt];
    }
  }
  //倒计时任务
  for (int i = 0; i < DTimerMax; i++) {
    if(Dtimers[i].time>0){
      doc["taskDLists"][i][0] = String(i);
      doc["taskDLists"][i][1] = String((Dtimers[i].time - millis()));
      doc["taskDLists"][i][2] = tList[Dtimers[i].runInt];
    }
  }
  //任务列表
  for (int i = 0; i < Tasks; i++) {
    doc["TasksList"][i][0] = String(i);
    doc["TasksList"][i][1] = tList[i];
  }
  // 将 JSON 对象转换为 JSON 字符串
  String jsonString;
  serializeJson(doc, jsonString);
  //发送数据
  mqttClient.publish(mqtt_topic_pub.c_str(), jsonString.c_str());
}


//响应控制端操作事件
void eventGo(String id, String value) {
  //默认
  if(id=="restart"){
    ESP.restart();
  }else if(id=="addTask"){
    // 找到 '|' 的位置
    int pipeIndex = value.indexOf('|');
    if (pipeIndex != -1) {
      // 使用 substring() 方法获取前半部分和后半部分
      long firstPart = value.substring(0, pipeIndex).toInt();
      int secondPart = value.substring(pipeIndex + 1).toInt();
      startTimer(firstPart, secondPart);
    }
  }else if(id=="addDTask"){
    // 找到 '|' 的位置
    int pipeIndex = value.indexOf('|');
    if (pipeIndex != -1) {
      // 使用 substring() 方法获取前半部分和后半部分
      long firstPart = value.substring(0, pipeIndex).toInt();
      int secondPart = value.substring(pipeIndex + 1).toInt();
      startDTimer(firstPart, secondPart);
    }
  }else if(id=="delTask"){
    if(timers[value.toInt()].time){
      timers[value.toInt()].time = 0;
    }
  }else if(id=="delDTask"){
    if(Dtimers[value.toInt()].time){
      Dtimers[value.toInt()].time = 0;
    }
  }else{
    // 交由用户自写逻辑处理
    perTask(id, value);
  }
  //默认操作完成后发送最新状态数据
  sendUpdate();
}

//  MQTT消息回调函数，该函数会在PubSubClient对象的loop方法中被调用 默认不变
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  //收到的消息
  String msg = "";
  //组成完整的消息
  for (int i = 0; i < length; i++) {
    msg = msg + (char)payload[i];
  }
  //ping事件
  if (msg == "ping") {
    //通过设定好的内容回应
    sendUpdate();  //默认不变
  }else{
    // 创建一个 JSON 文档对象来解析接收到的数据，其他执行事件
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, msg);
    //判断数据可靠性
    if (error) {
      Serial.print("解析 JSON 数据失败: ");
      Serial.println(error.c_str());
      return;
    } else {
      // 遍历 JSON 对象的键值对
      for (JsonPair kv : doc.as<JsonObject>()) {
        const char *key = kv.key().c_str();
        String value = kv.value().as<String>(); // 强制转为字符串
        //移交处理
        eventGo(key, value);
      }
    }
  }
}

// 初始化
void wen_iot_init(){
  ap_init();
  // 设置MQTT客户端
  mqttClient.setClient(tcpClient);  //***
  mqttClient.setServer(mqtt_broker_addr, mqtt_broker_port); //***
  mqttClient.setBufferSize(mqtt_client_buff_size);  //***
  mqttClient.setCallback(mqtt_callback);  //***

  // 初始化NTP客户端
  timeClient.begin(); //***
  // 更新时间
  timeClient.update();  //***
  if(timeClient.getEpochTime() > 1514764800){
    // 获取北京时间
    int hours = timeClient.getHours();  //***
    int minutes = timeClient.getMinutes();  //***
    int seconds = timeClient.getSeconds();  //***
    int todayTotalSeconds = ((hours * 3600 + minutes * 60 + seconds) * 1000) - millis();  //***
    initialTime = todayTotalSeconds;
  }else{
    timeRight = false;
  }
}

// 任务事件
void wen_iot_run(){
  ap_serve();
  //判断当前时间是否准确，如果不准确则持续更新
  if(!timeRight){
    // 更新时间
    timeClient.update();  //***
    if(timeClient.getEpochTime() > 1514764800){
      // 获取北京时间
      int hours = timeClient.getHours();  //***
      int minutes = timeClient.getMinutes();  //***
      int seconds = timeClient.getSeconds();  //***
      int todayTotalSeconds = ((hours * 3600 + minutes * 60 + seconds) * 1000) - millis();  //***
      initialTime = todayTotalSeconds;
      timeRight = true;
    }
  }
  //
  unsigned long currentMillis = millis();  // 读取当前时间
  nowTime = (initialTime + currentMillis) - iotDayTime;  //计算当前北京时间
  //如果超过了24小时那么就记录当前设备已经运行的事件并减去那么总和就将重新计算也就是从0时开始
  if(nowTime>=86400000){
    iotDayTime = millis();  //记录时间用于减去
    initialTime = 0;  //新的一天直接从本地从零计算
    nowTime = (initialTime + currentMillis) - iotDayTime;  //重新计算当前北京时间
  }

  // 自动连接MQTT服务器
  if (!mqttClient.connected())  // 如果未连接
  {
    if (currentMillis - previousConnectMillis > intervalConnectMillis) {
      previousConnectMillis = currentMillis;
      mqtt_client_id += String(WiFi.macAddress());     // 每个客户端需要有唯一的ID，不然上线时会把其他相同ID的客户端踢下线
      if (mqttClient.connect(mqtt_client_id.c_str()))  // 尝试连接服务器
      // if (mqttClient.connect(mqtt_client_id.c_str(), mqtt_username, mqtt_password))
      {
        sendUpdate();
        mqttClient.subscribe(mqtt_topic_sub.c_str());                    // 连接成功后可以订阅主题
      }
    }
  }
  //定时任务计划
  for (int i = 0; i < TimerMax; i++) {
    //寻找到当前有任务的列
    if (timers[i].time>0) {
      if(nowTime==timers[i].time){
        runTask(timers[i].runInt);  //执行任务
        sendUpdate(); //更新状态
        delay(1000);  //避免重复执行
      }
    }
  }
  //倒计时任务
  for (int i = 0; i < DTimerMax; i++) {
    //寻找到当前有任务的列
    if (Dtimers[i].time>0) {
      if(currentMillis==Dtimers[i].time){
        runTask(Dtimers[i].runInt);  //执行任务
        Dtimers[i].time = 0;  //关闭定时任务
        sendUpdate(); //更新状态
        delay(1000);  //避免重复执行
      }
    }
  }
  // 处理MQTT事务
  mqttClient.loop();
}