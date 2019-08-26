const char* csGlowSampler_frag = STRINGIFY(
                                           
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

uniform sampler2D u_edge_sampler;
uniform float u_distance;
uniform vec2 u_texSize;
uniform vec4 u_baseColor;
uniform vec4 u_glowColor;

void main()
{
    vec4 normalColor = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    vec2 samplerCoord = v_texCoord;
    float scaleDX = u_distance / u_texSize.x;
    float scaleDY = u_distance / u_texSize.y;
    
    float scaleFactorX = 0.5 / scaleDX;
    float scaleFactorY = 0.5 / scaleDY;
    
    if (v_texCoord.x < scaleDX && v_texCoord.y < scaleDY) //top left
    {
        samplerCoord = vec2(v_texCoord.x * scaleFactorX, v_texCoord.y * scaleFactorY);
    }
    else if (v_texCoord.x > 1.0 - scaleDX && v_texCoord.y < scaleDY) // top right
    {
        samplerCoord = vec2((1.0 - v_texCoord.x) * scaleFactorX, v_texCoord.y * scaleFactorY);
    }
    else if (v_texCoord.y > 1.0 - scaleDY && v_texCoord.x > 1.0 - scaleDX) // bottom right
    {
        samplerCoord = vec2((1.0 - v_texCoord.x) * scaleFactorX, (1.0 - v_texCoord.y) * scaleFactorY);
    }
    else if (v_texCoord.y > 1.0 - scaleDY && v_texCoord.x < scaleDX) // bottom left
    {
        samplerCoord = vec2(v_texCoord.x * scaleFactorX, (1.0 - v_texCoord.y) * scaleFactorY);
    }
    else if (v_texCoord.x < scaleDX) //left
    {
        samplerCoord = vec2(v_texCoord.x * scaleFactorX, 0.5);
    }
    else if (v_texCoord.y < scaleDY) //top
    {
        samplerCoord = vec2(0.5, v_texCoord.y * scaleFactorY);
    }
    else if (v_texCoord.y > 1.0 - scaleDY) // bottom
    {
        samplerCoord = vec2(0.5, (1.0 - v_texCoord.y) * scaleFactorY);
    }
    else if (v_texCoord.x > 1.0 - scaleDX) // right
    {
        samplerCoord = vec2((1.0 - v_texCoord.x) * scaleFactorX, 0.5);
    }
    else // center
    {
        gl_FragColor = normalColor;
        return;
    }
    
    vec4 sampleColor = texture2D(u_edge_sampler, samplerCoord);
    if (sampleColor.r < u_glowColor.r && sampleColor.g < u_glowColor.g && sampleColor.b < u_glowColor.b)
    {
        sampleColor = u_baseColor;
    }
    
    gl_FragColor = sampleColor;
}
);
