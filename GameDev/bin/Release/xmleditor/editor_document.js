import { OrbitControls } from "./controls/OrbitControls.js";
import { Vector3 } from "./math/Vector3.js";
import { view } from "./view.js";

import * as txml from "./txml.js";
import { genXML } from "./genXML.js";

function string_to_boolean(string) {
	switch (string.toLowerCase().trim()) {
		case "true":
		case "yes":
		case "1":
			return true;

		case "false":
		case "no":
		case "0":
		case null:
			return false;

		default:
			return Boolean(string);
	}
}

// Tags
const scene = {
	reset: (doc) => {
		doc.scene = new Scene();
	},
	create: async (doc, props, mode, parent) => {
		doc.scene = new Scene();
		return doc.scene;
	}
}


const camera = {
	reset: (doc) => {
		doc.camera = new PerspectiveCamera(45, doc.width / doc.height, 0.1, 100);
		doc.camera.setPosition(0, 1.5, 0);
	},

	create: async (doc, props, mode, parent) => {
		const fov = parseFloat(props.fov);
		const near = parseFloat(props.near);
		const far = parseFloat(props.far);
		doc.camera = new PerspectiveCamera(fov, doc.width / doc.height, near, far);
		return doc.camera;
	}
}

const control = {
	reset: (doc) => {
		if (doc.controls)
			doc.controls.dispose();
		doc.controls = new OrbitControls(doc.camera, doc.view);
		doc.controls.enableDamping = true;
		doc.controls.target.set(0, 1.5, -1);
	},
	create: async (doc, props, mode, parent) =>{
		const type = props.type;
		if (type == 'orbit') {
			const look_from = props.look_from.split(',');
			const look_at = props.look_at.split(',');
			const from_x = parseFloat(look_from[0]);
			const from_y = parseFloat(look_from[1]);
			const from_z = parseFloat(look_from[2]);
			const to_x = parseFloat(look_at[0]);
			const to_y = parseFloat(look_at[1]);
			const to_z = parseFloat(look_at[2]);
			doc.camera.setPosition(from_x, from_y, from_z);
			if (doc.controls != null)
				doc.controls.dispose();
			doc.controls = new OrbitControls(doc.camera, doc.view);
			doc.controls.enableDamping = true;
			doc.controls.target.set(to_x, to_y, to_z);
		}
	}
}

const fog = {
	create: async (doc, props, mode, parent) =>{		
		doc.scene.fog = new Fog();
		if (props.hasOwnProperty("density"))
		{
			doc.scene.fog.density = parseFloat(props.density);
		}
		return doc.scene.fog;
	}
}


const sky = {
    reset: (doc) => {
		let bg = new HemisphereBackground();   
        bg.setSkyColor(0.318, 0.318, 0.318);
        bg.setGroundColor(0.01, 0.025, 0.025);
        doc.scene.background = bg;
	},
	create: async (doc, props, mode, parent) => {
		const type = props.type;
		let create_env_light = true;
		if (props.hasOwnProperty("create_env_light"))
		{
			create_env_light = string_to_boolean(props.create_env_light);
		}
		
		if (type == "uniform")
		{
			let bg = new ColorBackground();
			let envLight = null;
			if (create_env_light) envLight = new AmbientLight();
			
			if (props.hasOwnProperty('color'))
			{
				const color = props.color.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				bg.setColor(r,g,b);
				if (create_env_light)
				{
					envLight.setColor(r,g,b);
				}
			}
			doc.scene.background = bg;
			if (create_env_light)
			{
				doc.scene.indirectLight = envLight;
			}
		}
		else if (type == "hemisphere")
		{
			let bg = new HemisphereBackground();   
			let envLight = null;
			if (create_env_light) envLight = new HemisphereLight();
			
			if (props.hasOwnProperty('skyColor'))
			{
				const color = props.skyColor.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				bg.setSkyColor(r,g,b);
				if (create_env_light)
				{
					envLight.setSkyColor(r,g,b);
				}
			}
			
			if (props.hasOwnProperty('groundColor'))
			{
				const color = props.groundColor.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				bg.setGroundColor(r,g,b);				
				envLight.setGroundColor(r,g,b);
			}
			
			doc.scene.background = bg;
			if (create_env_light)
			{
				doc.scene.indirectLight = envLight;
			}
		}
		else if (type == "cube")
		{
			const url = props.path;
			const arr_images = [url+"/"+props.posx, url+"/"+props.negx, url+"/"+props.posy, url+"/"+props.negy, url+"/"+props.posz, url+"/"+props.negz];

			let cube_img = imageLoader.loadCubeFromFile(
				url+"/"+props.posx, url+"/"+props.negx, 
				url+"/"+props.posy, url+"/"+props.negy, 
				url+"/"+props.posz, url+"/"+props.negz);

			let bg = new CubeBackground();
			bg.setCubemap(cube_img);
			doc.scene.background = bg;
			
			if(create_env_light)
			{
				let envMapCreator = new EnvironmentMapCreator();
				let envLight = envMapCreator.create(cube_img);
				doc.scene.indirectLight = envLight;
			}
		}
		
		return doc.scene.background;
	}
}

const env_light = {
	create: async (doc, props, mode, parent) => {
		const type = props.type;
		
		if (type == "uniform")
		{
			let envLight = new AmbientLight();			
			if (props.hasOwnProperty('color'))
			{
				const color = props.color.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				envLight.setColor(r,g,b);
			}
			doc.scene.indirectLight = envLight;
		}
		else if (type == "hemisphere")
		{
			let envLight = new HemisphereLight();
			
			if (props.hasOwnProperty('skyColor'))
			{
				const color = props.skyColor.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				envLight.setSkyColor(r,g,b);
			}
			
			if (props.hasOwnProperty('groundColor'))
			{
				const color = props.groundColor.split(',');
				const r = parseFloat(color[0]);
				const g = parseFloat(color[1]);
				const b = parseFloat(color[2]);
				envLight.setGroundColor(r,g,b);
			}			
			doc.scene.indirectLight = envLight;
		}
		else if (type == "cube")
		{
			const url = props.path;
			const arr_images = [url+"/"+props.posx, url+"/"+props.negx, url+"/"+props.posy, url+"/"+props.negy, url+"/"+props.posz, url+"/"+props.negz];

			let cube_img = imageLoader.loadCubeFromFile(
				url+"/"+props.posx, url+"/"+props.negx, 
				url+"/"+props.posy, url+"/"+props.negy, 
				url+"/"+props.posz, url+"/"+props.negz);

			
			let envMapCreator = new EnvironmentMapCreator();
			let envLight = envMapCreator.create(cube_img);
			doc.scene.indirectLight = envLight;
		}
		
		return doc.scene.indirectLight;
	}
}

const group = {
	create: async (doc, props, mode, parent) => {
		const group = new Object3D();
		if (props.hasOwnProperty('name')) {
			group.name = props.name;
		}
		if (parent != null) {
			parent.add(group);
		}
		else {
			doc.scene.add(group);
		}
		return group;
	}
}

const plane = {
	create: async (doc, props, mode, parent) => {
		const size = props.size.split(',');
		const width = parseFloat(size[0]);
		const height = parseFloat(size[1]);
				
		const plane = new SimpleModel();
		plane.createPlane(width, height);
		
		if (props.hasOwnProperty('name')) {
			plane.name = props.name;
		}

		if (parent != null) {
			parent.add(plane);
		}
		else {
			doc.scene.add(plane);
		}
		return plane;
	}
}


const box = {
	create: async (doc, props, mode, parent) => {
		const size = props.size.split(',');
		const width = parseFloat(size[0]);
		const height = parseFloat(size[1]);
		const depth = parseFloat(size[2]);
		
		const box = new SimpleModel();
		box.createBox(width, height, depth);		
		
		if (props.hasOwnProperty('name')) {
			box.name = props.name;
		}

		if (parent != null) {
			parent.add(box);
		}
		else {
			doc.scene.add(box);
		}
		return box;
	}
}

const sphere = {
	create: async (doc, props, mode, parent) => {
		const radius = parseFloat(props.radius);
		const widthSegments = parseInt(props.widthSegments);
		const heightSegments = parseInt(props.heightSegments);
		
		const sphere = new SimpleModel();
		sphere.createSphere(radius, widthSegments, heightSegments);
	
		if (props.hasOwnProperty('name')) {
			sphere.name = props.name;
		}
		if (parent != null) {
			parent.add(sphere);
		}
		else {
			doc.scene.add(sphere);
		}
		return sphere;
	}
}

const model = {
	create: async (doc, props, mode, parent) => {
		const url = props.src;
		const model = gltfLoader.loadModelFromFile(url);
		if (props.hasOwnProperty("name"))
		{
			model.name = props.name;
		}
		if (parent != null) {
			parent.add(model);
		}
		else {
			doc.scene.add(model);
		}
		return model;
	}
}

const directional_light = {
	create: async (doc, props, mode, parent) => {
		const light = new DirectionalLight();			
		
		if (props.hasOwnProperty('intensity')) {
			light.intensity = parseFloat(props.intensity);
		}		
		
		if (props.hasOwnProperty('castShadow') && string_to_boolean(props.castShadow))
		{
			let width = 512;
			let height = 512;
			if (props.hasOwnProperty('size')) {
				const size = props.size.split(',');
				width = parseInt(size[0]);
				height = parseInt(size[1]);
			}
			light.setShadow(true, width, height);
			
			if (props.hasOwnProperty('area')) {
				const area = props.area.split(',');
				let left = parseFloat(area[0]);
				let right = parseFloat(area[1]);
				let top = parseFloat(area[2]);
				let bottom = parseFloat(area[3]);		
				let near = parseFloat(area[4]);
				let far = parseFloat(area[5]);
				light.setShadowProjection(left, right, top, bottom, near, far);
			}
		}

		if (parent != null) {
			parent.add(light);
		}
		else {
			doc.scene.add(light);
		}
		return light;
	}
}


export class Document
{
	constructor(view)
	{
		this.view = view;
		this.width = view.clientWidth;
		this.height = view.clientHeight;
		this.Tags = { scene, camera, fog, sky, env_light, control, group, plane, box, sphere, model, directional_light };
		this.reset();
	}
	
	setSize(width, height)
	{
		this.width = width;
		this.height = height;
		
		if (this.camera)
		{
			this.camera.aspect = width / height;
			this.camera.updateProjectionMatrix();
		}
	}

    reset() 
	{
	    this.saved_text = "";
		for (let tag in this.Tags) 
		{
			if (this.Tags[tag].hasOwnProperty('reset')) 
			{
				this.Tags[tag].reset(this);
			}
		}
	}
	
	tick(delta)
	{
		if (this.controls)
		{
			if (this.controls.hasOwnProperty('update'))
			{
				this.controls.update();
			}
		}
	}
	
	render(renderer)
	{
		
		if (this.scene && this.camera) 
		{
			renderer.render(this.scene, this.camera);
		}
	}
	
	
	async create(tag, props, mode, parent = null) 
	{
	    if (!(tag in this.Tags)) return null;
	    
		const obj = await this.Tags[tag].create(this, props, mode, parent);
		
		if (props.hasOwnProperty('position')) 
		{
			const position = props.position.split(',');
			obj.setPosition(parseFloat(position[0]), parseFloat(position[1]), parseFloat(position[2]));
		}

		if (props.hasOwnProperty('rotation')) 
		{
			const rotation = props.rotation.split(',');
			obj.setRotation(parseFloat(rotation[0])* Math.PI / 180.0, parseFloat(rotation[1])* Math.PI / 180.0, parseFloat(rotation[2])* Math.PI / 180.0);		
		}

		if (props.hasOwnProperty('scale')) 
		{
			const scale = props.scale.split(',');
			obj.setScale(parseFloat(scale[0]), parseFloat(scale[1]), parseFloat(scale[2]));
		}

		if (props.hasOwnProperty('color')) 
		{
			const color = props.color.split(',');
			const r = parseFloat(color[0]);
			const g = parseFloat(color[1]);
			const b = parseFloat(color[2]);
			obj.setColor(r,g,b);
		}
		
		if (props.hasOwnProperty('texture'))
		{
			let img = imageLoader.loadFile(props.texture);
			obj.setColorTexture(img);
		}
		
		return obj;
	}
	
	remove(obj)
	{
		if (obj.parent) 
		{
			obj.parent.remove(obj);
		}
	}
	
	async load_xml_node(xmlNode, mode, parent = null)
	{
		if (parent = null) {
			parent = this;
		}
		for (let child of xmlNode.children) {			
			const obj = await this.create(child.tagName, child.attributes, mode, parent);
			if (obj===null) continue;
			await this.load_xml_node(child, mode, obj);
		}
	}
	
	async load_xml(xmlText, mode)
	{
		this.xml_nodes = txml.parse(xmlText, {keepComments: true});
		let root = null;
		for (let top of this.xml_nodes)
		{
			if (top.tagName == 'document')
			{
				root = top;
				break;
			}
		}
		if (root)
		{
			await this.load_xml_node(root, mode);
		}
		this.saved_text = genXML(this.xml_nodes);
	}
	
	is_modified()
	{
	    let gen_xml = genXML(this.xml_nodes);
	    return gen_xml != this.saved_text;
	}
	
	get_xml()
	{
	    this.saved_text = genXML(this.xml_nodes);
	    return this.saved_text;
	}

}





