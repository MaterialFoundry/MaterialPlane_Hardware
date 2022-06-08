const webserverVersion = "v1.0.3";

let ws;                         //Websocket variable
let wsOpen = false;             //Bool for checking if websocket has ever been opened => changes the warning message if there's no connection
let wsInterval;                 //Interval timer to detect disconnections
let generalSettings = {};
let calSettings = {};
let irSettings = {};
let networkSettings = {};
let fwVersion;
let hwVersion;
let forceRefresh = false;

startWebsocket();

/**
 * When the document is loaded
 */
document.addEventListener("DOMContentLoaded", function(event) { 
    openTab("Status");

    document.getElementById("webserverVersion").innerHTML = webserverVersion;
    document.getElementById("APs").addEventListener("change", (event) => { document.getElementById("SSID").value = event.target.value });
    
    for (let element of document.getElementsByClassName("expandable")) {
        element.addEventListener("click",(event) => {
            let thisElement = event.target;
            if (event.target.className == "expandableIcon") thisElement = event.target.parentElement;
            let nextElement = thisElement.nextElementSibling;
            const collapse = nextElement.className == "collapsed" ? false : true;
            nextElement.className = collapse ? "collapsed" : "";
            thisElement.children[0].src = collapse ? "right.png" : "down.png";
        })
    }
    
    document.getElementById("debugEn").addEventListener("change", (event) =>            { sendWS(`SET DEBUG ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("serialOut").addEventListener("change", (event) =>          { sendWS(`SET SERIALOUT ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("resetSettings").addEventListener("click", (event) =>       { 
        if (confirm(`Are you sure you want to reset all settings to their default values? This is irreversable, and will require you to reconfigure everything, including the WiFi settings.`))
            sendWS(`SET DEFAULT \n`) 
    });
    document.getElementById("restart").addEventListener("click", (event) =>             { sendWS(`RESTART \n`) });

    document.getElementById("updateDeviceName").addEventListener("click", (event) =>    { sendWS(`SET WIFI NAME "${document.getElementById("deviceName").value}" \n`) });
    document.getElementById("scanWiFi").addEventListener("click", (event) =>            { sendWS(`SCAN WIFI \n`) });
    document.getElementById("connectWiFi").addEventListener("click", (event) => {
        if (document.getElementById("SSID").value == "")
            alert("No access point selected");
        sendWS(`SET WIFI SSID ${document.getElementById("SSID").value} "${document.getElementById("password").value}" \n`)
    });
    document.getElementById("updatePort").addEventListener("click", (event) =>                  { sendWS(`SET WS PORT ${document.getElementById("wsPort").value} \n`) });

    document.getElementById("mpAutoExposure").addEventListener("click", (event) =>              { sendWS(`SET IR AUTOEXPOSE \n`) });
    document.getElementById("mpSensorExposure").addEventListener("change", (event) =>           { sendWS(`SET IR EXPOSURE ${event.target.value} \n`) });
    document.getElementById("mpSensorExposureNumber").addEventListener("change", (event) =>     { sendWS(`SET IR EXPOSURE ${event.target.value} \n`) });
    document.getElementById("mpSensorFramePeriod").addEventListener("change", (event) =>        { sendWS(`SET IR FRAMEPERIOD ${event.target.value} \n`) });
    document.getElementById("mpSensorFramePeriodNumber").addEventListener("change", (event) =>  { sendWS(`SET IR FRAMEPERIOD ${event.target.value} \n`) });
    document.getElementById("mpSensorGain").addEventListener("change", (event) =>               { sendWS(`SET IR GAIN ${event.target.value} \n`) });
    document.getElementById("mpSensorGainNumber").addEventListener("change", (event) =>         { sendWS(`SET IR GAIN ${event.target.value}\n`) });
    document.getElementById("mpSensorMinBrightness").addEventListener("change", (event) =>         { sendWS(`SET IR BRIGHTNESS ${event.target.value} \n`) });
    document.getElementById("mpSensorMinBrightnessNumber").addEventListener("change", (event) =>   { sendWS(`SET IR BRIGHTNESS ${event.target.value} \n`) });
    document.getElementById("mpSensorNoise").addEventListener("change", (event) =>              { sendWS(`SET IR NOISE ${event.target.value} \n`) });
    document.getElementById("mpSensorNoiseNumber").addEventListener("change", (event) =>        { sendWS(`SET IR NOISE ${event.target.value} \n`) });
    document.getElementById("mpSensorAverage").addEventListener("change", (event) =>            { sendWS(`SET IR AVERAGE ${event.target.value} \n`) });
    document.getElementById("mpSensorAverageNumber").addEventListener("change", (event) =>      { sendWS(`SET IR AVERAGE ${event.target.value} \n`) });
    document.getElementById("mpSensorDropDelay").addEventListener("change", (event) =>          { sendWS(`SET IR DROPDELAY ${event.target.value} \n`) });
    document.getElementById("mpSensorDropDelayNumber").addEventListener("change", (event) =>    { sendWS(`SET IR DROPDELAY ${event.target.value} \n`) });
    document.getElementById("mpSensorMirrorX").addEventListener("change", (event) =>            { sendWS(`SET CAL MIRRORX ${event.target.checked ? "1" : "0"} \n`) });
    document.getElementById("mpSensorMirrorY").addEventListener("change", (event) =>            { sendWS(`SET CAL MIRRORY ${event.target.checked ? "1" : "0"} \n`) });
    document.getElementById("mpSensorRotate").addEventListener("change", (event) =>             { sendWS(`SET CAL ROTATION ${event.target.checked ? "1" : "0"} \n`) });
    document.getElementById("mpSensorOffsetX").addEventListener("change", (event) =>            { sendWS(`SET CAL OFFSETX ${event.target.value} \n`) });
    document.getElementById("mpSensorOffsetXNumber").addEventListener("change", (event) =>      { sendWS(`SET CAL OFFSETX ${event.target.value} \n`) });
    document.getElementById("mpSensorOffsetY").addEventListener("change", (event) =>            { sendWS(`SET CAL OFFSETY ${event.target.value} \n`) });
    document.getElementById("mpSensorOffsetYNumber").addEventListener("change", (event) =>      { sendWS(`SET CAL OFFSETY ${event.target.value} \n`) });
    document.getElementById("mpSensorScaleX").addEventListener("change", (event) =>             { sendWS(`SET CAL SCALEX ${event.target.value} \n`) });
    document.getElementById("mpSensorScaleXNumber").addEventListener("change", (event) =>       { sendWS(`SET CAL SCALEX ${event.target.value} \n`) });
    document.getElementById("mpSensorScaleY").addEventListener("change", (event) =>             { sendWS(`SET CAL SCALEY ${event.target.value} \n`) });
    document.getElementById("mpSensorScaleYNumber").addEventListener("change", (event) =>       { sendWS(`SET CAL SCALEY ${event.target.value} \n`) });
    document.getElementById("mpSensorCalEn").addEventListener("change", (event) =>              { sendWS(`SET CAL CALIBRATION ${event.target.checked ? "1" : "0"} \n`) });
    document.getElementById("mpSensorOffsetEn").addEventListener("change", (event) =>           { sendWS(`SET CAL OFFSET ${event.target.checked ? "1" : "0"} \n`) });
    
});

/**
 * When a button is pressed, reload the page after 500ms
 */
function submitMessage() {
    //alert("Saved value to ESP SPIFFS");
    setTimeout(function(){ document.location.reload(false); }, 500);
}

/**
 * Hide or show display elements when a tab selector is pressed
 */
function openTab(tabName) {
    let content = document.getElementsByClassName("tabContent");
    for (let i=0; i<content.length; i++)
        content[i].style.display = "none";

    let buttons = document.getElementsByClassName("tabButtons");
    for (let i=0; i<buttons.length; i++) 
        buttons[i].className = buttons[i].className.replace(" active","");

    document.getElementById(tabName).style.display = "block";

    document.getElementById(tabName + "Btn").className = "tabButtons active";
}

/**
 * Start the websocket connection
 */
async function startWebsocket() {
    console.log("starting WS");
    const ip = 'ws://'+document.location.host + ":3000";
    console.log('IP',ip);
    ws = new WebSocket(ip);
    clearInterval(wsInterval);

    ws.onopen = function() {
        if (forceRefresh) window.location.href = window.location.href;
        console.log("Material Sensor: Websocket connected");
        wsOpen = true;
        clearInterval(wsInterval);
        wsInterval = setInterval(resetWS, 5000);
    }

    ws.onmessage = function(msg){
        analyzeMessage(msg.data);
        clearInterval(wsInterval);
        wsInterval = setInterval(resetWS, 5000);
    }

    clearInterval(wsInterval);
    wsInterval = setInterval(resetWS, 1000);
}

/**
 * Try to reset the websocket if a connection is lost
 */
 function resetWS(delay){
    clearInterval(wsInterval);
    if (wsOpen) {
        document.getElementById("waitMessage").style = "";
        document.getElementById("mainBody").style = "display:none";
        document.getElementById("waitMessageTxt").innerHTML = `
            Lost connection with the sensor.<br>
            Attempting to reestablish, please wait.`;

        wsOpen = false;
        console.log("Websocket disconnected");
        if (delay != undefined) setTimeout(startWebsocket,delay);
        else startWebsocket();
    }
    else if (ws.readyState == 3){
        console.log("Can't connect to websocket server");
        startWebsocket();
    }
}

/**
 * Send data to the websocket server
 * @param {String} txt Data to send
 */
function sendWS(txt){
    if (wsOpen)
        ws.send(txt);
}

const pointColors = ['#FF0000', '#00AD00', '#0000FF', '#FFFF00', '#FF00FF', '#7F00FF', '#007FFF', '#FF7F00', '#000000'];

/**
 * Analyze the data received from the websocket server
 * @param {String} msg Received data
 */
function analyzeMessage(msg) {
    //console.log("wsMessage",msg);
    let data;
    try {
        data = JSON.parse(msg);
    } catch (error) {
        console.warn("Could not parse JSON: " + msg);
        return;
    }
    //console.log('data',data)

    if (data.status == "ping") {
        document.getElementById("waitMessage").style = "display:none";
        document.getElementById("mainBody").style = "";
        document.getElementById("chargingState").innerHTML = data.battery.charging == 1 ? "Charging" : "Not Charging";
        document.getElementById("usbConnected").innerHTML = data.battery.usbActive == 1 ? "Yes" : "No";
        document.getElementById("batteryVoltage").innerHTML = `${data.battery.voltage.toFixed(2)}V`;
        document.getElementById("batteryPercentage").innerHTML = `${data.battery.percentage}%`;
    }
    else if (data.status == "update") {
        generalSettings = data.sett;
        calSettings = data.cal;
        irSettings = data.ir;
        networkSettings = data.network;
        fwVersion = data.firmware;
        hwVersion = data.hardware;

        let elements = document.getElementsByClassName("mpBeta");
        let display = hwVersion == 'Beta' ? '' : 'none';
        for (let elmnt of elements) elmnt.style.display = display;

        elements = document.getElementsByClassName("diy_basic");
        display = hwVersion == 'DIY Basic' ? 'none' : '';
        for (let elmnt of elements) elmnt.style.display = display;

        document.getElementById("hwVersion").innerHTML = hwVersion;
        document.getElementById("fwVersion").innerHTML = `v${fwVersion}`;
        document.getElementById("debugEn").checked = generalSettings.debug;
        document.getElementById("serialOut").checked = generalSettings.serialOut;

        document.getElementById("connectionStatus").innerHTML = networkSettings.connected ? "Connected" : "Not Connected";
        document.getElementById("ssid").innerHTML = `"${networkSettings.ssid}"`;
        document.getElementById("ipAddress").innerHTML = networkSettings.ipAddress;
        document.getElementById("deviceName").value = networkSettings.name;
        document.getElementById("wsMode").innerHTML = networkSettings.wsMode;
        document.getElementById("wsPort").value = networkSettings.wsPort;
        let clients = `${networkSettings.wsClients.length}`;
        clients += networkSettings.wsClients.length == 1 ? ` client<br>` : ` clients<br>`;
        for (let i=0; i<networkSettings.wsClients.length; i++)
            clients += `${i+1}: ${networkSettings.wsClients[i]}<br>`;
        document.getElementById("wsClients").innerHTML = clients;

        document.getElementById("mpSensorFramePeriod").value=irSettings.framePeriod;
        document.getElementById("mpSensorFramePeriodNumber").value=irSettings.framePeriod;
        document.getElementById("mpSensorExposure").value=irSettings.exposure;
        document.getElementById("mpSensorExposureNumber").value=irSettings.exposure;
        document.getElementById("mpSensorGain").value=irSettings.gain;
        document.getElementById("mpSensorGainNumber").value=irSettings.gain;

        document.getElementById("mpSensorMinBrightness").value=irSettings.brightness;
        document.getElementById("mpSensorMinBrightnessNumber").value=irSettings.brightness;
        document.getElementById("mpSensorNoise").value=irSettings.noise;
        document.getElementById("mpSensorNoiseNumber").value=irSettings.noise;
        document.getElementById("mpSensorAverage").value=irSettings.averageCount;
        document.getElementById("mpSensorAverageNumber").value=irSettings.averageCount;
        document.getElementById("mpSensorDropDelay").value=irSettings.dropDelay;
        document.getElementById("mpSensorDropDelayNumber").value=irSettings.dropDelay;

        document.getElementById("mpSensorMirrorX").checked=calSettings.mirrorX;
        document.getElementById("mpSensorMirrorY").checked=calSettings.mirrorY;
        document.getElementById("mpSensorRotate").checked=calSettings.rotation;
        document.getElementById("mpSensorOffsetX").value=calSettings.offsetX;
        document.getElementById("mpSensorOffsetXNumber").value=calSettings.offsetX;
        document.getElementById("mpSensorOffsetY").value=calSettings.offsetY;
        document.getElementById("mpSensorOffsetYNumber").value=calSettings.offsetY;
        document.getElementById("mpSensorScaleX").value=calSettings.scaleX;
        document.getElementById("mpSensorScaleXNumber").value=calSettings.scaleX;
        document.getElementById("mpSensorScaleY").value=calSettings.scaleY;
        document.getElementById("mpSensorScaleYNumber").value=calSettings.scaleY;
        document.getElementById("mpSensorCalEn").checked=calSettings.calibrationEnable;
        document.getElementById("mpSensorOffsetEn").checked=calSettings.offsetEnable;
    }
    else if (data.status == "wifiStations") {
        const stations = data.data;
        ssidElement = document.getElementById("APs");
        for (let i=0; i<stations.length; i++) {
            const station = stations[i];
            let newOption = document.createElement("option");
            newOption.value = `"${station.ssid}"`;
            newOption.innerHTML = `${station.ssid} (${station.rssi}dBm/${station.authMode})`;
            ssidElement.appendChild(newOption);
        }
    }
    else if (data.status == "IR data") {
        const points = data.data;
        
        for (let i=0; i<4; i++) {
            let point = points[i];
            if (point == undefined || isNaN(point.x)  || isNaN(point.y)) {
                point = {
                    x: 0,
                    y: 0,
                    avgBrightness: 0,
                    maxBrightness: 0,
                    area: 0,
                    radius: 0,
                    id: 0,
                }
                
            }
            if (i == 0) {
                document.getElementById("baseId").innerHTML = point?.id == undefined ? 0 : point.id;
                document.getElementById("baseCmd").innerHTML = point?.command == undefined ? 0 : point.command;
            }
            
            document.getElementById(`cal_x${i}`).innerHTML = Math.round(point.x);
            document.getElementById(`cal_y${i}`).innerHTML = Math.round(point.y);
            document.getElementById(`cal_ab${i}`).innerHTML = point.avgBrightness == undefined ? "NA" : Math.round(point.avgBrightness);
            document.getElementById(`cal_mb${i}`).innerHTML = Math.round(point.maxBrightness);
            document.getElementById(`cal_a${i}`).innerHTML = point.area == undefined ? "NA" : Math.round(point.area);


            let color = "black";
            if (data.x < 0 || data.x > 4096 || data.y < 0 || data.y > 4096) color = "red";
            if (data.maxBrightness == 0) color = "grey";

            document.getElementById(`iteration${i}`).style.color = (point.maxBrightness == 0) ? 'grey' : pointColors[i];
            document.getElementById(`cal_x${i}`).style.color=color;
            document.getElementById(`cal_y${i}`).style.color=color;
            document.getElementById(`cal_ab${i}`).style.color=color;
            document.getElementById(`cal_mb${i}`).style.color=color;
            document.getElementById(`cal_a${i}`).style.color=color;
        }

        let stage = document.getElementById('stage');
        if(stage.getContext) {
            var ctx = stage.getContext('2d');
        }
        ctx.fillStyle = '#FF0000';
        ctx.clearRect(0, 0, stage.width, stage.height);
        for (let point of points) {
            if (point == undefined || point.x == undefined) continue;
            if (point.x > 0 && point.y < 4096) {
                const x = point.x/4096*stage.width;
                const y = point.y/4096*stage.height;
                ctx.fillStyle = pointColors[point.point];
                ctx.beginPath();
                ctx.arc(x,y,3,0,2*Math.PI,false);
                ctx.fill();
                ctx.fillText(point.point, x + 5, y + 10);
            }
        }
    }
}

$('form').submit(function(e){
    e.preventDefault();
    const fileName = document.getElementById("updateFirmware").value;
    if (fileName === '') {
      console.log('No file selected')
      return;
    }
    console.log(`Uploading file '${fileName}' to the sensor`)
    
    var form = $('#upload_form')[0];
    var data = new FormData(form);
    let done = false;
    document.getElementById("prgBar").style.display = "";

    $.ajax({
      url: '/update',
      type: 'POST',
      data: data,
      contentType: false,
      processData:false,
      xhr: function() {
        var xhr = new window.XMLHttpRequest();
        xhr.upload.addEventListener('progress', function(evt) {
          if (evt.lengthComputable) {
            var per = evt.loaded / evt.total;
            //$('#prg').html('progress: ' + Math.round(per*100) + '%');
            document.getElementById("prg").style.width = Math.round(per*100) + '%';
            console.log('progress: ' + Math.round(per*100) + '%');
            if (done == false && Math.round(per*100) == 100) {
                console.log('Upload done');
                done = true;
                ws.close();
                setTimeout(()=>{
                    resetWS(20000);
                    //$('#prg').html('progress: 0%');
                    document.getElementById("prg").style.width = 0 + '%';
                    document.getElementById("prgBar").style.display = "none";
                    if (fileName.includes('webserver')) forceRefresh = true;
                },1000);
                
            }
          }
        }, false);
        return xhr;
      },
      success:function(d, s) {console.log('success!')},
      error: function (a, b, c) {}
    });
  });