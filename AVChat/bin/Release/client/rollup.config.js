let lst_in = ['avchat.js'];

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

