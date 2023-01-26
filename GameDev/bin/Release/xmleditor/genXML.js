function genNode(node, level)
{
    let code = "";
    for(let i=0;i<level;i++) code+="\t";
    
    if (typeof node == "string")
    {
        code += `${node}\n`;
        return code;
    }
    
    let name = node.tagName;
    code += `<${name}`;
    
    let attributes = node.attributes;
    for(let att in attributes)
    {
        let value = attributes[att];
        code+=` ${att}=\"${value}\"`;
    }
    
    let children = node.children;
    
    if (children.length<1)
    {
        code += `/>\n`;
        return code;
    }
    else
    {
        code += ">\n";
    }
    
    for (let child of children)
    {
        code += genNode(child, level+1);
    }
    
    for(let i=0;i<level;i++) code+="\t";
    code += `</${name}>\n`;

    return code;    
}

export function genXML(nodes)
{
    let xml = "";
    for (let top of nodes)
    {
        if (top.tagName =="?xml")
        {
           let version = top.attributes.version;
           xml += `<?xml version=\"${version}\"?>\n`;
        }
        else
        {
            xml += genNode(top, 0);
        }
        
    }
    return xml;
    
}

