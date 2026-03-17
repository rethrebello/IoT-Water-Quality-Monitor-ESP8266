#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define TdsSensorPin A0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "CTPL_Guest";
const char* password = "P@ssw0rd";

WiFiServer server(80);

float voltage;
float tdsValue;
float stableTDS = 0;
float lastTDS = 0;

float minTDS = 10000;
float maxTDS = 0;
float avgTDS = 0;

float temperature = 0;
float minTemp = 10000;
float maxTemp = 0;
float avgTemp = 0;
int tempReadingCount = 0;

int readingCount = 0;

String statusText;
String recommendation;
String trend;
String waterType;
String alert;
String stability;

String tempStatus;
String tempTrend;
float lastTemp = 0;

String webpage;
String tempPage;

void setup(){

Serial.begin(115200);
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());
Wire.begin(D2,D1);

display.begin(SSD1306_SWITCHCAPVCC,0x3C);
display.clearDisplay();

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED){
delay(500);
}

server.begin();

Serial.println("Server started!");
Serial.print("Open this in your browser: http://");
Serial.println(WiFi.localIP());

// Show IP on OLED
display.clearDisplay();

// Show IP on OLED
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.print("WiFi Connected!");
display.setCursor(0,16);
display.print("IP Address:");
display.setCursor(0,30);
display.setTextSize(1);
display.print(WiFi.localIP());
display.setCursor(0,48);
display.print("Open in browser");
display.display();
delay(5000);

webpage = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>IoT Water Quality Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0;font-family:Segoe UI;}

body.light{background:linear-gradient(135deg,#dce9ff,#f8cdda);color:#111;}
body.dark{background:linear-gradient(135deg,#0d0d0d,#1a1a2e);color:white;}

body.light .metric{background:white;color:black;}
body.dark .metric{background:#2a2a3e;color:white;}

body.light .main{background:white;}
body.dark .main{background:#2a2a3e;}

body.light canvas{background:white;}
body.dark canvas{background:#2a2a3e;}

.dashboard{width:96vw;height:96vh;display:grid;grid-template-rows:7vh 18vh 22vh 42vh;gap:1vh;}
.header{display:flex;justify-content:space-between;align-items:center;font-size:1.5rem;}
.score{background:white;color:black;padding:4px 10px;border-radius:6px;font-size:0.9rem;}
.main{display:flex;justify-content:center;align-items:center;border-radius:12px;}
.tds{font-size:5vw;font-weight:bold;color:#0077ff;}
.metrics{display:grid;grid-template-columns:repeat(5,1fr);gap:0.6vh;}
.metric{border-radius:8px;padding:6px;font-size:0.8rem;text-align:center;}
.graphs{display:grid;grid-template-columns:1fr 1fr 1fr;gap:1vh;}
canvas{border-radius:10px;height:100% !important;width:100% !important;}
.alert-banner{position:absolute;top:1vh;left:50%;transform:translateX(-50%);background:red;color:white;padding:6px 16px;font-weight:bold;border-radius:6px;display:none;animation:blink 1s infinite;z-index:99;}
@keyframes blink{0%{opacity:1;}50%{opacity:0.3;}100%{opacity:1;}}
.temp-btn{background:#e74c3c;color:white;border:none;padding:8px 16px;border-radius:8px;font-size:0.9rem;cursor:pointer;}
.temp-btn:hover{background:#c0392b;}
.icon-btn{background:white;color:#333;border:none;padding:8px 14px;border-radius:8px;font-size:0.9rem;cursor:pointer;font-weight:bold;}
.icon-btn:hover{background:#eee;}

.email-modal{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.6);z-index:200;justify-content:center;align-items:center;}
.email-modal.show{display:flex;}
.email-box{background:white;color:black;padding:24px;border-radius:12px;width:320px;display:flex;flex-direction:column;gap:12px;}
.email-box input{padding:8px;border-radius:6px;border:1px solid #ccc;font-size:0.9rem;}
.email-box button{padding:8px;border-radius:6px;border:none;cursor:pointer;font-size:0.9rem;}
.save-btn{background:#0077ff;color:white;}
.close-btn{background:#ccc;color:black;}
</style>
</head>
<body class="dark">
<div id="alertBanner" class="alert-banner">WATER QUALITY UNSAFE</div>

<div class="email-modal" id="emailModal">
<div class="email-box">
<b>Email Alert Settings</b>
<input type="email" id="alertEmail" placeholder="Enter your email"/>
<input type="number" id="alertThreshold" placeholder="TDS threshold (default 500)"/>
<button class="save-btn" onclick="saveEmail()">Save</button>
<button class="close-btn" onclick="closeEmail()">Cancel</button>
<small id="emailNote" style="color:green;"></small>
</div>
</div>

<div class="dashboard">
<div class="header">
<div>IoT Water Quality Dashboard</div>
<div style="display:flex;gap:8px;align-items:center;flex-wrap:wrap;">
<button class="temp-btn" onclick="window.location='/temp'">Temperature Page</button>
<button class="icon-btn" onclick="toggleTheme()">Theme</button>
<button class="icon-btn" onclick="downloadCSV()">CSV</button>
<button class="icon-btn" onclick="openEmail()">Alerts</button>
<div class="score">Score: <span id="score">0</span>/100</div>
</div>
</div>
<div class="main">
<div class="tds"><span id="tds">0</span> ppm</div>
</div>
<div class="metrics">
<div class="metric">Status<br><span id="status"></span></div>
<div class="metric">Type<br><span id="type"></span></div>
<div class="metric">Trend<br><span id="trend"></span></div>
<div class="metric">Stability<br><span id="stability"></span></div>
<div class="metric">Hardness<br><span id="hard"></span></div>
<div class="metric">Min<br><span id="min"></span></div>
<div class="metric">Max<br><span id="max"></span></div>
<div class="metric">Average<br><span id="avg"></span></div>
<div class="metric">Recommendation<br><span id="rec"></span></div>
<div class="metric">Uptime<br><span id="uptime"></span></div>
</div>
<div class="graphs">
<canvas id="liveChart"></canvas>
<canvas id="gaugeCanvas"></canvas>
<canvas id="hardChart"></canvas>
</div>
</div>

<script>
var isDark = true;
var csvRows = [["Time","TDS","Avg","Hardness","Status","Score"]];
var alertEmail = "";
var alertThreshold = 500;
var lastAlertSent = false;

function toggleTheme(){
isDark=!isDark;
document.body.className=isDark?"dark":"light";
}

function downloadCSV(){
var content = csvRows.map(r=>r.join(",")).join("\n");
var blob = new Blob([content],{type:"text/csv"});
var a = document.createElement("a");
a.href = URL.createObjectURL(blob);
a.download = "water_quality_log.csv";
a.click();
}

function openEmail(){document.getElementById("emailModal").classList.add("show");}
function closeEmail(){document.getElementById("emailModal").classList.remove("show");}
function saveEmail(){
alertEmail=document.getElementById("alertEmail").value;
alertThreshold=parseInt(document.getElementById("alertThreshold").value)||500;
document.getElementById("emailNote").innerHTML="Saved! Alerts set for TDS > "+alertThreshold;
}

function sendEmailAlert(tds){
if(!alertEmail) return;
fetch("https://formsubmit.co/ajax/"+alertEmail,{
method:"POST",
headers:{"Content-Type":"application/json","Accept":"application/json"},
body:JSON.stringify({subject:"Water Quality Alert",message:"TDS level is "+tds+" ppm which exceeds your threshold of "+alertThreshold+" ppm. Please check your water quality monitor."})
});
}

var liveChart=new Chart(document.getElementById("liveChart"),{
type:"line",
data:{labels:[],datasets:[{label:"Live TDS",data:[],borderColor:"#0077ff",backgroundColor:"rgba(0,119,255,0.1)",fill:true}]},
options:{animation:false,responsive:true}
});

var hardChart=new Chart(document.getElementById("hardChart"),{
type:"line",
data:{labels:[],datasets:[{label:"Hardness",data:[],borderColor:"#e67e22",backgroundColor:"rgba(230,126,34,0.1)",fill:true}]},
options:{animation:false,responsive:true}
});

function drawGauge(canvas, value, max, label, color){
var ctx=canvas.getContext("2d");
var w=canvas.width, h=canvas.height;
ctx.clearRect(0,0,w,h);

var bg = isDark ? "#2a2a3e" : "#ffffff";
ctx.fillStyle=bg;
ctx.fillRect(0,0,w,h);

var cx=w/2, cy=h*0.62, r=Math.min(w,h)*0.38;
var startAngle=Math.PI, endAngle=2*Math.PI;
var ratio=Math.min(value/max,1);
var valAngle=Math.PI+(ratio*Math.PI);

ctx.beginPath();
ctx.arc(cx,cy,r,startAngle,endAngle);
ctx.strokeStyle="#e0e0e0";
ctx.lineWidth=18;
ctx.stroke();

var grad=ctx.createLinearGradient(cx-r,cy,cx+r,cy);
grad.addColorStop(0,"#27ae60");
grad.addColorStop(0.5,"#f1c40f");
grad.addColorStop(1,"#e74c3c");

ctx.beginPath();
ctx.arc(cx,cy,r,startAngle,valAngle);
ctx.strokeStyle=color;
ctx.lineWidth=18;
ctx.lineCap="round";
ctx.stroke();

ctx.fillStyle=isDark?"white":"#111";
ctx.font="bold "+Math.floor(w*0.13)+"px Segoe UI";
ctx.textAlign="center";
ctx.fillText(value,cx,cy);

ctx.font=Math.floor(w*0.07)+"px Segoe UI";
ctx.fillStyle="#aaa";
ctx.fillText(label,cx,cy+Math.floor(h*0.1));
}

var gaugeCanvas=document.getElementById("gaugeCanvas");

setInterval(function(){
fetch("/data").then(res=>res.json()).then(data=>{
document.getElementById("tds").innerHTML=data.tds;
document.getElementById("score").innerHTML=data.score;
document.getElementById("status").innerHTML=data.status;
document.getElementById("type").innerHTML=data.type;
document.getElementById("trend").innerHTML=data.trend;
document.getElementById("stability").innerHTML=data.stability;
document.getElementById("hard").innerHTML=data.hard;
document.getElementById("avg").innerHTML=data.avg;
document.getElementById("min").innerHTML=data.min;
document.getElementById("max").innerHTML=data.max;
document.getElementById("rec").innerHTML=data.rec;
document.getElementById("uptime").innerHTML=data.uptime;

var banner=document.getElementById("alertBanner");
if(data.tds>500){
banner.style.display="block";
if(!lastAlertSent){sendEmailAlert(data.tds);lastAlertSent=true;}
}else{
banner.style.display="none";
lastAlertSent=false;
}

var now=new Date();
var timeStr=now.getHours()+":"+now.getMinutes()+":"+now.getSeconds();
csvRows.push([timeStr,data.tds,data.avg,data.hard,data.status,data.score]);

liveChart.data.labels.push("");
liveChart.data.datasets[0].data.push(data.tds);
hardChart.data.labels.push("");
hardChart.data.datasets[0].data.push(data.hard);

if(liveChart.data.labels.length>40){liveChart.data.labels.shift();liveChart.data.datasets[0].data.shift();}
if(hardChart.data.labels.length>40){hardChart.data.labels.shift();hardChart.data.datasets[0].data.shift();}

liveChart.update();
hardChart.update();

var tdsColor="#27ae60";
if(data.tds>500) tdsColor="#e74c3c";
else if(data.tds>300) tdsColor="#f1c40f";

drawGauge(gaugeCanvas,data.tds,1000,"ppm",tdsColor);
});
},1000);
</script>
</body>
</html>
)=====";

tempPage = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>Temperature Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0;font-family:Segoe UI;}

body.light{background:linear-gradient(135deg,#ffe5e5,#fff3cd);color:#111;}
body.dark{background:linear-gradient(135deg,#2c3e50,#e74c3c);color:white;}

body.light .metric{background:white;color:black;}
body.dark .metric{background:rgba(255,255,255,0.15);color:white;}

body.light .main{background:white;}
body.dark .main{background:rgba(255,255,255,0.1);}

body.light canvas{background:white;}
body.dark canvas{background:rgba(255,255,255,0.1);}

.dashboard{width:96vw;height:96vh;display:grid;grid-template-rows:7vh 18vh 20vh 46vh;gap:1vh;}
.header{display:flex;justify-content:space-between;align-items:center;font-size:1.5rem;}
.nav-btn{background:white;color:#e74c3c;border:none;padding:8px 16px;border-radius:8px;font-size:0.9rem;cursor:pointer;font-weight:bold;}
.nav-btn:hover{background:#f0f0f0;}
.icon-btn{background:white;color:#333;border:none;padding:8px 14px;border-radius:8px;font-size:0.9rem;cursor:pointer;font-weight:bold;}
.main{display:flex;justify-content:center;align-items:center;border-radius:12px;gap:60px;}
.temp-val{font-size:5vw;font-weight:bold;color:#e74c3c;text-align:center;}
.temp-label{font-size:1rem;color:#888;text-align:center;}
.metrics{display:grid;grid-template-columns:repeat(5,1fr);gap:0.6vh;}
.metric{border-radius:8px;padding:6px;font-size:0.8rem;text-align:center;}
.graphs{display:grid;grid-template-columns:1fr 1fr 1fr;gap:1vh;}
canvas{border-radius:10px;height:100% !important;width:100% !important;}
.alert-banner{position:absolute;top:1vh;left:50%;transform:translateX(-50%);background:#e74c3c;color:white;padding:6px 16px;font-weight:bold;border-radius:6px;display:none;animation:blink 1s infinite;z-index:99;}
@keyframes blink{0%{opacity:1;}50%{opacity:0.3;}100%{opacity:1;}}
</style>
</head>
<body class="dark">
<div id="alertBanner" class="alert-banner">HIGH TEMPERATURE ALERT</div>
<div class="dashboard">
<div class="header">
<div>Temperature Dashboard</div>
<div style="display:flex;gap:8px;align-items:center;">
<button class="nav-btn" onclick="window.location='/'">Back to Main</button>
<button class="icon-btn" onclick="toggleTheme()">Theme</button>
<button class="icon-btn" onclick="downloadCSV()">CSV</button>
</div>
</div>
<div class="main">
<div>
<div class="temp-val"><span id="temp">0</span> C</div>
<div class="temp-label">Current Temperature</div>
</div>
<div>
<div class="temp-val" style="color:#3498db;"><span id="tds">0</span> ppm</div>
<div class="temp-label">Current TDS</div>
</div>
</div>
<div class="metrics">
<div class="metric">Temp Status<br><span id="tstatus"></span></div>
<div class="metric">Temp Trend<br><span id="ttrend"></span></div>
<div class="metric">Min Temp<br><span id="tmin"></span> C</div>
<div class="metric">Max Temp<br><span id="tmax"></span> C</div>
<div class="metric">Avg Temp<br><span id="tavg"></span> C</div>
<div class="metric">TDS Score<br><span id="score"></span>/100</div>
<div class="metric">Water Type<br><span id="type"></span></div>
<div class="metric">Uptime<br><span id="uptime"></span>s</div>
<div class="metric">Hardness<br><span id="hard"></span> mg/L</div>
<div class="metric">TDS Min<br><span id="tdsmin"></span> ppm</div>
</div>
<div class="graphs">
<canvas id="tempLiveChart"></canvas>
<canvas id="tempGaugeCanvas"></canvas>
<canvas id="tempTdsChart"></canvas>
</div>
</div>
<script>
var isDark=true;
var csvRows=[["Time","Temp","AvgTemp","TDS","TempStatus"]];

function toggleTheme(){
isDark=!isDark;
document.body.className=isDark?"dark":"light";
}

function downloadCSV(){
var content=csvRows.map(r=>r.join(",")).join("\n");
var blob=new Blob([content],{type:"text/csv"});
var a=document.createElement("a");
a.href=URL.createObjectURL(blob);
a.download="temperature_log.csv";
a.click();
}

var tempLiveChart=new Chart(document.getElementById("tempLiveChart"),{
type:"line",
data:{labels:[],datasets:[{label:"Live Temp (C)",data:[],borderColor:"#e74c3c",backgroundColor:"rgba(231,76,60,0.1)",fill:true}]},
options:{animation:false,responsive:true}
});

var tempTdsChart=new Chart(document.getElementById("tempTdsChart"),{
type:"line",
data:{labels:[],datasets:[
{label:"TDS (ppm)",data:[],borderColor:"#3498db",backgroundColor:"rgba(52,152,219,0.1)",fill:true},
{label:"Temp (C)",data:[],borderColor:"#e74c3c",backgroundColor:"rgba(231,76,60,0.1)",fill:true}
]},
options:{animation:false,responsive:true}
});

var tempGaugeCanvas=document.getElementById("tempGaugeCanvas");

function drawTempGauge(canvas,value,label){
var ctx=canvas.getContext("2d");
var w=canvas.width,h=canvas.height;
ctx.clearRect(0,0,w,h);

var bg=isDark?"rgba(255,255,255,0.1)":"#ffffff";
ctx.fillStyle=bg;
ctx.fillRect(0,0,w,h);

var cx=w/2,cy=h*0.62,r=Math.min(w,h)*0.38;
var ratio=Math.min(value/60,1);
var valAngle=Math.PI+(ratio*Math.PI);

ctx.beginPath();
ctx.arc(cx,cy,r,Math.PI,2*Math.PI);
ctx.strokeStyle="#e0e0e0";
ctx.lineWidth=18;
ctx.stroke();

var color="#27ae60";
if(value>35) color="#e74c3c";
else if(value>30) color="#f1c40f";

ctx.beginPath();
ctx.arc(cx,cy,r,Math.PI,valAngle);
ctx.strokeStyle=color;
ctx.lineWidth=18;
ctx.lineCap="round";
ctx.stroke();

ctx.fillStyle=isDark?"white":"#111";
ctx.font="bold "+Math.floor(w*0.13)+"px Segoe UI";
ctx.textAlign="center";
ctx.fillText(value+"C",cx,cy);

ctx.font=Math.floor(w*0.07)+"px Segoe UI";
ctx.fillStyle="#aaa";
ctx.fillText(label,cx,cy+Math.floor(h*0.1));
}

setInterval(function(){
fetch("/data").then(res=>res.json()).then(data=>{
document.getElementById("temp").innerHTML=data.temp;
document.getElementById("tds").innerHTML=data.tds;
document.getElementById("tstatus").innerHTML=data.tstatus;
document.getElementById("ttrend").innerHTML=data.ttrend;
document.getElementById("tmin").innerHTML=data.tmin;
document.getElementById("tmax").innerHTML=data.tmax;
document.getElementById("tavg").innerHTML=data.tavg;
document.getElementById("score").innerHTML=data.score;
document.getElementById("type").innerHTML=data.type;
document.getElementById("uptime").innerHTML=data.uptime;
document.getElementById("hard").innerHTML=data.hard;
document.getElementById("tdsmin").innerHTML=data.min;

var banner=document.getElementById("alertBanner");
if(data.temp>35){banner.style.display="block";}else{banner.style.display="none";}

var now=new Date();
var timeStr=now.getHours()+":"+now.getMinutes()+":"+now.getSeconds();
csvRows.push([timeStr,data.temp,data.tavg,data.tds,data.tstatus]);

tempLiveChart.data.labels.push("");
tempLiveChart.data.datasets[0].data.push(data.temp);
tempTdsChart.data.labels.push("");
tempTdsChart.data.datasets[0].data.push(data.tds);
tempTdsChart.data.datasets[1].data.push(data.temp);

if(tempLiveChart.data.labels.length>40){tempLiveChart.data.labels.shift();tempLiveChart.data.datasets[0].data.shift();}
if(tempTdsChart.data.labels.length>40){tempTdsChart.data.labels.shift();tempTdsChart.data.datasets[0].data.shift();tempTdsChart.data.datasets[1].data.shift();}

tempLiveChart.update();
tempTdsChart.update();

drawTempGauge(tempGaugeCanvas,data.temp,"Temperature");
});
},1000);
</script>
</body>
</html>
)=====";

}

void loop(){

int total=0;
for(int i=0;i<30;i++){
total+=analogRead(TdsSensorPin);
delay(10);
}

int avgValue=total/30;
voltage=avgValue*(3.3/1024.0);

tdsValue=(133.42*voltage*voltage*voltage
-255.86*voltage*voltage
+857.39*voltage)*0.5;

stableTDS = 0.7*stableTDS + 0.3*tdsValue;

temperature = 20.0 + (voltage * 5.0);

if(stableTDS<150){statusText="Excellent";recommendation="Very pure water";waterType="RO Water";}
else if(stableTDS<300){statusText="Good";recommendation="Safe for drinking";waterType="Drinking Water";}
else if(stableTDS<500){statusText="Poor";recommendation="Filtration needed";waterType="Tap Water";}
else{statusText="Unsafe";recommendation="Avoid drinking";waterType="Contaminated";}

if(stableTDS>lastTDS+5) trend="Rising";
else if(stableTDS<lastTDS-5) trend="Falling";
else trend="Stable";

if(abs(stableTDS-lastTDS)<2) stability="Stable";
else stability="Fluctuating";

lastTDS=stableTDS;

if(stableTDS>500) alert="Unsafe";
else alert="Normal";

if(stableTDS<minTDS) minTDS=stableTDS;
if(stableTDS>maxTDS) maxTDS=stableTDS;

readingCount++;
avgTDS=((avgTDS*(readingCount-1))+stableTDS)/readingCount;

if(temperature<minTemp) minTemp=temperature;
if(temperature>maxTemp) maxTemp=temperature;
tempReadingCount++;
avgTemp=((avgTemp*(tempReadingCount-1))+temperature)/tempReadingCount;

if(temperature>35) tempStatus="Hot";
else if(temperature>30) tempStatus="Warm";
else if(temperature>20) tempStatus="Normal";
else tempStatus="Cold";

if(temperature>lastTemp+0.5) tempTrend="Rising";
else if(temperature<lastTemp-0.5) tempTrend="Falling";
else tempTrend="Stable";

lastTemp=temperature;

int score=100-(stableTDS/10);
if(score<0) score=0;

float hardness=stableTDS*0.65;
unsigned long uptime=millis()/1000;

display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.print("Water Quality");
display.setTextSize(2);
display.setCursor(0,16);
display.print((int)stableTDS);
display.print(" ppm");
display.setTextSize(1);
display.setCursor(0,40);
display.print("Temp: ");
display.print((int)temperature);
display.print(" C");
display.setCursor(0,52);
display.print("Status: ");
display.print(statusText);
display.display();

WiFiClient client=server.available();
if(!client) return;

String req=client.readStringUntil('\r');
client.flush();

if(req.indexOf("/data")!=-1){

client.println("HTTP/1.1 200 OK");
client.println("Content-Type: application/json");
client.println();

client.print("{\"tds\":");client.print((int)stableTDS);
client.print(",\"status\":\"");client.print(statusText);
client.print("\",\"rec\":\"");client.print(recommendation);
client.print("\",\"type\":\"");client.print(waterType);
client.print("\",\"trend\":\"");client.print(trend);
client.print("\",\"alert\":\"");client.print(alert);
client.print("\",\"stability\":\"");client.print(stability);
client.print("\",\"score\":");client.print(score);
client.print(",\"hard\":");client.print((int)hardness);
client.print(",\"min\":");client.print((int)minTDS);
client.print(",\"max\":");client.print((int)maxTDS);
client.print(",\"avg\":");client.print((int)avgTDS);
client.print(",\"uptime\":");client.print(uptime);
client.print(",\"temp\":");client.print((int)temperature);
client.print(",\"tstatus\":\"");client.print(tempStatus);
client.print("\",\"ttrend\":\"");client.print(tempTrend);
client.print("\",\"tmin\":");client.print((int)minTemp);
client.print(",\"tmax\":");client.print((int)maxTemp);
client.print(",\"tavg\":");client.print((int)avgTemp);
client.print("}");

}
else if(req.indexOf("/temp")!=-1){

client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println();
client.print(tempPage);

}
else{

client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println();
client.print(webpage);

}

delay(1);

}