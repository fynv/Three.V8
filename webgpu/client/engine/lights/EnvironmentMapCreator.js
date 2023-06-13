import get_module from './EnvMap.js'
import { coeffs } from "./filter_coeffs.js"
import { CubeBackground } from "../backgrounds/Background.js"
import { EnvironmentMap } from "./EnvironmentMap.js"
import { CreateCubeTexture } from "../renderers/GPUUtils.js"
import { ReflectionMap } from "./IndirectLight.js"

const shader_downsample = `
@group(0) @binding(0)
var tex_sampler: sampler;

@group(0) @binding(1)
var tex_hi_res : texture_cube<f32>;

@group(0) @binding(2)
var tex_lo_res : texture_storage_2d_array<rgba16float, write>;

fn get_dir_0(u: f32, v: f32) -> vec3f
{
    return vec3(1.0, v, -u);
}

fn get_dir_1(u: f32, v: f32) -> vec3f
{
    return vec3(-1.0, v, u);
}

fn get_dir_2(u: f32, v: f32) -> vec3f
{
    return vec3(u, 1.0, -v);
}

fn get_dir_3(u: f32, v: f32) -> vec3f
{
    return vec3(u, -1.0, v);
}

fn get_dir_4(u: f32, v: f32) -> vec3f
{
    return vec3(u, v, 1.0);
}

fn get_dir_5(u: f32, v: f32) -> vec3f
{
    return vec3(-u, v, -1.0);
}

fn calcWeight(u: f32, v: f32) -> f32
{
    let val = u*u + v*v + 1.0;
    return val*sqrt(val);
}

@compute @workgroup_size(8,8, 1)
fn main(@builtin(global_invocation_id) id : vec3<u32>)
{
    let res_lo = textureDimensions(tex_lo_res).x;
    if (id.x < res_lo && id.y < res_lo)
    {
        let inv_res_lo = 1.0 / f32(res_lo);
        let u0 = (f32(id.x) * 2.0  + 1.0 - 0.75) * inv_res_lo - 1.0;
        let u1 = (f32(id.x) * 2.0  + 1.0 + 0.75) * inv_res_lo - 1.0;

        let v0 = (f32(id.y) * 2.0  + 1.0 - 0.75) * -inv_res_lo + 1.0;
        let v1 = (f32(id.y) * 2.0  + 1.0 + 0.75) * -inv_res_lo + 1.0;

        var weights = vec4(
            calcWeight(u0, v0),
            calcWeight(u1, v0),
            calcWeight(u0, v1),
            calcWeight(u1, v1)            
        );

        let wsum = 0.5 / ( weights.x + weights.y + weights.z + weights.w);
        weights = weights * wsum +0.125;

        var dir: vec3f;
        var color: vec4f;

        switch id.z
        {
            default
            {
                dir = get_dir_0(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_0(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_0(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_0(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }
            case 1
            {
                dir = get_dir_1(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_1(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_1(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_1(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }
            case 2
            {
                dir = get_dir_2(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_2(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_2(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_2(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }
            case 3
            {
                dir = get_dir_3(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_3(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_3(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_3(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }
            case 4
            {
                dir = get_dir_4(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_4(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_4(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_4(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }
            case 5
            {
                dir = get_dir_5(u0, v0);
                color = textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.x;

                dir = get_dir_5(u1, v0);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.y;

                dir = get_dir_5(u0, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.z;

                dir = get_dir_5(u1, v1);
                color += textureSampleLevel(tex_hi_res, tex_sampler, dir, 0.0) * weights.w;
            }            
        }
        textureStore(tex_lo_res, id.xy, id.z, color);
    }   
}
`;

const shader_filter1 = `
@group(0) @binding(0)
var tex_sampler: sampler;

@group(0) @binding(1)
var tex_in : texture_cube<f32>;

@group(0) @binding(2)
var tex_out0 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(3)
var tex_out1 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(4)
var tex_out2 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(5)
var tex_out3 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(6)
var<uniform> coeffs: array<array<array<array<vec4f, 24>, 3>, 5>, 7>;

const NUM_TAPS = 32u;
const BASE_RESOLUTION = 128u;

fn get_dir(uv: vec2f, face: u32) -> vec3f
{
    switch face
    {
        default
        {
            return vec3(1.0, uv.y, -uv.x);
        }
        case 1
        {
            return vec3(-1.0, uv.y, uv.x);
        }
        case 2
        {
            return vec3(uv.x, 1.0, -uv.y);
        }
        case 3
        {
            return vec3(uv.x, -1.0, uv.y);
        }
        case 4
        {
            return vec3(uv.x, uv.y, 1.0);
        }
        case 5
        {
            return vec3(-uv.x, uv.y, -1.0);
        }
    }
}

@compute @workgroup_size(64,1,1)
fn main(@builtin(global_invocation_id) global_id : vec3<u32>)
{
    var id = global_id;
    var level:u32;
	if ( id.x < ( 128 * 128 ) )
	{
		level = 0;
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 ) )
	{
		level = 1;
		id.x -= ( 128 * 128 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 ) )
	{
		level = 2;
		id.x -= ( 128 * 128 + 64 * 64 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 ) )
	{
		level = 3;
		id.x -= ( 128 * 128 + 64 * 64 + 32 * 32 );
	}
    else
    {
        return;
    }

    id.z = id.y;
    let res = BASE_RESOLUTION >> level;
    id.y = id.x / res;
    id.x -= id.y * res;

    let uv = vec2( (f32(id.x)*2.0+1.0)/f32(res) - 1.0, -(f32(id.y)*2.0+1.0)/f32(res) + 1.0);
    let dir = get_dir(uv, id.z);
    let frameZ = normalize(dir);

    let adir = abs(dir);

    var color = vec4(0.0);
    for (var axis = 0u; axis <3u; axis++)
    {
        let otherAxis0 = 1 - ( axis & 1 ) - ( axis >> 1 );
        let otherAxis1 = 2 - ( axis >> 1 );

        let frameweight = ( max( adir[otherAxis0], adir[otherAxis1] ) - 0.75 ) / 0.25;
        if ( frameweight > 0.0 )
        {
            var UpVector: vec3f;
            switch axis
            {
                default
                {
                    UpVector = vec3(1.0, 0.0, 0.0);
                }
                case 1
                {
                    UpVector = vec3(0.0, 1.0, 0.0);
                }
                case 2
                {
                    UpVector = vec3(0.0, 0.0, 1.0);
                }
            }
            let frameX = normalize( cross( UpVector, frameZ ) );
            let frameY = cross( frameZ, frameX );

            var Nx = dir[otherAxis0];
            var Ny = dir[otherAxis1];
            let Nz = adir[axis];

            let NmaxXY = max(abs(Ny), abs(Nx));
            Nx /= NmaxXY;
			Ny /= NmaxXY;

            var theta: f32;
            if ( Ny < Nx )
			{
				if ( Ny <= -0.999 )
                {
					theta = Nx;
                }
				else
                {
					theta = Ny;
                }
			}
			else
			{
				if ( Ny >= 0.999 )
                {
					theta = -Nx;
                }
				else
                {
					theta = -Ny;
                }
			}

            var phi: f32;
            if ( Nz <= -0.999 )
			{
				phi = -NmaxXY;
			}
			else if ( Nz >= 0.999 )
			{
				phi = NmaxXY;
			}
			else
			{
				phi = Nz;
			}

            let theta2 = theta *theta;
            let phi2 = phi * phi;

            for (var iSuperTap = 0u; iSuperTap < NUM_TAPS / 4; iSuperTap++ )
            {
                let index = ( NUM_TAPS / 4 ) * axis + iSuperTap;
                var coeffsDir0: array<vec4f, 3>;
                var coeffsDir1: array<vec4f, 3>;
                var coeffsDir2: array<vec4f, 3>;
                var coeffsLevel: array<vec4f, 3>;
                var coeffsWeight: array<vec4f, 3>;

                for (var iCoeff = 0; iCoeff < 3; iCoeff++)
                {
                    coeffsDir0[iCoeff] = coeffs[level][0][iCoeff][index];
					coeffsDir1[iCoeff] = coeffs[level][1][iCoeff][index];
					coeffsDir2[iCoeff] = coeffs[level][2][iCoeff][index];
					coeffsLevel[iCoeff] = coeffs[level][3][iCoeff][index];
					coeffsWeight[iCoeff] = coeffs[level][4][iCoeff][index];
                }

                for (var iSubTap = 0; iSubTap < 4; iSubTap++)
				{
                    var sample_dir
                        = frameX * ( coeffsDir0[0][iSubTap] + coeffsDir0[1][iSubTap] * theta2 + coeffsDir0[2][iSubTap] * phi2 )
                        + frameY * ( coeffsDir1[0][iSubTap] + coeffsDir1[1][iSubTap] * theta2 + coeffsDir1[2][iSubTap] * phi2 )
                        + frameZ * ( coeffsDir2[0][iSubTap] + coeffsDir2[1][iSubTap] * theta2 + coeffsDir2[2][iSubTap] * phi2 );

                    var sample_level = coeffsLevel[0][iSubTap] + coeffsLevel[1][iSubTap] * theta2 + coeffsLevel[2][iSubTap] * phi2;

                    var sample_weight = coeffsWeight[0][iSubTap] + coeffsWeight[1][iSubTap] * theta2 + coeffsWeight[2][iSubTap] * phi2;
                    sample_weight *= frameweight;

                    sample_dir /= max( abs( sample_dir.x ), max( abs( sample_dir.y ), abs( sample_dir.z ) ) );
                    sample_level += 0.75 * log2( dot( sample_dir, sample_dir ) );

                    color += vec4(textureSampleLevel(tex_in, tex_sampler, sample_dir, sample_level).xyz * sample_weight, sample_weight);
                }
            }
        }
    }

    color /= color.w;
    color.x = max( 0.0, color.x );
    color.y = max( 0.0, color.y );
    color.z = max( 0.0, color.z );
    color.w = 1.0;

    switch level
    {
        default
        {
            textureStore(tex_out0, id.xy, id.z, color);
        }
        case 1
        {
            textureStore(tex_out1, id.xy, id.z, color);
        }
        case 2
        {
            textureStore(tex_out2, id.xy, id.z, color);
        }
        case 3
        {
            textureStore(tex_out3, id.xy, id.z, color);
        }
    }    
}
`;

const shader_filter2 = `
@group(0) @binding(0)
var tex_sampler: sampler;

@group(0) @binding(1)
var tex_in : texture_cube<f32>;

@group(0) @binding(2)
var tex_out4 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(3)
var tex_out5 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(4)
var tex_out6 : texture_storage_2d_array<rgba16float, write>;

@group(0) @binding(5)
var<uniform> coeffs: array<array<array<array<vec4f, 24>, 3>, 5>, 7>;

const NUM_TAPS = 32u;
const BASE_RESOLUTION = 128u;

fn get_dir(uv: vec2f, face: u32) -> vec3f
{
    switch face
    {
        default
        {
            return vec3(1.0, uv.y, -uv.x);
        }
        case 1
        {
            return vec3(-1.0, uv.y, uv.x);
        }
        case 2
        {
            return vec3(uv.x, 1.0, -uv.y);
        }
        case 3
        {
            return vec3(uv.x, -1.0, uv.y);
        }
        case 4
        {
            return vec3(uv.x, uv.y, 1.0);
        }
        case 5
        {
            return vec3(-uv.x, uv.y, -1.0);
        }
    }
}

@compute @workgroup_size(64,1,1)
fn main(@builtin(global_invocation_id) global_id : vec3<u32>)
{
    var id = global_id;
    var level:u32;
    if ( id.x < ( 8 * 8 ) )
	{
		level = 4;		
	}
    else if ( id.x < ( 8 * 8 + 4 * 4 ) )
	{
		level = 5;
		id.x -= ( 8 * 8 );
	}
	else if ( id.x < ( 8 * 8 + 4 * 4 + 2 * 2 ) )
	{
		level = 6;
		id.x -= ( 8 * 8 + 4 * 4 );
	}
	else
	{
		return;
	}
    id.z = id.y;
    let res = BASE_RESOLUTION >> level;
    id.y = id.x / res;
    id.x -= id.y * res;

    let uv = vec2( (f32(id.x)*2.0+1.0)/f32(res) - 1.0, -(f32(id.y)*2.0+1.0)/f32(res) + 1.0);
    let dir = get_dir(uv, id.z);
    let frameZ = normalize(dir);

    let adir = abs(dir);

    var color = vec4(0.0);
    for (var axis = 0u; axis <3u; axis++)
    {
        let otherAxis0 = 1 - ( axis & 1 ) - ( axis >> 1 );
        let otherAxis1 = 2 - ( axis >> 1 );

        let frameweight = ( max( adir[otherAxis0], adir[otherAxis1] ) - 0.75 ) / 0.25;
        if ( frameweight > 0.0 )
        {
            var UpVector: vec3f;
            switch axis
            {
                default
                {
                    UpVector = vec3(1.0, 0.0, 0.0);
                }
                case 1
                {
                    UpVector = vec3(0.0, 1.0, 0.0);
                }
                case 2
                {
                    UpVector = vec3(0.0, 0.0, 1.0);
                }
            }
            let frameX = normalize( cross( UpVector, frameZ ) );
            let frameY = cross( frameZ, frameX );

            var Nx = dir[otherAxis0];
            var Ny = dir[otherAxis1];
            let Nz = adir[axis];

            let NmaxXY = max(abs(Ny), abs(Nx));
            Nx /= NmaxXY;
			Ny /= NmaxXY;

            var theta: f32;
            if ( Ny < Nx )
			{
				if ( Ny <= -0.999 )
                {
					theta = Nx;
                }
				else
                {
					theta = Ny;
                }
			}
			else
			{
				if ( Ny >= 0.999 )
                {
					theta = -Nx;
                }
				else
                {
					theta = -Ny;
                }
			}

            var phi: f32;
            if ( Nz <= -0.999 )
			{
				phi = -NmaxXY;
			}
			else if ( Nz >= 0.999 )
			{
				phi = NmaxXY;
			}
			else
			{
				phi = Nz;
			}

            let theta2 = theta *theta;
            let phi2 = phi * phi;

            for (var iSuperTap = 0u; iSuperTap < NUM_TAPS / 4; iSuperTap++ )
            {
                let index = ( NUM_TAPS / 4 ) * axis + iSuperTap;
                var coeffsDir0: array<vec4f, 3>;
                var coeffsDir1: array<vec4f, 3>;
                var coeffsDir2: array<vec4f, 3>;
                var coeffsLevel: array<vec4f, 3>;
                var coeffsWeight: array<vec4f, 3>;

                for (var iCoeff = 0; iCoeff < 3; iCoeff++)
                {
                    coeffsDir0[iCoeff] = coeffs[level][0][iCoeff][index];
					coeffsDir1[iCoeff] = coeffs[level][1][iCoeff][index];
					coeffsDir2[iCoeff] = coeffs[level][2][iCoeff][index];
					coeffsLevel[iCoeff] = coeffs[level][3][iCoeff][index];
					coeffsWeight[iCoeff] = coeffs[level][4][iCoeff][index];
                }

                for (var iSubTap = 0; iSubTap < 4; iSubTap++)
				{
                    var sample_dir
                        = frameX * ( coeffsDir0[0][iSubTap] + coeffsDir0[1][iSubTap] * theta2 + coeffsDir0[2][iSubTap] * phi2 )
                        + frameY * ( coeffsDir1[0][iSubTap] + coeffsDir1[1][iSubTap] * theta2 + coeffsDir1[2][iSubTap] * phi2 )
                        + frameZ * ( coeffsDir2[0][iSubTap] + coeffsDir2[1][iSubTap] * theta2 + coeffsDir2[2][iSubTap] * phi2 );

                    var sample_level = coeffsLevel[0][iSubTap] + coeffsLevel[1][iSubTap] * theta2 + coeffsLevel[2][iSubTap] * phi2;

                    var sample_weight = coeffsWeight[0][iSubTap] + coeffsWeight[1][iSubTap] * theta2 + coeffsWeight[2][iSubTap] * phi2;
                    sample_weight *= frameweight;

                    sample_dir /= max( abs( sample_dir.x ), max( abs( sample_dir.y ), abs( sample_dir.z ) ) );
                    sample_level += 0.75 * log2( dot( sample_dir, sample_dir ) );

                    color += vec4(textureSampleLevel(tex_in, tex_sampler, sample_dir, sample_level).xyz * sample_weight, sample_weight);
                }
            }
        }
    }

    color /= color.w;
    color.x = max( 0.0, color.x );
    color.y = max( 0.0, color.y );
    color.z = max( 0.0, color.z );
    color.w = 1.0;

    switch level
    {
        default
        {
            textureStore(tex_out4, id.xy, id.z, color);
        }
        case 5
        {
            textureStore(tex_out5, id.xy, id.z, color);
        }
        case 6
        {
            textureStore(tex_out6, id.xy, id.z, color);
        }        
    }
}
`;

function GetPipelineEnvmapDownsample()
{
    if (!("envmap_downsample" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_downsample });
        engine_ctx.cache.pipelines.envmap_downsample = engine_ctx.device.createComputePipeline({
            layout: 'auto',
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.envmap_downsample;

}

function GetPipelineEnvmapFilter1()
{
    if (!("envmap_filter1" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_filter1 });
        engine_ctx.cache.pipelines.envmap_filter1 = engine_ctx.device.createComputePipeline({
            layout: 'auto',
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.envmap_filter1;

}

function GetPipelineEnvmapFilter2()
{
    if (!("envmap_filter2" in engine_ctx.cache.pipelines))
    {
        let shaderModule = engine_ctx.device.createShaderModule({ code: shader_filter2 });
        engine_ctx.cache.pipelines.envmap_filter2 = engine_ctx.device.createComputePipeline({
            layout: 'auto',
            compute: {
                module: shaderModule,
                entryPoint: 'main',
            },
        });
    }
    return engine_ctx.cache.pipelines.envmap_filter2;

}



export class EnvironmentMapCreator
{
    constructor()
    {        
        this.buf_coeffs = engine_ctx.createBuffer(coeffs.buffer, GPUBufferUsage.UNIFORM);
        this.pipeline_downsample = GetPipelineEnvmapDownsample();
        this.pipeline_filter1 = GetPipelineEnvmapFilter1();
        this.pipeline_filter2 = GetPipelineEnvmapFilter2();
        this.sampler = engine_ctx.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear',
        });
        this.tex_src = engine_ctx.device.createTexture({
            dimension: '2d',          
            size: [128, 128, 6],
            format: 'rgba16float',
            mipLevelCount: 8,
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.COPY_SRC
        });
    }

    _createSH(shCoefficients)
    {
        let download_buffer =  engine_ctx.createBuffer0(128*128*6*4*2, GPUBufferUsage.COPY_DST|GPUBufferUsage.MAP_READ);    
        let commandEncoder = engine_ctx.device.createCommandEncoder();
        commandEncoder.copyTextureToBuffer(
            {
                texture: this.tex_src
            },
            {
                buffer: download_buffer,
                bytesPerRow: 128*4*2,
                rowsPerImage: 128
            },
            {
                width: 128,
                height: 128,
                depthOrArrayLayers: 6
            }
        );
        engine_ctx.device.queue.submit([commandEncoder.finish()]);

        (async()=>{
            let module = await get_module();
            let p_coeffs = module.ccall("alloc", "number", ["number"], [4*4*9]);
            let p_fp16 = module.ccall("alloc", "number", ["number"], [128*128*4*2]);
            await download_buffer.mapAsync(GPUMapMode.READ);
            let buf = download_buffer.getMappedRange();
            let u8arr = new Uint8Array(buf);
            module.HEAPU8.set(u8arr, p_fp16);
            download_buffer.unmap();
            module.ccall("CreateSH", null, ["number", "number"], [p_coeffs, p_fp16]);
            let f32Coeffs = new Float32Array(4*9);
            let u8view_out = new Uint8Array(f32Coeffs.buffer);
            let u8view_in =  new Uint8Array(module.HEAPU8.buffer, p_coeffs, 4*4*9);
            u8view_out.set(u8view_in);
            module.ccall("dealloc", null, ["number"], [p_coeffs]);
            module.ccall("dealloc", null, ["number"], [p_fp16]);

            for (let i=0;i<9;i++)
            {
                shCoefficients[i][0] = f32Coeffs[i*4];
                shCoefficients[i][1] = f32Coeffs[i*4+1];
                shCoefficients[i][2] = f32Coeffs[i*4+2];
            }
        })();
    }

    _createReflection(cubemap)
    {
        let reflection = new ReflectionMap();

        const commandEncoder = engine_ctx.device.createCommandEncoder();
        const passEncoder = commandEncoder.beginComputePass();

        passEncoder.setPipeline(this.pipeline_downsample);
        
        {
            let bind_group = engine_ctx.device.createBindGroup({
                layout: this.pipeline_downsample.getBindGroupLayout(0),
                entries: [
                    {
                        binding: 0,
                        resource: this.sampler
                    },
                    {
                        binding: 1,
                        resource: cubemap.createView({
                            dimension: 'cube'
                        })
                    },
                    {
                        binding: 2,
                        resource: this.tex_src.createView({
                            dimension: '2d-array',
                            baseMipLevel: 0,
                            mipLevelCount: 1
                        })
                    }
                ],
            });
            passEncoder.setBindGroup(0, bind_group);
            passEncoder.dispatchWorkgroups(128/8, 128/8, 6);
        }

        let size = 64;
        for (let level=0; level<7; level++, size/=2)
        {
            let bind_group = engine_ctx.device.createBindGroup({
                layout: this.pipeline_downsample.getBindGroupLayout(0),
                entries: [
                    {
                        binding: 0,
                        resource: this.sampler
                    },
                    {
                        binding: 1,
                        resource: this.tex_src.createView({
                            dimension: 'cube',
                            baseMipLevel: level,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 2,
                        resource: this.tex_src.createView({
                            dimension: '2d-array',
                            baseMipLevel: level+1,
                            mipLevelCount: 1
                        })
                    }
                ],
            });
            passEncoder.setBindGroup(0, bind_group);
            passEncoder.dispatchWorkgroups(Math.floor((size+7)/8), Math.floor((size+7)/8), 6);
        }

        passEncoder.setPipeline(this.pipeline_filter1);
        {
            let bind_group = engine_ctx.device.createBindGroup({
                layout: this.pipeline_filter1.getBindGroupLayout(0),
                entries: [
                    {
                        binding: 0,
                        resource: this.sampler
                    },
                    {
                        binding: 1,
                        resource: this.tex_src.createView({
                            dimension: 'cube',                           
                        })
                    },
                    {
                        binding: 2,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 0,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 3,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 1,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 4,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 2,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 5,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 3,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 6,
                        resource:{
                            buffer: this.buf_coeffs
                        }
                    }
                ]
            });
            passEncoder.setBindGroup(0, bind_group);
            let blocks = Math.floor((128 * 128 + 64 * 64 + 32 * 32 + 16 * 16  + 63) / 64);
            passEncoder.dispatchWorkgroups(blocks, 6, 1);
        }

        passEncoder.setPipeline(this.pipeline_filter2);
        {
            let bind_group = engine_ctx.device.createBindGroup({
                layout: this.pipeline_filter2.getBindGroupLayout(0),
                entries: [
                    {
                        binding: 0,
                        resource: this.sampler
                    },
                    {
                        binding: 1,
                        resource: this.tex_src.createView({
                            dimension: 'cube',                           
                        })
                    },
                    {
                        binding: 2,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 4,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 3,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 5,
                            mipLevelCount: 1
                        })
                    },
                    {
                        binding: 4,
                        resource: reflection.texture.createView({
                            dimension: '2d-array',
                            baseMipLevel: 6,
                            mipLevelCount: 1
                        })
                    },                    
                    {
                        binding: 5,
                        resource:{
                            buffer: this.buf_coeffs
                        }
                    }
                ]
            });
            passEncoder.setBindGroup(0, bind_group);
            let blocks = Math.floor((8 * 8 + 4 * 4 + 2 * 2  + 63) / 64);
            passEncoder.dispatchWorkgroups(blocks, 6, 1);
        }

        passEncoder.end();
        engine_ctx.device.queue.submit([commandEncoder.finish()]);

        return reflection;
    }

    create(image)
    {
        let envMap = new EnvironmentMap();

        if (image instanceof CubeBackground)
        {
            let cubemap = image.cubemap;
            envMap.reflection = this._createReflection(cubemap);
            this._createSH(envMap.shCoefficients);
        }
        else
        {
            let cubemap = CreateCubeTexture(image);
            envMap.reflection = this._createReflection(cubemap);
            this._createSH(envMap.shCoefficients);
        }

        return envMap;
        
    }

}

