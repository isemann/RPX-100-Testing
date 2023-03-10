'use strict';

var spectrum, logger, ws, wsCtrl;

function connectWebSocket(spectrum) {

    ws = new WebSocket("ws://" + window.location.host.substring(0, window.location.host.indexOf(':')) + ":8084");
    // ws = new WebSocket("ws://" + window.location.host.substring(0, window.location.host.indexOf(':')) + ":8082");
    // ws = new WebSocket("ws://" + window.location.host + ":8082");

    spectrum.setWebSocket(ws);
  
    ws.onopen = function(evt) {
        console.log("connected!");
    }
    ws.onclose = function(evt) {
        console.log("closed");
        setTimeout(function() {
            connectWebSocket(spectrum);
        }, 1000);
    }
    ws.onerror = function(evt) {
        console.log("error: " + evt.message);
    }
    ws.onmessage = function (evt) {
        var data = JSON.parse(evt.data);
        if (data.s) {
            spectrum.addData(data.s);
            let num = parseFloat(data.center);
            let bw = parseFloat(data.span);
            let tx = parseFloat(data.txFreq);
            document.getElementById('center').innerHTML = 'RX: ' + (num/1000000).toFixed(3) + ' MHz |';
            document.getElementById('tx').innerHTML = ' TX: ' + (tx/1000000).toFixed(3) + ' MHz';
            document.getElementById('min').innerHTML = ((num - bw/2)/1000000).toFixed(3) + ' MHz';
            document.getElementById('max').innerHTML = ((num + bw/2)/1000000).toFixed(3) + ' MHz';
        } else {
            if (data.center) {
                spectrum.setCenterHz(data.center);
                
            }
            if (data.span) {
                spectrum.setSpanHz(data.span);
            }
            if (data.gain) {
                spectrum.setGain(data.gain);
            }
            if (data.framerate) {
                spectrum.setFps(data.framerate);
            }
            spectrum.log(" > Freq:" + data.center / 1000000 + " MHz | Span: " + data.span / 1000000 + " MHz | Gain: " + data.gain + "dB | Fps: " + data.framerate);
        }
    }
}

function cmd(c, p) {
    let command = c + ':' + p;
    ws.send(command);
}

function main() {
    
    // Create spectrum object on canvas with ID "waterfall"
    spectrum = new Spectrum(
        "waterfall", {
            spectrumPercent: 25,
            logger: 'log',            
    });

    // Connect to websocket
    connectWebSocket(spectrum);

    // Bind keypress handler
    window.addEventListener("keydown", function (e) {
        spectrum.onKeypress(e);
    });

    
}

window.onload = main;
