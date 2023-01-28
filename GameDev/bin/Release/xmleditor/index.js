﻿import { Document } from "./editor_document.js";
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
    return "";
}

function pick_obj(key)
{
    doc.pick_obj(key);
    return "";
}

function tuning(args)
{
    let input = JSON.parse(args);
    doc.tuning(input);
    return "";
}

function generate(x)
{
    doc.generate();
    return "";
}

function create(args)
{
    let input = JSON.parse(args);
    let base_key = input.base_key;
    let tag = input.tag;
    doc.req_create(base_key, tag);
    return "";
}

function remove(key)
{
    doc.req_remove(key);
    return "";
}

function init(width, height)
{
    renderer = new GLRenderer();
    doc = new Document(view);
    clock = new Clock();
    
    message_map = { isModified, setXML, getXML, picking, pick_obj, tuning, generate, create, remove};
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



