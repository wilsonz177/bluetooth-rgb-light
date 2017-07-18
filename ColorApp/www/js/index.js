console.log("starting index.js");

var SCAN_TIME = 7000;           // Scan for 7 seconds
var CONNECTION_TIMEOUT = 7000;  // Wait for 7 seconds for a valid connection

// *********   Global variables used for all IDs that are used in multiple functions
var refreshDevicesModal = null;
var connectingModal = null;
var deviceList = null;
var deviceObjectMap = null;
var pageNavigator = null;
var connectingDevice = null;
var connectionTimeout = null;

var lightSwitch = null;

var myService        = "a337c90b-4e78-469e-905f-8a3248257e45";
var lightStatusChar = "a02f";
var rgbChar = "b13a";
var alarmOnTimeChar= "c25d";
var alarmOnChar= "d95e";
var alarmOffTimeChar = "e67b";
var alarmOffChar = "f22f";
var timeChar = "0ad7";
var onAtChar = "1be8";
var offAtChar = "2cf9";
var rgbDefaultChar = "3fa1";

// *********   Functions for scanning and scan related events


function scanFailed() {
    refreshDevicesModal.hide();
}

function scanStop() {
    ble.stopScan();
    refreshDevicesModal.hide();
}

function deviceDiscovered(device) {
    // Debugging: Console log of details of item
    // console.log(JSON.stringify(device));

    if(deviceObjectMap.get(device.id) === undefined ) {
        // New Device. Add it to the collection and to the window
        deviceObjectMap.set(device.id, device);

        // Identify the name or use a default
        var name = "(none)";
        if (device.name !== undefined) {
            name = device.name;
        }

        // Create the Onsen List Item
        var item = ons._util.createElement('<ons-list-item modifier="chevron" tappable> ' +
            '<ons-row><ons-col><span class="list-item__title" style="font-size: 4vw;">' + device.id + '</span></ons-col></ons-row>' +
            '<ons-row><ons-col><span class="list-item__subtitle" style="font-size: 2vw;">RSSI:' + device.rssi + '</span></ons-col><ons-col><span style="font-size: 2vw;">Name: ' + name + '</span></ons-col></ons-row>' +
            '</ons-list-item>');

        // Set the callback function
        item.addEventListener('click', deviceSelected, false);

        // Associate the element in the list with the object
        item.device = device;

        // Iterate through the list and add item in place by RSSI
        var descendants = deviceList.getElementsByTagName('ons-list-item');
        var i;
        for(i=0;i<descendants.length;i++) {
            if(device.rssi > descendants[i].device.rssi) {
                descendants[i].parentNode.insertBefore(item, descendants[i]);
                return;
            }
        }
        // If it hasn't already returned, it wasn't yet inserted.
        deviceList.append(item);
    }
}

function startScan() {
    // Disable the window
    refreshDevicesModal.show();

    // Empty the list (on screen and Map)
    deviceList.innerHTML = "";
    deviceObjectMap = new Map();

    // Start the scan
    ble.scan([], SCAN_TIME, deviceDiscovered, scanFailed);

    // Re-enable the window when scan done
    setTimeout(scanStop, SCAN_TIME);
}

// ***** BLE Functions ********

//receives info on whether or not the light is on and off
//flips the switch depending on the data
function lightStatusData(buffer) {
    var array = new Uint8Array(buffer);
    console.log("ayo", array[0]);
    lightSwitch.checked =  (array[0] !== 0);
    
}

//converts the color number received from the RedBearDuo and scales it out of 100
function convertColorValue(num){
  return (num / 255) * 100;
}

//converts the color number received from the range slider to scale it out of 255
function unconvertColorValue(num){
  return (num / 100) * 255;
}

//callback function for when ble successfully reads the current color
//changes the value displayed  in both text and on the ranges with their respective colors
function rgbCharRead(buffer){
  var array1 = new Uint8Array(buffer);
  console.log("red", array1[0], "green", array1[1], "blue", array1[2]);
  var red = convertColorValue(array1[0]);
  var green = convertColorValue(array1[1]);
  var blue = convertColorValue(array1[2]);
  document.getElementById('currentRedValue').textContent = red;
  document.getElementById('currentGreenValue').textContent = green;
  document.getElementById('currentBlueValue').textContent = blue;
  document.getElementById('currentRedRange').value = red;
  document.getElementById('currentGreenRange').value = green;
  document.getElementById('currentBlueRange').value = blue;
}

//callback function for when ble successfully reads the default color
//changes the value displayed  in both text and on the ranges with their respective colors
function rgbDefaultCharRead(buffer){
  var array2 = new Uint8Array(buffer);
  console.log("red", array2[0], "green", array2[1], "blue", array2[2]);
  var red = convertColorValue(array2[0]);
  var green = convertColorValue(array2[1]);
  var blue = convertColorValue(array2[2]);
  document.getElementById('defaultRedValue').textContent = red;
  document.getElementById('defaultGreenValue').textContent = green;
  document.getElementById('defaultBlueValue').textContent = blue;
  document.getElementById('defaultRedRange').value = red;
  document.getElementById('defaultGreenRange').value = green;
  document.getElementById('defaultBlueRange').value = blue;
}


//call back function changes the switch on the on alarm
function alarmOnCharRead(buffer){
    var array = new Uint8Array(buffer);
    
    document.getElementById('onTimerSwitch').checked = (array[0] !== 0);
    console.log("alarm on char buffer: ", array[0]);
}

//changes time shown on the on alarm
function alarmOnTimeCharRead(buffer){
    var array = new Uint8Array(buffer);
    
    //var t = array[0];
    document.getElementById('onTimerTime').textContent = array[0];
    document.getElementById('onTimerInput').value = array[0];
    console.log("alarm on time char buffer: ", array[0]);
}

//changes the switch show on the off alarm
function alarmOffCharRead(buffer){
    var array = new Uint8Array(buffer);
    
    document.getElementById('offTimerSwitch').checked = (array[0] !== 0);
    console.log("alarm off char buffer: ", array[0]);
}


//changes the time shown on the off alarm
function alarmOffTimeCharRead(buffer){
    var array = new Uint8Array(buffer);
    
    //var t = array[0];
    document.getElementById('offTimerTime').textContent = array[0];
    document.getElementById('offTimerInput').value = array[0];
    console.log("alarm off time char buffer: ", array[0]);
}

//changes the onAt info shown on  screen
function onAtCharRead(buffer){
    var array = new Uint8Array(buffer);
    document.getElementById('onAlarmSwitch').checked = (array[0] !== 0);
    document.getElementById('onAlarmTimeHour').textContent = array[1];
    document.getElementById('onAlarmTimeMinute').textContent = array[2];
    document.getElementById('onAlarmTimeSecond').textContent = array[3];
    document.getElementById('onAlarmHourInput'). value = array[1];
    document.getElementById('onAlarmMinuteInput'). value = array[2];
    document.getElementById('onAlarmSecondInput'). value = array[3];
    console.log('finished the read');
}

//changes
function offAtCharRead(buffer){
    var array = new Uint8Array(buffer);
    document.getElementById('offAlarmSwitch').checked = (array[0] !== 0);
    document.getElementById('offAlarmTimeHour').textContent = array[1];
    document.getElementById('offAlarmTimeMinute').textContent = array[2];
    document.getElementById('offAlarmTimeSecond').textContent = array[3];
    document.getElementById('offAlarmHourInput'). value = array[1];
    document.getElementById('offAlarmMinuteInput'). value = array[2];
    document.getElementById('offAlarmSecondInput'). value = array[3];
    console.log('finished the read2');
}




// ********   Functions for device connection related events

function deviceConnectionSuccess(device) {
    clearTimeout(connectionTimeout);
    connectingModal.hide();
    connectingDevice = device;
    console.log('connection success');
    //start receiving notifications of the light status
    ble.startNotification(connectingDevice.id, myService, lightStatusChar, lightStatusData, failure);
}

function success(){
  console.log("ble success");
}

function failure() {
    console.log("ble failed");
}

function deviceConnectionFailure(device) {
    console.log("Device Disconnected");
    pageNavigator.popPage();
    refreshDevicesModal.hide();
    connectingDevice = null;
}

function deviceConnectionTimeout() {
    // Device connection failure
    connectingModal.hide();
    pageNavigator.popPage();
    refreshDevicesModal.hide();
    if(connectingDevice !== null) {
        clearInterval(connectingDevice.pollingTimer);
        ble.disconnect(connectingDevice.id);
    }
}

function disconnectDevice() {
    console.log("Disconnecting");
    if(connectingDevice !== null) {
        clearInterval(connectingDevice.pollingTimer);
        ble.disconnect(connectingDevice.id);
    }
}

// ***** Function for user-interface selection of a device
function deviceSelected(evt) {
    var device = evt.currentTarget.device;
    // Initiate a connection and switch screens; Pass in the "device" object
    pageNavigator.pushPage('page1.html', {data: {device: evt.currentTarget.device}});
    connectingDevice = device;
    ble.connect(device.id, deviceConnectionSuccess, deviceConnectionFailure);
    connectionTimeout = setTimeout(deviceConnectionTimeout, CONNECTION_TIMEOUT);
}


// *****  Function for initial startup

ons.ready(function() {
  console.log('ready');
  console.log('fuck yeah im ready');
   // Initialize global variables
    refreshDevicesModal = document.getElementById('refreshDevicesModal');
    pageNavigator = document.querySelector('#pageNavigator');
    

    var refreshButton = document.getElementById('refreshButton');
    refreshButton.addEventListener('click',  function() {
            console.log("Refresh; Showing modal");
            startScan();
    } );

    deviceList = document.getElementById('deviceList');

    // Add a "disconnect" when app auto-updates
    if(typeof window.phonegap !== 'undefined') {
        // Works for iOS (not Android)
        var tmp = window.phonegap.app.downloadZip;
        window.phonegap.app.downloadZip = function (options) {
            disconnectDevice();
            tmp(options);
        };
    }

    var pullHook = document.getElementById('pull-hook');
    pullHook.onAction = function(done) {
        startScan();
        // Call the "done" function in to hide the "Pull to Refresh" message (but delay just a little)
        setTimeout(done, 500);
    };

    
});


// *** Functions for page navigation (page change) events

document.addEventListener('init', function(event) {
  
  var page = event.target;
  
  if (page.id === 'page1') {
    
     // Enable the modal window
    connectingModal = document.getElementById('connectingModal');
     lightSwitch = document.querySelector('#lightSwitch');
    connectingModal.show();
    
    lightSwitch.addEventListener("change", function(event){
      console.log("switch: ", event.switch.checked);
      var data2 = new Uint8Array(1);
      
      data2[0] = event.switch.checked ? 3 : 2;
      
      ble.write(connectingDevice.id, myService, lightStatusChar, data2.buffer, success, failure);
      console.log('did i happen');
    });
    
    //event listener for resetting the time
    page.querySelector('#resetTime').addEventListener("click", function(){
        console.log('reset time button worked');
      var d = new Date();
      console.log(d);
      var d1 = parseInt(d.getTime()/1000);
      console.log(d1);
      var data7 = new Uint8Array(4);
      data7[0] = d1 >> 24;
      data7[1] = d1 >> 16;
      data7[2] = d1 >> 8;
      data7[3] = d1;
      ble.write(connectingDevice.id, myService, timeChar, data7.buffer, success, failure);
      page.querySelector("#timeSpan").textContent = d.toDateString();
    });
   
    console.log('also here');
    page.querySelector('#onFade').addEventListener("click", function(){
      var data3 = new Uint8Array(1);
      data3[0] = 1;
      console.log("fade on pls");
      ble.write(connectingDevice.id, myService, lightStatusChar, data3.buffer, success, failure);
    });
    page.querySelector('#offFade').addEventListener("click", function(){
      var data4 = new Uint8Array(1);
      data4[0] = 0;
      console.log("fade off pls");
      ble.write(connectingDevice.id, myService, lightStatusChar, data4.buffer, success, failure);
    });
    page.querySelector('#alarms0').addEventListener("click", function() {
      document.querySelector('#pageNavigator').pushPage('alarms.html');
      console.log('show me alarms');
    });
    page.querySelector('#timers0').addEventListener("click", function() {
      document.querySelector('#pageNavigator').pushPage('timers.html');
    });

    page.querySelector('#changeCurrentColor').addEventListener("click", function() {
        console.log('i want to see current color');
      document.querySelector('#pageNavigator').pushPage('currentColor.html');
      console.log('pls see current color');
    });
    page.querySelector('#changeDefaultColor').addEventListener("click", function() {
      document.querySelector('#pageNavigator').pushPage('defaultColor.html');
    });
    
  }

  if (page.id === 'currentColor'){
    ble.read(connectingDevice.id, myService, rgbChar, rgbCharRead, failure);
    page.querySelector('#currentColorButton').addEventListener('click', function(){
      var red = parseInt(page.querySelector('#currentRedRange').value);
      var green = parseInt(page.querySelector('#currentGreenRange').value);
      var blue = parseInt(page.querySelector('#currentBlueRange').value);
      var data5 = new Uint8Array(3);
      data5[0] = unconvertColorValue(red);
      data5[1] = unconvertColorValue(green);
      data5[2] = unconvertColorValue(blue);
      ble.write(connectingDevice.id, myService, rgbChar, data5.buffer, success, failure);
    });
  }
  
  if (page.id === 'defaultColor'){
     ble.read(connectingDevice.id, myService, rgbDefaultChar, rgbDefaultCharRead, failure);
     page.querySelector('#defaultColorButton').addEventListener('click', function(){
      var red = parseInt(page.querySelector('#defaultRedRange').value);
      var green = parseInt(page.querySelector('#defaultGreenRange').value);
      var blue = parseInt(page.querySelector('#defaultBlueRange').value);
      console.log(blue);
      var data6 = new Uint8Array(3);
      data6[0] = unconvertColorValue(red);
      data6[1] = unconvertColorValue(green);
      data6[2] = unconvertColorValue(blue);
      ble.write(connectingDevice.id, myService, rgbDefaultChar, data6.buffer, success, failure);
      
    });
  }
  
  if (page.id === 'alarms'){
    ble.read(connectingDevice.id, myService, onAtChar, onAtCharRead, failure);
    ble.read(connectingDevice.id, myService, offAtChar, offAtCharRead, failure);
    
    page.querySelector('#onAlarmButton').addEventListener('click', function(){
        var data12 = new Uint8Array(4);
        data12[0] = page.querySelector('#onAlarmSwitch').checked ? 1 : 0;
        data12[1] = page.querySelector('#onAlarmHourInput').value;
        data12[2] = page.querySelector('#onAlarmMinuteInput').value;
        data12[3] = page.querySelector('#onAlarmSecondInput').value;
        console.log('hey');
        ble.write(connectingDevice.id, myService, onAtChar, data12.buffer, success, failure);
        console.log('complete');
    });
    
    page.querySelector('#offAlarmButton').addEventListener('click', function(){
        var data13 = new Uint8Array(4);
        data13[0] = page.querySelector('#offAlarmSwitch').checked ? 1 : 0;
        data13[1] = page.querySelector('#offAlarmHourInput').value;
        data13[2] = page.querySelector('#offAlarmMinuteInput').value;
        data13[3] = page.querySelector('#offAlarmSecondInput').value;
        console.log('hey2');
        ble.write(connectingDevice.id, myService, offAtChar, data13.buffer, success, failure);
        console.log('complete2');
    });
  }
  
  if (page.id === 'timers'){
    ble.read(connectingDevice.id, myService, alarmOnChar, alarmOnCharRead, failure);
    ble.read(connectingDevice.id, myService, alarmOnTimeChar, alarmOnTimeCharRead, failure);
    ble.read(connectingDevice.id, myService, alarmOffChar, alarmOffCharRead, failure);
    ble.read(connectingDevice.id, myService, alarmOffTimeChar, alarmOffTimeCharRead, failure);
    
    page.querySelector('#onTimerButton').addEventListener('click', function(){
        var data9 = new Uint8Array(1);
        data9[0] = page.querySelector('#onTimerInput').value;
        console.log(data9[0]);
        ble.write(connectingDevice.id, myService, alarmOnTimeChar, data9.buffer, success, failure);
    });
    
    page.querySelector('#onTimerSwitch').addEventListener('change', function(event){
        console.log("ontimer switch: ", event.switch.checked);
        var data8 = new Uint8Array(1);
        
        data8[0] = event.switch.checked ? 1 : 0;
        
        ble.write(connectingDevice.id, myService, alarmOnChar, data8.buffer, success, failure);
        
    });
    
    
    page.querySelector('#offTimerButton').addEventListener('click', function(){
        var data10 = new Uint8Array(1);
        data10[0] = page.querySelector('#offTimerInput').value;
        console.log("hey", data10[0]);
        ble.write(connectingDevice.id, myService, alarmOffTimeChar, data10.buffer, success, failure);
    });
    
    page.querySelector('#offTimerSwitch').addEventListener('change', function(event){
        console.log("offtimer switch: ", event.switch.checked);
        var data11 = new Uint8Array(1);
        
        data11[0] = event.switch.checked ? 1 : 0;
        
        ble.write(connectingDevice.id, myService, alarmOffChar, data11.buffer, success, failure);
        
    });
    
    
  }
  
});

console.log("loaded index.js");