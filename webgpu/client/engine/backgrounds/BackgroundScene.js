import {Background} from "./Background.js"

export class BackgroundScene extends Background
{
    constructor(scene, near = 10.0, far = 10000.0)
    {
        super();
        this.scene = scene;
        this.near = near;
        this.far = far;
    }
}


