# Changelog Material Plane Sensor Firmware
### v2.1.2 - 09-11-2021
Fixes:
<ul>
    <li>Messed up something in the last update that greatly reduced the sensitivity of the ID sensor, this should be fixed now</li>
</ul>

### v2.1.1 - 05-11-2021
Fixes:
<ul>
    <li>Fixed a problem with the IR32 library that resulted in compilation errors</li>
    <li>Improved battery voltage calculation on beta hardware</li>
</ul>

Other:
<ul>
    <li>On boot, LEDs switch on earlier, instead of waiting for WiFi to connect, so you get faster feedback about the sensor being on</li>
    <li>Made charging led fade smoother</li>
</ul>

### v2.1.0 - 12-10-2021
Additions:
<ul>
    <li>Added auto exposure procedure (beta hw only)</li>
    <li>Added a device name (DNS name), which allows the user to configure a name, and connect to the sensor through "[name].local" ("materialsensor.local", by default)</li>
    <li>The sensor now hosts a webserver that allows sensor configuration accessible through "[name].local" ("materialsensor.local", by default). This can be accessed through any web browser. <b>Please check the documentation on github on how to upload the webserver data to the sensor</b></li>
    <li>If the sensor cannot connect to an access point, it will create its own. This allows users to set up the sensor through a web browser, instead of using the serial port. Access point name is "[name]" ("materialsensor", by default). It redirects all traffic to its configuration page, so to access it, the user can try to connect to any website</li>
    <li>When charging the battery, the charge indicator now fades instead of blinking</li>
</ul>

Fixes:
<ul>
    <li>Rotation now works</li>
    <li>Rotation is now performed before mirroring, so the correct axis is mirrored</li>
    <li>Websocket server will now start whenever WiFi is connected, not only at boot</li>
</ul>

Other:
<ul>
    <li>Battery percentage calculation has been moved to the sensor and has been made more accurate (it's still a rough estimate, though)</li>
    <li>Minor changes to the communication protocol between sensor and module</li>
</ul>

### v2.0.1 - 16-07-2021
Fixes:
<ul>
    <li>When no bases are found, sensor now sends correct command to drop the token</li>
    <li>Fixed issue where sensor would stay in calibration mode if calibration was not successfully completed</li>
</ul>

### v2.0.0 - 15-07-2021
Initial release of v2