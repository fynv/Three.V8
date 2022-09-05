let lst_in = ['game.js', 'game2.js', 'game_cube.js', 'game_materials.js', 'gui_demo.js'];

let lst = [];
for (let fn_in of lst_in)
{
    lst.push({
        input:  fn_in,
        output: {
            file: 'bundle_' + fn_in
        }
    });
}

export default lst;

