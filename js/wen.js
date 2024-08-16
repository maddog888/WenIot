/* 2023年11月07日16:06:34 MadDog-Wen */
const chat = $('#chat');    //消息日志
const messageInput = $('#message'); //发送消息框
const sendButton = $('#send');  //发送按钮

let pingTimer; // 定时器用于发送 Ping 消息
let pongReceived = true; // 标记是否收到 Pong 响应

$(document).ready(function() {
    // 当连接建立时触发
    socket.addEventListener('open', (event) => {
        $('#webState').html('服务端：<span class="badge text-bg-success">在线</span>');
        sendPing();
        // 定时发送 Ping 消息
        pingTimer = setInterval(sendPing, 2000); // 10秒发送一次 Ping
    });

    // 当接收到消息时触发
    socket.addEventListener('message', (event) => {
        const receivedMessage = event.data;
        displayMessage(receivedMessage);
        // 收到响应
        pongReceived = true;
    });

    // 当连接关闭时触发
    socket.addEventListener('close', (event) => {
        closeWeb('服务端：<span class="badge text-bg-secondary">离线</span>');
    });

    // 当发生错误时触发
    socket.addEventListener('error', (event) => {
        closeWeb('服务端：<span class="badge text-bg-info">连接中断</span>');
    });

    // 发送消息
    sendButton.click(function() {
        socket.send(messageInput.val());
        displayMessage(`服务端: ${messageInput.val()}`);
    });


    /*** 
     * 
     * 以下为通用监听
     * 
    */
    // 使用事件委托来监听所有复选框的状态变化
    $('input[type="checkbox"]').on('change', function() {
        // 获取复选框的 id 属性
        var checkboxId = $(this).attr('id');

        if (this.checked) {
            sendMsg(checkboxId, 1);
        } else {
            sendMsg(checkboxId, 0);
        }
    });

    // 监听所有 A 标签的点击事件
    $(document).on('click', 'a', function() {
        //获取按下的ID
        var aboxId = $(this).attr('id');
        //发送按下的消息
        sendMsg(aboxId, 1);
    });
    /*** 
     * 
     * 通用监听结束
     * 
    */
    //添加定时计划
    $("#addTask").click(function() {
        // 获取时间输入元素的值
        var timeValue = $("#taskTime").val();

        // 将时间字符串分割为小时和分钟
        var timeParts = timeValue.split(":");
        var hour = timeParts[0];
        var minute = timeParts[1];
        var TotalSeconds = (hour * 3600 + minute * 60 + 1) * 1000;
        // 关闭模态框
        $("#addTaskModal").modal("hide");

        sendMsg("addTask",TotalSeconds + "|" + $("#taskid").val());
    });
    //添加倒计时计划
    $("#addDTask").click(function() {
        //var DTotalSeconds = ($("#Dtaskmin").val() + $("#Dtasks").val()) * 1000;
        var DTotalSeconds = ($("#Dtaskmin").val() * 60 + Math.floor($("#Dtasks").val())) * 1000;
        // 关闭模态框
        $("#addDTaskModal").modal("hide");

        sendMsg("addDTask",DTotalSeconds + "|" + $("#Dtaskid").val());
    });
});

// 用户重新回到页面，刷新当前页面
$(document).on('visibilitychange', function() {
    if (document.visibilityState === 'visible') {
        location.reload();
    }
});


function sendMsg(id,state){
    // 创建一个空的 JSON 对象
    var jsonData = {};
    // 向 JSON 对象添加键值对
    jsonData[id] = state;
    // 将 JSON 对象转换为 JSON 格式的字符串
    var jsonString = JSON.stringify(jsonData);
    socket.send(jsonString);
    displayMessage(`服务端: ${jsonString}`);
}


// 处理接收到的消息
function displayMessage(message) {
    $('#iotState').html('设备端：<span class="badge text-bg-primary">在线</span>');
    // 清空现有列表内容
    $('#taskList').empty();
    $('#taskDList').empty();

    // 在聊天框中显示消息
    chat.append(message + "\n");
    
    // 解析 JSON 消息
    const data = JSON.parse(message);
    
    // 遍历 JSON 对象
    $.each(data, function(key, value) {
        if(key=="taskLists"){
            var taskLists = "";
            value.forEach(function(task) {
                if(task){
                    var taskId = task[0];
                    var taskTime = task[1];
                    var taskName = task[2];

                    var minutes = Math.floor((taskTime / 60000) % 60);
                    var hours = Math.floor((taskTime / 3600000));


                    // 创建列表项
                    var listItem = '<li class="list-group-item d-flex justify-content-between align-items-center">' + hours + "时" + minutes + "分" + taskName + '<span onclick="javascript:sendMsg(\'delTask\',' + taskId +');" class="badge bg-danger">×</span></li>';

                    taskLists += listItem;
                }
            });
            // 将列表项添加到 <ul>
            if(taskLists){
                $('#taskList').append(taskLists+"<br>");
            }
        }else if(key=="taskDLists"){
            var taskDLists = "";
            value.forEach(function(task) {
                if(task){
                    var taskId = task[0];
                    var taskTime = task[1];
                    var taskName = task[2];

                    var seconds = taskTime / 1000; // 换算为秒
                    var days = Math.floor(seconds / (3600 * 24));
                    var hours = Math.floor((seconds % (3600 * 24)) / 3600);
                    var minutes = Math.floor((seconds % 3600) / 60);
                    var remainingSeconds = Math.floor(seconds % 60);

                    var resultString = days + " 天 " + hours + " 小时 " + minutes + " 分钟 " + remainingSeconds + " 秒";

                    // 创建列表项
                    var listItem = '<li class="list-group-item d-flex justify-content-between align-items-center">' + resultString + "后" + taskName + '<span onclick="javascript:sendMsg(\'delDTask\',' + taskId +');" class="badge bg-danger">×</span></li>';

                    taskDLists += listItem;
                }
            });
            // 将列表项添加到 <ul>
            if(taskDLists){
                $('#taskDList').append(taskDLists+"<br>");
            }
            
        }else if(key=="TasksList"){
            if(!$('#taskid').val()){
                var TasksList = "";
                value.forEach(function(task) {
                    if(task){
                        var taskId = task[0];
                        var taskName = task[1];
                        
                        // 创建列表项
                        var listItem = '<option value="' + taskId + '">' + taskName + '</option>';

                        TasksList += listItem;
                    }
                });
                // 将列表项添加到 <ul>
                $('#taskid').append(TasksList);
                $('#Dtaskid').append(TasksList);
            }
        }else{
            // 查找具有相应 id 的元素
            var element = $("#" + key);

            // 获取元素的 type 属性
            var elementType = element.attr("type");

            if (elementType === 'checkbox') {
                if (value == 1) {
                    element.prop('checked', true);
                } else {
                    element.prop('checked', false);
                }
            }else {
                // 处理其他类型的元素
                element.text(value);
            }
        }
    });
}

// 发送 Ping 消息
function sendPing() {
    if (pongReceived) {
        // 发送 Ping 消息
        socket.send('ping');
        pongReceived = false;
    } else {
        // 如果在上一次 Ping 后没有收到 Pong
        $('#iotState').html('设备端：<span class="badge text-bg-dark">离线</span>');
        pongReceived = true;
        //持续获取状态
    }
}

//处理断开事件
function closeWeb(msg){
    //设置服务端状态
    $('#webState').html(msg);
    // 清除 Ping 定时器
    clearInterval(pingTimer);
}