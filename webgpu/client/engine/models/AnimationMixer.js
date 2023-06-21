import { AnimationFrame } from "./Animation.js"

function linear_interpolate(y0, y1, t)
{
    return (1.0 - t)*y0 + t*y1;
}

export class AnimationMixer
{
    constructor()
    {
        this.animations = [];
        this.animation_dict = {};
        this.currentPlaying = [];
    }
    
    addAnimation(anim)
    {
        this.animations.push(anim);
        this.animation_dict[anim.name] = this.animations.length - 1;
    }

    addAnimations(anims)
    {
        for (let anim of anims)
        {
            this.addAnimation(anims);
        }
    }
    
    startAnimation(name)
    {        
        let weight = this.currentPlaying.length>0?0:1;
        this.currentPlaying.push({ name, time_start: Date.now(), weight})
    }

    stopAnimation(idx)
    {
        this.currentPlaying.splice(idx,1);
    }
    
    setWeights(weights)
    {
        for (let i=0; i< weights.length && i<this.currentPlaying.length; i++)
        {
            this.currentPlaying[i].weight = weights[i];
        }
    }
    
    getFrame()
    {
        let dst_frame = new AnimationFrame();

        let t = Date.now();
        let acc_weight = 0;

        for (let playback of this.currentPlaying)
        {
            if (!(playback.name in this.animation_dict)) continue;
            let id_anim = this.animation_dict[playback.name];
            let anim = this.animations[id_anim];
            let duration = anim.duration * 1000.0;
            let x = 0;
            if (duration >0)
            {
                while(t-playback.time_start>=duration)
                {
                    playback.time_start += duration;
                }
                x= t - playback.time_start;
            }

            let weight = playback.weight;
            let fac = weight / (acc_weight + weight);
            let src_frame = anim.get_frame(x*0.001);

            for (let src_morph of src_frame.morphs)
            {
                let dst_morph = null;
                for (let _dst_morph of dst_frame.morphs)
                {
                    if (_dst_morph.name == src_morph.name)
                    {
                        dst_morph = _dst_morph;
                        break;
                    }
                }

                if (dst_morph==null)
                {
                    dst_frame.morphs.push_back(src_morph);
                }
                else
                {
                    let num_elem_src = src_morph.weights.length;
                    let num_elem_dst = dst_morph.weights.length;
                    if (num_elem_dst < num_elem_src)
                    {
                        dst_morph.weights = dst_morph.weights.concat(src_morph.weights.slice(num_elem_dst, num_elem_src));
                    }

                    for (let k=0; k<num_elem_src; k++)
                    {
                        dst_morph.weights[k] = linear_interpolate(dst_morph.weights[k], src_morph.weights[k], fac);
                    }
                }
            }

            for (let src_trans of src_frame.translations)
            {
                let dst_trans = null;
                for (let _dst_trans of dst_frame.translations)
                {
                    if(_dst_trans.name == src_trans.name)
                    {
                        dst_trans = _dst_trans;
                        break;
                    }
                }

                if(dst_trans ==null)
                {
                    dst_frame.translations.push(src_trans);
                }
                else
                {
                    dst_trans.translation.x = linear_interpolate(dst_trans.translation.x, src_trans.translation.x, fac);
                    dst_trans.translation.y = linear_interpolate(dst_trans.translation.y, src_trans.translation.y, fac);
                    dst_trans.translation.z = linear_interpolate(dst_trans.translation.z, src_trans.translation.z, fac);
                }
            }

            for (let src_rot of src_frame.rotations)
            {
                let dst_rot = null;
                for (let _dst_rot of dst_frame.rotations)
                {
                    if(_dst_rot.name == src_rot.name)
                    {
                        dst_rot = _dst_rot;
                        break;
                    }
                }

                if(dst_rot == null)
                {
                    dst_frame.rotations.push(src_rot);
                }
                else
                {
                    dst_rot.rotation.slerpQuaternions(dst_rot.rotation, src_rot.rotation, fac);
                }
            }

            for (let src_scale of src_frame.scales)
            {
                let dst_scale = null;
                for (let _dst_scale of dst_frame.scales)
                {
                    if (_dst_scale.name == src_scale.name)
                    {
                        dst_scale = _dst_scale;
                        break;
                    }
                }

                if (dst_scale == null)
                {
                    dst_frame.scales.push(src_scale);
                }
                else
                {
                    dst_scale.scale.x = linear_interpolate(dst_scale.scale.x, src_scale.scale.x, fac);
                    dst_scale.scale.y = linear_interpolate(dst_scale.scale.y, src_scale.scale.y, fac);
                    dst_scale.scale.z = linear_interpolate(dst_scale.scale.z, src_scale.scale.z, fac);
                }
            }

            acc_weight += weight;
        }

        return dst_frame;             
        
    }
    
}