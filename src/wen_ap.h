// Wen_ap.h
// 2024年8月31日
// 为ESP设计的通用WEB配网模块-代码简单粗暴，界面用的黑色哈~个人感觉良好！
// 开源协议：MIT！！！亲们随便造！
// 使用方法简单！！！
// 开源项目地址：https://github.com/maddog888/wen_ap
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include <Preferences.h>

Preferences preferences;

ESP8266WebServer web(80);
DNSServer dnsServer;

#define AP_SSID "WenPeng"  //名称
#define AP_PASSWORD ""  //密码
IPAddress IP(2,2,2,2);  //路由地址

const int ledPin = LED_BUILTIN;  // 这里使用预定义的LED_BUILTIN常量

String configPage = "<!Doctype html><html lang='zh'><head><meta charset='utf-8'><meta name='viewport'content='width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0'><title>Wen Iot</title><style>html,body{font-family:Arial,sans-serif;text-align:center;background-color:#edeff7}.card-container{margin-top:3px;padding:18px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);background-color:#edeff7;display:none}.loading-overlay{position:fixed;top:0;left:0;width:100%;height:100%;background-color:rgba(56,56,56,0.8);z-index:9999;display:none;justify-content:center;align-items:center}.loading-spinner{border:4px solid rgba(0,0,0,0.1);border-left-color:#3c3c3c;border-radius:50%;width:40px;height:40px;animation:spin 0.5s linear infinite}@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}input,input{width:200px;padding:6px 12px;margin:3px 5px;border:0px;border-radius:4px;font-size:14px;transition:border-color 0.15s ease-in-out,box-shadow 0.15s ease-in-out}input:focus,input:focus{border-color:#ffffff;outline:none;box-shadow:inset 0 1px 1px rgba(255,255,255,0.075),0 0 5px rgba(255,255,255,0.5)}button{width:90%;height:39px;padding:6px 12px;margin:3px 5px;border:1px solid#525252;border-radius:4px;background-color:#3c3c3c;color:white;cursor:pointer;transition:background-color 0.15s ease-in-out,border-color 0.15s ease-in-out}button:hover{background-color:#2c2c2c;border-color:#3c3c3c}ul{list-style-type:none;padding:0;margin:10px}li{display:flex;align-items:center;justify-content:space-between;background-color:#ffffff;margin:8px;padding:15px 20px;border-radius:8px;font-size:0.9em;box-shadow:0 4px 6px rgba(0,0,0,0.1);transition:transform 0.2s;text-align:left;width:300px}li:hover{transform:translateY(-5px);box-shadow:0 6px 8px rgba(0,0,0,0.15)}li span{margin-left:auto}.winput{background:#ffffff;border-radius:5px;padding:1px 10px 1px 10px;font-size:1em;margin-bottom:10px}</style></head><body><div class='loading-overlay'><div id='ww'class='card-container'><div style='width: 100%;'><div class='winput'>名称<input id='ssid'placeholder='网络名称'/></div><div class='winput'>密码<input type='password'id='password'placeholder='WPA2/WPA3密码'/></div><button onclick='connect()'>保存并连接</button>&nbsp;<a onclick='wconnect(0)'>取消</a></div></div><div class='loading-spinner'></div><div style='position: absolute;margin-top: 100px;color: #ffffff;'id='loaddingText'>正在加载...</div></div><div style='display: inline-block;min-width: 300px;'>设备WIFi配网<ul id='list'></ul><ul><li onclick='wconnect(1)'>其他网络</li></ul><button onclick='getlist()'>刷新附近WiFi列表</button></div><script lang='javascript'>init=()=>{bt=document.querySelector('.loading-overlay');ww=document.getElementById('ww');loadding=document.querySelector('.loading-spinner');loaddingText=document.getElementById('loaddingText');list=document.getElementById('list');ssid=document.getElementById('ssid');pwd=document.getElementById('password')};wbt=(type=1)=>{if(type){bt.style.display='flex'}else{bt.style.display='none'}};wconnect=(type=1,name='')=>{if(type){wbt();loadding.style.display='none';loaddingText.style.display='none';ww.style.display='flex';ssid.value=name;pwd.focus()}else{wbt(0);ww.style.display='none'}};wloadding=(state=Number,title='正在加载...')=>{if(state){wbt();loadding.style.display='flex';loaddingText.style.display='flex';loaddingText.textContent=title}else{wbt(0);loadding.style.display='none'}};wmessage=(time=1,title='未知错误')=>{wbt();loadding.style.display='none';loaddingText.style.display='flex';loaddingText.textContent=title;setTimeout(()=>{wbt(0)},time*1000)};toList=(data=Array)=>{data.forEach(s=>{var newItem=document.createElement('li');if(s.type==1){newItem.textContent=s.ssid;newItem.onclick=function(){connect(s.ssid)}}else{newItem.innerHTML=s.ssid+'<span>◆</span>';newItem.onclick=function(){wconnect(1,s.ssid)}}list.appendChild(newItem)})};getlist=()=>{wloadding(1);fetch('/getList').then(res=>{return res.json()}).then(res=>{list.innerHTML=null;toList(res);wloadding(0)}).catch(()=>{wmessage(1.5,'无法获取WiFi列表')})};connect=(wssid='')=>{wconnect(0);wloadding(1,'正在连接...');if(wssid){ssid.value=wssid}fetch(`/to?ssid=${ssid.value}&pwd=${pwd.value}`).then(res=>{return res.json()}).then(res=>{wloadding(1,'连接成功，正在配网中...')}).catch(()=>{wmessage(1.5,'连接失败，请重试！');pwd.value=''})};window.onload=()=>{init();getlist()};</script></body></html>";

String ScanResult; //WiFi数据列表
bool first = true; //是否为首次扫描
bool apstate = true;  //是否打开AP

String ssid = ""; //wifi名称
String pwd = ""; //WiFi密码

void wifi_scan(int scanNum);  //函数声明

// web服务
void ap_serve(){
  // 判断 是否打开监听 AP 模式
  if(apstate){
    web.handleClient(); //监听并处理web请求
    dnsServer.processNextRequest(); //监听并处理dns请求
  }
}

//  首页回调函数
void www_configPage(){
  web.send(200, "text/html", configPage);
  return;
}

//  获取WiFi列表回调函数
void www_getList(){
  wifi_scan(WiFi.scanNetworks());
  web.send(200, "text/html", ScanResult);
  return;
}

//  尝试连接并保存WiFi信息
void www_save_wifi(){
  ssid = web.arg("ssid");
  pwd = web.arg("pwd");
  Serial.println("开始连接");
  Serial.println(ssid);
  Serial.println("");
  Serial.println(pwd);
  WiFi.begin(ssid, pwd);
  for(size_t i = 0; i < 30; i++){
    if(WiFi.status() == WL_CONNECTED){
      WiFi.softAPdisconnect(true); // 关闭 AP
      dnsServer.stop(); // 关闭 DNS
      web.send(200, "text/plain", "{\"state\": 1}");
      web.close(); // 关闭 web 
      Serial.println("已联网");
      // 保存WiFi信息
      preferences.begin("wen", false); // 命名空间和只读/读写模式
      preferences.putString("ssid", ssid);
      preferences.putString("pwd", pwd);
      preferences.end();
      digitalWrite(ledPin, HIGH); //关闭LED
      return;
    }else{
      delay(500);
    }
  }
  web.send(200, "text/plain", "");
  return;
}

//  web服务初始化
void ap_web_init(){
  web.begin();
  web.on("/", www_configPage);
  web.on("/getList", www_getList);
  web.on("/to", www_save_wifi);
  web.onNotFound(www_configPage);
  dnsServer.start((byte)53, "*", IP);
}


// 扫描附近WiFi
void wifi_scan(int scanNum){
  String type;
  bool Jfirst = true;
  // 构建WiFi列表json数据
  ScanResult = "[";
  for(int i = 0; i < scanNum; i++){
    WiFi.encryptionType(i) == ENC_TYPE_NONE ? type = "1" : type = "0";
    // WiFi.RSSI(i) dbm
    if(Jfirst){
      Jfirst = false;
      ScanResult += "{\"ssid\":\"" + WiFi.SSID(i) + "\"," + "\"type\":\"" + type + "\"}";
    }else{
      ScanResult += ", {\"ssid\":\"" + WiFi.SSID(i) + "\"," + "\"type\":\"" + type + "\"}";
    }
  }
  ScanResult += "]";

  // 启动web服务
  if(first){
    ap_web_init();
    first = false;
  }else{
    web.send(200, "text/html", ScanResult);
  }
}

// 配网初始化
void ap_init(){
  // 判断是否存在了WiFi信息，存在则直接连接
  preferences.begin("wen", true); // 命名空间和只读/读写模式
  ssid = preferences.getString("ssid", "null");
  pwd = preferences.getString("pwd", "null");
  Serial.println("");
  Serial.println("ssid: " + ssid);
  preferences.end();
  if(ssid!="null"){
    WiFi.begin(ssid, pwd);
    // 大概连接1分钟，避免停电路由器开机慢而导致无法连接的问题
    for(size_t i = 0; i < 120; i++){
      if(WiFi.status() == WL_CONNECTED){
        Serial.println("历史信息联网成功");
        apstate = false;
        return;
      }else{
        delay(500);
      }
    }
  }

  // 如果没有配置且连接失败则打开AP
  if(apstate){
    pinMode(ledPin, OUTPUT);  //初始化LED引脚
    digitalWrite(ledPin, LOW); //打开LED
    WiFi.mode(WIFI_AP_STA); //使用STA模式
    WiFi.persistent(false); //关闭持久化存储
    WiFi.softAPConfig(IP, IP, IPAddress(255,255,255,0));  //配置AP
    WiFi.softAP(AP_SSID, AP_PASSWORD, 1, 0, 2); //启动
    // 扫描附近WiFi 并 启动web服务
    WiFi.scanNetworksAsync(wifi_scan);
  }
}
