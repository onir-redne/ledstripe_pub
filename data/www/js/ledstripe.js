var picker = null;


window.onload = function() {
    // create
    picker = new KellyColorPicker({ 
        place : 'color_picker_canvas',
        method: 'triangle',
        size: window.innerWidth - window.innerWidth / 6,
        methodSwitch: true,
        userEvents : { 
        
            change : function(self) {
                // on color chnge
                if (!self.selectedInput) return;
                if (self.getCurColorHsv().v < 0.5)
                    self.selectedInput.style.color = "#FFF";
                else
                    self.selectedInput.style.color = "#000";
    
                var rgbCurrent = self.getCurColorRgb();
                var hsvCurrent = self.getCurColorHsv();
                self.selectedInput.value = self.getCurColorHex();    
                self.selectedInput.style.background = self.selectedInput.value;
                var baseColorRgb = HSVtoRGB(hsvCurrent.h, 1.0, 1.0);
                self.selectedInput.children.base_color.style.background = rgbToHex(baseColorRgb.r, baseColorRgb.g, baseColorRgb.b);
                //.getCurColorRgb() - return current color in RGB format. Return array {r : var, g : var, b : var};
                

                // call ajax update color
                var led_stripe = 0;
                if (self.selectedInput.id == 'left_stripe_color') {
                    led_stripe = 0;
                } else {
                    led_stripe = 1;
                }

                $.ajax({
                    'url' : '/ajax/setcolor',
                    'type' : 'GET',
                    'data' : {
                        'r' : rgbCurrent.r,
                        'g' : rgbCurrent.g,
                        'b' : rgbCurrent.b,
                        'l' : led_stripe
                    }
                });
            }
        }
    });

    picker.getWheel().width = 50;
    picker.getSvFigCursor().radius = 35;
    picker.getSvFig().radius = 25;
    picker.getWheelCursor().height = 30;
    picker.getWheelCursor().lineWeight = 2;
    // update to applay size options
    picker.updateView(true);
    
    // addition user methods \ variables 
    picker.editInput = function(target) {
    
        if (picker.selectedInput) {
            picker.selectedInput.classList.remove('selected');
            picker.selectedInput.children.base_color.classList.remove('selected');
        }

        if (target) 
            picker.selectedInput = target;
        if (!picker.selectedInput) 
            return false;
        
        picker.selectedInput.classList.add('selected');
        picker.selectedInput.children.base_color.classList.add('selected');
        picker.setColor(picker.selectedInput.value);
    }
    
    // initialize 
    var mInputs = document.getElementsByClassName('multi-input');
    for (var i = 0; i < mInputs.length; i++) {
        picker.editInput(mInputs[i]);
    }
}


var savedColors = {
    add : function(sender) {
        var rgbCurrent = picker.getCurColorRgb();
        $.ajax({
            'url' : '/ajax/savedcolors_add',
            'type' : 'GET',
            'data' : {
                'r' : rgbCurrent.r,
                'g' : rgbCurrent.g,
                'b' : rgbCurrent.b
            }
        });
    }
}

/**
 * get leds state and update cotent acordingy..
 */
function ajaxGetLestripesState() {

}

/* ---------------------------------------------------
 tools
*/
function HSVtoRGB(h, s, v) {
    var r, g, b, i, f, p, q, t;
    if (arguments.length === 1) {
        s = h.s, v = h.v, h = h.h;
    }
    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return {
        r: Math.round(r * 255),
        g: Math.round(g * 255),
        b: Math.round(b * 255)
    };
}

function rgbToHex(r, g, b) {
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  }

