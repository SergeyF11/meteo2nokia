#pragma once

// HTML-шаблон с ползунками для кастомизации портала
const char* slider_html = R"rawliteral(
    <script>
    function updateSliderValue(sliderId, valueId) {
        document.getElementById(valueId).innerHTML = document.getElementById(sliderId).value;
    }
    </script>
    
    <style>
    .slider-container {
        margin-bottom: 15px;
        padding: 10px;
        background: #f5f5f5;
        border-radius: 5px;
    }
    .slider-label {
        display: inline-block;
        width: 150px;
    }
    .slider-value {
        display: inline-block;
        width: 30px;
        text-align: center;
    }
    </style>
    
    <div class='slider-container'>
        <span class='slider-label'>Контраст дисплея 1:</span>
        <input type='range' id='contrast1' name='contrast1' min='0' max='100' value='%d' 
               oninput='updateSliderValue("contrast1", "contrast1Value")'>
        <span class='slider-value' id='contrast1Value'>%d</span>
    </div>
    
    <div class='slider-container'>
        <span class='slider-label'>Контраст дисплея 2:</span>
        <input type='range' id='contrast2' name='contrast2' min='0' max='100' value='%d'
               oninput='updateSliderValue("contrast2", "contrast2Value")'>
        <span class='slider-value' id='contrast2Value'>%d</span>
    </div>
    )rawliteral";