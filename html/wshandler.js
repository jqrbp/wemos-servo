var gWebSock;
var gWebSocketConnected = false;

function utf8ToHexChr(_v) {
    return ('0' + _v.toString(16)).slice(-2);
}

function utf8ToHexStr(_arr) {
    return Array.from(_arr).map(function(v){return utf8ToHexChr(v)}).join(',').toUpperCase();
}

function webSocketSend(str) {
    if (gWebSocketConnected == true) {
        gWebSock.send(str)
    }
}

function new_ws(urlpath, protocol)
{
	if (typeof MozWebSocket != "undefined")
		return new MozWebSocket(urlpath, protocol);

	return new WebSocket(urlpath, protocol);
}

function get_appropriate_ws_url(doc_url, extra_url)
{
	var pcol;
	var u = document.URL;
	if (doc_url != null) {
		u = doc_url
	}

	/*
	 * We open the websocket encrypted if this page came on an
	 * https:// url itself, otherwise unencrypted
	 */

	if (u.substring(0, 5) === "https") {
		pcol = "wss://";
		u = u.substr(8);
	} else {
		pcol = "ws://";
		if (u.substring(0, 4) === "http")
			u = u.substr(7);
	}

	u = u.split("/");
	var p = u[0].split(":");

	/* + "/xxx" bit is for IE10 workaround */

	return pcol + p[0] + ":8081" + "/" + extra_url;
}

function webSocketConnect(_docUrl = null, _extraUrl = "", _dataCb = null) {
    if ("WebSocket" in window)
    {
        gWebSock = new_ws(get_appropriate_ws_url(_docUrl, _extraUrl), ['arduino','chat']);
        try {
            gWebSock.onopen = function(evt) { console.log('websock open'); gWebSocketConnected = true;};
            gWebSock.onclose = function(evt) { console.log('websock close'); };
            gWebSock.onerror = function(evt) { console.log(evt); };
            gWebSock.onmessage = function(e) {
				if(_dataCb != null) {
					_dataCb(e);
				}
            };
        } catch(exception) {
            alert("Error " + exception);  
        }
    } else {
        alert("WebSocket Not Supported!");
    }
}
