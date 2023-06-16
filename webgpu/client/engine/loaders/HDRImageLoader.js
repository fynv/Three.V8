import get_module from './HDRLoad.js'

export class HDRImage
{
    constructor(width, height)
    {
        this.width = width;
        this.height = height;
        this.array_rgb9e5 = new ArrayBuffer(width*height*4);
    }
}

export class HDRImageLoader
{
    constructor()
    {


    }
    
    async loadFile(url, flip_x = false)
    {
        const response = await fetch(url);
        const arrBuf = await response.arrayBuffer();
        
        if (this.module == null) 
        {
            this.module = await get_module();
        }

        let size = arrBuf.byteLength;        
        let data = this.module.ccall("alloc", "number", ["number"], [size]);
        this.module.HEAPU8.set(new Uint8Array(arrBuf), data);
        let ret = this.module.ccall("LoadMemory", "number", ["number", "number", "number"], [data, size, flip_x?1:0]);

        let buf_size = new Int32Array(2);
        let u8view_size = new Uint8Array(buf_size.buffer);
        u8view_size.set(new Uint8Array(this.module.HEAPU8.buffer, ret, 4*2));
        
        let width = buf_size[0];
        let height = buf_size[1];

        let image = new HDRImage(width, height);
        let u8view_rgbe = new Uint8Array(image.array_rgb9e5);
        u8view_rgbe.set(new Uint8Array(this.module.HEAPU8.buffer, ret + 4*2, width*height*4));               

        this.module.ccall("dealloc", null, ["number"], [data]);
        this.module.ccall("dealloc", null, ["number"], [ret]);

        return image;
        
    }

    async loadCubeFromFile(urls, flip_x = true)
    {
        let promises = [];    
        if (flip_x)
        {
            promises.push(this.loadFile(urls[1], true));
            promises.push(this.loadFile(urls[0], true));
            promises.push(this.loadFile(urls[2], true));
            promises.push(this.loadFile(urls[3], true));
            promises.push(this.loadFile(urls[4], true));
            promises.push(this.loadFile(urls[5], true));
        }
        else
        {
            promises.push(this.loadFile(urls[0], false));
            promises.push(this.loadFile(urls[1], false));
            promises.push(this.loadFile(urls[2], false));
            promises.push(this.loadFile(urls[3], false));
            promises.push(this.loadFile(urls[4], false));
            promises.push(this.loadFile(urls[5], false));
        }
        return await Promise.all(promises);
    }

}

