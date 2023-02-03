import { EventDispatcher } from "./controls/EventDispatcher.js";

class View extends EventDispatcher {

    get clientWidth() {
        return gamePlayer.width;
    }

    get clientHeight() {
        return gamePlayer.height;
    }

    setPointerCapture() {
        gamePlayer.message("setPointerCapture", "");
    }

    releasePointerCapture() {
        gamePlayer.message("releasePointerCapture", "");
    }
}

const view = new View();

function makeMouseEvent(e, type) {
    let event = {
        type: type,
        pointerType: "mouse",
        pointerId: 0,
        clientX: e.x,
        clientY: e.y,
        deltaY: e.delta,
        button: e.button
    };

    return event;
}

function OnMouseDown(e) {
    let event = makeMouseEvent(e, "pointerdown");
    view.dispatchEvent(event);
}

function OnMouseUp(e) {
    let event = makeMouseEvent(e, "pointerup");
    view.dispatchEvent(event);
}

function OnMouseMove(e) {
    let event = makeMouseEvent(e, "pointermove");
    view.dispatchEvent(event);
}

function OnMouseWheel(e) {
    let event = makeMouseEvent(e, "wheel");
    view.dispatchEvent(event);
}

setCallback('OnMouseDown', OnMouseDown);
setCallback('OnMouseUp', OnMouseUp);
setCallback('OnMouseMove', OnMouseMove);
setCallback('OnMouseWheel', OnMouseWheel);


function makeTouchEvent(e, type) {
    let event = {
        type: type,
        pointerType: "touch",        
        pointerId: e.pointerId,
        pageX: e.x,
        pageY: e.y,
        deltaY: 0,
        button: -1
    };

    return event;
}

function OnTouchDown(e) {
    let event = makeTouchEvent(e, "pointerdown");
    view.dispatchEvent(event);
}

function OnTouchUp(e) {
    let event = makeTouchEvent(e, "pointerup");
    view.dispatchEvent(event);
}

function OnTouchMove(e) {
    let event = makeTouchEvent(e, "pointermove");
    view.dispatchEvent(event);
}

setCallback('OnTouchDown', OnTouchDown);
setCallback('OnTouchUp', OnTouchUp);
setCallback('OnTouchMove', OnTouchMove);


class UIViewDispatcher extends EventDispatcher {
    
    constructor(view3d){
        super();
        this.view = view3d;        
        view3d.onMouseDown = (e)=>{
            let event = makeMouseEvent(e, "pointerdown");
            this.dispatchEvent(event);
        };        
        view3d.onMouseUp = (e)=>{
            let event = makeMouseEvent(e, "pointerup");
            this.dispatchEvent(event);
        };
        view3d.onMouseMove = (e)=>{
            let event = makeMouseEvent(e, "pointermove");
            this.dispatchEvent(event);
        };
        view3d.onMouseWheel = (e)=>{
            let event = makeMouseEvent(e, "wheel");
            this.dispatchEvent(event);
        };
        view3d.onTouchDown = (e)=>{
            let event = makeTouchEvent(e, "pointerdown");
            print(JSON.stringify(event));
            this.dispatchEvent(event);
        }
        view3d.onTouchUp = (e)=>{
            let event = makeTouchEvent(e, "pointerup");
            print(JSON.stringify(event));
            this.dispatchEvent(event);
        }
        view3d.onTouchMove = (e)=>{
            let event = makeTouchEvent(e, "pointermove");
            print(JSON.stringify(event));
            this.dispatchEvent(event);
        }
    }

    get clientWidth() {
        return this.view.size.x;
    }

    get clientHeight() {
        return this.view.size.y;
    }

    setPointerCapture() {
        gamePlayer.message("setPointerCapture", "");
    }

    releasePointerCapture() {
        gamePlayer.message("releasePointerCapture", "");
    }
}


export { view, UIViewDispatcher };

