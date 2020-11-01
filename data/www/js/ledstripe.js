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

    navigationHelpers.init();
    powerManagement.init();
    stripeState.init();

    navigationHelpers.ShowLoadingOverlay();

    main_color_picker = new KellyColorPicker({ 
        place : 'color_picker_canvas',
        method: 'quad',
        size: window.innerWidth - window.innerWidth / 8,
        methodSwitch: true,
        userEvents : { 
        
            change : function(self) {
                // on color chnge
                if (!self.selectedInput) return;
                var rgbCurrent = self.getCurColorRgb();
                var hsvCurrent = self.getCurColorHsv();
                var baseColorRgb = hsvToRgb(hsvCurrent.h, hsvCurrent.s, 1.0);      
                self.selectedInput.style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";          
                self.selectedInput.value = self.getCurColorHex()
                stripeState.setColor(self.selectedInput.getAttribute('data-stripe-id'), rgbCurrent.r, rgbCurrent.g, rgbCurrent.b);
                // $.ajax({
                //     'url' : '/ajax/setcolor',
                //     'type' : 'GET',
                //     'data' : {
                //         'r' : rgbCurrent.r,
                //         'g' : rgbCurrent.g,
                //         'b' : rgbCurrent.b,
                //         'l' : self.selectedInput.getAttribute('data-stripe-id')
                //     }
                // });
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
                var baseColorRgb = hsvToRgb(hsvCurrent.h, hsvCurrent.s, 1.0);      
                document.getElementById('color_picker_color').style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";
                stripeState.setColorPeek(rgbCurrent.r, rgbCurrent.g, rgbCurrent.b);
                // $.ajax({
                //     'url' : '/ajax/setpeek',
                //     'type' : 'GET',
                //     'data' : {
                //         'r' : rgbCurrent.r,
                //         'g' : rgbCurrent.g,
                //         'b' : rgbCurrent.b
                //     }
                // });
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
        
        self.saved_colors_list = $('#saved_colors_list');
        self.color_peeker_dialog_text = $('#color_peeker_dialog_text');
        self.color_peeker_dialog_name = $('#color_peeker_dialog_name');
        self.color_peeker_dialog = $('#color_peeker_dialog');
        self.color_peeker_dialog_OK = $('#color_peeker_dialog_OK');

        self.transition_sets_list = $('#transition_sets_list');
        self.trans_edit_dialog_text = $('#trans_edit_dialog_text');
        self.trans_edit_dialog_name = $('#trans_edit_dialog_name');
        self.trans_edit_dialog = $('#trans_edit_dialog');
        self.trans_edit_dialog_OK = $('#color_peeker_dialog_OK');
        
        self.loading_overlay = $('#loading_overlay');
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
        //dialog_color_picker.rgb = {r: color.r, g: color.g, b: color.b};
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
        dialog_color_picker.setColorByHex(rgb(color.r, color.g, color.b));
        dialog_color_picker.updateView(true);

        // bind handler
        self.color_peeker_dialog_OK.on('click', ok_handler);        
        $.mobile.changePage('#color_peeker_dialog');
    },

    ShowTransEditDialog : function(src, ok_handler, set_name, message, set_values_array) {
        self.trans_edit_dialog_OK.unbind('click');
        if(message) {
            self.trans_edit_dialog_text.text(message);
        } else {
            self.trans_edit_dialog_text.text("");
        }
        if(set_name) {
            self.trans_edit_dialog_name.show();
        } else {
            self.trans_edit_dialog_name.hide();
        }

        if(set_values_array) {
            // load array
        } else {
            // empty set
        }
        
        
        // bind handler
        self.trans_edit_dialog_OK.on('click', ok_handler);        
        $.mobile.changePage('#trans_edit_dialog');
    }
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
                hsv = rgbToHsv(c.r, c.g, c.b);
                var vc = hsvToRgb(hsv.h, hsv.s, 1);
                var cid = response.colors[i].id;
                var name = response.colors[i].name;
                
                /*if(!(response.colors[i].s & 0x02)) {
                    name += '';
                }*/

                /**
                <div data-type="horizontal" class="ui-grid-c ui-shadow ui-corner-all saved-color-element" style="background: linear-gradient(0deg, rgba(100,130,200,1) 40%, rgba(200,220,255,1) 100%);">
                    <div class="ui-block-a">
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-edit ui-btn-icon-notext"></a>
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-delete ui-btn-icon-notext"></a>
                    </div>
                    <div class="ui-block-b"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp">A</a></div>
                    <div class="ui-block-c"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp">B</a></div>
                    <div class="ui-block-d"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp">C</a></div>
                    <div class="saved-color-text center-wrapper">demo color</div>
                </div>
                 */
                
                new_html += `
                <div data-type="horizontal" class="ui-grid-c ui-shadow ui-corner-all saved-color-element" style="background: linear-gradient(0deg, rgba(`+ c.r +`,`+ c.g +`,`+ c.b +`,1) 40%, rgba(` + vc.r + `,` + vc.g + `,` + vc.b + `,1) 100%);">
                    <div class="ui-block-a">
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-edit ui-btn-icon-notext" onclick="navigationHelpers.ShowColorPickerDialog(event.target, function() { savedColors.set(` + cid + `); }, true, 'edit color' ,'` + name + `',{ r:` + c.r + `, g:` + c.g + `, b:` + c.b + `}, 1);"></a>
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-delete ui-btn-icon-notext" onclick="navigationHelpers.ShowConfirmationDialog(event.target, function() { savedColors.del(` + cid + `); });"></a>
                    </div>
                    <div class="ui-block-b"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp" onclick="stripeState.setColor(0, `+ c.r +`,`+ c.g +`,`+ c.b +`);">A</a></div>
                    <div class="ui-block-c"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp" onclick="stripeState.setColor(1, `+ c.r +`,`+ c.g +`,`+ c.b +`);">B</a></div>
                    <div class="ui-block-d"><a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-action ui-btn-icon-down saved-color-text input-quad-transp" onclick="stripeState.setColor(2, `+ c.r +`,`+ c.g +`,`+ c.b +`);">C</a></div>
                    <div class="saved-color-text center-wrapper">` + name + `</div>
                </div>`;
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


/**
 * Color transition Editor
 */
var colorTransitionEditor = {

    init : function() {

    },

    
}

/**
 * set stripe to color, transition or spectrum
 * enable color peek
 */
var stripeState = {

    init : function() {

    },

    setColor : function(id, r, g, b) {
        $.ajax({
            'url' : '/ajax/setcolor',
            'type' : 'GET',
            'data' : {
                'r' : r,
                'g' : g,
                'b' : b,
                'l' : id
            }
        });
    },

    setColorPeek : function(r, g, b) {
        $.ajax({
            'url' : '/ajax/setpeek',
            'type' : 'GET',
            'data' : {
                'r' : r,
                'g' : g,
                'b' : b
            }
        });
    },

    setTransition : function(id) {

    },

    setSpectrum : function(id) {

    }
 }

/**
 * Power timer and switch
 */
var powerManagement = {

    poweroff_slider : null,
    poweroff_slider_user_int : false,
    power_sw : null,
    sync_switch : false,

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
            
            if(jsondata.sync != 0 && !self.sync_switch) {
                $("#falsh-sync-button").css("border-color", "brown");
                $("#falsh-sync-button").css("border-width", "3px");
                self.sync_switch = true;
            } else if(jsondata.sync == 0 && self.sync_switch) {
                $("#falsh-sync-button").css("border-color", "grey");
                $("#falsh-sync-button").css("border-width", "1px");
                self.sync_switch = false;
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

function hsvToRgb(h, s, v) {
    var r, g, b, i, f, p, q, t;

    if (h && s === undefined && v === undefined) {
        s = h.s, v = h.v, h = h.h;
    }

    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0:
            r = v, g = t, b = p;
            break;
        case 1:
            r = q, g = v, b = p;
            break;
        case 2:
            r = p, g = v, b = t;
            break;
        case 3:
            r = p, g = q, b = v;
            break;
        case 4:
            r = t, g = p, b = v;
            break;
        case 5:
            r = v, g = p, b = q;
            break;
    }

    return {
        r: Math.floor(r * 255),
        g: Math.floor(g * 255),
        b: Math.floor(b * 255)
    };
}

function rgbToHsv(r, g, b) {
    if (r && g === undefined && b === undefined) {
        g = r.g, b = r.b, r = r.r;
    }

    r = r / 255, g = g / 255, b = b / 255;
    var max = Math.max(r, g, b), min = Math.min(r, g, b);
    var h, s, v = max;

    var d = max - min;
    s = max == 0 ? 0 : d / max;

    if (max == min) {
        h = 0; // achromatic
    } else {
        switch (max) {
            case r:
                h = (g - b) / d + (g < b ? 6 : 0);
                break;
            case g:
                h = (b - r) / d + 2;
                break;
            case b:
                h = (r - g) / d + 4;
                break;
        }
        h /= 6;
    }

    return {h: h, s: s, v: v};
}

function hexToRgb(hex) {
    var dec = parseInt(hex.charAt(0) == '#' ? hex.slice(1) : hex, 16);
    return {r: dec >> 16, g: dec >> 8 & 255, b: dec & 255};
}

function rgbToHex(color) {
    var componentToHex = function (c) {
        var hex = c.toString(16);
        return hex.length === 1 ? "0" + hex : hex;
    };

    return "#" + componentToHex(color.r) + componentToHex(color.g) + componentToHex(color.b);
}
