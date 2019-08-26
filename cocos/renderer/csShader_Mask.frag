const char* csMask_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform sampler2D u_texture;
uniform sampler2D u_mask;
uniform vec2 u_texSize;
uniform vec2 u_maskSize;
uniform vec2 u_offset;
uniform float u_midsize;
                                    

void main()
{
    vec4 texColor = texture2D(u_texture, v_texCoord);
    float convertsX = v_texCoord.x * u_texSize.x;
    float convertsY = v_texCoord.y * u_texSize.y;
    convertsX = convertsX - u_offset.x;
    convertsY = convertsY - u_offset.y;
    vec2 maskCoord;
    if(u_midsize == 1.0)
        maskCoord =  vec2(  (convertsX / u_maskSize.x) ,1.0 - (convertsY /u_maskSize.y) );
    else
        maskCoord=  vec2(  (convertsX / u_maskSize.x) ,(convertsY /u_maskSize.y) );
    vec4 maskColor = texture2D(u_mask, maskCoord);
    
    if(texColor.a == 0.0)
    {
        discard;
        return;
    }
    
    
    float holder = texColor.a;
    texColor.a = 0.0;
      if(maskColor.a >= 0.005 && holder > 0.0)
    {
        texColor.a = holder;
        if(maskColor.a < 1.0)
        {
            for(float x = -3.0; x<= 3.0;x=x+3.0)
            {
                for(float y = -3.0; y<= 3.0;y=y+3.0)
                {
                    vec4 theColor;
                    if(u_midsize == 1.0)
                      theColor  =texture2D(u_mask, vec2( (convertsX + x) / u_maskSize.x  ,1.0 - (convertsY + y) / u_maskSize.y  ) );
                    else
                        theColor =texture2D(u_mask, vec2( (convertsX + x) / u_maskSize.x  ,(convertsY + y) / u_maskSize.y  ) );
                    if( theColor.a < 0.005 )
                    {
//                        if((maskColor.b +maskColor.r) < maskColor.g  && texColor.a > 0.45)
//                        {
//                            if(holder >= 0.75 )
//                                texColor.a = maskColor.a;
//                            else
//                                texColor.a = 0.0;
//                        }
//                        else
                        texColor.a = 0.0;
                        gl_FragColor =   texColor;
                        return;
                    }
                }  
            }  
        }  
    }
//    texColor.r = texColor.r / texColor.a;
//    texColor.b = texColor.b / texColor.a;
//    texColor.g = texColor.g / texColor.a;
    gl_FragColor =   texColor;
}
);
