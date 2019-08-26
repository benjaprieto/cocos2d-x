const char* csGreenScreen_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec3 vertices;
                                           
uniform sampler2D u_texture;
uniform vec3 u_size;

void main()
{
    vec4 texColor = texture2D(u_texture,v_texCoord);
    if(u_size.z == 1.0)
    {
        if(texColor.a > 0.0 )
        {
            texColor.r = texColor.r / texColor.a;
            texColor.b = texColor.b / texColor.a;
            texColor.g = texColor.g / texColor.a;
        }
        gl_FragColor = texColor;
        return;
    }
    float offsetX = 1.0/u_size.x;
    float offsetY = 1.0/u_size.y;
    if(texColor.g == 1.0 && texColor.r == 0.0 && texColor.b == 0.0)
        texColor.a = 0.0;
    else if(texColor.r == 0.0 && texColor.b == 0.0 && texColor.a > 0.0 && texColor.a < 1.0)
    {
        if(texColor.g == 0.0)
        {
            if(texColor.a > 0.0 )
            {
                texColor.r = texColor.r / texColor.a;
                texColor.b = texColor.b / texColor.a;
                texColor.g = texColor.g / texColor.a;
            }
            gl_FragColor = texColor;
            return;
        }
        texColor.a = 0.0;
    }
    else if (texColor.r < texColor.g)
    {
        if (texColor.b < texColor.g)
        {
            if (texColor.a < 0.02)
            {
                texColor.a = 0.0;
                gl_FragColor = texColor;
                return;
            }
  
            for(float x = -2.0; x<= 2.0;x=x+1.0)
            {
                for(float y = -2.0; y<= 2.0;y=y+1.0)
                {
                    vec4 theColor =texture2D(u_texture, vec2( v_texCoord.x +(offsetX *x) ,v_texCoord.y+ (offsetY *y) ) );
                    if( theColor.g == 1.0 && theColor.r == 0.0 && theColor.b == 0.0 )
                    {
                        if(texColor.a < 1.0)
                            texColor.a = 0.0;
                        else
                        {
                            float holder = (texColor.r + texColor.b)*0.5;
                            float holder2 = texColor.g - holder;
                            texColor.a = 1.0 - holder2;
                            texColor.g = holder;
                        }
                        if(texColor.a > 0.0 )
                        {
                            texColor.r = texColor.r / texColor.a;
                            texColor.b = texColor.b / texColor.a;
                            texColor.g = texColor.g / texColor.a;
                        }
                        gl_FragColor =   texColor;
                        return;
                    }                                                             
                    
                }  
            }   
        }    
    }
    if(texColor.a > 0.0 )
    {
        texColor.r = texColor.r / texColor.a;
        texColor.b = texColor.b / texColor.a;
        texColor.g = texColor.g / texColor.a;
    }
    gl_FragColor = texColor;
}
);
