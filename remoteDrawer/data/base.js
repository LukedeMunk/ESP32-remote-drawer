/*
 * File:      base.js
 * Authors:   Luke de Munk
 * 
 * General javascript for web page.
 */
const BYTES_IN_COLOR = 4;
const PENCIL = 0;
const ERASER = 1;
const BACKGROUND_COLOR = RGBToHex(0, 0, 0);

var canvas, ctx;
var flag = false;
var dot_flag = false;
var prevX = 0;
var currX = 0;
var prevY = 0;
var currY = 0;
var lineColor = RGBToHex(255, 255, 255);
var pencilType = PENCIL;

/**************************************************************************/
/*!
  @brief    Initialises page.
*/
/**************************************************************************/
function init() {
    document.getElementById('can').style.background = BACKGROUND_COLOR;
    canvas = document.getElementById('can');
    ctx = canvas.getContext("2d");
    w = canvas.width;
    h = canvas.height;

    canvas.addEventListener("mousemove", function (e) {
        findxy('move', e)
    }, false);
    canvas.addEventListener("mousedown", function (e) {
        findxy('down', e)
    }, false);
    canvas.addEventListener("mouseup", function (e) {
        findxy('up', e)
    }, false);
    canvas.addEventListener("mouseout", function (e) {
        findxy('out', e)
    }, false);
}

/**************************************************************************/
/*!
  @brief    Sends a new pixel to the display.
*/
/**************************************************************************/
function sendNewPixel(currX, currY) {
    $.ajax({
        url: "/update_xy",
        type: "get",
        data: {
            x: currX,
            y: currY
        },
        success: function(response) {},
        error: function(xhr) {}
    });
}

/**************************************************************************/
/*!
  @brief    Sends the color to the display.
*/
/**************************************************************************/
function sendColor() {
    $.ajax({
        url: "/update_color",
        type: "get",
        data: {color: lineColor},
        success: function(response) {},
        error: function(xhr) {}
    });
}

/**************************************************************************/
/*!
  @brief    Sends the pencil type.
*/
/**************************************************************************/
function sendPencilType() {
    $.ajax({
        url: "/update_pencil_type",
        type: "get",
        data: {pencil: pencilType},
        success: function(response) {},
        error: function(xhr) {}
    });
}

/**************************************************************************/
/*!
  @brief    Sends the erase command to the display.
*/
/**************************************************************************/
function sendEraseCommand() {
    $.ajax({
        url: "/erase_canvas",
        type: "get",
        success: function(response) {},
        error: function(xhr) {}
    });
}

/**************************************************************************/
/*!
  @brief    Sends the line color to the display.
*/
/**************************************************************************/
$("#color").change(
    function() {
        lineColor = $(this).val();
        var color = lineColor.replace("#", "");
        
        $.ajax({
            url: "/update_color",
            type: "get",
            data: { color: color },
            success: function(response) {},
            error: function(xhr) {}
        });
    }
);

/**************************************************************************/
/*!
  @brief    Sends the line width to the display.
*/
/**************************************************************************/
$("#lineWidth").change(
    function() {
        lineWidth = $(this).val();
        
        $.ajax({
            url: "/update_line_width",
            type: "get",
            data: { width: lineWidth },
            success: function(response) {},
            error: function(xhr) {}
        });
    }
);

/**************************************************************************/
/*!
  @brief    Draws on the canvas, then sends to the display.
*/
/**************************************************************************/
function draw() {
    ctx.beginPath();
    ctx.moveTo(prevX, prevY);
    ctx.lineTo(currX, currY);

    if (pencilType == PENCIL) {
        ctx.strokeStyle = lineColor;
    } else {
        ctx.strokeStyle = BACKGROUND_COLOR;
    }

    ctx.lineWidth = lineWidth;
    ctx.stroke();
    ctx.closePath();
    
    sendNewPixel(currX, currY);                                             //send to ESP
}

/**************************************************************************/
/*!
  @brief    Erases the canvas, then sends erase command to the display.
*/
/**************************************************************************/
function erase() {
    var erase = confirm("Want to clear?");
    if (erase) {
        ctx.clearRect(0, 0, w, h);
        document.getElementById("canvas_img").style.display = "none";

        sendEraseCommand();
    }
}

/**************************************************************************/
/*!
  @brief    Sets the pencil type to eraser.
*/
/**************************************************************************/
function setEraser() {
    pencilType = ERASER;
    sendPencilType();
}

/**************************************************************************/
/*!
  @brief    Sets the pencil type to pencil.
*/
/**************************************************************************/
function setPencil() {
    pencilType = PENCIL;
    sendPencilType();
}

/**************************************************************************/
/*!
  @brief    Converts R, G, B values to a hex string.
*/
/**************************************************************************/
function RGBToHex(r, g, b) {
    r = r.toString(16);
    g = g.toString(16);
    b = b.toString(16);
  
    if (r.length == 1) {
        r = "0" + r;
    }
    if (g.length == 1) {
        g = "0" + g;
    }
    if (b.length == 1) {
        b = "0" + b;
    }
  
    return "#" + r + g + b;
}

/**************************************************************************/
/*!
  @brief    Help function for initialising canvas.
*/
/**************************************************************************/
function findxy(res, e) {
    if (res == 'down') {
        prevX = currX;
        prevY = currY;
        currX = e.clientX - canvas.offsetLeft;
        currY = e.clientY - canvas.offsetTop;

        flag = true;
        dot_flag = true;

        if (dot_flag) {
            ctx.beginPath();

            if (pencilType == PENCIL) {
                ctx.fillStyle = lineColor;
            } else {
                ctx.fillStyle = BACKGROUND_COLOR;
            }

            ctx.fillRect(currX, currY, 2, 2);
            ctx.closePath();
            dot_flag = false;
        }
    }

    if (res == 'up' || res == "out") {
        flag = false;
    }

    if (res == 'move') {
        if (flag) {
            prevX = currX;
            prevY = currY;
            currX = e.clientX - canvas.offsetLeft;
            currY = e.clientY - canvas.offsetTop;
            draw();
        }
    }
}