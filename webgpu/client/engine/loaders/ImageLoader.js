export class ImageLoader
{
    constructor()
    {

    }

    async loadFile(url)
    {
        const img = document.createElement('img');
        img.src = url;
        await img.decode();
        return await createImageBitmap(img);
    }

    async loadCubeFromFile(urls)
    {
        let promises = [];    
        for (let i=0; i<6; i++)
        {
            promises.push(this.loadFile(urls[i]));
        }        
        let ret = [];
        for (let i=0; i<6; i++)
        {
            ret.push(await promises[i]);
        }
        return ret;
    }

}