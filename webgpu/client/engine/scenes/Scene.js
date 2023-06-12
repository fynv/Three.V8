import {Object3D} from "../core/Object3D.js"
import {Lights} from "../lights/Lights.js"

export class Scene extends Object3D
{
    constructor()
    {
        super();
        this.background = null;
        this.indirectLight = null;
        this.lights = new Lights();        
        this.clear_lists();
    }

    clear_lists()
    {
        this.simple_models = [];
        this.gltf_models = [];     
        this.lights.clear_lists();   
    }
}


