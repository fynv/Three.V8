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
