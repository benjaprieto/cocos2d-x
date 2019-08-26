const char* csBlurVertical_vert = STRINGIFY(
                                              
attribute vec4 a_position;
attribute vec2 a_texCoord;

\n#ifdef GL_ES\n
varying mediump vec2 v_texCoord;
varying mediump vec2 v_blurTexCoords[14];
\n#else\n
varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];
\n#endif\n
                                            
//varying vec2 v_texCoord;
//varying vec2 v_blurTexCoords[14];

void main()
{
    gl_Position = CC_MVPMatrix * a_position;
    v_texCoord = a_texCoord;
    v_blurTexCoords[ 0] = v_texCoord + vec2(0.0, -0.028);
  //  v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -0.024);
    v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -0.020);
 //   v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -0.016);
    v_blurTexCoords[ 2] = v_texCoord + vec2(0.0, -0.012);
 //   v_blurTexCoords[ 5] = v_texCoord + vec2(0.0, -0.008);
    v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -0.004);
  //  v_blurTexCoords[ 7] = v_texCoord + vec2(0.0, 0.004);
    v_blurTexCoords[ 4] = v_texCoord + vec2(0.0, 0.008);
  //  v_blurTexCoords[ 9] = v_texCoord + vec2(0.0, 0.012);
    v_blurTexCoords[5] = v_texCoord + vec2(0.0, 0.016);
  //  v_blurTexCoords[11] = v_texCoord + vec2(0.0, 0.020);
    v_blurTexCoords[6] = v_texCoord + vec2(0.0, 0.024);
    v_blurTexCoords[7] = v_texCoord + vec2(0.0, 0.028);
}
);