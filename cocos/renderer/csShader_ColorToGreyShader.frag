const char* csColorToGrey_frag = STRINGIFY(
                                              
//\n#ifdef GL_ES\n
//precision mediump float;
//\n#endif\n
                                           
\n#ifdef GL_ES\n
varying mediump vec2 v_texCoord;
\n#else\n
varying vec2 v_texCoord;
\n#endif\n

//varying vec2 v_texCoord;
//uniform sampler2D CC_Texture0;

void main()                             
{                                       
    vec4 normalColor = texture2D(CC_Texture0, v_texCoord);
    float grey = dot(normalColor.rgb, vec3(0.299, 0.587, 0.114));
    
    gl_FragColor = vec4(1.0, 0.0, 0.0, normalColor.a);
}
);
