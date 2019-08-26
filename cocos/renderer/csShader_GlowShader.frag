const char* csGlow_frag = STRINGIFY(
                                              
//\n#ifdef GL_ES\n
//precision mediump float;
//\n#endif\n

\n#ifdef GL_ES\n
varying mediump vec2 v_texCoord;
varying mediump vec2 v_blurTexCoords[14];
\n#else\n
varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];
\n#endif\n
                                    
//varying vec2 v_texCoord;
//varying vec2 v_blurTexCoords[14];
//uniform sampler2D CC_Texture0;

void main()
{
    gl_FragColor = vec4(0.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 0])*(0.0044299121055113265*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 1])*(0.00895781211794*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 2])*(0.0215963866053*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 3])*(0.0443683338718*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 4])*(0.0776744219933);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 5])*(0.115876621105);
    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 6])*(0.147308056121);
    gl_FragColor += texture2D(CC_Texture0, v_texCoord)*(0.159576912161);
    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 7])*(0.147308056121);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 8])*(0.115876621105);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[ 9])*(0.0776744219933);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[10])*(0.0443683338718*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[11])*(0.0215963866053*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[12])*(0.00895781211794*2.0);
//    gl_FragColor += texture2D(CC_Texture0, v_blurTexCoords[13])*(0.0044299121055113265*2.0);
}
);