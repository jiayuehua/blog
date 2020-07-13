// The Star Wars cellular automaton (rule B2/S345/4).
//
// This JavaScript implementation (c) 2020 Arthur O'Dwyer.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.


// Normally we'd have only the following cell values:
//   0 - dead
//   1 - alive
//   2 - refractory A
//   3 - refractory B
// But for kicks let's add a high-order bit for "faction".
// Bits 0xC encode a cell's "faction."
// When a cell goes into state 1, it will have 2 live neighbors.
// Its faction becomes the combination of its live neighbors' factions:
//   0x4 + 0x0 = 0x4
//   0x4 + 0x4 = 0x4
//   0x4 + 0x8 = 0x0

function starWars(canvas, margin, pixelsPerCell, busyBorders, wrapBorders) {
    var W = 2*margin + ~~(canvas.width / pixelsPerCell);
    var H = 2*margin + ~~(canvas.height / pixelsPerCell);

    var peacefulCell = function (x, y) {
        if (0 <= x && x < W && 0 <= y && y < H) {
            return this.data[y*W+x];
        }
        return 0;
    };
    var wrapCell = function (x, y) {
        x = (x + W) % W;
        y = (y + H) % H;
        return this.data[y*W+x];
    };
    var busyCell = function (x, y) {
        if (0 <= x && x < W && 0 <= y && y < H) {
            return this.data[y*W+x];
        } else if (y < 0) {
            return 0x4 + Math.round(Math.random());
        } else if (y >= H) {
            return 0x8 + Math.round(Math.random());
        }
        return 0;
    };

    var result = {
        W: W,
        H: H,
        Margin: margin,
        data: null,
        initial_data: null,
        displayIndices: null,
        randomize: function() {
            var result = Array(W*H);
            for (var i = 0; i < result.length; ++i) {
                result[i] = Math.round(Math.random());
            }
            this.initial_data = result;
            this.reset();
        },
        initializeWithPattern: function (pattern) {
            var result = new Array(W*H).fill(0);
            var PH = pattern.length;
            for (var py = 0; py < PH; ++py) {
                var PW = pattern[py].length;
                for (var px = 0; px < PW; ++px) {
                    var x = ((W - PW) >> 1) + px;
                    var y = ((H - PH) >> 1) + py;
                    result[y*W+x] = pattern[py][px];
                }
            }
            this.initial_data = result;
            this.reset();
        },
        reset: function() {
            this.data = this.initial_data.slice();
        },
        clear: function() {
            this.data = Array(W*H).fill(0);
        },
        cell: busyBorders ? busyCell : wrapBorders ? wrapCell : peacefulCell,
        is_live_cell: function (x, y) {
            return (this.cell(x, y) & 3) == 1;
        },
        live_cell_should_continue_living: function (x, y) {
            var count = (
                this.is_live_cell(x-1, y-1) + this.is_live_cell(x, y-1) + this.is_live_cell(x+1, y-1) +
                this.is_live_cell(x-1, y) + this.is_live_cell(x+1, y) +
                this.is_live_cell(x-1, y+1) + this.is_live_cell(x, y+1) + this.is_live_cell(x+1, y+1)
            );
            return (count == 3 || count == 4 || count == 5);
        },
        faction_of_cell: function (x, y) {
            var c = this.cell(x, y);
            if (c == 0x0 + 1) return 1;
            if (c == 0x4 + 1) return 2;
            if (c == 0x8 + 1) return 3;
            return 0;
        },
        dead_cell_should_start_living: function (x, y) {
            var counts = [0, 0, 0, 0];
            counts[this.faction_of_cell(x-1, y-1)] += 1;
            counts[this.faction_of_cell(x  , y-1)] += 1;
            counts[this.faction_of_cell(x+1, y-1)] += 1;
            counts[this.faction_of_cell(x-1, y  )] += 1;
            counts[this.faction_of_cell(x+1, y  )] += 1;
            counts[this.faction_of_cell(x-1, y+1)] += 1;
            counts[this.faction_of_cell(x  , y+1)] += 1;
            counts[this.faction_of_cell(x+1, y+1)] += 1;
            if (counts[1] + counts[2] + counts[3] == 2) {
                if (counts[1] == 2) return 1;
                if (counts[2] == 0) return 3;
                if (counts[3] == 0) return 2;
                return 1;
            }
            return 0;
        },
        tweak_one_random_cell: function () {
            var i = Math.floor(Math.random() * (W*H)) % (W*H);
            this.data[i] = (this.data[i] & ~3) | ((this.data[i] + 1) & 3);
        },
        next_step: function () {
            var result = Array(W*H);
            for (var x = 0;  x < W; ++x) {
                for (var y = 0; y < H; ++y) {
                    var c = this.cell(x, y);
                    if (c == 0) {
                        var faction = this.dead_cell_should_start_living(x, y);
                        if (faction != 0) {
                            result[y*W+x] = 1 + ((faction - 1) << 2);
                        } else {
                            result[y*W+x] = 0;
                        }
                    } else if ((c & 3) == 1) {
                        if (this.live_cell_should_continue_living(x, y)) {
                            result[y*W+x] = c;
                        } else {
                            result[y*W+x] = c + 1;
                        }
                    } else if ((c & 3) == 2) {
                        result[y*W+x] = c + 1;
                    } else {
                        result[y*W+x] = 0;
                    }
                }
            }
            this.data = result;
        },
        createDisplayIndices: function (canvas) {
            // This function should be called whenever the canvas is resized.
            var datalength = canvas.width * canvas.height;
            this.displayIndices = Array(datalength);
            for (var i = 0; i < datalength; ++i) {
                var cy = ~~(i / canvas.width);
                var cx = i % canvas.width;
                var py = margin + ~~((H - 2*margin) * cy / canvas.height);
                var px = margin + ~~((W - 2*margin) * cx / canvas.width);
                this.displayIndices[i] = py*W+px;
            }
        },
        display: function (canvas) {
            var context = canvas.getContext('2d');
            var imageData = context.getImageData(0, 0, canvas.width, canvas.height);
            var data = imageData.data;
            console.assert(data.length == canvas.width * canvas.height * 4);
            var datalength = canvas.width * canvas.height;
            var indices = this.displayIndices;
            console.assert(datalength == indices.length);
            for (var i = 0; i < datalength; ++i) {
                var color = this.data[indices[i]];
                data[4*i+0] = [32, 255, 192, 128, 32, 255, 255, 255, 32,   0,   0,   0][color];
                data[4*i+1] = [32,  64,   0,   0, 32, 255, 128,   0, 32, 255, 255,  32][color];
                data[4*i+2] = [32, 255, 192, 128, 32,   0,   0,   0, 32,   0, 128, 255][color];
                data[4*i+3] = 255;
            }
            context.putImageData(imageData, 0, 0);
        },
        draw_dragging_through: function (x, y) {
            console.assert(0 <= x && x < W);
            console.assert(0 <= y && y < H);
            this.data[y*W+x] += 1;
            this.data[y*W+x] %= 4;
        },
    };
    result.createDisplayIndices(canvas);
    return result;
}

function hookUpCanvasEditing(life, canvas) {
    var drawing = false;
    var previous_x = -1;
    var previous_y = -1;
    canvas.addEventListener('mousedown', function (e) {
        previous_x = -1;
        previous_y = -1;
        drawing = true;
    });
    canvas.addEventListener('mousemove', function (e) {
        if (!drawing) return;
        var rect = canvas.getBoundingClientRect();
        var cx = (event.clientX - rect.left);
        var cy = (event.clientY - rect.top);
        var px = life.Margin + ~~((life.W - 2 * life.Margin) * cx / canvas.width);
        var py = life.Margin + ~~((life.H - 2 * life.Margin) * cy / canvas.height);
        if (px != previous_x || py != previous_y) {
            life.draw_dragging_through(px, py);
            previous_x = px;
            previous_y = py;
            life.display(canvas);
        }
    });
    canvas.addEventListener('mouseup', function (e) {
        drawing = false;
    });
}

function starWarsWithButtons(canvas, {
    buttons=['play', 'step', 'reset'],
    pixelsPerCell=2, busyBorders=false, wrapBorders=false,
    margin=10,
    millisecondsPerFrame=50,
    tweakForEternalInterest=false,
    initialPattern=null,
    initiallyAdvance=0,
}) {
    var life = starWars(canvas, margin, pixelsPerCell, busyBorders, wrapBorders);
    if (initialPattern !== null) {
        life.initializeWithPattern(initialPattern);
    } else {
        life.randomize();
    }
    for (var i = 0; i < initiallyAdvance; ++i) {
        life.next_step();
    }
    hookUpCanvasEditing(life, canvas);
    life.display(canvas);

    // Create the desired buttons, as siblings of the canvas element.
    var playButton = null;
    if (buttons.includes('play')) {
        playButton = document.createElement("button");
        playButton.innerHTML = 'Play';
        playButton.addEventListener('click', function () {
            if (stopPlayingIfNecessary()) {
                // okay, stopped
            } else {
                intervalId = window.setInterval(function () {
                    life.next_step();
                    if (tweakForEternalInterest) {
                        life.tweak_one_random_cell();
                    }
                    life.display(canvas);
                }, millisecondsPerFrame);
                playButton.innerHTML = 'Pause';
            }
        });
    }

    var intervalId = null;
    var stopPlayingIfNecessary = function () {
        if (intervalId === null) {
            return false;
        } else {
            window.clearInterval(intervalId);
            if (playButton !== null) {
                playButton.innerHTML = 'Play';
            }
            intervalId = null;
            return true;
        }
    };

    if (buttons.length != 0) {
        var buttonsDiv = document.createElement("div");
        canvas.parentElement.appendChild(buttonsDiv);
        for (buttonName of buttons) {
            buttonsDiv.appendChild(document.createTextNode(" "))
            if (buttonName == 'play') {
                buttonsDiv.appendChild(playButton);
            } else if (buttonName == 'step') {
                var stepButton = document.createElement("button");
                stepButton.innerHTML = 'Step';
                stepButton.addEventListener('click', function() {
                    stopPlayingIfNecessary();
                    life.next_step();
                    life.display(canvas);
                });
                buttonsDiv.appendChild(stepButton);
            } else if (buttonName == 'reset') {
                var resetButton = document.createElement("button");
                resetButton.innerHTML = 'Reset';
                resetButton.addEventListener('click', function() {
                    stopPlayingIfNecessary();
                    life.reset();
                    life.display(canvas);
                });
                buttonsDiv.appendChild(resetButton);
            } else if (buttonName == 'clear') {
                var clearButton = document.createElement("button");
                clearButton.innerHTML = 'Clear';
                clearButton.addEventListener('click', function() {
                    stopPlayingIfNecessary();
                    life.clear();
                    life.display(canvas);
                });
                buttonsDiv.appendChild(clearButton);
            } else {
                console.assert(false);
            }
        }
    }
}
