#pragma once
bool portalActive = false;
char portalHtml[1024];

const char* slider_html = R"rawliteral(
<!doctype html>
<html><head>
<meta name=viewport content="width=device-width,initial-scale=1">
<style>
.contrast-settings {
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #eee;
}
.slider-row {
  margin: 12px 0;
  display: flex;
  align-items: center;
}
.slider-row label {
  display: inline-block;
  width: 120px;
}
.slider-row input {
  flex-grow: 1;
  margin: 0 10px;
}
.slider-value {
  width: 30px;
  text-align: center;
  font-weight: bold;
}
</style>
<script>
function updateSlider(sliderId, valueId) {
  document.getElementById(valueId).textContent = document.getElementById(sliderId).value;
}
window.onload = function() {
  updateSlider('c1', 'v1');
  updateSlider('c2', 'v2');
}
</script>
</head><body>
<div id="contrast-container" class="contrast-settings">
<h3>Настройки контраста дисплеев</h3>
<div class="slider-row">
  <label for="c1">Дисплей 1:</label>
  <input type="range" id="c1" name="contrast1" min="0" max="100" value="%d" oninput="updateSlider('c1','v1')">
  <span class="slider-value" id="v1">%d</span>
</div>
<div class="slider-row">
  <label for="c2">Дисплей 2:</label>
  <input type="range" id="c2" name="contrast2" min="0" max="100" value="%d" oninput="updateSlider('c2','v2')">
  <span class="slider-value" id="v2">%d</span>
</div>
</div>
</body></html>
)rawliteral";

// HTML-шаблон с ползунками для кастомизации портала
const char* slider1_html = R"rawliteral(
<!doctype html>
<html><head>
<meta name=viewport content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial,sans-serif;margin:10px;background:#fff}
.s{display:block;margin:15px 0}
.s input{width:150px;vertical-align:middle}
.s span{display:inline-block;width:35px;text-align:center}
</style>
<script>
function u(i,v){document.getElementById(i).textContent=v}
window.onload=function(){
    u('v1',document.getElementById('c1').value);
    u('v2',document.getElementById('c2').value);
}
</script>
</head><body>
<div class=s>
<label>Контраст 1: <span id=v1>%d</span></label>
<input type=range id=c1 name=contrast1 min=0 max=100 value=%d oninput="u('v1',value)">
</div>
<div class=s>
<label>Контраст 2: <span id=v2>%d</span></label>
<input type=range id=c2 name=contrast2 min=0 max=100 value=%d oninput="u('v2',value)">
</div>
</body></html>
)rawliteral";

// const char *slider_html = R"rawliteral(
// <!DOCTYPE html><html><head>
// <meta name="viewport" content="width=device-width, initial-scale=1">
// <style>
// body{font-family:Arial,sans-serif;margin:20px;background:#fff;color:#333;overflow:hidden}
// .slider{margin:15px 0}
// .slider label{display:inline-block;width:120px}
// .slider input{width:200px;vertical-align:middle}
// .value{display:inline-block;width:35px;text-align:center;font-weight:bold}
// </style>
// <script>
// function u(v){document.getElementById('v'+v).innerText=document.getElementById('c'+v).value}
// window.onload=function(){u(1);u(2)}
// </script>
// </head><body>
// <div class="slider">
//     <label>Контраст 1:</label>
//     <input type="range" id="c1" name="contrast1" min="0" max="100" value="%d" oninput="u(1)">
//     <span class="value" id="v1">%d</span>
// </div>
// <div class="slider">
//     <label>Контраст 2:</label>
//     <input type="range" id="c2" name="contrast2" min="0" max="100" value="%d" oninput="u(2)">
//     <span class="value" id="v2">%d</span>
// </div>
// </body></html>
// )rawliteral";

// const char * slider_html = R"rawliteral(
//     <!DOCTYPE html>
//     <html>
//     <head>
//         <meta name="viewport" content="width=device-width, initial-scale=1">
//         <style>
//             body {
//                 font-family: Arial, sans-serif;
//                 margin: 20px;
//                 background-color: #f9f9f9;
//             }
//             .form-container {
//                 background: white;
//                 padding: 20px;
//                 border-radius: 8px;
//                 box-shadow: 0 2px 4px rgba(0,0,0,0.1);
//                 margin-bottom: 20px;
//             }
//             .slider-container {
//                 margin-bottom: 15px;
//                 padding: 10px;
//                 background: #f5f5f5;
//                 border-radius: 5px;
//             }
//             .slider-label {
//                 display: inline-block;
//                 width: 150px;
//                 font-weight: bold;
//             }
//             .slider-value {
//                 display: inline-block;
//                 width: 30px;
//                 text-align: center;
//                 font-weight: bold;
//             }
//             input[type="range"] {
//                 width: 200px;
//                 vertical-align: middle;
//             }
//             input[type="text"], input[type="password"] {
//                 width: 100%;
//                 padding: 8px;
//                 margin: 5px 0 15px;
//                 box-sizing: border-box;
//                 border: 1px solid #ddd;
//                 border-radius: 4px;
//             }
//             .submit-btn {
//                 background-color: #4CAF50;
//                 color: white;
//                 padding: 10px 15px;
//                 border: none;
//                 border-radius: 4px;
//                 cursor: pointer;
//                 font-size: 16px;
//             }
//             .submit-btn:hover {
//                 background-color: #45a049;
//             }
//         </style>
//     </head>
//     <body>
//         <div class="form-container">
//             <h2>Настройки WEatherSTation</h2>
            
//             <div class="slider-container">
//                 <span class="slider-label">Контраст дисплея 1:</span>
//                 <input type="range" id="contrast1" name="contrast1" min="0" max="100" value="%d"
//                        oninput="updateSliderValue('contrast1', 'contrast1Value')">
//                 <span class="slider-value" id="contrast1Value">%d</span>
//             </div>
    
//             <div class="slider-container">
//                 <span class="slider-label">Контраст дисплея 2:</span>
//                 <input type="range" id="contrast2" name="contrast2" min="0" max="100" value="%d"
//                        oninput="updateSliderValue('contrast2', 'contrast2Value')">
//                 <span class="slider-value" id="contrast2Value">%d</span>
//             </div>
    
//             <script>
//                 function updateSliderValue(sliderId, valueId) {
//                     document.getElementById(valueId).innerHTML = document.getElementById(sliderId).value;
//                 }
//                 // Инициализация значений при загрузке
//                 window.onload = function() {
//                     updateSliderValue('contrast1', 'contrast1Value');
//                     updateSliderValue('contrast2', 'contrast2Value');
//                 };
//             </script>
//         </div>
//     </body>
//     </html>
//     )rawliteral";