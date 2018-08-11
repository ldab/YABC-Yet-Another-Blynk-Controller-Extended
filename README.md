# YABC-Yet-Another-Blynk-Controller-Extended
<h4>Extended version of a Cloud Controller based on ESP8266 and Blynk - Yet Another Blynk Controller a ESP8266 based 3x relay controller with Blynk APP. Control temperature, humidity and others thru the Cloud</h4>
<ul>
<li>YABC can be found at:&nbsp;<a href="http://untrol.io/" rel="nofollow">untrol.io</a>&nbsp;and&nbsp;<a href="https://www.tindie.com/" rel="nofollow">Tindie</a></li>
<li>This is an extended version of Blynk App you will need to buy some energy as the App requires 4,400 energy points, you'll find a simpler version at:&nbsp;<a href="https://github.com/ldab/YABC-Yet-Another-Blynk-Controller-SIMPLE" target="_blank" rel="noopener">YABC Simple</a>&nbsp;and you won't need to buy energy initially.</li>
</ul>
<p><a href="https://www.tindie.com/stores/lbispo/?ref=offsite_badges&amp;utm_source=sellers_lbispo&amp;utm_medium=badges&amp;utm_campaign=badge_medium"><img style="display: block; margin-left: auto; margin-right: auto;" src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-mediums.png" alt="I sell on Tindie" width="150" height="78" /></a></p>
<p>&nbsp;</p>

<p>The firmware has been writen using PlatformIO which is a nice, neat IDE, check it out:&nbsp;<a href="https://platformio.org/platformio-ide" rel="nofollow">Learn how to install PlatformIO IDE</a></p>
<p>If you're feeling brave and want to customize or improve the Firmware Over the Air (OTA), check this link:&nbsp;<a title="Over-the-Air (OTA) update" href="http://docs.platformio.org/en/latest/platforms/espressif8266.html#over-the-air-ota-update" rel="nofollow">Over-the-Air (OTA) update</a>&nbsp;basically you change your sketch and upload it by running the following line on PlatformIO terminal:</p>
<blockquote>
<p><em>platformio run --target upload --upload-port IP_ADDRESS_HERE</em></p>
</blockquote>
<h3><a id="user-content-or-if-you-want-to-use-arduino-idearduino-ide-ota" class="anchor" href="https://github.com/ldab/YABC---Yet-Another-Blynk-Controller/blob/master/README.md#or-if-you-want-to-use-arduino-idearduino-ide-ota" aria-hidden="true"></a><a id="user-content-or-if-you-want-to-use-arduino-idearduino-ide-ota" href="https://github.com/ldab/MODBridge-and-Blynk/blob/master/README.md#or-if-you-want-to-use-arduino-idearduino-ide-ota"></a>OR if you want to use Arduino IDE:&nbsp;<a href="https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#arduino-ide" rel="nofollow">Arduino IDE OTA</a></h3>
<h3><a id="user-content-download-blynk-appgetting-started-with-blynk" class="anchor" href="https://github.com/ldab/YABC---Yet-Another-Blynk-Controller/blob/master/README.md#download-blynk-appgetting-started-with-blynk" aria-hidden="true"></a><a id="user-content-download-blynk-app-getting-started-with-blynk" href="https://github.com/ldab/MODBridge-and-Blynk/blob/master/README.md#download-blynk-app-getting-started-with-blynk"></a>Download Blynk App:&nbsp;<a href="https://www.blynk.cc/getting-started/" rel="nofollow"><strong>Getting Started with Blynk</strong></a></h3>
<ol>
<li>Download Blynk App:&nbsp;<a href="http://j.mp/blynk_Android" rel="nofollow">http://j.mp/blynk_Android</a>&nbsp;or&nbsp;<a href="http://j.mp/blynk_iOS" rel="nofollow">http://j.mp/blynk_iOS</a></li>
<li>Touch the QR-code icon and point the camera to the code below
<ul>
<li style="text-align: center;"><img src="https://image.ibb.co/gxZFDz/Untitled.png" alt="" width="274" height="98" /></li>
</ul>
</li>
<li>Enjoy my app!</li>
</ol>
<p style="text-align: center;"><img src="https://image.ibb.co/hZ9Afe/Screenshot_20180803_114522_Blynk.jpg" alt="" width="188" height="334" />&nbsp;<img src="https://image.ibb.co/d0RbwK/Screenshot_20180808_122157_Blynk.jpg" alt="" width="188" height="334" />&nbsp;<img src="https://image.ibb.co/fOhJY9/clone_1181297599.png" alt="" width="230" height="230" /></p>

<h2>How to start?</h2>
<p>I will send the YABC board with the firmware already flashed, has all you need to control Heating and Cooling using Blynk Cloud App.</p>
<p>Briefly press the buton and the board will start the Access point mode YABC- followed by the 6 last digits of the board MAC address:</p>
<p><img style="display: block; margin-left: auto; margin-right: auto;" src="https://image.ibb.co/fbxcAp/blynk_iot_temperature_controller.jpg" alt="Blynk iot temperature controller" width="200" height="356" /></p>
<p>Connect to the network (no password required) and the Captive Portal should start automatically, if it doesn't start, please go to your internet browser and go to <a href="http://192.168.4.1/">192.168.4.1</a>&nbsp;the following configuration screens will be available, insert your WiFi credentials, Blynk Authentication number retrieved from the App when clonning the above project and select your temperature probe and save.</p>
<p><img src="https://image.ibb.co/hUwCAp/untrol_yabc_wifimanager_2.jpg" alt="" width="200" height="356" />&nbsp;<img src="https://image.ibb.co/ebviGU/untrol_yabc_wifimanager_3.jpg" alt="" width="200" height="356" />&nbsp;<img src="https://image.ibb.co/cgwZO9/untrol_yabc_wifimanager_1.jpg" alt="" width="200" height="356" /></p>
<p>After savind the configuration the board will restart and connect to the WiFi informed and if everything is right you can start using your Blynk App and Control the temperature with your own IoT controller.</p>
