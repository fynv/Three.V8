export function fix_anims(anims)
{
    for (let anim of anims)
    {        
        if ("translations" in anim)
        {
            let translations = anim.translations;
            for (let trans of translations)
            {
                trans.name = "mixamorig:"+trans.name;
            }
        }
        
        if ("rotations" in anim)
        {
            let rotations = anim.rotations;            
            for (let rot of rotations)
            {
                rot.name = "mixamorig:"+rot.name;                                
            }
        }
    }
}

export function load(doc)
{
	doc.load_xml_url("mp_doc_module.xml");
    let socket = io();
    socket.emit("identity", "");
    socket.on("identity", (identity)=>{
        const username = identity.username;

        let props = {
            src: identity.src_avatar, 
            name_idle: identity.name_idle,
            name_forward: identity.name_forward,
            name_backward: identity.name_backward,
            camera_position: "0.0, 2.0, -4.0",
            url_anim: identity.url_anim,
            fix_anims: "fix_anims"
        };
        const avatar = doc.create("avatar", props);

        const status = { username: username, state: 'unknown'};
        const update_avatar = (doc, avatar, delta) => 
        {                
            if (status.state!='idle' || avatar.state!='idle')
            {
                status.state = avatar.state;
                status.position = avatar.position;
                status.quaternion = avatar.quaternion;   
                socket.emit("avatar_status", status);
            }
        };            
        doc.set_tick(avatar, update_avatar);

        const users = {};

        const update_user_info = (user, info) =>{
            console.log(info);
            if (info.hasOwnProperty('state'))
            {
                if (user.state!=info.state)
                {
                    doc.Tags.character.set_state(doc, user, info.state);
                }
            }
            if (info.hasOwnProperty('position'))
            {
                user.position.set(info.position.x, info.position.y, info.position.z);
            }
            if (info.hasOwnProperty('quaternion'))
            {
                user.quaternion.set(info.quaternion._x, info.quaternion._y, info.quaternion._z, info.quaternion._w);
            }
            
        };

        socket.on("full_status", (info_users)=>{
            for (let _username in users)
            {
                const user = users[_username];
                if (!info_users.hasOwnProperty(_username))
                {
                    doc.remove(user);
                    delete users[_username];
                }
            }
            
            for (let _username in info_users)
            {
                const info = info_users[_username];                    
                if (_username!=username)
                {
                    if (users.hasOwnProperty(_username))
                    {
                        const user = users[_username];
                        update_user_info(user, info);
                    }
                    else
                    {
                        let props = {
                            src: info.src_avatar, 
                            name_idle: info.name_idle,
                            name_forward: info.name_forward,
                            name_backward: info.name_backward,
                            url_anim: info.url_anim,
                            fix_anims: "fix_anims"
                        };
                        const user = doc.create("character", props);                            
                        users[_username] = user;
                        update_user_info(user, info);
                    }
                }
            }
            
        });

        socket.on("avatar_status", (info)=>{
            if (users.hasOwnProperty(info.username))
            {
                const user = users[info.username];
                update_user_info(user, info);
            }

        });
        
    });

}
