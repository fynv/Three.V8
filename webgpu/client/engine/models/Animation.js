import { Vector3 } from "../math/Vector3.js"
import { Quaternion } from "../math/Quaternion.js";

export class MorphFrame
{
    constructor()
    {
        this.name = "";
        this.weights = [];
    }
}

export class TranslationFrame
{
    constructor()
    {
        this.name = "";
        this.translation = new Vector3(0,0,0);
    }
}

export class RotationFrame
{
    constructor()
    {
        this.name = "";
        this.rotation = new Quaternion(0,0,0,1);
    }
}

export class ScaleFrame
{
    constructor()
    {
        this.name = "";
        this.scale = new Vector3(1,1,1);
    }
}

export class AnimationFrame
{
    constructor()
    {
        this.morphs = [];
        this.translations = [];
        this.rotations = [];
        this.scales = [];
    }
}

function linear_interpolate(y0, y1, t)
{
    return (1.0 - t)*y0 + t*y1;
}

function cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td)
{
    return (2.0 * t3 - 3.0 * t2 + 1.0) * y0 + (-2.0 * t3 + 3.0 * t2) * y1	+ td * ((t3 - 2.0 * t2 + t) * s0 + (t3 - t2) * s1);
}

function upperbound(arr, target)
{
    var l = 0;
    var r = arr.length;
    while(l<r)
    {
        var mid = Math.floor((l+r)/2);                    
        if(arr[mid] <= target)
        {
            l = mid+1;
        }
        else
        {
            r = mid;
        }
    }
    return l;
}

export class MorphTrack
{
    constructor()
    {
        this.name = "";
        this.targets = 0;
        this.interpolation = "LINEAR";
        this.times = new Float32Array();
        this.values = new Float32Array();
    }

    get_frame(x)
    {
        let frame = new MorphFrame();
        frame.name = this.name;
        frame.weights = new Array(this.targets);

        let idx = upperbound(this.times, x);

        if (idx==0)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                for (let i=0; i<this.targets; i++)
                {
                    frame.weights[i] = this.values[i];
                }
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                let offset = this.targets;
                for (let i=0; i<this.targets; i++)
                {
                    frame.weights[i] = this.values[offset + i];
                }
            }
        }
        else if (idx == this.times.length)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                let offset = this.targets *(this.times.length-1);
                for (let i=0; i<this.targets; i++)
                {
                    frame.weights[i] = this.values[offset + i];
                }
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                let offset = this.targets *(this.times.length-1)*3 + this.targets;
                for (let i=0; i<this.targets; i++)
                {
                    frame.weights[i] = this.values[offset + i];
                }
            }
        }
        else
        {
            let x0 = this.times[idx-1];
            let x1 = this.times[idx];
            if (this.interpolation == "STEP")
            {
                let offset = this.targets * (idx-1);
                for (let i=0; i<this.targets; i++)
                {
                    frame.weights[i] = this.values[offset + i];
                }
            }
            else if (this.interpolation == "LINEAR")
            {
                let offset0 = this.targets * (idx-1);
                let offset1 = this.targets * idx;
                let t = (x-x0)/(x1-x0);
                for (let i=0; i<this.targets; i++)
                {
                    let y0 = this.values[offset0 + i];
                    let y1 = this.values[offset1 + i];
                    frame.weights[i] = linear_interpolate(y0, y1, t);
                }
            }
            else if(this.interpolation == "CUBICSPLINE")
            {
                let offset0 =  this.targets * (idx-1) * 3;
                let offset1 =  this.targets * idx * 3;
                let t = (x-x0)/(x1-x0);
                let t2 = t*t;
                let t3 = t2*t;
                let td = x1 - x0;
                for (let i=0; i<this.targets; i++)
                {
                    let y0 = this.values[offset0 + this.targets + i];
                    let s0 = this.values[offset0 + this.targets*2 + i];                    
                    let s1 = this.values[offset1 + i];
                    let y1 = this.values[offset1 + this.targets + i];
                    frame.weights[i] = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }
            }
        }

        return frame;
    }
}

export class TranslationTrack
{
    constructor()
    {
        this.name = "";        
        this.interpolation = "LINEAR";
        this.times = new Float32Array();
        this.values = new Float32Array();
    }

    get_frame(x)
    {
        let frame = new TranslationFrame();
        frame.name = this.name;

        let idx = upperbound(this.times, x);
        if (idx==0)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                frame.translation.set(this.values[0], this.values[1], this.values[2]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                frame.translation.set(this.values[3], this.values[4], this.values[5]);
            }
        }
        else if (idx == this.times.length)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                let offset = 3 *(this.times.length-1);
                frame.translation.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                let offset = 3 *(this.times.length-1)*3 + 3;
                frame.translation.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
        }
        else
        {
            let x0 = this.times[idx-1];
            let x1 = this.times[idx];
            if (this.interpolation == "STEP")
            {
                let offset = 3 * (idx-1);
                frame.translation.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
            else if (this.interpolation == "LINEAR")
            {
                let offset0 = 3 * (idx-1);
                let offset1 = 3 * idx;
                let t = (x-x0)/(x1-x0);                
                {
                    let y0 = this.values[offset0];
                    let y1 = this.values[offset1];
                    frame.translation.x = linear_interpolate(y0, y1, t);
                }
                {
                    let y0 = this.values[offset0+1];
                    let y1 = this.values[offset1+1];
                    frame.translation.y = linear_interpolate(y0, y1, t);
                }
                {
                    let y0 = this.values[offset0+2];
                    let y1 = this.values[offset1+2];
                    frame.translation.z = linear_interpolate(y0, y1, t);
                }
            }
            else if(this.interpolation == "CUBICSPLINE")
            {
                let offset0 =  3 * (idx-1) * 3;
                let offset1 =  3 * idx * 3;
                let t = (x-x0)/(x1-x0);
                let t2 = t*t;
                let t3 = t2*t;
                let td = x1 - x0;
                {
                    let y0 = this.values[offset0 + 3];
                    let s0 = this.values[offset0 + 3*2];
                    let s1 = this.values[offset1];
                    let y1 = this.values[offset1 + 3];
                    frame.translation.x = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }

                {
                    let y0 = this.values[offset0 + 3 + 1];
                    let s0 = this.values[offset0 + 3*2 + 1];
                    let s1 = this.values[offset1 + 1];
                    let y1 = this.values[offset1 + 3 + 1];
                    frame.translation.y = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }

                {
                    let y0 = this.values[offset0 + 3 + 2];
                    let s0 = this.values[offset0 + 3*2 + 2];
                    let s1 = this.values[offset1 + 2];
                    let y1 = this.values[offset1 + 3 + 2];
                    frame.translation.z = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }                
            }
        }
        return frame;
    }
}

export class RotationTrack
{
    constructor()
    {
        this.name = "";        
        this.interpolation = "LINEAR";
        this.times = new Float32Array();
        this.values = new Float32Array();
    }

    get_frame(x)
    {
        let frame = new RotationFrame();
        frame.name = this.name;

        let idx = upperbound(this.times, x);
        if (idx==0)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                frame.rotation.set(this.values[0], this.values[1], this.values[2], this.values[3]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                frame.rotation.set(this.values[4], this.values[5], this.values[6], this.values[7]);
            }
        }
        else if (idx == this.times.length)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                let offset = 4 *(this.times.length-1);
                frame.rotation.set(this.values[offset], this.values[offset+1], this.values[offset+2], this.values[offset+3]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                let offset = 4 *(this.times.length-1)*3 + 4;
                frame.rotation.set(this.values[offset], this.values[offset+1], this.values[offset+2], this.values[offset+3]);
            }
        }
        else
        {
            let x0 = this.times[idx-1];
            let x1 = this.times[idx];
            if (this.interpolation == "STEP")
            {
                let offset = 4 * (idx-1);
                frame.rotation.set(this.values[offset], this.values[offset+1], this.values[offset+2], this.values[offset+3]);
            }
            else if (this.interpolation == "LINEAR")
            {
                let offset0 = 4 * (idx-1);
                let offset1 = 4 * idx;
                let t = (x-x0)/(x1-x0);

                let y0 = new Quaternion(
                    this.values[offset0], 
                    this.values[offset0 + 1],
                    this.values[offset0 + 2],
                    this.values[offset0 + 3]);

                let y1 = new Quaternion(
                    this.values[offset1], 
                    this.values[offset1 + 1],
                    this.values[offset1 + 2],
                    this.values[offset1 + 3]);

                frame.rotation.slerpQuaternions(y0, y1, t);
            }
            else if(this.interpolation == "CUBICSPLINE")
            {
                let offset0 =  4 * (idx-1) * 3;
                let offset1 =  4 * idx * 3;
                let t = (x-x0)/(x1-x0);

                let y0 = new Quaternion(
                    this.values[offset0 + 4], 
                    this.values[offset0 + 4 + 1],
                    this.values[offset0 + 4 + 2],
                    this.values[offset0 + 4 + 3]);

                let y1 = new Quaternion(
                    this.values[offset1 + 4], 
                    this.values[offset1 + 4 + 1],
                    this.values[offset1 + 4 + 2],
                    this.values[offset1 + 4 + 3]);
                
                frame.rotation.slerpQuaternions(y0, y1, t);
            }
        }
        return frame;
    }
}

export class ScaleTrack
{
    constructor()
    {
        this.name = "";        
        this.interpolation = "LINEAR";
        this.times = new Float32Array();
        this.values = new Float32Array();
    }
    
    get_frame(x)
    {
        let frame = new ScaleFrame();
        frame.name = this.name;

        let idx = upperbound(this.times, x);
        if (idx==0)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                frame.scale.set(this.values[0], this.values[1], this.values[2]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                frame.scale.set(this.values[3], this.values[4], this.values[5]);
            }
        }
        else if (idx == this.times.length)
        {
            if (this.interpolation == "STEP" || this.interpolation == "LINEAR")
            {
                let offset = 3 *(this.times.length-1);
                frame.scale.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
            else if (this.interpolation == "CUBICSPLINE")
            {
                let offset = 3 *(this.times.length-1)*3 + 3;
                frame.scale.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
        }
        else
        {
            let x0 = this.times[idx-1];
            let x1 = this.times[idx];
            if (this.interpolation == "STEP")
            {
                let offset = 3 * (idx-1);
                frame.scale.set(this.values[offset], this.values[offset+1], this.values[offset+2]);
            }
            else if (this.interpolation == "LINEAR")
            {
                let offset0 = 3 * (idx-1);
                let offset1 = 3 * idx;
                let t = (x-x0)/(x1-x0);                
                {
                    let y0 = this.values[offset0];
                    let y1 = this.values[offset1];
                    frame.scale.x = linear_interpolate(y0, y1, t);
                }
                {
                    let y0 = this.values[offset0+1];
                    let y1 = this.values[offset1+1];
                    frame.scale.y = linear_interpolate(y0, y1, t);
                }
                {
                    let y0 = this.values[offset0+2];
                    let y1 = this.values[offset1+2];
                    frame.scale.z = linear_interpolate(y0, y1, t);
                }
            }
            else if(this.interpolation == "CUBICSPLINE")
            {
                let offset0 =  3 * (idx-1) * 3;
                let offset1 =  3 * idx * 3;
                let t = (x-x0)/(x1-x0);
                let t2 = t*t;
                let t3 = t2*t;
                let td = x1 - x0;
                {
                    let y0 = this.values[offset0 + 3];
                    let s0 = this.values[offset0 + 3*2];
                    let s1 = this.values[offset1];
                    let y1 = this.values[offset1 + 3];
                    frame.scale.x = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }

                {
                    let y0 = this.values[offset0 + 3 + 1];
                    let s0 = this.values[offset0 + 3*2 + 1];
                    let s1 = this.values[offset1 + 1];
                    let y1 = this.values[offset1 + 3 + 1];
                    frame.scale.y = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }

                {
                    let y0 = this.values[offset0 + 3 + 2];
                    let s0 = this.values[offset0 + 3*2 + 2];
                    let s1 = this.values[offset1 + 2];
                    let y1 = this.values[offset1 + 3 + 2];
                    frame.scale.z = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
                }                
            }
        }
        return frame;
    }
}


export class AnimationClip
{
    constructor()
    {
        this.name = "";
        this.duration = 0;

        this.morphs = [];
        this.translations = [];
        this.rotations = [];
        this.scales = [];
    }

    get_frame(x)
    {
        let frame = new AnimationFrame();
        for (let morph of this.morphs)
        {
            frame.morphs.push(morph.get_frame(x));            
        }
        for (let trans of this.translations)
        {
            frame.translations.push(trans.get_frame(x));
        }
        for (let rot of this.rotations)
        {
            frame.rotations.push(rot.get_frame(x));
        }
        for (let scale of this.scales)
        {
            frame.scales.push(scale.get_frame(x));
        }

        return frame;
    }

}
