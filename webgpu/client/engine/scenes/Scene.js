import {Object3D} from "../core/Object3D.js"

export class Scene extends Object3D
{
    constructor()
    {
        super();
        this.background = null;
        this.clear_lists();
    }

    clear_lists()
    {
        this.simple_models = [];
        this.gltf_models = [];
        this.directional_lights = [];
    }
}


