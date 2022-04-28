/**
 * MegaMol
 * Copyright (c) 2022, MegaMol Dev Team
 * All rights reserved.
 */

vec3 line_dir[3] = {
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
    };

out vec4 obj_pos; //
out vec3 cam_pos; //
out mat3 rotate_world_into_tensor; //
out vec3 absradii;

out vec4 vert_color;
out flat vec3 dir_color;

// this re-activates the old code that draws the whole box and costs ~50% performance for box glyphs
// note that you need the single-strip-per-box instancing call in the Renderer for this (14 * #glyphs)
// the new code uses 4 * (3 * #glyphs)
//#define UNOPTIMIZED

void main() {

    uint inst = gl_InstanceID / 3;
    uint line_inst = gl_InstanceID % 3;
    uint vertex = gl_VertexID % 2;
    vec3 normal = line_dir[line_inst];

    // get the current glyphs (center) position
    obj_pos = vec4(pos_array[inst].x, pos_array[inst].y, pos_array[inst].z, 1.0);

    // get the current glyphs orientation (represented by its quaternion)
    vec4 quat_c = vec4(quat_array[inst].x, quat_array[inst].y, quat_array[inst].z, quat_array[inst].w); //quat[inst];

    // from the glyphs quaternion, calculate the rotation tensor
    rotate_world_into_tensor = quaternion_to_matrix(quat_c);
    mat3 rotate_points = transpose(rotate_world_into_tensor);

    // all about the glyphs radii
    bool ignore_radii = (options & OPTIONS_IGNORE_RADII) > 0;
    vec3 radii = vec3(rad_array[inst].x, rad_array[inst].y, rad_array[inst].z); //rad[inst];

    if (ignore_radii) {
        radii = vec3(1.0);
    } else {
        // need a minimum radius to avoid rendering artifacts
        radii.x = radii.x < min_radius ? min_radius : radii.x;
        radii.y = radii.y < min_radius ? min_radius : radii.y;
        radii.z = radii.z < min_radius ? min_radius : radii.z;
    }
    absradii = abs(radii) * scaling;


    // shift, rotate, and scale cam
    cam_pos = rotate_world_into_tensor * (cam.xyz - obj_pos.xyz);

    vec3 transformed_normal = rotate_points * normal;

#ifdef UNOPTIMIZED
#else
    // if our cube face looks away from the camera, we need to replace it with the opposing one
    vec3 view_vec = cam.xyz - obj_pos.xyz;
    if (dot(view_vec, transformed_normal) < 0) {
        normal = -normal;
    }
#endif

    // color stuff
    // TODO: rework
    vec3 dir_color1 = max(vec3(0), normal * sign(radii));
    vec3 dir_color2 = vec3(1) + normal * sign(radii);
    dir_color = any(lessThan(dir_color2, vec3(0.5))) ? dir_color2 * vec3(0.5) : dir_color1;

    uint flag = FLAG_ENABLED;
    bool flags_available = (options & OPTIONS_USE_FLAGS) > 0;
    if (flags_available) {
        flag = flags_array[(flag_offset + inst)];
    }

    vec4 col = vec4(col_array[inst].x, col_array[inst].y, col_array[inst].z, col_array[inst].w);

    bool use_per_axis_color = (options & OPTIONS_USE_PER_AXIS_COLOR) > 0;
    if(use_per_axis_color){
        //col.r = face == 0 ? radii.x : face == 1 ? radii.y : radii.z;
        col.r = 0;//face == 0 ? radii.x : face == 1 ? radii.y : radii.z;
    }

    vert_color = compute_color(col, flag, tf_texture, tf_range, global_color, flag_selected_col, flag_softselected_col, options);


    // scale and rotate corner_pos
    //corner_pos.xyz *= absradii;
    //corner_pos.xyz = rotate_points * corner_pos.xyz;

    // corners relative to world space glyph positions
    //vec4 ws_pos = obj_pos + corner_pos;
    vec4 ws_pos = obj_pos;

    gl_Position =  mvp * ws_pos;

    bool clip_available = (options & OPTIONS_USE_CLIP) > 0;
    if (clip_available) {
        vec3 plane_normal = clip_data.xyz;
        float dist = dot(plane_normal, ws_pos.xyz) + clip_data.w;
        gl_ClipDistance[0] = dist;
    }

    if (flags_available) {
        if (!bitflag_isVisible(flag)) {
            gl_ClipDistance[0] = -1.0;
        } else {
            gl_ClipDistance[0] = 1.0;
        }
    }
}
