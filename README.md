# Weniot
这个项目采用的是通用思路，已经实现了定时和倒计时功能，只需要新建功能和名称添加到对应的地方即可帮助你快速搭建完全属于自己的物联网，包含了服务端，用户端以及设备端代码。
> 相关视频：https://www.bilibili.com/video/BV1QsHxeMEGD

## 预览
![页面预览1](/src/1.png)

## 初衷
这个项目是我自己入门写的，大神可以忽略，哈哈哈！适合新手入门！自己DIY一些实用好玩的东西！

## 功能
1.使用了另外一个开源的WiFi配网项目实现快速配网！[https://github.com/maddog888/wen_ap](https://github.com/maddog888/wen_ap)

2.已实现校准北京时间，定时任务，倒计时任务。

3.已实现MQTT通信交互，默认使用公共的服务器，可直接上传体验。

4.通用思路，只需要编写逻辑代码和定义功能名称，即可快速实现交互。

5.WEB用户端的内容展示和功能也是通用思路，模块化添加对应的名称即可快速完成项目。

# 用户控制端
使用的是Web实现，可以直接在浏览器运行，用户控制端演示：[https://maddog888.github.io/WenIot/](https://maddog888.github.io/WenIot/)

# MQTT 服务端
使用的是 Node.js 环境实现，本质是Javascript，直接挂起即可，如果使用该文件自建MQTT服务器，要使用对应的Web端文件。

# 硬件端 ESP8266
Arduino，打开src文件中的wen_wifi.ino

Platformio，打开main.cpp文件即可

> MadDogQQ交流群：591668872
