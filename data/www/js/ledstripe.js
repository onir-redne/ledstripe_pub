/**
 * @author Dominik Pająk (onir) <onir.redne@mdrk.net>
 * @copyright 2020 Dominik Pająk
 * @license GPLv3
 * 
 * -------------------------------------------------
 * Color picker is: Kelly ("KellyColorPicker")
 * by Rubchuk Vladimir <torrenttvi@gmail.com>
 * (c) 2015-2020 Rubchuk Vladimir
 * license: GPLv3
 * version: 1.21
 * -------------------------------------------------
 * Color conversion functions: RGBtoHSV(), HSVtoRGB() both find online 
 * on JavaScript dicussion formus, If you're an author and want your code to be removed, 
 * you want to me to add credits or licensing information please contact me.
 * RGBtoHSV() found on: http://javascripter.net/faq/rgb2hsv.htm
 * HSVtoRGB() found on: https://stackoverflow.com/questions/17242144/javascript-convert-hsb-hsv-color-to-rgb-accurately
 */ 

var updateTimer = null;
$(function(){
    $( "[data-role='header'], [data-role='footer']" ).toolbar();
});


window.onload = function() {

    // setup navigation
    navigationHelpers.init();
    powerManagement.init();
    navigationHelpers.ShowLoadingOverlay();

    main_color_picker = new KellyColorPicker({ 
        place : 'color_picker_canvas',
        method: 'quad',
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
                var baseColorRgb = tools.HSVtoRGB(hsvCurrent.h, hsvCurrent.s, 1.0);      
                self.selectedInput.style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";          
                self.selectedInput.value = self.getCurColorHex()

                $.ajax({
                    'url' : '/ajax/setcolor',
                    'type' : 'GET',
                    'data' : {
                        'r' : rgbCurrent.r,
                        'g' : rgbCurrent.g,
                        'b' : rgbCurrent.b,
                        'l' : self.selectedInput.getAttribute('data-stripe-id')
                    }
                });
            }
        }
    });

    dialog_color_picker = new KellyColorPicker({ 
        place : 'color_dialog_canvas',
        method: 'quad',
        methodSwitch: true,
        userEvents : { 
        
            change : function(self) {
                // on color chnge
                var rgbCurrent = self.getCurColorRgb();
                var hsvCurrent = self.getCurColorHsv();
                var baseColorRgb = tools.HSVtoRGB(hsvCurrent.h, hsvCurrent.s, 1.0);      
                document.getElementById('color_picker_color').style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";

                $.ajax({
                    'url' : '/ajax/setpeek',
                    'type' : 'GET',
                    'data' : {
                        'r' : rgbCurrent.r,
                        'g' : rgbCurrent.g,
                        'b' : rgbCurrent.b
                    }
                });
            }
        }
    });

    // setup main color picker
    main_color_picker.getWheel().width = 40;
    main_color_picker.getSvFigCursor().radius = 35;
    main_color_picker.getSvFig().radius = 25;
    main_color_picker.getWheelCursor().height = 30;
    main_color_picker.getWheelCursor().lineWeight = 2;
    // update to applay size options
    main_color_picker.updateView(true);
    
    // addition user methods \ variables 
    main_color_picker.editInput = function(target) {
    
        if (main_color_picker.selectedInput) {
            main_color_picker.selectedInput.classList.remove('selected');
        }

        if (target) 
            main_color_picker.selectedInput = target;
        if (!main_color_picker.selectedInput) 
            return false;
        
        main_color_picker.selectedInput.classList.add('selected');
        main_color_picker.setColor(main_color_picker.selectedInput.value);
    }

    
    // initialize 
    var mInputs = document.getElementsByClassName('multi-input');
    for (var i = 0; i < mInputs.length; i++) {
        main_color_picker.editInput(mInputs[i]);
    }

    $( document ).on( 'pagecontainerchange', function( event, ui ) {
        navigationHelpers.PageChangeHandler(event, ui);
      });

    // set led status and schedule update timer.
    powerManagement.ajaxGetLestripesState(); 
    updateTimer = setInterval(powerManagement.ajaxGetLestripesState, 10000);
    navigationHelpers.HideLoadingOverlay();
}


/**
 * Navigation help
 */
var navigationHelpers = {

    confirmation_dialog_text : null,
    confirmation_dialog_OK : null,
    color_peeker_dialog_text : null,
    color_peeker_dialog_OK : null,
    loading_overlay : null,
    saved_colors_list : null,
    color_peeker_dialog_text : null,
    color_peeker_dialog_name : null,
    color_peeker_dialog : null,

    init : function() {
        self.confirmation_dialog_text =  $('#confirmation_dialog_text');
        self.confirmation_dialog_OK = $('#confirmation_dialog_OK');
        self.color_peeker_dialog_text = $('#color_peeker_dialog_text');
        self.color_peeker_dialog_OK = $('#color_peeker_dialog_OK');
        self.loading_overlay = $('#loading_overlay');
        self.saved_colors_list = $('#saved_colors_list');
        self.color_peeker_dialog_text = $('#color_peeker_dialog_text');
        self.color_peeker_dialog_name = $('#color_peeker_dialog_name');
        self.color_peeker_dialog = $('#color_peeker_dialog');
    },

    ShowLoadingOverlay : function() {
        //var loading_obj = $('#loading_overlay');
        var theme =  self.loading_overlay.jqmData( "theme" ) || $.mobile.loader.prototype.options.theme;
        var msgText =  self.loading_overlay.jqmData( "msgtext" ) || $.mobile.loader.prototype.options.text;
        var textVisible =  self.loading_overlay.jqmData( "textvisible" ) || $.mobile.loader.prototype.options.textVisible;
        var textonly = !! self.loading_overlay.jqmData( "textonly" );
        var html =  self.loading_overlay.jqmData( "html" ) || "";
        $.mobile.loading( "show", {text: msgText, textVisible: textVisible, theme: theme, textonly: textonly, html: html});
    },

    HideLoadingOverlay : function() {
        $.mobile.loading( "hide" );
    },

    PageChangeHandler : function(event, ui) {
        if(ui.toPage[0].id == 'saved-colors') {
            this.ShowLoadingOverlay();
            savedColors.load(self.saved_colors_list);
            this.HideLoadingOverlay();
        }
    },

    ShowConfirmationDialog : function(src, ok_handler, message) {
        self.confirmation_dialog_OK.unbind('click');
        if(message) {
            self.confirmation_dialog_text.text(message);
        } else {
            self.confirmation_dialog_text.text("Arey you sure?");
        }
        $.mobile.changePage('#confirmation_dialog');
        self.confirmation_dialog_OK.on('click', ok_handler);  
    },

    ShowColorPickerDialog : function(src, ok_handler, set_name, message, color_name, color) {
        self.color_peeker_dialog_OK.unbind('click');
        if(message) {
            self.color_peeker_dialog_text.text(message);
        } else {
            self.color_peeker_dialog_text.text("");
        }
        if(set_name) {
            self.color_peeker_dialog_name.show();
            self.color_peeker_dialog_name.val(color_name)
        } else {
            self.color_peeker_dialog_name.hide();
        }
        dialog_color_picker.rgb = {r: color.r, g: color.g, b: color.b};
        //resize picker to dialog
        var dialog_width = self.color_peeker_dialog.width();
        // setup main color picker
        dialog_color_picker.getWheel().width = 30;
        dialog_color_picker.getSvFigCursor().radius = 30;
        dialog_color_picker.getSvFig().radius = 20;
        dialog_color_picker.getWheelCursor().height = 20;
        dialog_color_picker.getWheelCursor().lineWeight = 2;
        dialog_color_picker.resize(dialog_width - dialog_width / 3);
        // update to applay size options
        dialog_color_picker.updateView(true);

        // bind handler
        self.color_peeker_dialog_OK.on('click', ok_handler);        
        $.mobile.changePage('#color_peeker_dialog');
    },
}


/**
 * Saved colors
 */
var savedColors = {
    /**
     * Get current colors from device
     * @param {*} target 
     */
    load : function(target) {
        $.getJSON('/ajax/savedcolors_get', function(response) {
            target.html('');
            var new_html = '';
            for (i = 0; i < response.colors.length; i++) {
                var c = {r: response.colors[i].r, g : response.colors[i].g, b : response.colors[i].b};
                var vc = tools.RGBfromHV(response.colors[i].r, response.colors[i].g, response.colors[i].b);
                var cid = response.colors[i].id;
                var name = response.colors[i].name;
                
                // if(!(response.colors[i].s & 0x01)) {  
                //     c = {r: 255, g: 255, b: 255};
                //     vc = {r: 255, g: 255, b: 255};
                // }

                if(!(response.colors[i].s & 0x02)) {
                    name += ' *';
                }
                
                // if(name.length == 0) {
                //     name = 'empty';
                // }

                new_html += '<a href="#" class="ui-shadow ui-btn ui-corner-all saved-color-element" style="background: linear-gradient(90deg, rgba('+ c.r +','+ c.g +','+ c.b +',1) 70%, rgba(' + vc.r + ',' + vc.g + ',' + vc.b + ',1) 100%);">\
                                <div class="ui-grid-g">\
                                    <div class="ui-block-a ui-input-btn ui-btn ui-corner-all ui-icon-edit ui-btn-icon-notext saved-color-button" onclick="navigationHelpers.ShowColorPickerDialog(event.target, function() { savedColors.set(' + cid + '); }, true, "edit color" ,' +  name + ', ' + rgba('+ c.r +','+ c.g +','+ c.b +',1) + ' ");">\
                                        <input type="button" data-enhanced="true" value="">\
                                    </div>\
                                    <div class="ui-block-b ui-input-btn ui-btn ui-corner-all ui-icon-delete ui-btn-icon-notext saved-color-button" onclick="navigationHelpers.ShowConfirmationDialog(event.target, function() { savedColors.delete(' + cid + '); }, "Do you realy want to remove?");">\
                                        <input type="button" data-enhanced="true" value="">\
                                    </div>\
                                </div>\
                                <div class="saved-color-text">' +  name + '</div></a>';
                }
            // when empty list just or free slots left append ADD button
            if(response.free > 0) {
                new_html += "<button class=\"ui-btn ui-icon-plus ui-corner-all ui-btn-icon-left\" onclick=\"navigationHelpers.ShowColorPickerDialog(event.target, function() { savedColors.add(); }, true, 'Add color', '', { r: 255, g: 255, b: 255 });\">Add color</button>";
            }
            target.html(new_html);
        });
    },

    /**
     *  Add to free slot 
     * @param {*} sender 
     * @param {*} rgb_color 
     * @param {*} hsv_color 
     * @param {*} name 
     */
    add : function() {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#color_peeker_dialog_name').val();
        $.getJSON('/ajax/savedcolors_set', {
            'r' : rgb_color.r,
            'g' : rgb_color.g,
            'b' : rgb_color.b,
            'name': name},
            function(response) {
                // reload colors on success
                self.load();
            });
    },

    set : function(cid)  {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#color_peeker_dialog_name').val();
        $.getJSON('/ajax/savedcolors_set', {
            'id' : cid,
            'r' : rgb_color.r,
            'g' : rgb_color.g,
            'b' : rgb_color.b,
            'name': name},
            function(response) {
                // reload colors on success
                self.load();
            });
    },

    delete : function(cid)  {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#color_peeker_dialog_name').val();
        $.getJSON('/ajax/savedcolors_del', {'id' : cid},
            function(response) {
                // reload colors on success
                self.load();
            });
    },

    sync : function() {
        $.getJSON('/ajax/savedcolors_sync', {},
        function(response) {
            // reload colors on success
            self.load();
        });
    }
}

var powerManagement = {

    poweroff_slider : null,
    poweroff_slider_user_int : false,
    power_sw : null,

    init : function() {
        self.poweroff_slider = $('#power_off_timer');
        self.power_sw = $('#power_sw');
        // attach power timer event handler
        self.poweroff_slider.on("slidestart", powerManagement.poweroffTimerOnSlideStart);
        self.poweroff_slider.on("slidestop", powerManagement.poweroffTimerOnSlideEnd);
    },

    ajaxSetLedstripesPowerTimer : function(time) {
        $.ajax({
            'url' : '/ajax/powertimer',
            'type' : 'GET',
            'data' : {
                'timer' : time * 60
            }
        });
    },

    ajaxSetLedstripesPower : function(self) {
        if(self.value == 'on') {
            $.ajax({'url' : '/ajax/poweron'});
        } else {
            $.ajax({'url' : '/ajax/poweroff'});
        }
    },

    ajaxGetLestripesState : function() {
        $.getJSON('/ajax/getstripesstate', function(jsondata) {
            
            if(!self.poweroff_slider_user_int) {
                self.poweroff_slider.val((jsondata.timer / 60).toFixed(1));
                self.poweroff_slider.slider( "refresh" );
            }
            if(jsondata.power == 1) {
                if($("#power_sw option:selected").val() != 'on')
                    self.power_sw.val("on").change();
            } else {
                if($("#power_sw option:selected").val() != 'off')
                    self.power_sw.val("off").change();
            }      
         });
    },

    poweroffTimerOnSlideStart : function(event, ui) {
        self.poweroff_slider_user_int = true;   // prevent updating when user interacts
    },

    poweroffTimerOnSlideEnd : function(event, ui) {
        //event.preventDefault();
        //event.stopPropagation();
        powerManagement.ajaxSetLedstripesPowerTimer(self.poweroff_slider.val());
        self.poweroff_slider_user_int = false;
    },

    poweroffTimerOnChange : function(event, ui) {
        //event.preventDefault();
        //event.stopPropagation();
        //powerManagement.ajaxSetLedstripesPowerTimer(self.poweroff_slider.val());
    },
}


/**
 * get leds state and update cotent acordingy..
 */






/* ---------------------------------------------------
 tools
*/
var tools = {
    RGBtoHSV : function(r,g,b) {
    var computedH = 0;
    var computedS = 0;
    var computedV = 0;

    //remove spaces from input RGB values, convert to int
    var r = parseInt( (''+r).replace(/\s/g,''),10 ); 
    var g = parseInt( (''+g).replace(/\s/g,''),10 ); 
    var b = parseInt( (''+b).replace(/\s/g,''),10 ); 

    if ( r==null || g==null || b==null ||
        isNaN(r) || isNaN(g)|| isNaN(b) ) {
        alert ('Please enter numeric RGB values!');
        return;
    }
    if (r<0 || g<0 || b<0 || r>255 || g>255 || b>255) {
        alert ('RGB values must be in the range 0 to 255.');
        return;
    }
    r=r/255; g=g/255; b=b/255;
    var minRGB = Math.min(r,Math.min(g,b));
    var maxRGB = Math.max(r,Math.max(g,b));

    // Black-gray-white
    if (minRGB==maxRGB) {
        computedV = minRGB;
        return [0,0,computedV];
    }

    // Colors other than black-gray-white:
    var d = (r==minRGB) ? g-b : ((b==minRGB) ? r-g : b-r);
    var h = (r==minRGB) ? 3 : ((b==minRGB) ? 1 : 5);
    computedH = 60*(h - d/(maxRGB - minRGB));
    computedS = (maxRGB - minRGB)/maxRGB;
    computedV = maxRGB;

    return {
        h: computedH,
        s: computedS,
        v: computedV
    };
},

HSVtoRGB : function (h, s, v) {
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
},

RGBfromHV : function (r, g, b) {
    var hsv = self.RGBtoHSV(r, g, b);
    return self.HSVtoRGB(hsv.h, hsv.s, 1);
},

RGBtoHex : function (r, g, b) {
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  }

}

