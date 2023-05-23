import {Color} from "../math/Color.js"

export class Background
{
    constructor()
    {

    }
}

export class ColorBackground extends Background
{
    constructor()
    {
        super();
        this.color = new Color(1.0, 1.0, 1.0);
    }
}



