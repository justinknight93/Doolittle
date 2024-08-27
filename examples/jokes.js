var screenWidth = width();
var screenHeight = height();
var cBG = color(0, 0, 0);
var clink = color(150, 150, 255);
var cText = color(255, 255, 255);
var cText2 = color(255, 255, 0);

var textMultiplier = 2;
var charsPerLine = 40 / textMultiplier;
var textWidth = 6;
var textHeight = 8;
var margin = 4;
var realCharsPerLine = charsPerLine - (margin * 2) / textWidth;

setTextSize(textMultiplier);
setTextColor(cText);

var result = null;
var isSetup = false;
var setup = "";
var punchline = "";


function stringToLines(str) {
    var words = str.split(" ");
    var lines = [];
    var line = "";
    for (var i = 0; i < words.length; i++) {
        if (line.length + words[i].length + 1 <= realCharsPerLine) {
            line += words[i] + " ";
        } else {
            lines.push(line);
            line = words[i] + " ";
        }
    }
    lines.push(line);
    return lines;
}

function render(text, color) {
    setTextColor(color);
    drawFillRect(0, 0, screenWidth, screenHeight, cBG);
    var renderLines = stringToLines(text);
    for (var i = 0; i < renderLines.length; i++) {
        drawString(renderLines[i], margin, i * (textHeight * textMultiplier) + margin);
    }
}

function main() {
    if (!result) {
        render("Press any key to get a joke", cText);
        result = true;
    }

    var keys = getKeysPressed();
    if (!keys.length) {
        return;
    }

    if (keys[0]) {
        if (!isSetup) {
            result = httpGet("https://official-joke-api.appspot.com/random_joke");
            try {
                setup = JSON.parse(result.body).setup;
                punchline = JSON.parse(result.body).punchline;
            } catch (e) {
                setup = "Error";
                punchline = "Try again";
            }
            render(setup, cText);
        } else {
            render(punchline, cText2);
        }
        isSetup = !isSetup;
    }
}

while (true) {
    main();
    delay(100);
}