export class MorphFrame
{
    constructor()
    {
        this.name = "";
        this.weights = [];
    }
}

export class AnimationFrame
{
    constructor()
    {
        this.morphs = [];
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

export class AnimationClip
{
    constructor()
    {
        this.name = "";
        this.duration = 0;

        this.morphs = [];
    }

    get_frame(x)
    {
        let frame = new AnimationFrame();
        for (let morph of this.morphs)
        {
            frame.morphs.push(morph.get_frame(x));            
        }
        return frame;
    }

}
