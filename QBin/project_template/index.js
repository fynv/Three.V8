import { Document } from "./document.js";
import { Clock } from "./utils/Clock.js";
import { view } from "./view.js";

import * as module from "./scene.js";

async function init(width, height) {
	renderer = new GLRenderer();
	doc = new Document(view);
	clock = new Clock();
	
	await doc.load(module);	
}

function render(width, height, size_changed) {
	if (size_changed)
	{
		doc.setSize(width, height);
	}
	let delta = clock.getDelta();
	doc.tick(delta);
	doc.render(renderer);
}

setCallback('init', init);
setCallback('render', render);

