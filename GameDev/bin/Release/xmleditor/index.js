import { Document } from "./editor_document.js";
import { Clock } from "./utils/Clock.js";
import { view } from "./view.js";

function isModified(x)
{
    return JSON.stringify(doc.is_modified());
}

function setXML(xml)
{
    doc.reset();
    doc.load_xml(xml, "local");
    return "";
}

function getXML(x)
{
    return doc.get_xml();
}

function picking(state)
{
    let bstate = state=="on";
    doc.picking(bstate);
}

function init(width, height)
{
    renderer = new GLRenderer();
    doc = new Document(view);
    clock = new Clock();
    
    message_map = { isModified, setXML, getXML, picking };
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



