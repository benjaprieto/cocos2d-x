const char* csTint_frag = STRINGIFY(
                                              
//\n#ifdef GL_ES\n
//precision mediump float;
//\n#endif\n
                                    
//\n#ifdef GL_ES\n
//varying lowp vec4 v_fragmentColor;
//varying mediump vec2 v_texCoord;
//\n#else\n
//varying vec2 v_texCoord;
//\n#endif\n

                                    
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n
                                    
varying vec4 v_fragmentColor;
//varying vec2 TextureCoordOut;
varying vec2 v_texCoord;
                                    
//varying vec2 v_texCoord;
//uniform sampler2D CC_Texture0;
uniform float strength;  
uniform vec3 u_tintColor;               
uniform float alpha;
                                    
void main()                             
{
    vec4 normalColor = texture2D(CC_Texture0, v_texCoord).rgba;
//I don't know why this if statement is here, uncomment it if you see weird tinting behaviour 
//    if (normalColor.a < 1.0)
        gl_FragColor = vec4((normalColor.rgb+(u_tintColor.rgb*normalColor.a))*alpha, normalColor.a*alpha);
//    else
//        gl_FragColor = vec4(normalColor.rgb+u_tintColor, 1);
    
    gl_FragColor = vec4((normalColor.rgb+(u_tintColor.rgb*normalColor.a)), normalColor.a);
}                                       
);