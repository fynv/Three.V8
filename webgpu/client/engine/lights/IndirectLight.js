export class ReflectionMap
{
    constructor()
    {
        this.texture = null;
    }
}

export class IndirectLight
{
    constructor()
    {
        this.diffuse_thresh = 0.2;
		this.diffuse_high = 0.8;
		this.diffuse_low = 0.2;

		this.specular_thresh = 0.2;
		this.specular_high = 0.8;
		this.specular_low = 0.2;

        this.reflection = null;
    }
}

