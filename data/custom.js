
// Callback for Enable/Disable
function onCheckboxChangeHandler(e){
    console.log('value of checkbox : ', e.checked);

    if(e.checked) return nanoajax.ajax({url:'/enable'}, function (code, responseText) { alert(responseText); }); // If checked, we enable
    return nanoajax.ajax({url:'/disable'}, function (code, responseText) { alert(responseText); });   // else we disable
}

// Callback for Change Speed
function onChangeNumber(e){
    if(e.name=="Speed Percent"){
        return nanoajax.ajax({url:'/setSpeed/' + e.value}, function (code, responseText) { alert(responseText); });   // Set the Speed
    }
}

// For Increase Speed
function increaseValue(elName) {
    var value = parseInt(document.getElementById(elName).value, 10);
    value = isNaN(value) ? 0 : value;
    value+=10;
    document.getElementById(elName).value = value;

    if(elName=="Speed Percent"){
        return nanoajax.ajax({url:'/increase/10'}, function (code, responseText) { alert(responseText); });   // Set the Speed
    }
  }
  
  // For decrease Speed
  function decreaseValue(elName) {
    var value = parseInt(document.getElementById(elName).value, 10);
    value = isNaN(value) ? 0 : value;
    value < 1 ? value = 1 : '';
    value-=10;
    document.getElementById(elName).value = value;

    if(elName=="Speed Percent"){
        return nanoajax.ajax({url:'/decrease/10'}, function (code, responseText) { alert(responseText); });   // Set the Speed
    }

    //console.log(document.getElementById(elName).value);
  }

// Get the Status in JSON Format
function getStatus(){
    var stateDot=document.getElementById("state");

    return nanoajax.ajax({url:'/status', timeout: 5000}, function (code, jsonStatus) {
        if(code<200 || code>=300 || !jsonStatus){
            if(stateDot) stateDot.style.backgroundColor="red";
            return null;
        }
        refreshPageContent(jsonStatus);

        if(stateDot) stateDot.style.backgroundColor="green";
        return jsonStatus;

    });   
}

// Refresh the page content with Json Content
function refreshPageContent(strJson){
    var currentDate = '[' + new Date().toUTCString() + '] ';
    console.log(currentDate);
    console.log(strJson);

    const obj = JSON.parse(strJson);

    refreshOneParam("valueOf_AmplitudePercent",obj.amplitudePct);
    refreshOneParam("valueOf_SpeedPercent",obj.speedPct);

    refreshOneParam("valueOf_RPM",obj.RPM);
    refreshOneParam("valueOf_PushSpeedRatio",obj.pushSpeedRatio);
    refreshOneParam("valueOf_PullSpeedRatio",obj.pullSpeedRatio);

}

// Modify One Status Value
function refreshOneParam(paramName,value){
    var element = document.getElementById(paramName);
    element.innerHTML = value;
}


// LoadDing Function
function load() {
    getStatus();

    setTimeout(load, 5000); // Each 5 seconds
}
load();