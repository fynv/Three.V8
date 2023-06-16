import { HDRImageLoader } from "./HDRImageLoader.js"
import { CreateHDRTexture } from "../renderers/GPUUtils.js"

export class LightmapLoader
{
    constructor()
    {


    }

    async fromHDR(url)
    {
        let hdr_loader = new HDRImageLoader();
        let img = await hdr_loader.loadFile(url);
        return CreateHDRTexture(img);
    }

}
