#include <ArduinoJson.h>   //Json模块库文件-必须

//设置需要用到的针脚
const int on_a = D0;  //插座继电器开关
//开关打开时间计算
unsigned long onaRunMillis = 0;  // 毫秒时间记录

// 这个是手动DIY写的逻辑事件
void Uona(String value){
  //如果是请求打开
  if(value=="1"){
    //因为使用继电器接的是常开端口，所以0是通电
    digitalWrite(on_a, 0);   //为插座通电
    onaRunMillis = millis();  //记录插座运行时间
  }else{
    digitalWrite(on_a, 1);  //给插座断电
    onaRunMillis = 0; //清除插座运行时间
  }
}


// 以下代码不固定，但事件和变量名基本固定
//  固定：写在导入weniot之前-毫秒转时间
String getTime(unsigned long elapsedTime) {
  // 先把毫秒转为秒 即 / 1000
  // 得到秒之后，C中是取余数，过程是：比如121秒 / 60 = 2; 切分份数
  // 121 - (60 × 2) = 1秒
  // 如果是120则为0，所以结果只会是0-59
  // 分钟同理
  unsigned long seconds = (elapsedTime / 1000) % 60;
  unsigned long minutes = (elapsedTime / 60000) % 60;
  unsigned long hours = (elapsedTime / 3600000);

  // 将时间格式化为文本数据
  String timeData = String(hours) + "时";
  timeData += String(minutes) + "分";
  timeData += String(seconds) + "秒";

  return timeData;
}

// 固定：写在导入weniot之前-需要（实时）传给开发板的数据
DynamicJsonDocument returnData() {
  DynamicJsonDocument doc(1024);  //初始化
  // 在此编写doc[""] = "";即可

  // 示例：如果存在时间，则取运行时间减去记录时间，就是打开的时间
  if(onaRunMillis>0){
    doc["onaTime"] = getTime(millis() - onaRunMillis);
  }else{
    // 如果不存在但已经通电则记录当前运行时间
    if(!digitalRead(on_a)){
      onaRunMillis = millis();
    }
    doc["onaTime"] = 0;
  }
  // 返回插座通电状态
  doc["ona"] = (digitalRead(on_a) ? 0 : 1); //获取相反状态是0就传1，是1就传0

  // 返回不变
  return doc; //返回
}

// 固定：写在导入weniot之前-编写逻辑代码
void perTask(String id, String value){
   //如果是ona，则控制插座通断电
  if(id=="ona"){
    Uona(value);
  }
}

// 固定：定时任务最大列数 默认 5个
#define TimerMax 5
// 固定：倒计时任务最大列数 默认 5个
#define DTimerMax 5
// 固定：定义任务列表数量-0开始，0、1，则是2个
#define Tasks 2
// 固定：写在导入weniot之前-编写任务对应的逻辑代码
void runTask(int Tid){
  if(Tid==0){
    Uona("1");  //打开
  }else if(Tid==1){
    Uona("0");  //关闭
  }
}
String mqtt_title = "wificz"; //定义项目名称-用于配置客户端ID-主题-等信息
// 以上代码不固定，但事件和变量名基本固定
#include "wen_iot.h" //导入weniot-更多配置可看代码文件



void setup() {
  //定义串口
  Serial.begin(9600);
  wen_iot_init(); //初始化weniot

  //初始化针脚
  pinMode(on_a, OUTPUT);  //插座继电器开关

  //配置任务列表-对应在runTask事件中的逻辑
  tList[0] = "打开插座";
  tList[1] = "关闭插座";
}

void loop() {
  wen_iot_run();  //weniot事件
}
