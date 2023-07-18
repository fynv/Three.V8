export function load(doc)
{
	doc.load_xml_url("doc_module2.xml");
}

export function fix_anims(anims)
{
    for (let anim of anims)
    {        
        if ("translations" in anim)
        {
            let translations = anim.translations;
            for (let trans of translations)
            {
                trans.name = "mixamorig:"+trans.name;
            }
        }
        
        if ("rotations" in anim)
        {
            let rotations = anim.rotations;            
            for (let rot of rotations)
            {
                rot.name = "mixamorig:"+rot.name;                                
            }
        }
    }
}

