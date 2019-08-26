const char* csGreyToColor_frag = STRINGIFY(

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
uniform float strength;               
varying vec4 v_fragmentColor;       

void main()                             
{                                       
    vec4 normalColor = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    if (strength == 0.0)    
    {
        gl_FragColor = normalColor;
        return;
    }
    float color = (normalColor.r+normalColor.g+normalColor.b)/3.0;
    if (color > 0.99)
    {
        gl_FragColor = vec4(normalColor.rgb, 0.0);
        return;
    }
    vec4 greyColor = vec4(color, color, color, 1);
    
    vec4 newColor = mix(normalColor, greyColor, strength);
    gl_FragColor = vec4(newColor.rgb, normalColor.a - (color * 0.1));
}
);
