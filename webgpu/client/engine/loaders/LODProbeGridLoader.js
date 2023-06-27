import {LODProbeGrid} from "../lights/LODProbeGrid.js"

export class LODProbeGridLoader
{
    constructor()
    {

    }

    async loadFile(url)
    {
        const response = await fetch(url);
        const arrBuf = await response.arrayBuffer();

        let probe_grid = new LODProbeGrid();
        const view_float = new Float32Array(arrBuf);
        const view_int = new Int32Array(arrBuf);
        probe_grid.coverage_min.x = view_float[0];
        probe_grid.coverage_min.y = view_float[1];
        probe_grid.coverage_min.z = view_float[2];
        probe_grid.coverage_max.x = view_float[3];
        probe_grid.coverage_max.y = view_float[4];
        probe_grid.coverage_max.z = view_float[5];
        probe_grid.base_divisions.x = view_int[6];
        probe_grid.base_divisions.y = view_int[7];
        probe_grid.base_divisions.z = view_int[8];
        probe_grid.sub_division_level = view_int[9];

        let num_probes = view_int[10];
        let num_indices = view_int[11];
        probe_grid.vis_res = view_int[12];
        probe_grid.pack_size = view_int[13];
        probe_grid.pack_res = view_int[14];

        probe_grid.probe_data = new Float32Array(num_probes*40);
        {
            let view_in = new Float32Array(arrBuf, 15*4, num_probes*40);
            probe_grid.probe_data.set(view_in);
        }

        probe_grid.sub_index = new Int32Array(num_indices*4);
        {
            let view_in = new Int32Array(arrBuf, 15*4 + num_probes*40 * 4, num_indices);
            probe_grid.sub_index.set(view_in);
        }

        let pack_res = probe_grid.pack_res;
        probe_grid.visibility_data = new Uint16Array(pack_res*pack_res*2);
        {
            let view_in = new Uint16Array(arrBuf, 15*4 + num_probes*40 * 4 + num_indices*4, pack_res*pack_res*2);
            probe_grid.visibility_data.set(view_in);
        }

        probe_grid.allocate_probes();

        return probe_grid;

    }

}