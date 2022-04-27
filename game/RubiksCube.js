class RubiksCube
{    
    constructor()
    {
        this.reset();
    }    
    
    reset()
    {
        this.map = Array.from(Array(6*9).keys());
        this.dirs = Array(6*9).fill(0); 
    }
    
    clone(input)
    {      
        this.map = [...input.map];
        this.dirs = [...input.dirs];
        return this;        
    }
    
    multiply(input)
    {
        let map_old = [...this.map];
        let dirs_old = [...this.dirs];
        
        for (let i = 0; i < 6 * 9; i++)
        {
            let j = input.map[i];
            let ddir = input.dirs[i];
            this.map[i] = map_old[j];
            this.dirs[i] = (dirs_old[j] + ddir) % 4;
        }
        return this;
    }
    
    divide(input)
    {
        let map_old = [...this.map];
        let dirs_old = [...this.dirs];
        
        for (let i = 0; i < 6 * 9; i++)
        {
            let j = input.map[i];
            let ddir = input.dirs[i];
            this.map[j] = map_old[i];
            this.dirs[j] = (dirs_old[i] + 4 - ddir) % 4;
        }
        return this;
    }
    
    
    static s_Initialize()
    {
        this.s_cube_o = new RubiksCube();
        let text = fileLoader.loadTextFile("bases.json");
        let base_data = JSON.parse(text);        
        
        this.s_bases = new Array(18);
        for (let i =0; i<18; i++)
        {
            this.s_bases[i] = new RubiksCube();
            this.s_bases[i].map = base_data[i].map;
            this.s_bases[i].dirs = base_data[i].dirs;
        }
        
        this.s_RCW = new RubiksCube();
        this.s_RCW.multiply(this.s_bases[0]);
        
        this.s_RCCW = new RubiksCube();
        this.s_RCCW.multiply(this.s_bases[5]);
        
        this.s_R2CW = new RubiksCube();
        this.s_R2CW.multiply(this.s_bases[0]);
        this.s_R2CW.multiply(this.s_bases[1]);
        
        this.s_R2CCW = new RubiksCube();
        this.s_R2CCW.multiply(this.s_bases[5]);
        this.s_R2CCW.multiply(this.s_bases[4]);
        
        this.s_LCW = new RubiksCube();
        this.s_LCW.multiply(this.s_bases[3]);
        
        this.s_LCCW = new RubiksCube();
        this.s_LCCW.multiply(this.s_bases[2]);
        
        this.s_L2CW = new RubiksCube();
        this.s_L2CW.multiply(this.s_bases[3]);
        this.s_L2CW.multiply(this.s_bases[4]);
        
        this.s_L2CCW = new RubiksCube();
        this.s_L2CCW.multiply(this.s_bases[2]);
        this.s_L2CCW.multiply(this.s_bases[1]);
        
        this.s_UCW = new RubiksCube();
        this.s_UCW.multiply(this.s_bases[6]);
        
        this.s_UCCW = new RubiksCube();
        this.s_UCCW.multiply(this.s_bases[11]);
        
        this.s_U2CW = new RubiksCube();
        this.s_U2CW.multiply(this.s_bases[6]);
        this.s_U2CW.multiply(this.s_bases[7]);
        
        this.s_U2CCW = new RubiksCube();
        this.s_U2CCW.multiply(this.s_bases[11]);
        this.s_U2CCW.multiply(this.s_bases[10]);
        
        this.s_DCW = new RubiksCube();
        this.s_DCW.multiply(this.s_bases[9]);
        
        this.s_DCCW = new RubiksCube();
        this.s_DCCW.multiply(this.s_bases[8]);
        
        this.s_D2CW = new RubiksCube();
        this.s_D2CW.multiply(this.s_bases[9]);
        this.s_D2CW.multiply(this.s_bases[10]);
        
        this.s_D2CCW = new RubiksCube();
        this.s_D2CCW.multiply(this.s_bases[8]);
        this.s_D2CCW.multiply(this.s_bases[7]);
        
        this.s_FCW = new RubiksCube();
        this.s_FCW.multiply(this.s_bases[12]);
        
        this.s_FCCW = new RubiksCube();
        this.s_FCCW.multiply(this.s_bases[17]);
        
        this.s_F2CW = new RubiksCube();
        this.s_F2CW.multiply(this.s_bases[12]);
        this.s_F2CW.multiply(this.s_bases[13]);
        
        this.s_F2CCW = new RubiksCube();
        this.s_F2CCW.multiply(this.s_bases[17]);
        this.s_F2CCW.multiply(this.s_bases[16]);
        
        this.s_BCW = new RubiksCube();
        this.s_BCW.multiply(this.s_bases[15]);
        
        this.s_BCCW = new RubiksCube();
        this.s_BCCW.multiply(this.s_bases[14]);
        
        this.s_B2CW = new RubiksCube();
        this.s_B2CW.multiply(this.s_bases[15]);
        this.s_B2CW.multiply(this.s_bases[16]);
        
        this.s_B2CCW = new RubiksCube();
        this.s_B2CCW.multiply(this.s_bases[14]);
        this.s_B2CCW.multiply(this.s_bases[13]);     
        
        this.s_op_map = {};
        this.s_op_map['R'] = [ this.s_RCW, this.s_RCCW ]; this.s_op_map['r'] = [ this.s_R2CW, this.s_R2CCW ];
        this.s_op_map['L'] = [ this.s_LCW, this.s_LCCW ]; this.s_op_map['l'] = [ this.s_L2CW, this.s_L2CCW ];
        this.s_op_map['U'] = [ this.s_UCW, this.s_UCCW ]; this.s_op_map['u'] = [ this.s_U2CW, this.s_U2CCW ];
        this.s_op_map['D'] = [ this.s_DCW, this.s_DCCW ]; this.s_op_map['d'] = [ this.s_D2CW, this.s_D2CCW ];
        this.s_op_map['F'] = [ this.s_FCW, this.s_FCCW ]; this.s_op_map['f'] = [ this.s_F2CW, this.s_F2CCW ];
        this.s_op_map['B'] = [ this.s_BCW, this.s_BCCW ]; this.s_op_map['b'] = [ this.s_B2CW, this.s_B2CCW ];
    }
    
    static parse_seq(seq)
    {
        let operations = [];
        let notes = [];
        let groups = [];
        let group_id = 0;
        let in_group = false;
        
        for (let i = 0; i < seq.length; i++)
        {
            let c = seq[i];
            let op = null;
            let note = "";

            if (this.s_op_map.hasOwnProperty(c))
            {
                if (i < seq.length - 1 && (seq[i + 1] == '\'' || seq[i + 1] == '`'))
                {
                    op = this.s_op_map[c][1];
                    note = c + "\'";
                }
                else
                {
                    op = this.s_op_map[c][0];
                    note = c;
                } 
          
                let count = 1;
                if (i < seq.length - 1 && seq[i + 1] == '2')
                {
                    count = 2;
                    i++;
                }

                if (!in_group) group_id++;

                for (let j = 0; j < count; j++)
                {
                    operations.push(op);
                    notes.push(note);
                    groups.push(group_id);
                }
            }
            else if (c == '(')
            {
                in_group = true;
                group_id++;
            }
            else if (c == ')')
            {
                in_group = false;
            }
        }
        
        return {
            operations,
            notes,
            groups            
        };
    }
    
    exec_seq(seq)
    {
        let res = RubiksCube.parse_seq(seq);
        for (let op of res.operations)
        {
            this.multiply(op);
        }
    }
}

export { RubiksCube };
