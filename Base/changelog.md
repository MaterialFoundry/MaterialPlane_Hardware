# Changelog Material Plane Base Firmware
### v1.1.0 - 20-11-2021
Fixes:
<ul>
    <li>There were situations where the base would deactivate even though it was tilted. This issue should be greatly reduced</li>
</ul>

Additions:
<ul>
    <li>Added a new 'calibration mode'. When in this mode, the base will be always enabled, which can help with aligning and calibrating the sensor. This mode can be entered by holding the 
        base upside down while switching it on, the red LED will blink to indicate it is in calibration mode. To switch back to normal mode: hold base normal side up, and switch it off and on.
    </li>
</ul>

Other:
<ul>
    <li>Switched over to hardware SPI instead of bit-banging, which makes communication with the accelerometer faster</li>
    <li>Some code cleanup and documentation improvements</li>
</ul>

### v1.0.0 - 05-11-2021
Initial release