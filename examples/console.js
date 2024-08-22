var screenWidth = width();
var screenHeight = height();
var cBG = color(0, 0, 0);
var cResult = color(150, 150, 255);
var cText = color(255, 255, 255);
var input = "";
var cursorIndex = 0;
var result = "";
setTextSize(1);

function arrayIncludes(array, value) {
    for (var i = 0; i < array.length; i++) {
        if (array[i] === value) {
            return true;
        }
    }
    return false;
}

function updateScreen() {
    drawFillRect(0, 0, screenWidth, screenHeight, cBG);
    setTextColor(cText);
    drawString(input, 4, 4);
    drawString(" ".repeat(cursorIndex) + "^", 4, 14);
    setTextColor(cResult);

    var resultCopy = result.toString();
    var lines = [];
    var maxLength = 39;

    for (var i = 0; i < resultCopy.length; i += maxLength) {
        lines.push(resultCopy.slice(i, i + maxLength));
    }

    for (var j = 0; j < lines.length && j < 4; j++) {
        drawString(lines[j], 4, 24 + j * 10);
    }
}

updateScreen();

while (true) {
    var prevInput = input;
    var prevCursorIndex = cursorIndex;
    var keys = getKeysPressed();
    if (keys.length > 0) {
        if (keys[0] === "Delete") {
            if (cursorIndex > 0) {
                input = input.slice(0, cursorIndex - 1) + input.slice(cursorIndex);
                cursorIndex--;
            }
        } else if (arrayIncludes(keys, "Function") && arrayIncludes(keys, ",")) {
            if (cursorIndex > 0) {
                cursorIndex--;
            }
        } else if (arrayIncludes(keys, "Function") && arrayIncludes(keys, "/")) {
            if (cursorIndex < input.length) {
                cursorIndex++;
            }
        } else if (keys[0] === "Enter") {
            try {
                result = eval(input);
            } catch (e) {
                result = e;
            }
        } else if (!arrayIncludes(keys, "Function") && !arrayIncludes(keys, "Enter") && !arrayIncludes(keys, "Delete") && !arrayIncludes(keys, "Option") && !arrayIncludes(keys, "Alt") && !arrayIncludes(keys, "Tab")) {
            input = input.slice(0, cursorIndex) + keys[0] + input.slice(cursorIndex);
            cursorIndex++;
        }
        updateScreen();
    }
}