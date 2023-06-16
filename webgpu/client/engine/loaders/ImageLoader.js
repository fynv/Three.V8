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
        let bitmap = await createImageBitmap(img, { premultiplyAlpha: "none"});        
        return bitmap;
    }

    async loadCubeFromFile(urls)
    {
        let promises = [];    
        for (let i=0; i<6; i++)
        {
            promises.push(this.loadFile(urls[i]));
        }  
        return await Promise.all(promises);
    }

}