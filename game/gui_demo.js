import { Vector3 } from "./math/Vector3.js";
import { OrbitControls } from "./controls/OrbitControls.js";
import { view, UIViewDispatcher } from "./view.js";

function httpGetAsync(url, is_text = false)
{
    return new Promise((resolve, reject) => {
        http.getAsync(url, is_text, (suc, data)=>
        {
            resolve(data);
        });
    });
}

function getImage(url)
{
    let data = http.get(url);
    return imageLoader.loadMemory(data);
}

async function getImageAsync(url)
{
    let data = await httpGetAsync(url);
    return imageLoader.loadMemory(data);
}

controls = null;

function ClearUI()
{
    UIManager.remove(ui_area);
}

function setupUI()
{
    if (!gamePlayer.hasFont("default"))
    {
        gamePlayer.createFontFromFile("default", "assets/fonts/NotoSansSC-Bold.otf");
    }
    
    ui_area = new UIArea();
    ui_area.setOrigin(15.0, 30.0);
    ui_area.setSize(320.0, 480.0);
    UIManager.add(ui_area);
    
    panel2 = new UIPanel();
    panel2.setOrigin(0.0, 0.0);
    panel2.setSize(300.0, 200.0);
    ui_area.add(panel2);
    
    {
        text_msg = new UIText();
        text_msg.text = "Accurate GUI Layout";
        text_msg.block = panel2;
        text_msg.setStyle({ "alignmentVertical": 0});
        text_msg.setOrigin(0.0, 30.0);    
        ui_area.add(text_msg); 
        
        edit = new UILineEdit();
        edit.setOrigin(50.0, 60.0);
        edit.block = panel2;
        edit.text = "你好ABC，Can you see me?";
        ui_area.add(edit);
        
        btn = new UIButton();
        btn.setOrigin(50.0, 120.0);
        btn.setSize(90.0, 40.0);
        btn.block = panel2;    
        btn.onClick = ClearUI;
        ui_area.add(btn);        
        {
        
            img = imageLoader.loadFile("assets/textures/ok.png");
            btn_img = new UIImage();
            btn_img.setImage(img);
            btn_img.block = btn;
            btn_img.setSize(30,30);
            ui_area.add(btn_img);
            
            btn_text = new UIText();
            btn_text.text = "OK";
            btn_text.block = btn;
            btn_text.setStyle({ "alignmentHorizontal": 0 });
            btn_text.setOrigin(40.0, 0.0); 
            btn.onLongPress = () =>
            {
                print("Long Press");
            }   
            ui_area.add(btn_text); 
        }
        
        btn2 = new UIButton();
        btn2.setOrigin(150.0, 120.0);
        btn2.setSize(110.0, 40.0);
        btn2.block = panel2;
        btn2.onClick = ClearUI;
        ui_area.add(btn2);
        {        
            img2 = imageLoader.loadFile("assets/textures/cancel.png");
            btn_img2 = new UIImage();
            btn_img2.setImage(img2);
            btn_img2.block = btn2;
            btn_img2.setSize(30,30);
            ui_area.add(btn_img2);
            
            btn_text2 = new UIText();
            btn_text2.text = "Cancel";
            btn_text2.block = btn2;
            btn_text2.setStyle({ "alignmentHorizontal": 0 });
            btn_text2.setOrigin(40.0, 0.0);
            ui_area.add(btn_text2);   
        }
    }
    
    sview = new UIScrollViewer();    
    sview.setOrigin(0.0, 220.0);
    sview.setSize(280.0, 240.0);    
    ui_area.add(sview); 
    {
        picture = new UIImage();
        picture.block = sview;
        {
            img3 = imageLoader.loadFile("assets/textures/uv-test-col.png");            
            picture.setImage(img3);
            picture.setSize(180,180);
        }        
        ui_area.add(picture);
        sview.setContentSize(200,400);
        
        btn3 = new UIButton();
        btn3.block = sview;
        btn3.setOrigin(20, 200);
        btn3.setSize(100.0, 40.0);
        ui_area.add(btn3);
        {
            btn_text3 = new UIText();
            btn_text3.text = "Test1";
            btn_text3.block = btn3;                       
            ui_area.add(btn_text3);   
        }
        
        btn4 = new UIButton();
        btn4.block = sview;
        btn4.setOrigin(20, 250);
        btn4.setSize(100.0, 40.0);
        ui_area.add(btn4);
        {
            btn_text4 = new UIText();
            btn_text4.text = "Test2";
            btn_text4.block = btn4;                       
            ui_area.add(btn_text4);   
        }
    }
}

async function init(width, height)
{
    renderer = new GLRenderer();
    scene = new Scene();
    
    camera = new PerspectiveCamera(45.0, width / height, 0.1, 100.0);
    camera.setPosition(0.0, 0.0, 7.0);
    
    directional_light = new DirectionalLight();
    directional_light.intensity = 4.0;
    directional_light.setPosition(5.0, 10.0, 5.0);
    directional_light.setShadow(true, 4096, 4096);
    directional_light.setShadowProjection(-10.0, 10.0, -10.0, 10.0, 0.0, 50.0);
    
    directional_light.diffuseThresh = 0.2*2.0;
    directional_light.diffuseHigh = 0.8 *2.0;
    directional_light.diffuseLow = 0.2 *2.0;
    directional_light.specularThresh = 0.2*2.0;
    directional_light.specularHigh = 0.8 *2.0;
    directional_light.specularLow = 0.2 *2.0;
    
    scene.add(directional_light);
    
    bg = new HemisphereBackground();   
    bg.setSkyColor(1.0, 1.0, 1.0);
    bg.setGroundColor(0.02843, 0.07819, 0.07819);
    scene.background = bg;
    
    envLight = new HemisphereLight();
    envLight.setSkyColor(1.0, 1.0, 1.0);
    envLight.setGroundColor(0.02843, 0.07819, 0.07819);
        
    envLight.diffuseThresh = 0.2*0.5;
    envLight.diffuseHigh = 0.8 *0.5;
    envLight.diffuseLow = 0.2 *0.5;
    scene.indirectLight = envLight;
    
    /*background = new CubeBackground();

    {
        let cube_img = new imageLoader.loadCubeFromFile(
        "textures/sky_cube_face0.jpg", "assets/textures/sky_cube_face1.jpg",
        "textures/sky_cube_face2.jpg", "assets/textures/sky_cube_face3.jpg",
        "textures/sky_cube_face4.jpg", "assets/textures/sky_cube_face5.jpg");        
        background.setCubemap(cube_img);                
        
        let envMapCreator = new EnvironmentMapCreator();
        envMap = envMapCreator.create(cube_img);    
    }
 
    scene.background = background;
    scene.indirectLight = envMap;*/
    
    box = new SimpleModel();
    box.name = "box";
    box.createBox(2.0, 2.0, 2.0);
    box.translateX(-1.5);

    let axis = new Vector3(1.0, 1.0, 0.0);
    axis.normalize();
    box.rotateOnAxis(axis, 1.0);
    {
        let img = imageLoader.loadFile("assets/textures/uv-test-bw.png");        
        box.setColorTexture(img);
    }    
    
    //box.setToonShading(1);
    scene.add(box);
    
    sphere = new SimpleModel();
    sphere.name = "sphere";
    sphere.createSphere(1.0);
    sphere.translateX(1.5);

    {
        let img = imageLoader.loadFile("assets/textures/uv-test-col.png");        
        sphere.setColorTexture(img);
    }
    
    sphere.metalness = 0.5;
    sphere.roughness = 0.5;
    //sphere.setToonShading(1);
    scene.add(sphere);
    
    ground = new SimpleModel();
    ground.createPlane(10.0, 10.0);    
    ground.translateY(-1.7);
    ground.rotateX(-3.1416*0.5);
    scene.add(ground);  
    
    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;
    
    setupUI();
}


function render(width, height, size_changed) 
{
    ui_area.scale = width / 360.0;
    
    if (size_changed) 
    {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }

    if (controls && controls.hasOwnProperty('update'))
    {
        controls.update();
    }
    
    renderer.render(scene, camera);
}

setCallback('init', init);
setCallback('render', render);

