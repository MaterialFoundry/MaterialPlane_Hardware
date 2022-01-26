let ws;                         //Websocket variable
let wsOpen = false;             //Bool for checking if websocket has ever been opened => changes the warning message if there's no connection
let wsInterval;                 //Interval timer to detect disconnections
let generalSettings = {};
let calSettings = {};
let irSettings = {};
let networkSettings = {};
let fwVersion;
let hwVersion;

const webserverVersion = "v1.0.1";

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
    document.getElementById("updatePort").addEventListener("click", (event) =>          { sendWS(`SET WS PORT ${document.getElementById("wsPort").value} \n`) });

    document.getElementById("autoExposure").addEventListener("click", (event) =>        { sendWS(`SET IR AUTOEXPOSE \n`) });
    document.getElementById("exposure").addEventListener("change", (event) =>           { sendWS(`SET IR EXPOSURE ${event.target.value/100} \n`) });
    document.getElementById("exposureNumber").addEventListener("change", (event) =>     { sendWS(`SET IR EXPOSURE ${event.target.value} \n`) });
    document.getElementById("framePeriod").addEventListener("change", (event) =>        { sendWS(`SET IR FRAMEPERIOD ${event.target.value/10} \n`) });
    document.getElementById("framePeriodNumber").addEventListener("change", (event) =>  { sendWS(`SET IR FRAMEPERIOD ${event.target.value} \n`) });
    document.getElementById("gain").addEventListener("change", (event) =>               { sendWS(`SET IR GAIN ${event.target.value/10} \n`) });
    document.getElementById("gainNumber").addEventListener("change", (event) =>         { sendWS(`SET IR GAIN ${event.target.value}\n`) });
    document.getElementById("brightness").addEventListener("change", (event) =>         { sendWS(`SET IR BRIGHTNESS ${event.target.value} \n`) });
    document.getElementById("brightnessNumber").addEventListener("change", (event) =>   { sendWS(`SET IR BRIGHTNESS ${event.target.value} \n`) });
    document.getElementById("noise").addEventListener("change", (event) =>              { sendWS(`SET IR NOISE ${event.target.value} \n`) });
    document.getElementById("noiseNumber").addEventListener("change", (event) =>        { sendWS(`SET IR NOISE ${event.target.value} \n`) });
    document.getElementById("average").addEventListener("change", (event) =>            { sendWS(`SET IR AVERAGE ${event.target.value} \n`) });
    document.getElementById("averageNumber").addEventListener("change", (event) =>      { sendWS(`SET IR AVERAGE ${event.target.value} \n`) });
    document.getElementById("mirX").addEventListener("change", (event) =>               { sendWS(`SET CAL MIRRORX ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("mirY").addEventListener("change", (event) =>               { sendWS(`SET CAL MIRRORY ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("rot").addEventListener("change", (event) =>                { sendWS(`SET CAL ROTATION ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("xOffset").addEventListener("change", (event) =>            { sendWS(`SET CAL OFFSETX ${event.target.value} \n`) });
    document.getElementById("xOffsetNumber").addEventListener("change", (event) =>      { sendWS(`SET CAL OFFSETX ${event.target.value} \n`) });
    document.getElementById("yOffset").addEventListener("change", (event) =>            { sendWS(`SET CAL OFFSETY ${event.target.value} \n`) });
    document.getElementById("yOffsetNumber").addEventListener("change", (event) =>      { sendWS(`SET CAL OFFSETY ${event.target.value} \n`) });
    document.getElementById("xScale").addEventListener("change", (event) =>             { sendWS(`SET CAL SCALEX ${event.target.value} \n`) });
    document.getElementById("xScaleNumber").addEventListener("change", (event) =>       { sendWS(`SET CAL SCALEX ${event.target.value} \n`) });
    document.getElementById("yScale").addEventListener("change", (event) =>             { sendWS(`SET CAL SCALEY ${event.target.value} \n`) });
    document.getElementById("yScaleNumber").addEventListener("change", (event) =>       { sendWS(`SET CAL SCALEY ${event.target.value} \n`) });
    document.getElementById("calEn").addEventListener("change", (event) =>              { sendWS(`SET CAL CALIBRATION ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("offsetEn").addEventListener("change", (event) =>           { sendWS(`SET CAL OFFSET ${event.target.checked ? "TRUE" : "FALSE"} \n`) });
    document.getElementById("calBtn").addEventListener("click", (event) => {
        let msg = "PERFORM CALIBRATION ";
        if (document.getElementById("calMethod").value == "SinglePoint")
            msg += "SINGLE";
        else if (document.getElementById("calMethod").value == "MultiPoint")
            msg += "MULTI";
        else if (document.getElementById("calMethod").value == "Offset")
            msg = "OFFSET";
        msg += " \n";
        sendWS(msg);
    });
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
    const ip = `ws://${ipAddress}:${wsPort}`;
    console.log('IP',ip);
    ws = new WebSocket(ip);
    clearInterval(wsInterval);

    ws.onopen = function() {
        console.log("Material Sensor: Websocket connected",ws);
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
 function resetWS(){
    if (wsOpen) {
        document.getElementById("waitMessage").style = "";
        document.getElementById("mainBody").style = "display:none";
        document.getElementById("waitMessageTxt").innerHTML = `
            Lost connection with the sensor.<br>
            Attempting to reestablish, please wait.`;

        wsOpen = false;
        console.log("Websocket disconnected");
        startWebsocket();
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

/**
 * Analyze the data received from the websocket server
 * @param {String} msg Received data
 */
function analyzeMessage(msg) {
    //console.log("wsMessage",msg);
    let data = JSON.parse(msg);
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

        document.getElementById("hwVersion").innerHTML = hwVersion;
        document.getElementById("fwVersion").innerHTML = `v${fwVersion}`;
        document.getElementById("debugEn").checked = generalSettings.debug;
        document.getElementById("serialOut").checked = generalSettings.serialOut;

        if (hwVersion == "DIY Full" || hwVersion == "DIY Basic") {
            let elements = document.getElementsByClassName("PAJsensor");
            for (let element of elements) 
                element.style.display = "none";
        }

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

        document.getElementById("exposure").value = irSettings.exposure*100;
        document.getElementById("exposureNumber").value = irSettings.exposure;
        document.getElementById("framePeriod").value = irSettings.framePeriod*10;
        document.getElementById("framePeriodNumber").value = irSettings.framePeriod;
        document.getElementById("gain").value = irSettings.gain*10;
        document.getElementById("gainNumber").value = irSettings.gain;
        document.getElementById("brightness").value = irSettings.brightness;
        document.getElementById("brightnessNumber").value = irSettings.brightness;
        document.getElementById("noise").value = irSettings.noise;
        document.getElementById("noiseNumber").value = irSettings.noise;
        document.getElementById("average").value = irSettings.averageCount;
        document.getElementById("averageNumber").value = irSettings.averageCount;
        document.getElementById("mirX").checked = calSettings.mirrorX;
        document.getElementById("mirY").checked = calSettings.mirrorY;
        document.getElementById("rot").checked = calSettings.rotation;
        document.getElementById("xOffset").value = calSettings.offsetX;
        document.getElementById("xOffsetNumber").value = calSettings.offsetX;
        document.getElementById("yOffset").value = calSettings.offsetY;
        document.getElementById("yOffsetNumber").value = calSettings.offsetY;
        document.getElementById("xScale").value = calSettings.scaleX;
        document.getElementById("xScaleNumber").value = calSettings.scaleX;
        document.getElementById("yScale").value = calSettings.scaleY;
        document.getElementById("yScaleNumber").value = calSettings.scaleY;
        document.getElementById("calEn").checked = calSettings.calibrationEnable;
        document.getElementById("offsetEn").checked = calSettings.offsetEn;
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
            const point = points[i];
            if (point == undefined) continue;
            if (i == 0) {
                document.getElementById("baseId").innerHTML=point.id;
                document.getElementById("baseCmd").innerHTML=point.command;
            }
            if (point != undefined && point.x != undefined) {
                document.getElementById(`cal_x${i}`).innerHTML = point.x;
                document.getElementById(`cal_y${i}`).innerHTML = point.y;
                document.getElementById(`cal_ab${i}`).innerHTML = point.avgBrightness == undefined ? "NA" : point.avgBrightness;
                document.getElementById(`cal_mb${i}`).innerHTML = point.maxBrightness;
                document.getElementById(`cal_a${i}`).innerHTML = point.area == undefined ? "NA" : point.area;
            }
            else {
                document.getElementById(`cal_x${i}`).innerHTML = 0;
                document.getElementById(`cal_y${i}`).innerHTML = 0;
                document.getElementById(`cal_ab${i}`).innerHTML = 0;
                document.getElementById(`cal_mb${i}`).innerHTML = 0;
                document.getElementById(`cal_a${i}`).innerHTML = 0;
            }

            let color = "black";
            if (data.x < 0 || data.x > 4096 || data.y < 0 || data.y > 4096) color = "red";
            if (data.maxBrightness == 0) color = "grey";

            document.getElementById(`iteration${i}`).style.color=color;
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
        ctx.clearRect(0, 0, 400, 250);
        for (let point of points) {
            if (point.x > 0 && point.y < 4096) {
                ctx.beginPath();
                ctx.arc(point.x/4096*400,point.y/4096*250,3,0,2*Math.PI,false);
                ctx.fill();
            }
        }
    }
}