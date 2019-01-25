void main(void) {
    
    gl_Position = vertex;
    float rad = vertex.w;
    if (CONSTRAD > -0.5) {
      gl_Position.w = CONSTRAD;
      rad = CONSTRAD;
    }
    
    // clipping
    if (any(notEqual(clipDat.xyz, vec3(0, 0, 0)))) {
        float od = dot(vertex.xyz, clipDat.xyz) - rad;
        if (od > clipDat.w) {
          gl_Position = vec4(1.0, 1.0, 1.0, 0.0);
        }
    }   
    
    if (COLTAB_SIZE > 0.0) {    
        float cid = MAX_COLV - MIN_COLV;    
        if (cid < 0.000001) {
            inColor = texture(colTab, 0.5 / COLTAB_SIZE);
        } else {
            cid = (color.r - MIN_COLV) / cid;
            cid = clamp(cid, 0.0, 1.0);
        
            cid *= (1.0 - 1.0 / COLTAB_SIZE);
            cid += 0.5 / COLTAB_SIZE;
        
            inColor = texture(colTab, cid);
        }
    } else {
        inColor = color;
    }
}