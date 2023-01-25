import { Document } from "./editor_document.js";
import { Clock } from "./utils/Clock.js";
import { view } from "./view.js";

function set_xml(xml)
{
    doc.reset();
    doc.load_xml(xml, "local");
    
    let rexml = doc.get_xml();
    print(rexml);
    
    return "";
}

function get_xml(x)
{
    return doc.get_xml();
}

function init(width, height)
{
    renderer = new GLRenderer();
    doc = new Document(view);
    clock = new Clock();
    
    message_map = { 
        "setXML": set_xml,
        "getXML": get_xml,
    };

}

function render(width, height, size_changed)
{
    if (size_changed)
	{
		doc.setSize(width, height);
	}
	let delta = clock.getDelta();
	doc.tick(delta);
	doc.render(renderer);

}

function message_handling(name, msg)
{
    if (name in message_map)
    {
        return message_map[name](msg);
    }
    return "";
}

setCallback('init', init);
setCallback('render', render);
setCallback('message', message_handling);



