
struct MeshShaderParams
{
    vec4 glpyh_position;
    vec4 probe_direction;
    float scale;

    float min_value;
    float max_value;

    float sample_cnt;
    float samples[32];

    int probe_id;
    int state;
    int padding0;
    int padding1;
};
