const char* csGreenScreenDroid_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec3 vertices;
uniform sampler2D u_texture;
uniform vec3 u_size;

vec4 calShit(vec4 texColor)
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
    return  texColor;
}

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
    float aBool = 0.0;
    if(texColor.g == 1.0 && texColor.r == 0.0 && texColor.b == 0.0)
        texColor.a = 0.0;
    else if(texColor.r == 0.0 && texColor.b == 0.0 && texColor.a > 0.0 && texColor.a < 1.0)
    {
        if(texColor.g != 0.0)
            texColor.a = 0.0;
    }
    else if (texColor.r < texColor.g)
    {
        if (texColor.b < texColor.g)
        {
            if (texColor.a < 0.02)
            {
                texColor.a = 0.0;
            }
            else
            {
                vec4 theColor5 =  texture2D(u_texture, vec2( v_texCoord.x + (offsetX *4.0) ,v_texCoord.y + (offsetY *4.0) ) );
                vec4 theColor6 =  texture2D(u_texture, vec2( v_texCoord.x - (offsetX *4.0) ,v_texCoord.y + (offsetY *4.0) ) );
                vec4 theColor7 =  texture2D(u_texture, vec2( v_texCoord.x + (offsetX *4.0) ,v_texCoord.y - (offsetY *4.0) ) );
                vec4 theColor8 =  texture2D(u_texture, vec2( v_texCoord.x - (offsetX *4.0) ,v_texCoord.y - (offsetY *4.0) ) );
                if( theColor5.g == 1.0 && theColor5.r == 0.0 && theColor5.b == 0.0 )
                {
                    texColor =   calShit(texColor);
                    aBool = 1.0;
                }
                if( theColor6.g == 1.0 && theColor6.r == 0.0 && theColor6.b == 0.0 && aBool == 0.0 )
                {
                    texColor =   calShit(texColor);
                    aBool = 1.0;
                }
                if( theColor7.g == 1.0 && theColor7.r == 0.0 && theColor7.b == 0.0 && aBool == 0.0 )
                {
                    texColor =   calShit(texColor);
                    aBool = 1.0;  
                } 
                if( theColor8.g == 1.0 && theColor8.r == 0.0 && theColor8.b == 0.0 && aBool == 0.0)                          
                {       
                    texColor =   calShit(texColor);                          
                    aBool = 1.0;  
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
