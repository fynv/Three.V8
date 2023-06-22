import {ProbeGrid} from "../lights/ProbeGrid.js"

export class ProbeGridLoader
{
    constructor()
    {

    }

    async loadFile(url)
    {
        const response = await fetch(url);
        const arrBuf = await response.arrayBuffer();

        let probe_grid = new ProbeGrid();
        const view_float = new Float32Array(arrBuf);
        const view_int = new Int32Array(arrBuf);
        probe_grid.coverage_min.x = view_float[0];
        probe_grid.coverage_min.y = view_float[1];
        probe_grid.coverage_min.z = view_float[2];
        probe_grid.coverage_max.x = view_float[3];
        probe_grid.coverage_max.y = view_float[4];
        probe_grid.coverage_max.z = view_float[5];
        probe_grid.divisions.x = view_int[6];
        probe_grid.divisions.y = view_int[7];
        probe_grid.divisions.z = view_int[8];
        probe_grid.ypower = view_float[9];
        probe_grid.vis_res = view_int[10];
        probe_grid.pack_size = view_int[11];
        probe_grid.pack_res = view_int[12];

        let count = probe_grid.divisions.x * probe_grid.divisions.y * probe_grid.divisions.z;
        probe_grid.probe_data = new Float32Array(count*36);
        {
            let view_in = new Float32Array(arrBuf, 13*4, count*36);
            probe_grid.probe_data.set(view_in);
        }
                
        let pack_res = probe_grid.pack_res;
        probe_grid.visibility_data = new Uint16Array(pack_res*pack_res*2);
        {
            let view_in = new Uint16Array(arrBuf, 13*4 + count*36*4, pack_res*pack_res*2);
            probe_grid.visibility_data.set(view_in);
        }

        probe_grid.allocate_probes();

        return probe_grid;
    }


}

