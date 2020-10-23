var picker = null;
var updateTimer = null;
$(function(){
    $( "[data-role='header'], [data-role='footer']" ).toolbar();
});

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
                var baseColorRgb = HSVtoRGB(hsvCurrent.h, hsvCurrent.s, 1.0);      
                self.selectedInput.style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";          

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

    picker.getWheel().width = 40;
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
            //picker.selectedInput.children.base_color.classList.remove('selected');
        }

        if (target) 
            picker.selectedInput = target;
        if (!picker.selectedInput) 
            return false;
        
        picker.selectedInput.classList.add('selected');
        //picker.selectedInput.children.base_color.classList.add('selected');
        picker.setColor(picker.selectedInput.value);
    }
    
    // initialize 
    var mInputs = document.getElementsByClassName('multi-input');
    for (var i = 0; i < mInputs.length; i++) {
        picker.editInput(mInputs[i]);
    }

    // set led status updat timer to 10s.
    updateTimer = setInterval(ajaxGetLestripesState, 1000);
}


var savedColors = {
    add : function(sender, rgb_color, hsv_color, name) {
        var rgbCurrent = picker.getCurColorRgb();
        var hsvCurrent = picker.getCurColorHsv();
        $.getJSON('/ajax/savedcolors_add', {
            'r' : rgb_color.r,
            'g' : rgb_color.g,
            'b' : rgb_color.b,
            'h': hsv_color.h,
            's': hsv_color.s,
            'v': hsv_color.v,
            'name': name},
            function(response) {
                            
            });
    }

}


/**
 * get leds state and update cotent acordingy..
 */
function ajaxGetLestripesState() {
    $.getJSON('/ajax/getstripesstate', function(jsondata) {

        $('#power_off_timer').val((jsondata.timer / 60).toFixed(1));
        $('#power_off_timer').slider( "refresh" );
        if(jsondata.power == 1) {
            if($("#power_sw option:selected").val() != 'on')
                $('#power_sw').val("on").change();
        } else {
            if($("#power_sw option:selected").val() != 'off')
                $('#power_sw').val("off").change();
        }      
     });
}

function ajaxSetLedstripesPower(self) {
    if(self.value == 'on') {
        $.ajax({'url' : '/ajax/poweron'});
    } else {
        $.ajax({'url' : '/ajax/poweroff'});
    }
}

function ajaxSetLedstripesPowerTimer(self) {
    $.ajax({
        'url' : '/ajax/powertimer',
        'type' : 'GET',
        'data' : {
            'timer' : self.value * 60
        }
    });
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

