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
 * SVG draggable examples found on: https://github.com/petercollingridge/code-for-blog/tree/master/svg-interaction
 */ 

var updateTimer = null;
$(function(){
    $( "[data-role='header'], [data-role='footer']" ).toolbar();
});


window.onload = function() {

    navigationHelpers.init();
    powerManagement.init();
    stripeState.init();
    //window.innerWidth & window.innerHeight
    colorTransitionEditor.init('svg_trans_editor', 0, 0, window.innerWidth /2, window.innerHeight /2.6, 60000, 8);

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
                if (!self.selectedInput) self.selectedInput = document.getElementById('color_picker_color');
                self.selectedInput.style.background = "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))";
                self.selectedInput.value = self.getCurColorHex();
                stripeState.setColorPeek(rgbCurrent.r, rgbCurrent.g, rgbCurrent.b);
            }
        }
    });

     // addition user methods \ variables 
     dialog_color_picker.editInput = function(target) {
    
        if (dialog_color_picker.selectedInput) {
            dialog_color_picker.selectedInput.classList.remove('selected');
        }

        if (target) 
        dialog_color_picker.selectedInput = target;
        if (!dialog_color_picker.selectedInput) 
            return false;
        
        dialog_color_picker.selectedInput.classList.add('selected');
        dialog_color_picker.setColor(dialog_color_picker.selectedInput.value);
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
        this.confirmation_dialog_text =  $('#confirmation_dialog_text');
        this.confirmation_dialog_OK = $('#confirmation_dialog_OK');
        
        this.saved_colors_list = $('#saved_colors_list');
        this.color_peeker_dialog_text = $('#color_peeker_dialog_text');
        this.color_peeker_dialog_name = $('#color_peeker_dialog_name');
        this.color_peeker_dialog = $('#color_peeker_dialog');
        this.color_peeker_dialog_OK = $('#color_peeker_dialog_OK');

        this.transition_sets_list = $('#transition_sets_list');
        this.trans_edit_dialog_text = $('#trans_edit_dialog_text');
        this.trans_edit_dialog_name = $('#trans_edit_dialog_name');
        this.trans_edit_dialog = $('#trans_edit_dialog');
        this.trans_edit_dialog_OK = $('#trans_edit_dialog_OK');
        
        this.loading_overlay = $('#loading_overlay');

        this.selected_color1 = $('#color_picker_color');
        this.selected_color2 = $('#color_picker_color_second');
    },

    ShowLoadingOverlay : function() {
        //var loading_obj = $('#loading_overlay');
        var theme =  this.loading_overlay.jqmData( "theme" ) || $.mobile.loader.prototype.options.theme;
        var msgText =  this.loading_overlay.jqmData( "msgtext" ) || $.mobile.loader.prototype.options.text;
        var textVisible =  this.loading_overlay.jqmData( "textvisible" ) || $.mobile.loader.prototype.options.textVisible;
        var textonly = !! this.loading_overlay.jqmData( "textonly" );
        var html =  this.loading_overlay.jqmData( "html" ) || "";
        $.mobile.loading( "show", {text: msgText, textVisible: textVisible, theme: theme, textonly: textonly, html: html});
    },

    HideLoadingOverlay : function() {
        $.mobile.loading( "hide" );
    },

    PageChangeHandler : function(event, ui) {
        if(ui.toPage[0].id == 'saved-colors') {
            this.ShowLoadingOverlay();
            savedColors.load(this.saved_colors_list);
            this.HideLoadingOverlay();
        }else if (ui.toPage[0].id == 'transition-sets') {
            this.ShowLoadingOverlay();
            savedTransitions.load(this.transition_sets_list);
            this.HideLoadingOverlay();
        }
    },

    ShowConfirmationDialog : function(src, ok_handler, message) {
        this.confirmation_dialog_OK.unbind('click');
        if(message) {
            this.confirmation_dialog_text.text(message);
        } else {
            this.confirmation_dialog_text.text("Arey you sure?");
        }
        $.mobile.changePage('#confirmation_dialog');
        this.confirmation_dialog_OK.on('click', ok_handler);  
    },

    ShowColorPickerDialog : function(src, ok_handler, set_name, message, color_name, color1, color2) {
        this.color_peeker_dialog_OK.unbind('click');
        if(message) {
            this.color_peeker_dialog_text.text(message);
            this.color_peeker_dialog_text.show();
        } else {
            this.color_peeker_dialog_text.text("");
            this.color_peeker_dialog_text.hide();
        }
        if(set_name) {
            this.color_peeker_dialog_name.show();
            this.color_peeker_dialog_name.val(color_name)
        } else {
            this.color_peeker_dialog_name.hide();
        }
        if(color2) { 
            this.selected_color2.val(rgbToHex(color2));
            var rgbCurrent = color2;
            var hsvCurrent = rgbToHsv(color2);
            var baseColorRgb = hsvToRgb(hsvCurrent.h, hsvCurrent.s, 1.0);
            this.selected_color2.css("background", "-webkit-gradient(linear, left top, left bottom, from(rgba("+rgbCurrent.r+","+rgbCurrent.g+","+rgbCurrent.b+",1)), to(rgba("+baseColorRgb.r+","+baseColorRgb.g+","+baseColorRgb.b+",1)))"); 
            this.selected_color2.css("display","")

        } else {
            this.selected_color2.css("display","none")
        }
        // set selected input to 1
        dialog_color_picker.selectedInput = document.getElementById('color_picker_color');
        //dialog_color_picker.rgb = {r: color.r, g: color.g, b: color.b};
        //resize picker to dialog
        var dialog_width = this.color_peeker_dialog.width();
        // setup main color picker
        dialog_color_picker.getWheel().width = 30;
        dialog_color_picker.getSvFigCursor().radius = 30;
        dialog_color_picker.getSvFig().radius = 20;
        dialog_color_picker.getWheelCursor().height = 20;
        dialog_color_picker.getWheelCursor().lineWeight = 2;
        dialog_color_picker.resize(dialog_width - dialog_width / 3);
        // update to applay size options
        dialog_color_picker.setColorByHex(rgbToHex(color1));
        dialog_color_picker.updateView(true);

        // bind handler
        this.color_peeker_dialog_OK.on('click', ok_handler);        
        $.mobile.changePage('#color_peeker_dialog');
    },

    getPeekColor1Hex : function() {
        return this.selected_color1.val();
    },

    getPeekColor2Hex : function() {
        return this.selected_color2.val();
    },

    ShowTransEditDialog : function(src, ok_handler, set_name, message, set_values_array) {
        this.trans_edit_dialog_OK.unbind('click');
        if(message) {
            this.trans_edit_dialog_text.text(message);
            this.trans_edit_dialog_text.show();
        } else {
            this.trans_edit_dialog_text.text('');
            this.trans_edit_dialog_text.hide();
        }
        if(set_name) {
            this.trans_edit_dialog_name.show();
        } else {
            this.trans_edit_dialog_name.hide();
        }

        if(set_values_array) {
            // load array
            colorTransitionEditor.setupTransitions(set_values_array);
        } else {
            // empty set with one sample transition
            colorTransitionEditor.setupTransitions([
                {start: {r: 0, g: 0, b: 0}, stop: {r: 130, g: 220, b: 180}, time: 10000, name: 'sample'}
            ]);
        }
        
        // bind handler
        this.trans_edit_dialog_OK.on('click', ok_handler);        
        // redraw
        $.mobile.changePage('#trans_edit_dialog');
    }
}

var savedTransitions = {
    load : function(target) {
        $.getJSON('/ajax/savedtrans_get', function(response) {
            var new_html = '';
            target.html('');
            
            for (var i = 0; i < response.sets.length; i++) {
                var tmp_set = response.sets[i];
                var tmp_total_time = 0;
                var cid = tmp_set.id;
                var name = tmp_set.name;

                for (var j = 0; j < tmp_set.trans.length; j++) {
                    tmp_total_time += tmp_set_trans[j].time;
                }

                var tmp_style_gradient = 'linear-gradient(90deg, ';
                var last_percents = 0;
                for (var j = 0; j < tmp_set.trans.length; j++) {
                    var percents = 100 * tmp_set_trans[j].time / tmp_total_time;
                    
                    tmp_style_gradient += 'rgba(' + tmp_set_trans.r1 + ',' + tmp_set_trans.g1 + ',' + tmp_set_trans.b1 +  ',1) ' + last_percents + '%,' + 'rgba(' + tmp_set_trans.r2 + ',' + tmp_set_trans.g2 + ',' + tmp_set_trans.b2 +  ',1) ' + last_percents + '%';
                    last_percents = percents;

                    if(j < tmp_set.trans.length - 1)
                        tmp_style_gradient += ',';
                }
                tmp_style_gradient += ');'
                
                new_html += `
                <div data-type="horizontal" class="ui-grid-c ui-shadow ui-corner-all saved-color-element" style="background: ` + tmp_style_gradient + `">
                    <div class="ui-block-a">
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-edit ui-btn-icon-notext" onclick="navigationHelpers.ShowColorPickerDialog(event.target, function() { savedColors.set(` + cid + `); }, true, 'edit color' ,'` + name + `',{ r:` + c.r + `, g:` + c.g + `, b:` + c.b + `}, null);"></a>
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
                new_html += `<button class="ui-btn ui-icon-plus ui-corner-all ui-btn-icon-left" onclick="navigationHelpers.ShowTransEditDialog(event.target, function() { savedTransitions.add(); }, true, null ,'', null);">Add transition</button>`;
            }
            target.html(new_html);
        });
    },

    add : function() {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#trans_edit_dialog_name').val();

        // get data from transition editor prepare get
        var get_data = {
            name: name,
        };

        for(var i = 0; i < colorTransitionEditor.transitions.length; i++) {
            get_data['r' + i + '1'] = colorTransitionEditor.transitions[i].start.r;
            get_data['g' + i + '1'] = colorTransitionEditor.transitions[i].start.g;
            get_data['b' + i + '1'] = colorTransitionEditor.transitions[i].start.b;
            get_data['r' + i + '2'] = colorTransitionEditor.transitions[i].stop.r;
            get_data['g' + i + '2'] = colorTransitionEditor.transitions[i].stop.g;
            get_data['b' + i + '2'] = colorTransitionEditor.transitions[i].stop.b;
            get_data['t' + i] = colorTransitionEditor.transitions[i].time;
        }

        $.getJSON('/ajax/savedtrans_set', get_data,
            function(response) {
                // reload trans sets on success
                this.load();
            });
    },

    set : function(tid)  {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#trans_edit_dialog_name').val();

        // get data from transition editor prepare get
        var get_data = {
            name: name,
        };

        if(tid != null) 
            get_data.push('id', tid);

        for(var i = 0; i < colorTransitionEditor.transitions.length; i++) {
            get_data.push('r' + i + '1', colorTransitionEditor.transitions[i].start.r);
            get_data.push('g' + i + '1', colorTransitionEditor.transitions[i].start.g);
            get_data.push('b' + i + '1', colorTransitionEditor.transitions[i].start.b);
            get_data.push('r' + i + '2', colorTransitionEditor.transitions[i].stop.r);
            get_data.push('g' + i + '2', colorTransitionEditor.transitions[i].stop.g);
            get_data.push('b' + i + '2', colorTransitionEditor.transitions[i].stop.b);
            get_data.push('t' + i, colorTransitionEditor.transitions[i].time);
        }

        $.getJSON('/ajax/savedtrans_set', get_data,
            function(response) {
                // reload trans sets on success
                this.load();
            });
    },

    delete : function(tid)  {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#color_peeker_dialog_name').val();
        $.getJSON('/ajax/savedtrans_del', {'id' : cid},
            function(response) {
                // reload colors on success
                this.load();
            });
    },

    sync : function() {
        $.getJSON('/ajax/savedtrans_sync', {},
        function(response) {
            // reload colors on success
            this.load();
        });
    }
}


/**
 * Saved colors
 */
var savedColors = {

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
                        <a href="#" class="ui-input-btn ui-btn ui-corner-all ui-icon-edit ui-btn-icon-notext" onclick="navigationHelpers.ShowColorPickerDialog(event.target, function() { savedColors.set(` + cid + `); }, true, 'edit color' ,'` + name + `',{ r:` + c.r + `, g:` + c.g + `, b:` + c.b + `}, null);"></a>
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
                this.load();
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
                this.load();
            });
    },

    delete : function(cid)  {
        var rgb_color = dialog_color_picker.getCurColorRgb();
        var name = $('#color_peeker_dialog_name').val();
        $.getJSON('/ajax/savedcolors_del', {'id' : cid},
            function(response) {
                // reload colors on success
                this.load();
            });
    },

    sync : function() {
        $.getJSON('/ajax/savedcolors_sync', {},
        function(response) {
            // reload colors on success
            this.load();
        });
    }
}


/**
 * Color transition Editor
 */
var colorTransitionEditor = {
    svg: null,
    svg_defs: null,
    svg_ruller: null,
   //svg_shadow_filter: null,
    svg_add_button: null,
    transitions: [],
    max_transitions: 8,
    max_time: 60000,
    total_time: 0,
    page_width: 100,
    page_height: 400,
    page_x: 0,
    page_y: 0,
    tbar_height: 400,
    tbar_width: 60,
    tbar_x: 50,
    tbar_y: 5,
    tsep_heigth: 10,
    tsep_width_mod: 1,
    tstops_hieght : 20,
    time_scale: 1.0,
    ruller_width: 5,
    ruller_resolution: 1.0,
    drag_selectedElement: false,
    drag_offset: null, 
    drag_transform: null,
    drag_time_diff: 0,
    timeadj_selectedElement: false,
    timeadj_transitionObj: false,
    timeadj_start_pos: 0,
    timeadj_precission_modifier: 1,
    timeadj_start_time: 0,
    date_obj: null,
    elements_couner: 0,
    
    init: function(obj_id, x, y, width, heigth, max_time, max_trans) {

        this.page_width = width;
        this.page_height = heigth
        this.page_x = x;
        this.page_y = y;
        this.max_time = max_time;
        this.max_transitions = max_trans;
        this.tbar_width = this.page_width * 0.5;
        this.tbar_height = this.page_height - 10;
        this.tbar_x = this.page_width / 2 - this.tbar_width / 2;
        this.date_obj = new Date();


        this.svg = document.getElementById(obj_id);
        this.svg.setAttributeNS(null, 'viewBox', '0 0 200 ' + String(this.tbar_height + 60));
        this.svg_defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
        this.svg.appendChild(this.svg_defs);
        this.svg_add_button = document.getElementById('svg_append_button');


        //this.svg.addEventListener('mousedown', this.startDrag);
        this.svg.addEventListener('mousemove', this.drag);
        //this.svg.addEventListener('mouseup', this.endDrag);
        this.svg.addEventListener('mouseleave', this.endDrag);
        this.svg.addEventListener('touchstart', this.startDrag);
        this.svg.addEventListener('touchmove', this.drag);
        this.svg.addEventListener('touchend', this.endDrag);
        this.svg.addEventListener('touchleave', this.endDrag);
        this.svg.addEventListener('touchcancel', this.endDrag);
    },

    getMousePosition : function(evt) {
        var CTM = colorTransitionEditor.svg.getScreenCTM();
        if (evt.touches) { evt = evt.touches[0]; }
        return {
          x: (evt.clientX - CTM.e) / CTM.a,
          y: (evt.clientY - CTM.f) / CTM.d
        };
    },
    
    startDrag : function(evt) {
        if (evt.target.classList.contains('draggable')) {
            colorTransitionEditor.drag_selectedElement = evt.target;
            
            colorTransitionEditor.drag_selectedElement.remove();
            colorTransitionEditor.svg.appendChild(colorTransitionEditor.drag_selectedElement);
            colorTransitionEditor.drag_selectedElement.setAttribute('style', 'filter:url(#dropshadow)');
            colorTransitionEditor.drag_offset = colorTransitionEditor.getMousePosition(evt);
            // Make sure the first transform on the element is a translate transform
            var transforms = colorTransitionEditor.drag_selectedElement.transform.baseVal;

            if (transforms.length === 0 || transforms.getItem(0).type !== SVGTransform.SVG_TRANSFORM_TRANSLATE) {
                // Create an transform that translates by (0, 0)
                var translate = colorTransitionEditor.svg.createSVGTransform();
                translate.setTranslate(0, 0);
                colorTransitionEditor.drag_selectedElement.transform.baseVal.insertItemBefore(translate, 0);
            }
            // Get initial translation
            colorTransitionEditor.drag_transform = transforms.getItem(0);
            colorTransitionEditor.drag_offset.x -= colorTransitionEditor.drag_transform.matrix.e;
            colorTransitionEditor.drag_offset.y -= colorTransitionEditor.drag_transform.matrix.f;

            // detect (clicks)
            if(evt.type == 'touchstart') {
                colorTransitionEditor.drag_time_diff = new Date().getTime();
            }

            //add-transition
        } else if (evt.target.classList.contains('add-transition')) { 
            if ((evt.type == 'touchstart') && (colorTransitionEditor.transitions.length < colorTransitionEditor.max_transitions)) {
            // add new transition with start color set to preceeding end color
                var start = {r: 200, g: 200, b: 200};
                var stop = {r: 200, g: 200, b: 200};
                if(colorTransitionEditor.transitions.length > 0) {
                    start = colorTransitionEditor.transitions[colorTransitionEditor.transitions.length - 1].stop;
                    stop = start;
                }
                //var tr = colorTransitionEditor.getTransitionForSvgObj(colorTransitionEditor.drag_selectedElement);
                colorTransitionEditor.appdendTransition({start: start, stop: stop, time: 10000, name: ''});
                colorTransitionEditor.refresh();
            }
        } else if (evt.target.classList.contains('time-edit')) {

            colorTransitionEditor.timeadj_selectedElement = evt.target;
            var tran = colorTransitionEditor.getTransitionForSvgTimeEedit(colorTransitionEditor.timeadj_selectedElement);
            colorTransitionEditor.timeadj_transitionObj = tran.obj;
            colorTransitionEditor.timeadj_transitionObj.svg.obj.remove();
            colorTransitionEditor.svg.appendChild(colorTransitionEditor.timeadj_transitionObj.svg.obj);
            colorTransitionEditor.timeadj_transitionObj.svg.obj.setAttribute('style', 'filter:url(#dropshadow)');
            colorTransitionEditor.timeadj_start_pos = colorTransitionEditor.getMousePosition(evt);
            colorTransitionEditor.timeadj_precission_angle = 1.0; // precision will be updated on move
            colorTransitionEditor.timeadj_start_time = colorTransitionEditor.timeadj_transitionObj.time; // store start time
            
            //create time edit vector (path + circle)
            colorTransitionEditor.makeTimeAdjustVector(colorTransitionEditor.timeadj_transitionObj);
        }
    },

    drag : function(evt) {
        if (colorTransitionEditor.drag_selectedElement) {
          evt.preventDefault();
          var coord = colorTransitionEditor.getMousePosition(evt);
          colorTransitionEditor.drag_transform.setTranslate(coord.x - colorTransitionEditor.drag_offset.x, coord.y - colorTransitionEditor.drag_offset.y);
        } else if(colorTransitionEditor.timeadj_selectedElement) {
            evt.preventDefault();
            var coord = colorTransitionEditor.getMousePosition(evt);
            var y_diff = Math.abs(coord.y - colorTransitionEditor.timeadj_start_pos.y);
            var x_diff = Math.abs(coord.x - colorTransitionEditor.timeadj_start_pos.x);
            colorTransitionEditor.timeadj_precission_modifier = 1000 / Math.floor((x_diff / 25) + 1);


            if(coord.y > 0 && coord.x > 0) {
                if(colorTransitionEditor.timeadj_start_pos.y > coord.y) y_diff *= -1;
                colorTransitionEditor.timeadj_transitionObj.time = colorTransitionEditor.timeadj_start_time + Math.floor(y_diff * colorTransitionEditor.timeadj_precission_modifier);
                if(colorTransitionEditor.timeadj_transitionObj.time < 100) colorTransitionEditor.timeadj_transitionObj.time = 100;
                else if(colorTransitionEditor.timeadj_transitionObj.time > colorTransitionEditor.max_time) colorTransitionEditor.timeadj_transitionObj.time = colorTransitionEditor.max_time;
            }

            colorTransitionEditor.recalcPositions();
            colorTransitionEditor.refresh();
            colorTransitionEditor.refreshTimeAdjustVector(colorTransitionEditor.timeadj_transitionObj, coord.x, coord.y);
        }
    },

    endDrag : function(evt) {
        if (colorTransitionEditor.drag_selectedElement) {
            colorTransitionEditor.drag_selectedElement.setAttribute('style', '');
            var i = 0
            if(colorTransitionEditor.drag_transform) {
                // find related transition element
                var tr = colorTransitionEditor.getTransitionForSvgObj(colorTransitionEditor.drag_selectedElement);

                // if moved far to left or right remove from array
                if(tr.index !== null && (colorTransitionEditor.drag_transform.matrix.e > colorTransitionEditor.tbar_width || colorTransitionEditor.drag_transform.matrix.e * -1 > colorTransitionEditor.tbar_width)) {
                    colorTransitionEditor.removeSvgObjects(tr.index);
                    colorTransitionEditor.drag_selectedElement.remove();
                    colorTransitionEditor.transitions.splice(tr.index, 1);
                    
                // if moved up or down further then next or previous swap'em
                } else if(tr.index !== null) { // move down
                    if(colorTransitionEditor.drag_transform.matrix.f >= tr.obj.position.height / 2 && tr.index < colorTransitionEditor.transitions.length - 1) {
                        var rmoved = colorTransitionEditor.transitions.splice(tr.index, 1);
                        colorTransitionEditor.transitions.splice(tr.index + 1, 0, rmoved[0]);
                    // move up
                    } else if(colorTransitionEditor.drag_transform.matrix.f * -1 >= tr.obj.position.height / 2  && tr.index > 0) {
                        var rmoved = colorTransitionEditor.transitions.splice(tr.index, 1);
                        colorTransitionEditor.transitions.splice(tr.index - 1, 0, rmoved[0]);
                    }else {
                        // if time was short and no significant move - show edit window
                        if(new Date().getTime() - colorTransitionEditor.drag_time_diff < 500) {
                            navigationHelpers.ShowColorPickerDialog(null, function () { 
                                colorTransitionEditor.updateTrnasitionColors(tr.index, hexToRgb(navigationHelpers.getPeekColor1Hex()), hexToRgb(navigationHelpers.getPeekColor2Hex()));
                            }, false, 'Transition', '', tr.obj.start, tr.obj.stop);
                        }
                    }
                } 

                // finally start animating back
                colorTransitionEditor.recalcPositions();
                colorTransitionEditor.refresh();
                colorTransitionEditor.drag_transform.setTranslate(0, 0);
                colorTransitionEditor.drag_selectedElement = false;

            }
        }else if(colorTransitionEditor.timeadj_selectedElement) {
            colorTransitionEditor.timeadj_transitionObj.svg.obj.setAttribute('style', '');
            colorTransitionEditor.timeadj_transitionObj.svg.time_adjust.remove();
            colorTransitionEditor.timeadj_selectedElement = false;
            colorTransitionEditor.timeadj_transitionObj = false;
        }
    },

    getTransitionForSvgObj : function(svg_object) {
        var transition_index = null;
        for(i = 0; i < this.transitions.length; i++) {
            if(this.transitions[i].svg.obj.id === svg_object.id) {
                transition_index = i;
                break;
            }
        }
        return { index: transition_index, obj: this.transitions[i] };
    },

    getTransitionForSvgTimeEedit : function(svg_timeedit) {
        var transition_index = null;
        for(i = 0; i < this.transitions.length; i++) {
            if(this.transitions[i].svg.obj.id === svg_timeedit.id) {
                transition_index = i;
                break;
            }
        }
        return { index: transition_index, obj: this.transitions[i] };
    },

    makeTimeAdjustVector : function(obj_parent) {
        obj_parent
        obj_parent.svg.time_adjust = document.createElementNS('http://www.w3.org/2000/svg', 'path');
        obj_parent.svg.time_adjust.setAttributeNS(null, 'fill', 'transparent');
        obj_parent.svg.time_adjust.setAttributeNS(null, 'stroke', '#606c7073');
        obj_parent.svg.time_adjust.setAttributeNS(null, 'stroke-width', '14.0'); 
        obj_parent.svg.time_adjust.setAttributeNS(null, 'stroke-linecap', 'round'); 
    
        this.svg.appendChild(obj_parent.svg.time_adjust);
        this.refreshTimeAdjustVector(obj_parent, this.timeadj_start_pos.x, this.timeadj_start_pos.y);
    },

    updateTrnasitionColors : function(index, start, stop) {
        this.transitions[index].start = start;
        this.transitions[index].stop = stop;
        // update grdient
        this.refreshGradient(this.transitions[index]);
    },

    /**
     * Setup new transition edit
     */
    setupTransitions : function(trans_array) {
        // remove all transitions
        this.removeTransitions();

        // add new ones
        for(var i = 0; i < trans_array.length; i++) {
            this.appdendTransition(trans_array[i]);
        }

        this.refresh();
    },

    appdendTransition : function(trans) {
        var int_trans = {
            start: trans.start,
            stop: trans.stop,
            time: trans.time,
            svg: {
                g: null,
                obj: null,  //stripe svg object
                gradient: null,     // related gradient (for color updates)
                gradient_stop1: null,
                gradient_stop2: null,
                time_path: null,    // top bar 
                info_box: null,     // box with informations (time)
                info_box_text: null,
                time_adjust: null,    // dragabble fine tunne (for precise time setting)
            },
            position: {
                x: 0,
                y: 0,
                width: 0,
                height: 0,
            },
            info_box_position: {
                x: 0,
                y: 0,
                width: 30,
                height: 16,
            },
            current_start_y: 0,
            current_end_y: 0,
            //name: trans.name // ignore name and generate one
            int_name: String(this.date_obj.getMilliseconds()) + '-' + String(this.elements_couner),
        };
        this.elements_couner++;
        
        // chacek wether to disable add button

        this.transitions.push(int_trans);
        this.recalcPositions();     // recalculate positions and dimensions
        this.makeSvgTrnasition(int_trans, this.transitions.length - 1);
    },

    removeTransitions : function() {
        for(var i = 0; i < this.transitions.length; i++) {
            for (const key in this.transitions[i].svg) {
                if (this.transitions[i].svg.hasOwnProperty(key) && this.transitions[i].svg[key] != null ) {
                    this.transitions[i].svg[key].remove();
                }
            }
        }
        this.transitions = [];
    },

    removeSvgObjects : function(index) {
        if(this.transitions.length <= index)
            return;

        for (const key in this.transitions[index].svg) {
            if (this.transitions[index].svg.hasOwnProperty(key) && this.transitions[index].svg[key] != null ) {
                this.transitions[index].svg[key].remove();
            }
        }
    },

    recalcXY2Time : function(trans_obj, x, y) {

    },

    recalcPositions : function() {
        var y_offset = 0;
        this.total_time = 0;
        this.time_scale = 1.0;
        var sep_height_total = 0;

        // prepare time scale
        for (var i = 0; i < this.transitions.length; i++) {
            this.total_time += this.transitions[i].time;
            //sep_height_total +=(2 * this.tsep_heigth);
        }
        //this.time_scale = (this.tbar_height - (sep_height_total))  / this.total_time;
        this.time_scale = this.tbar_height  / this.total_time;

        for (var i = 0; i < this.transitions.length; i++) {
            this.transitions[i].position.x = this.tbar_x;
            //this.transitions[i].position.y = this.tbar_y + y_offset + this.tsep_heigth;
            this.transitions[i].position.y = this.tbar_y + y_offset;
            this.transitions[i].position.width = this.tbar_width;
            this.transitions[i].position.height = this.transitions[i].time * this.time_scale;
            //y_offset += this.transitions[i].position.height + 2 * this.tsep_heigth;
            y_offset += this.transitions[i].position.height;
        }

        if(this.transitions.length < this.max_transitions) {
            // enable append  button
            this.svg_add_button.setAttributeNS(null, 'fill', '#107baf');
        } else {
            this.svg_add_button.setAttributeNS(null, 'fill', '#cce7e8');
        }
    },

    /**
     * Create transition elements
     * @param {Object} int_trans 
     */
    makeSvgTrnasition : function(int_trans, index) {
        // svg gradient
        if(int_trans.svg.gradient == null) {
            int_trans.svg.gradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
            int_trans.svg.gradient.setAttributeNS(null, 'id', 'svg_gradient-' + int_trans.int_name);
            int_trans.svg.gradient.setAttributeNS(null, 'gradientTransform', 'rotate(90)');
            int_trans.svg.gradient_stop1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
            int_trans.svg.gradient_stop2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
            
            // append ranges to gradient
            int_trans.svg.gradient.appendChild(int_trans.svg.gradient_stop1);
            int_trans.svg.gradient.appendChild(int_trans.svg.gradient_stop2);

            this.svg_defs.appendChild(int_trans.svg.gradient);
            this.refreshGradient(int_trans);
        } 

        // main gradient stripe
        if(int_trans.svg.obj == null) {

            int_trans.svg.obj = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            int_trans.svg.obj.setAttributeNS(null, 'rx', '5');
            int_trans.svg.obj.setAttributeNS(null, 'fill', "url('#svg_gradient-" + int_trans.int_name + "')");
            int_trans.svg.obj.setAttributeNS(null, 'class', "draggable");
            int_trans.svg.obj.setAttributeNS(null, 'id', 'svg_stripe-' + int_trans.int_name);

            this.svg.appendChild(int_trans.svg.obj);
            this.refreshStripe(int_trans);
        }

        // time path and infoobos
        if(int_trans.svg.time_path == null) {
            int_trans.svg.time_path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
            int_trans.svg.time_path.setAttributeNS(null, 'fill', 'transparent');
            int_trans.svg.time_path.setAttributeNS(null, 'stroke', '#5c6063');
            int_trans.svg.time_path.setAttributeNS(null, 'stroke-width', '0.75');
            this.svg.appendChild(int_trans.svg.time_path);
            
            int_trans.svg.info_box = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            int_trans.svg.info_box.setAttributeNS(null, 'rx', '5');
            int_trans.svg.info_box.setAttributeNS(null, 'fill', '#c8ccce');
            int_trans.svg.info_box.setAttributeNS(null, 'class', "time-edit");
            int_trans.svg.info_box.setAttributeNS(null, 'stroke-width', '0');
            int_trans.svg.info_box.setAttributeNS(null, 'stroke', '#5c6063');
            int_trans.svg.info_box.setAttributeNS(null, 'id', 'svg_stripe-' + int_trans.int_name);
            this.svg.appendChild(int_trans.svg.info_box);

            int_trans.svg.info_box_text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            int_trans.svg.info_box.setAttributeNS(null, 'class', "time-edit svg-time-text");
            this.svg.appendChild(int_trans.svg.info_box_text);
            
            this.refreshTimePath(int_trans, index);
        }
    },

    refresh : function() {
        for (var i = 0; i < this.transitions.length; i++) {
            this.refreshGradient(this.transitions[i]);
            this.refreshTimePath(this.transitions[i], i);
            this.refreshStripe(this.transitions[i]);
        }
    },
    
    refreshTimePath : function(obj, index) {
        
        var x_offset = obj.position.x + obj.position.width - 1;
        var x_curve = 8;
        var x_distance = 5;
        var x_infobox = -4;
        var w_infobox = obj.info_box_position.width; //30;
        var h_infobox = obj.info_box_position.height; // 16;
        if(((index + 1) % 2) === 0) {
            x_offset = obj.position.x + 1;
            x_curve = -8;
            x_distance = -5;
            x_infobox = 4 - w_infobox;
        }
        
        var y_curve = 5;
        if(obj.position.height <= 10) {
            y_curve = obj.position.height / 2;
        }
        
        
        var path1 = '';
        path1 += 'M ' + x_offset + ' ' + obj.position.y + ' L ' + (x_offset + x_distance) + ' ' + obj.position.y + ' Q ' +  (x_offset + x_distance + x_curve) + ' ' + obj.position.y + ' , ' + (x_offset + x_distance + x_curve) + ' ' + (obj.position.y +  y_curve);
        path1 += ' L ' + (x_offset + x_distance + x_curve) + ' ' + (obj.position.y + obj.position.height - y_curve);
        path1 += ' Q ' +  (x_offset + x_distance + x_curve) + ' ' + (obj.position.y + obj.position.height) + ' , ' + (x_offset + x_curve) + ' ' + (obj.position.y + obj.position.height) + ' L ' + x_offset + ' ' + (obj.position.y + obj.position.height);
        //path1 += ' M ' + (x_offset + x_distance + x_curve) + ' ' + (obj.position.y + obj.position.height / 2) +  ' L ' +  (x_offset + x_distance + x_curve +  x_infobox) + ' ' + (obj.position.y + obj.position.height / 2);
        
        obj.svg.time_path.setAttributeNS(null, 'd', path1);
        obj.svg.info_box.setAttributeNS(null, 'width', w_infobox);
        obj.svg.info_box.setAttributeNS(null, 'height',  h_infobox);    // will be recalculated later
        obj.svg.info_box.setAttributeNS(null, 'x', (x_offset + x_distance + x_curve + x_infobox));
        obj.svg.info_box.setAttributeNS(null, 'y',  (obj.position.y - (h_infobox / 2) + obj.position.height / 2));
        
        // update
        obj.info_box_position.x = (x_offset + x_distance + x_curve + x_infobox + 6);
        obj.info_box_position.y =  (obj.position.y + 4 + obj.position.height / 2);
        
        obj.svg.info_box_text.setAttributeNS(null, 'x', obj.info_box_position.x);
        obj.svg.info_box_text.setAttributeNS(null, 'y',  obj.info_box_position.y);
        obj.svg.info_box_text.textContent = String((obj.time / 1000).toPrecision(2))
    },

    refreshGradient : function(obj) {
        if(obj.svg.gradient != null) {
            obj.svg.gradient_stop1.setAttributeNS(null, 'offset', '0%');
            obj.svg.gradient_stop1.setAttributeNS(null, 'stop-color', 'rgb('+ obj.start.r +', '+ obj.start.g +', '+ obj.start.b +')');
            obj.svg.gradient_stop2.setAttributeNS(null, 'offset', '100%');
            obj.svg.gradient_stop2.setAttributeNS(null, 'stop-color', 'rgb('+ obj.stop.r +', '+ obj.stop.g +', '+ obj.stop.b +')');
        }
    },

    refreshStripe : function(obj) {
        if(obj.svg.obj != null) {
            obj.svg.obj.setAttributeNS(null, 'width', obj.position.width);
            obj.svg.obj.setAttributeNS(null, 'height',  obj.position.height);    // will be recalculated later
            obj.svg.obj.setAttributeNS(null, 'x', obj.position.x);
            obj.svg.obj.setAttributeNS(null, 'y', obj.position.y);
        }
    },

    refreshTimeAdjustVector : function(obj_parent, x, y) {
        var path1 = '';
        path1 += 'M ' + obj_parent.info_box_position.x + ' ' + obj_parent.info_box_position.y + ' L ' + x + ' ' + y;
        obj_parent.svg.time_adjust.setAttributeNS(null, 'd', path1);
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
        this.poweroff_slider = $('#power_off_timer');
        this.power_sw = $('#power_sw');
        // attach power timer event handler
        this.poweroff_slider.on("slidestart", powerManagement.poweroffTimerOnSlideStart);
        this.poweroff_slider.on("slidestop", powerManagement.poweroffTimerOnSlideEnd);
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
        if(powerManagement.power_sw.val() == 'on') {
            $.ajax({'url' : '/ajax/poweron'});
        } else {
            $.ajax({'url' : '/ajax/poweroff'});
        }
    },

    ajaxGetLestripesState : function() {
        $.getJSON('/ajax/getstripesstate', function(jsondata) {
            
            if(!powerManagement.poweroff_slider_user_int) {
                powerManagement.poweroff_slider.val((jsondata.timer / 60).toFixed(1));
                powerManagement.poweroff_slider.slider( "refresh" );
            }
            if(jsondata.power == 1) {
                if($("#power_sw option:selected").val() != 'on')
                powerManagement.power_sw.val("on").change();
            } else {
                if($("#power_sw option:selected").val() != 'off')
                powerManagement.power_sw.val("off").change();
            }
            
            if(jsondata.sync != 0 && !powerManagement.sync_switch) {
                $("#falsh-sync-button").css("border-color", "brown");
                $("#falsh-sync-button").css("border-width", "3px");
                powerManagement.sync_switch = true;
            } else if(jsondata.sync == 0 && powerManagement.sync_switch) {
                $("#falsh-sync-button").css("border-color", "grey");
                $("#falsh-sync-button").css("border-width", "1px");
                powerManagement.sync_switch = false;
            }
         });
    },

    poweroffTimerOnSlideStart : function(event, ui) {
        powerManagement.poweroff_slider_user_int = true;   // prevent updating when user interacts
    },

    poweroffTimerOnSlideEnd : function(event, ui) {
        //event.preventDefault();
        //event.stopPropagation();
        powerManagement.ajaxSetLedstripesPowerTimer(powerManagement.poweroff_slider.val());
        powerManagement.poweroff_slider_user_int = false;
    },

    poweroffTimerOnChange : function(event, ui) {
        //event.preventDefault();
        //event.stopPropagation();
        //powerManagement.ajaxSetLedstripesPowerTimer(this.poweroff_slider.val());
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
