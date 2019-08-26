const char* csForegroundImgMask_frag = STRINGIFY(
                                              
\n#ifdef GL_ES\n
precision mediump float;               
\n#endif\n

varying vec4 v_fragmentColor;  
varying vec2 v_texCoord;      

void main() 
{

    vec4 texColor = texture2D(CC_Texture0,v_texCoord);
    vec4 maskColor = texture2D(CC_Texture1,v_texCoord);
   
    if(maskColor.r > 0.95 && maskColor.g > 0.95 && maskColor.b > 0.95)
    {
        discard;
    }

    float colorSum = (1.0 - (maskColor.r + maskColor.g + maskColor.b)/3.0);
    texColor.r = (texColor.r - maskColor.r)/colorSum;
    texColor.b = (texColor.b - maskColor.b)/colorSum;
    texColor.g = (texColor.g - maskColor.g)/colorSum;
    
    texColor.r = texColor.r*colorSum;
    texColor.b = texColor.b*colorSum;
    texColor.g = texColor.g*colorSum;
    
    gl_FragColor = vec4(texColor.rgb,colorSum);
    
    

   
}
);
