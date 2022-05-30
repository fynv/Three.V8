import { RubiksCube } from "./RubiksCube.js";
import { Vector3 } from "./math/Vector3.js";
import { Matrix4 } from "./math/Matrix4.js";
import { Quaternion } from "./math/Quaternion.js";
import { Clock } from "./utils/Clock.js";
import { OrbitControls } from "./controls/OrbitControls.js";

import { view } from "./view.js";

function reset_cube()
{    
    rubiks_cube = new RubiksCube();  
    rubiks_cube.exec_seq("U'F2UB2D'R2UF2L2D'F2L'R'F'DBDU2B2R2");
}

function init(width, height) 
{
    RubiksCube.s_Initialize();
    reset_cube();
    solve = RubiksCube.parse_seq("F'LF2'BUR'U'B2R'U2'R'URU2R'URUBU'B'B'UBUFU'F'U2F'UFU2F'U'FULU'L'U'B'UBU2FU'F'U'L'ULRU2R'U'RU'R'R2URUR'U'R'U'R'UR'U2");
    
    rotations = new Array(26);
    for (let i=0;i<26;i++)
    {
        rotations[i] = new Quaternion();
    }
    
    renderer = new GLRenderer();
    scene = new Scene();
    
    camera = new PerspectiveCamera(45.0, width / height, 1.0, 1000.0);
    camera.setPosition(30.0, 60.0, 90.0);
    
    ground = new SimpleModel();
    ground.createPlane(300.0, 300.0);    
    ground.setPosition(-100, -51, -88);
    ground.rotateX(-Math.PI*0.5);
    scene.add(ground);  
    
    directional_light = new DirectionalLight();
    directional_light.intensity = 4.0;
    directional_light.setPosition(6.0, 6.0*0.51, 6.0*0.88);     
    directional_light.setShadow(true, 4096, 4096);
    directional_light.setShadowProjection(-100.0, 100.0, -100.0, 100.0, 0.0, 400.0);
    scene.add(directional_light);
    
    background = new CubeBackground();

    {
        let cube_img = new imageLoader.loadCubeFromFile(
        "assets/textures/sky_cube_face0.jpg", "assets/textures/sky_cube_face1.jpg",
        "assets/textures/sky_cube_face2.jpg", "assets/textures/sky_cube_face3.jpg",
        "assets/textures/sky_cube_face4.jpg", "assets/textures/sky_cube_face5.jpg");        
        background.setCubemap(cube_img);                
        
        let envMapCreator = new EnvironmentMapCreator();
        envMap = envMapCreator.create(cube_img);    
    }
 
    scene.background = background;
    scene.indirectLight = envMap;
    
    cube =  gltfLoader.loadModelFromFile("../game/assets/models/cube.glb");
    scene.add(cube); 

    controls = new OrbitControls(camera, view);
    controls.enableDamping = true;        
}

const s_dirs = [ new Vector3(1.0, 0.0, 0.0), new Vector3(-1.0, 0.0, 0.0), new Vector3(0.0, 1.0, 0.0), new Vector3(0.0, -1.0, 0.0), new Vector3(0.0, 0.0, 1.0), new Vector3(0.0, 0.0, -1.0) ];
const s_dirs2 = [ 
     new Vector3( 0.0, 0.0, -1.0),  new Vector3( 0.0, -1.0, 0.0),  new Vector3( 0.0, 0.0, 1.0),  new Vector3( 0.0, 1.0, 0.0),
     new Vector3( 0.0, 0.0, 1.0),  new Vector3( 0.0, -1.0, 0.0), new Vector3(0.0, 0.0, -1.0),  new Vector3( 0.0, 1.0, 0.0),
     new Vector3( 1.0, 0.0, 0.0),  new Vector3( 0.0, 0.0, 1.0),  new Vector3(-1.0, 0.0, 0.0),  new Vector3( 0.0, 0.0, -1.0),
     new Vector3( 1.0, 0.0, 0.0), new Vector3( 0.0, 0.0, -1.0), new Vector3( -1.0, 0.0, 0.0), new Vector3( 0.0, 0.0, 1.0),
     new Vector3( 1.0, 0.0, 0.0), new Vector3( 0.0, -1.0, 0.0), new Vector3( -1.0, 0.0, 0.0), new Vector3( 0.0, 1.0, 0.0),
     new Vector3( -1.0, 0.0, 0.0), new Vector3( 0.0, -1.0, 0.0), new Vector3( 1.0, 0.0, 0.0), new Vector3( 0.0, 1.0, 0.0)
];

const ids_center = [ 31, 22, 13, 4, 40, 49 ];
const ids_edge = [[ 16, 30 ], [7, 32], [ 43, 28], [ 52, 34], [ 14, 39], [41, 3], [ 5, 48], [ 50, 12], [ 10, 21], [1, 23], [ 37, 25], [ 46, 19]];
const ids_corner = [[ 27, 17, 42 ], [ 29, 44, 6], [ 35, 8, 51 ], [ 33, 53, 15], [ 24, 36, 11 ], [ 26, 0, 38], [ 20, 45, 2], [ 18, 9, 47] ];
let inverse_rest_matrices = null;

function compute_inverse_rest_matrices()
{
    inverse_rest_matrices = new Array(26);
    
    for (let i=0; i<6; i++)
    {
        let face_in = ids_center[i];
        let dir1_in = s_dirs[ Math.floor(face_in / 9)];
        let dir2_in = s_dirs2[ Math.floor(face_in / 9) * 4];        
        let dir3_in = dir1_in.clone();
        dir3_in.cross(dir2_in);
        dir3_in.normalize();
        inverse_rest_matrices[i] = new Matrix4();
        inverse_rest_matrices[i].makeBasis(dir1_in, dir2_in, dir3_in);
        inverse_rest_matrices[i].invert();
    }
    
    for (let i=0; i<12; i++)
    {
        let face_in = ids_edge[i];
        let dir1_in = s_dirs[ Math.floor(face_in[0] / 9)];
        let dir2_in = s_dirs[ Math.floor(face_in[1] / 9)];
        let dir3_in = dir1_in.clone();
        dir3_in.cross(dir2_in);
        dir3_in.normalize();
        inverse_rest_matrices[i + 6] = new Matrix4();
        inverse_rest_matrices[i + 6].makeBasis(dir1_in, dir2_in, dir3_in);
        inverse_rest_matrices[i + 6].invert();
    }
    
    for (let i=0; i<8; i++)
    {
        let face_in = ids_corner[i];
        let dir1_in = s_dirs[ Math.floor(face_in[0] / 9)];
        let dir2_in = s_dirs[ Math.floor(face_in[1] / 9)];
        let dir3_in = s_dirs[ Math.floor(face_in[2] / 9)];
        inverse_rest_matrices[i + 18] = new Matrix4();
        inverse_rest_matrices[i + 18].makeBasis(dir1_in, dir2_in, dir3_in);
        inverse_rest_matrices[i + 18].invert();
    }
}
 
function set_transforms(op = "", progress = 0.0)
{
    if (inverse_rest_matrices== null)
    {
        compute_inverse_rest_matrices();
    }
    
    let reverse = new RubiksCube();
    reverse.divide(rubiks_cube);
    
    let moving_face = -1;
    let trans_moving = new Matrix4();
    
    if (op == "R")
    {
        moving_face = 0;
        trans_moving.makeRotationX(-Math.PI*0.5*progress);
    }
    else if (op == "R'")
    {
        moving_face = 0;
        trans_moving.makeRotationX(Math.PI*0.5*progress);
    }
    else if (op == "L")
    {
        moving_face = 1;
        trans_moving.makeRotationX(Math.PI*0.5*progress);
    }
    else if (op == "L'")
    {
        moving_face = 1;
        trans_moving.makeRotationX(-Math.PI*0.5*progress);
    }
    else if (op == "U")
    {
        moving_face = 2;
        trans_moving.makeRotationY(-Math.PI*0.5*progress);
    }
    else if (op == "U'")
    {
        moving_face = 2;
        trans_moving.makeRotationY(Math.PI*0.5*progress);
    }
    else if (op == "D")
    {
        moving_face = 3;
        trans_moving.makeRotationY(Math.PI*0.5*progress);
    }
    else if (op == "D'")
    {
        moving_face = 3;
        trans_moving.makeRotationY(-Math.PI*0.5*progress);
    }
    else if (op == "F")
    {
        moving_face = 4;
        trans_moving.makeRotationZ(-Math.PI*0.5*progress);
    }
    else if (op == "F'")
    {
        moving_face = 4;
        trans_moving.makeRotationZ(Math.PI*0.5*progress);
    }
    else if (op == "B")
    {
        moving_face = 5;
        trans_moving.makeRotationZ(Math.PI*0.5*progress);
    }
    else if (op == "B'")
    {
        moving_face = 5;
        trans_moving.makeRotationZ(-Math.PI*0.5*progress);
    }
   
    let mat_out = new Matrix4();
    
    for (let i=0; i<6; i++)
    {
        let mat_in = inverse_rest_matrices[i];
        
        let face_in = ids_center[i];
        let face_out = reverse.map[face_in];
        let dir1_out = s_dirs[ Math.floor(face_out / 9)];
        let dir2_out = s_dirs2[ Math.floor(face_out / 9) * 4 + reverse.dirs[face_out]];
        let dir3_out = dir1_out.clone();
        dir3_out.cross(dir2_out);
        dir3_out.normalize();
        mat_out.makeBasis(dir1_out, dir2_out, dir3_out);
        
        mat_out.multiply(mat_in);        
        if (Math.floor(face_out/9) == moving_face)
        {
            mat_out.premultiply(trans_moving);
        }        
        rotations[i].setFromRotationMatrix(mat_out);
    }
    
    for (let i=0; i<12; i++)
    {
        let mat_in = inverse_rest_matrices[i+6];
        
        let face_in = ids_edge[i];
        let face_out = [reverse.map[face_in[0]], reverse.map[face_in[1]]];
        let dir1_out = s_dirs[Math.floor(face_out[0] / 9)];
        let dir2_out = s_dirs[Math.floor(face_out[1] / 9)];
        let dir3_out = dir1_out.clone();
        dir3_out.cross(dir2_out);
        dir3_out.normalize();
        mat_out.makeBasis(dir1_out, dir2_out, dir3_out);        

        mat_out.multiply(mat_in);        
        if (Math.floor(face_out[0] /9) == moving_face || Math.floor(face_out[1]/9) == moving_face)
        {
            mat_out.premultiply(trans_moving);
        }        
        rotations[i+6].setFromRotationMatrix(mat_out);
    }
    
    for (let i=0; i<8; i++)
    {
        let mat_in = inverse_rest_matrices[i+18];
        
        let face_in = ids_corner[i];
        let face_out = [reverse.map[face_in[0]], reverse.map[face_in[1]], reverse.map[face_in[2]]];
        let dir1_out = s_dirs[Math.floor(face_out[0] / 9)];
        let dir2_out = s_dirs[Math.floor(face_out[1] / 9)];
        let dir3_out = s_dirs[Math.floor(face_out[2] / 9)];
        mat_out.makeBasis(dir1_out, dir2_out, dir3_out);        

        mat_out.multiply(mat_in);        
        if (Math.floor(face_out[0] /9) == moving_face || Math.floor(face_out[1]/9) == moving_face || Math.floor(face_out[2]/9) == moving_face)
        {
            mat_out.premultiply(trans_moving);
        }
        rotations[i+18].setFromRotationMatrix(mat_out);
    }
    
    let rotation_tracks = [];
    for (let i=0; i<26; i++)
    {        
        name = "Cube."+String(i).padStart(3, '0');
        rot = {
            name: name,
            rotation: rotations[i]
        };
        rotation_tracks.push(rot);
    }
    
    let frame = 
    {
        rotations: rotation_tracks
    };
    cube.setAnimationFrame(frame);
}

let rotating = false;
let pos = 0;
let prog = 0.0;
const speed = 3.0;
let t0 = now()/1000.0;

function render(width, height, size_changed) {    
    if (size_changed) {
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    }    

    if (controls.hasOwnProperty('update'))
    {
        controls.update();
    }
    
    set_transforms(solve.notes[pos], prog);
    
    renderer.render(scene, camera);
      
    if (rotating)
    {
        let t1 = now()/1000.0;
        let dt = t1-t0;
        t0 = t1;
        
        prog += speed * dt;
        while (prog > 1.0)
        {
            prog -= 1.0;
            rubiks_cube.multiply(solve.operations[pos]);
            pos++;
            if (pos >= solve.operations.length)
            {
                rotating = false;              
            }            
        }
    }
    else
    {
        let t1 = now()/1000.0;
        if (t1 - t0 > 3.0)
        {
            pos = 0;
            reset_cube();
            rotating = true;
            t0 = t1;
        }
        
    }
}

setCallback('init', init);
setCallback('render', render);
