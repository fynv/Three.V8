/**
 * https://github.com/mrdoob/eventdispatcher.js/
 */

class EventDispatcher {

	addEventListener( type, listener ) {

		if ( this._listeners === undefined ) this._listeners = {};

		const listeners = this._listeners;

		if ( listeners[ type ] === undefined ) {

			listeners[ type ] = [];

		}

		if ( listeners[ type ].indexOf( listener ) === - 1 ) {

			listeners[ type ].push( listener );

		}

	}

	hasEventListener( type, listener ) {

		if ( this._listeners === undefined ) return false;

		const listeners = this._listeners;

		return listeners[ type ] !== undefined && listeners[ type ].indexOf( listener ) !== - 1;

	}

	removeEventListener( type, listener ) {

		if ( this._listeners === undefined ) return;

		const listeners = this._listeners;
		const listenerArray = listeners[ type ];

		if ( listenerArray !== undefined ) {

			const index = listenerArray.indexOf( listener );

			if ( index !== - 1 ) {

				listenerArray.splice( index, 1 );

			}

		}

	}

	dispatchEvent( event ) {
		if ( this._listeners === undefined ) return;

		const listeners = this._listeners;
		const listenerArray = listeners[event.type];		

		if ( listenerArray !== undefined ) {

			event.target = this;

			// Make a copy, in case listeners are removed while iterating.
			const array = listenerArray.slice( 0 );

			for ( let i = 0, l = array.length; i < l; i ++ ) {

				array[ i ].call( this, event );

			}

			event.target = null;

		}

	}

}

class MsgDispatcher extends EventDispatcher
{
    constructor(ws)
    {
        super();
        this.ws = ws;
        this.stage = 0;
        this.key = "";
        ws.onMessage = (msg) => this.onMsg(msg);
    }
    
    send(key, value)
    {
        this.ws.send(key);
        this.ws.send(value);
    }
    
    onMsg(msg)
    {
        if (this.stage == 0)
        {
            this.key = msg;
            this.stage = 1;
        }
        else if (this.stage == 1)
        {
            const event = {
                type: this.key,
                data: msg
            };
            this.dispatchEvent(event);
            this.key = "";
            this.stage = 0;
        }
    }
    
    sendObj(key, obj)
    {
        this.ws.send(key);
        this.ws.send(JSON.stringify(obj));
    }
    
    addEventListenerObj(key, callback)
    {
        this.addEventListener(key, (event)=>{
            let obj = JSON.parse(event.data);
            callback(obj);                        
        });
    }
}

function enter(room_id)
{
    let self = null;
    let users = {};
    let audio_recorder = null;
    let callbacks = {};
    
    //let ws = new WSClient("ws://127.0.0.1:8888");
    let ws = new WSClient("ws://192.168.10.104:8888");
    ws.onOpen = ()=>{
        messenger = new MsgDispatcher(ws);
        
        messenger.send("join_room", room_id);
        
        messenger.addEventListenerObj("identity", (identity)=>{
            self = identity;            
        });
        
        messenger.addEventListenerObj("users", (info_users)=>{
            for (let _username in users)
            {
                const user = users[_username];
                if (!info_users.hasOwnProperty(_username))
                {
                    user.audio_player.dispose();
                    messenger.removeEventListener(_username+"_audio", callbacks[_username]);
                    delete users[_username];
                }
            }
            
            for (let _username in info_users)
            {
                const user = info_users[_username];
                if (_username!=self.username)
                {
                    if (!users.hasOwnProperty(_username))
                    {
                        users[_username] = user;
                        user.audio_player = new OpusPlayer();                        
                        let callback = (event)=>{                            
                            user.audio_player.addPacket(event.data);
                        };                        
                        messenger.addEventListener(_username+"_audio", callback);
                        callbacks[_username] = callback;
                        
                    }
                }
            }
        });
        
        audio_recorder = new OpusRecorder();        
        audio_recorder.callback = (arr) => {
            messenger.send("audio", arr);
        };
        
    };
}

function setupUI()
{
    if (!gamePlayer.hasFont("default"))
    {
        gamePlayer.createFontFromFile("default", "assets/fonts/NotoSansSC-Bold.otf");
    }
    
    ui_area = new UIArea();
    ui_area.setOrigin(200.0, 90.0);
    ui_area.setSize(240.0, 180.0);
    UIManager.add(ui_area);
    
    let panel = new UIPanel();
    panel.setOrigin(0.0, 0.0);
    panel.setSize(240.0, 180.0);
    ui_area.add(panel);    
    {
        let text_msg = new UIText();
        text_msg.text = "输入房间号码：";
        text_msg.block = panel;
        text_msg.setStyle({ "alignmentVertical": 0});
        text_msg.setOrigin(0.0, 30.0);    
        ui_area.add(text_msg); 
        
        let edit = new UILineEdit();
        edit.setOrigin(50.0, 60.0);
        edit.block = panel;
        edit.text = "1602";
        ui_area.add(edit);
        
        let btn = new UIButton();
        btn.setOrigin(70.0, 120.0);
        btn.setSize(90.0, 40.0);
        btn.block = panel;    
        ui_area.add(btn);
        {
            img = imageLoader.loadFile("assets/textures/ok.png");
            btn_img = new UIImage();
            btn_img.setImage(img);
            btn_img.block = btn;
            btn_img.setSize(30,30);
            ui_area.add(btn_img);            
            
            btn_text = new UIText();
            btn_text.text = "确定";
            btn_text.block = btn;
            btn_text.setStyle({ "alignmentHorizontal": 0 });
            btn_text.setOrigin(40.0, 0.0);             
            ui_area.add(btn_text); 
        }
        
         btn.onClick = ()=>
         {
             UIManager.remove(ui_area);
             ui_area = null;
             enter(edit.text);
         };
    }
}

function init(width, height)
{
    renderer = new GLRenderer();
    scene = new Scene();
    
    bg = new HemisphereBackground();   
    bg.setSkyColor(0.318, 0.318, 0.318);
    bg.setGroundColor(0.01, 0.025, 0.025);
    scene.background = bg;

    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    
    setupUI();
}

function render(width, height, size_changed)
{
    if (ui_area)
    {
        ui_area.scale = height / 360.0;
    }
    
    if (size_changed) 
    {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }
    
    renderer.render(scene, camera);
}

setCallback('init', init);
setCallback('render', render);
