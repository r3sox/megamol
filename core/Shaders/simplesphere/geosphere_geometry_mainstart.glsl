layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 MVinv;
uniform mat4 MVP;

uniform vec4 viewAttr; // TODO: check fragment position if viewport starts not in (0, 0)
uniform vec4 lpos;

#ifndef CALC_CAM_SYS
uniform vec3 camIn;
uniform vec3 camUp;
uniform vec3 camRight;
#endif // CALC_CAM_SYS

in vec4 inColor[1];

out vec4 color;
out vec4 objPos;
out vec4 camPos;
out vec4 light;
out float rad;
out float squarRad;

void main(void) {
    
    // remove the sphere radius from the w coordinates to the rad varyings
    vec4 inPos = gl_in[0].gl_Position;
    rad = inPos.w;
    squarRad = rad*rad;
    inPos.w = 1.0;

    // object pivot point in object space    
    objPos = inPos; // no w-div needed, because w is 1.0 (Because I know)

    // calculate cam position
    camPos = MVinv[3]; // (C) by Christoph
    camPos.xyz -= objPos.xyz; // cam pos to glyph space
    
    // calculate light position in glyph space
    // USE THIS LINE TO GET POSITIONAL LIGHTING
    //lpos = MVinv * gl_LightSource[0].position - objPos;
    // USE THIS LINE TO GET DIRECTIONAL LIGHTING
    light = MVinv*normalize(lpos);
    
    color = inColor[0];    
     
#ifdef HALO
    squarRad = (rad + HALO_RAD) * (rad + HALO_RAD);
#endif // HALO

