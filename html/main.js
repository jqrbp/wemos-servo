var servoPosition = 30;
var servoWaitFeedbackFlag = false;
function bToNum(_bytes, _signed = false, _little_endian = false) {
    var num = 0;
    var num_of_bits = _bytes.length * 8;
    if (_little_endian) {
        for (var i = _bytes.length; i > 0; i--) {
            num = num * 256 + _bytes[i - 1];
        }
    } else {
        _bytes.forEach(function (c) {num=num * 256 + c;});
    }
    if (_signed) {
        // check if the MSB is set
        if ((num & 1 << (num_of_bits - 1)) > 0) {
            num = num - (1 << num_of_bits);
         }
    }
    return num;
}

function getEl(id) {
    return document.getElementById(id);
}

function httpPostAsync(theUrl, authStr, data, type, callback, onloadcb) {
    var xhr = new XMLHttpRequest();
    if (authStr != null) {
        xhr.withCredentials = true;
    }

    xhr.addEventListener("readystatechange", function() {
        if (this.readyState === 4) {
            callback(this.responseText);
        }
    });

    if (onloadcb != null) {
        xhr.onload = function() {
            onloadcb(xhr.responseText);
        }
    }

    xhr.open("POST", theUrl, true);
    if (authStr != null) {
        xhr.setRequestHeader("Authorization", authStr);
    }

    if (type != null) {
        xhr.setRequestHeader("Content-Type", type);
    }


    xhr.send(data);
}

function httpGetAsync(theUrl, callback) {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() {
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}

function ssidSaveOnlick() {
    var data = 'ssid=' + getEl("ssid-name").value;
    data+='&password='+getEl("ssid-passwd").value;
    data+='&hostname='+getEl("wifi-hostname").value;
    httpPostAsync("saveconfig", null, data, "application/x-www-form-urlencoded", null);
}

function servoPulseDone(_response) {
    alert(_response);
}

function servoPulseSet() {
    var data = 'min=' + getEl('servo-pulse-value-min').value;
    data+='&max='+getEl('servo-pulse-value-max').value;
    data+='&vmin='+getEl('servo-value-min').value;
    data+='&vmax='+getEl('servo-value-max').value;

    httpPostAsync("setpulse", null, data, "application/x-www-form-urlencoded", servoPulseDone);
}

function servoPulseGet() {
    httpGetAsync("getpulse", function(resp) {
        const obj = JSON.parse(resp);
        getEl('servo-pulse-value-min').value = obj.min;
        getEl('servo-pulse-value-max').value = obj.max;
        getEl('servo-value-min').value = obj.vmin;
        getEl('servo-value-max').value = obj.vmax;
        getEl('servo-range-value').value = obj.vin;
    });
}

function process_ws_obj(arr) {
    // const cur_value = bToNum(arr.slice(0,2), false, true);
    const in_value = bToNum(arr.slice(2,4), false, true);
    const min_value = bToNum(arr.slice(4,6), false, true);
    const max_value = bToNum(arr.slice(6,8), false, true);

    if (servoPosition < min_value) {
        servoPosition = min_value;
    }

    if (servoPosition > max_value) {
        servoPosition = max_value;
    }

    if (servoPosition != in_value) {
        webSocketSend(servoPosition);
    } else {
        servoWaitFeedbackFlag = false;
    }
}

function process_ws_packet(_evt) {
    // console.log(_evt);
    var dataIn = _evt.data;
    var dataType = typeof(_evt.data);
    
    switch(dataType) {
        case "object":
            reader = new FileReader();
            reader.onload = () => {
                var dataArr = new Uint8Array(reader.result);
                process_ws_obj(dataArr);
            };
            reader.readAsArrayBuffer(dataIn);
            break;
        case "string":
            console.log(dataIn);
            break;
        default:
            return;
    }
    
}

function wsURLOnConnect() {
    const wsurl = getEl('wsurl').value;
    webSocketConnect(wsurl,"",process_ws_packet);
}

function wsObjProcess(dataArr) {
    var hStr = utf8ToHexStr(dataArr);
    console.log(hStr);
}

function setServoPosition(new_value) {
    if (servoPosition != new_value) {
        servoPosition = new_value;
        if (!servoWaitFeedbackFlag) {
            webSocketSend(servoPosition);
            servoWaitFeedbackFlag = true;
        }
    };
}

function servoRangeReset() {
    getEl('servo-range-min').value = 10;
    getEl('servo-range-max').value = 160;
    getEl('servo-range-value').value = 30;
    getEl('servo-range').value = 30;
    setServoPosition(30);
}

// document.addEventListener('DOMContentLoaded', function () {
window.addEventListener('load', function() {
    getEl('servo-range').value = servoPosition;
    getEl('servo-range').addEventListener('input', function () {
        const new_value = Math.round(this.value);
        setServoPosition(new_value);
        getEl('servo-range-value').value = new_value;
    });

    getEl('servo-range-min').value = 10;
    getEl('servo-range-min').addEventListener('change', function() {
        getEl('servo-range').setAttribute("min", this.value);
    });

    getEl('servo-range-max').value = 160;
    getEl('servo-range-max').addEventListener('change', function() {
        getEl('servo-range').setAttribute("max", this.value);
    });

    getEl('servo-range-value').value = servoPosition;
    getEl('servo-range-value').addEventListener('change', function() {
        const new_value = Math.round(this.value);
        getEl('servo-range').value = new_value;
        setServoPosition(new_value);
    });

    servoPulseGet();

    getEl('wsurl').value = document.URL;
    webSocketConnect(null,"",process_ws_packet);
});