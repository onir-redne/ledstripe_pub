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
 * SVG draggable found on: https://github.com/petercollingridge/code-for-blog/tree/master/svg-interaction
 */ 

var updateTimer = null;
$(function(){
    $( "[data-role='header'], [data-role='footer']" ).toolbar();
});


window.onload = function() {

    navigationHelpers.init();
    powerManagement.init();
    stripeState.init();
    colorTransitionEditor.init('svg_trans_editor', 0, 0, 200, 300, 60000, 8);

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
        this.trans_edit_dialog_OK = $('#color_peeker_dialog_OK');
        
        this.loading_overlay = $('#loading_overlay');
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

    ShowColorPickerDialog : function(src, ok_handler, set_name, message, color_name, color) {
        this.color_peeker_dialog_OK.unbind('click');
        if(message) {
            this.color_peeker_dialog_text.text(message);
        } else {
            this.color_peeker_dialog_text.text("");
        }
        if(set_name) {
            this.color_peeker_dialog_name.show();
            this.color_peeker_dialog_name.val(color_name)
        } else {
            this.color_peeker_dialog_name.hide();
        }
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
        dialog_color_picker.setColorByHex('#ffffff');
        dialog_color_picker.updateView(true);

        // bind handler
        this.color_peeker_dialog_OK.on('click', ok_handler);        
        $.mobile.changePage('#color_peeker_dialog');
    },

    ShowTransEditDialog : function(src, ok_handler, set_name, message, set_values_array) {
        this.trans_edit_dialog_OK.unbind('click');
        if(message) {
            this.trans_edit_dialog_text.text(message);
        } else {
            this.trans_edit_dialog_text.text("");
        }
        if(set_name) {
            this.trans_edit_dialog_name.show();
        } else {
            this.trans_edit_dialog_name.hide();
        }

        if(set_values_array) {
            // load array
        } else {
            // empty set
        }
        
        
        // bind handler
        this.trans_edit_dialog_OK.on('click', ok_handler);        
        // redraw
        colorTransitionEditor.setupTransitions([
            {start: {r: 0, g: 0, b: 0}, stop: {r: 190, g: 220, b: 200}, time: 56000, name: 'stripe e'}, 
            {start: {r: 180, g: 0, b: 0}, stop: {r: 50, g: 80, b: 200}, time: 10000, name: 'stripe a'}, 
            {start: {r: 50, g: 80, b: 200}, stop: {r: 0, g: 180, b: 50}, time: 5000, name: 'stripe b'},
            {start: {r: 0, g: 180, b: 50}, stop: {r: 50, g: 200, b: 120}, time: 1500, name: 'stripe c'},
            {start: {r: 59, g: 90, b: 0}, stop: {r: 190, g: 220, b: 10}, time: 41000, name: 'stripe f'}, 
        ]);
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
    selectedElement: false,
    offset: null, 
    transform: null,
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


        this.svg.addEventListener('mousedown', this.startDrag);
        this.svg.addEventListener('mousemove', this.drag);
        this.svg.addEventListener('mouseup', this.endDrag);
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
            colorTransitionEditor.selectedElement = evt.target;
            
            colorTransitionEditor.selectedElement.remove();
            colorTransitionEditor.svg.appendChild(colorTransitionEditor.selectedElement);

            colorTransitionEditor.selectedElement.setAttribute('style', 'filter:url(#dropshadow)');

            colorTransitionEditor.offset = colorTransitionEditor.getMousePosition(evt);
            // Make sure the first transform on the element is a translate transform
            var transforms = colorTransitionEditor.selectedElement.transform.baseVal;

            if (transforms.length === 0 || transforms.getItem(0).type !== SVGTransform.SVG_TRANSFORM_TRANSLATE) {
                // Create an transform that translates by (0, 0)
                var translate = colorTransitionEditor.svg.createSVGTransform();
                translate.setTranslate(0, 0);
                colorTransitionEditor.selectedElement.transform.baseVal.insertItemBefore(translate, 0);
            }
            // Get initial translation
            colorTransitionEditor.transform = transforms.getItem(0);
            colorTransitionEditor.offset.x -= colorTransitionEditor.transform.matrix.e;
            colorTransitionEditor.offset.y -= colorTransitionEditor.transform.matrix.f;

        } else if ((evt.target == colorTransitionEditor.svg_add_button || evt.target.parentNode == colorTransitionEditor.svg_add_button) 
        && (evt.type == 'touchstart')
        && (colorTransitionEditor.transitions.length < colorTransitionEditor.max_transitions)) {
            // add new default transition
            colorTransitionEditor.appdendTransition({start: {r: 200, g: 200, b: 200}, stop: {r: 155, g: 155, b: 155}, time: 10000, name: ''});
            colorTransitionEditor.refresh();
        }
    },

    drag : function(evt) {
        if (colorTransitionEditor.selectedElement) {
          evt.preventDefault();
          var coord = colorTransitionEditor.getMousePosition(evt);
          colorTransitionEditor.transform.setTranslate(coord.x - colorTransitionEditor.offset.x, coord.y - colorTransitionEditor.offset.y);
        }
    },

    endDrag : function(evt) {
        if (colorTransitionEditor.selectedElement) {
            colorTransitionEditor.selectedElement.setAttribute('style', '');
            var i = 0
            if(colorTransitionEditor.transform) {
                // find related transition element
                var transition_index = null;
                for(i = 0; i < colorTransitionEditor.transitions.length; i++) {
                    if(colorTransitionEditor.transitions[i].svg.obj == colorTransitionEditor.selectedElement) {
                        transition_index = i;
                        break;
                    }
                }

                // if moved far to left remove from array
                if(transition_index !== null && colorTransitionEditor.transform.matrix.e > colorTransitionEditor.tbar_width) {
                    colorTransitionEditor.selectedElement.remove();
                    colorTransitionEditor.transitions.splice(transition_index, 1);
                    
                // if moved up or down further then next or previous swap'em
                } else if(transition_index !== null) { // move down
                    if(colorTransitionEditor.transform.matrix.f >= colorTransitionEditor.transitions[transition_index].position.height / 2 && transition_index < colorTransitionEditor.transitions.length - 1) {
                        var rmoved = colorTransitionEditor.transitions.splice(transition_index, 1);
                        colorTransitionEditor.transitions.splice(transition_index + 1, 0, rmoved[0]);
                    // move up
                    } else if(colorTransitionEditor.transform.matrix.f * -1 >= colorTransitionEditor.transitions[transition_index].position.height / 2  && transition_index > 0) {
                        var rmoved = colorTransitionEditor.transitions.splice(transition_index, 1);
                        colorTransitionEditor.transitions.splice(transition_index - 1, 0, rmoved[0]);
                    }
                }

                // finally start animating back
                colorTransitionEditor.recalcPositions();
                colorTransitionEditor.refresh();
                colorTransitionEditor.transform.setTranslate(0, 0);
                colorTransitionEditor.selectedElement = false;
            }
        }
    },

    /**
     * Setup new transition edit
     * @param {Array} trans_array input transitions array: {start: {r: Number, g: Number, b: Number}, stop: {r: Number, g: Number, b: Number}, time: Number, name: String}
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
                start_bar: null,    // top bar 
                stop_bar: null,    // bottom bar
                info_box: null,     // box with informations (time)
                fine_tune: null,    // dragabble fine tunne (for precise time setting)
            },
            handlers: {
                svgh_onmouse_down: null,
                svgh_onmouse_up: null,
                svgh_onmouse_move: null,
            },
            position: {
                x: 0,
                y: 0,
                width: 0,
                height: 0,
            },
            current_start_y: 0,
            current_end_y: 0,
            int_name: 'svg_tredt_gradient_' + trans.name.replace(/\s+/g, ''),
            //name: trans.name // ignore name and generate one
            int_name: String(this.date_obj.getMilliseconds()) + ' ' + String(this.elements_couner),
        };
        this.elements_couner++;
        
        // chacek wether to disable add button

        this.transitions.push(int_trans);
        this.recalcPositions();     // recalculate positions and dimensions
        this.makeSvgTrnasition(int_trans);
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
    makeSvgTrnasition : function(int_trans) {
        // svg gradient
        if(int_trans.svg.gradient == null) {
            int_trans.svg.gradient = document.createElementNS('http://www.w3.org/2000/svg', 'linearGradient');
            int_trans.svg.gradient.setAttributeNS(null, 'id', 'svg_gradient_' + int_trans.int_name);
            int_trans.svg.gradient.setAttributeNS(null, 'gradientTransform', 'rotate(90)');
            int_trans.svg.gradient_stop1 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
            int_trans.svg.gradient_stop2 = document.createElementNS('http://www.w3.org/2000/svg', 'stop');
            
            // append ranges to gradient
            int_trans.svg.gradient.appendChild(int_trans.svg.gradient_stop1);
            int_trans.svg.gradient.appendChild(int_trans.svg.gradient_stop2);

            this.refreshGradient(int_trans);
            
            // add gradient to defs
            this.svg_defs.appendChild(int_trans.svg.gradient);
        } 

        // main gradient stripe
        if(int_trans.svg.obj == null) {

            int_trans.svg.obj = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
            int_trans.svg.obj.setAttributeNS(null, 'width', int_trans.position.width);
            int_trans.svg.obj.setAttributeNS(null, 'height',  int_trans.position.height);    // will be recalculated later
            int_trans.svg.obj.setAttributeNS(null, 'x', int_trans.position.x);
            int_trans.svg.obj.setAttributeNS(null, 'y', int_trans.position.y);
            int_trans.svg.obj.setAttributeNS(null, 'rx', '5');
            int_trans.svg.obj.setAttributeNS(null, 'fill', "url('#svg_gradient_" + int_trans.int_name + "')");
            int_trans.svg.obj.setAttributeNS(null, 'class', "draggable");

            this.refreshStripe(int_trans);

            this.svg.appendChild(int_trans.svg.obj);
        }

        // terminator 1
        if(int_trans.svg.start_bar == null) {
            int_trans.svg.start_bar = document.createElementNS('http://www.w3.org/2000/svg', 'path');

            this.refreshStop(int_trans);

            this.svg.appendChild(int_trans.svg.start_bar);
        }
    },

    refresh : function() {
        for (var i = 0; i < this.transitions.length; i++) {
            this.refreshGradient(this.transitions[i]);
            this.refreshStop(this.transitions[i]);
            this.refreshStripe(this.transitions[i]);
        }
    },

    refreshStop : function(obj) {
        var p1_x = obj.position.x + obj.position.width + 2;
        var p1_y = obj.position.y + obj.position.height / 2;
        var p2_x = obj.position.x + obj.position.width + 8;
        var p2_y = obj.position.y + obj.position.height / 2;
        //var p3_x = obj.position.x + obj.position.width + 20;
        //var p3_y = obj.position.y + obj.position.width + 15;
        
        //obj.svg.start_bar.setAttributeNS(null, 'd', 'M ' + p1_x + ' ' + p1_y + ' Q ' + p2_x + ' ' + p2_y + ', ' + p3_x + ' ' + p3_y + ' T ' + p3_x + ' ' + p3_y);
        obj.svg.start_bar.setAttributeNS(null, 'd', 'M ' + p1_x + ' ' + p1_y + ' L ' + p2_x + ' ' + p2_y );
        obj.svg.start_bar.setAttributeNS(null, 'fill', 'transparent');
        obj.svg.start_bar.setAttributeNS(null, 'stroke', 'black');
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
        if(this.value == 'on') {
            $.ajax({'url' : '/ajax/poweron'});
        } else {
            $.ajax({'url' : '/ajax/poweroff'});
        }
    },

    ajaxGetLestripesState : function() {
        $.getJSON('/ajax/getstripesstate', function(jsondata) {
            
            if(!this.poweroff_slider_user_int) {
                this.poweroff_slider.val((jsondata.timer / 60).toFixed(1));
                this.poweroff_slider.slider( "refresh" );
            }
            if(jsondata.power == 1) {
                if($("#power_sw option:selected").val() != 'on')
                    this.power_sw.val("on").change();
            } else {
                if($("#power_sw option:selected").val() != 'off')
                    this.power_sw.val("off").change();
            }
            
            if(jsondata.sync != 0 && !this.sync_switch) {
                $("#falsh-sync-button").css("border-color", "brown");
                $("#falsh-sync-button").css("border-width", "3px");
                this.sync_switch = true;
            } else if(jsondata.sync == 0 && this.sync_switch) {
                $("#falsh-sync-button").css("border-color", "grey");
                $("#falsh-sync-button").css("border-width", "1px");
                this.sync_switch = false;
            }
         });
    },

    poweroffTimerOnSlideStart : function(event, ui) {
        this.poweroff_slider_user_int = true;   // prevent updating when user interacts
    },

    poweroffTimerOnSlideEnd : function(event, ui) {
        //event.preventDefault();
        //event.stopPropagation();
        powerManagement.ajaxSetLedstripesPowerTimer(this.poweroff_slider.val());
        this.poweroff_slider_user_int = false;
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
