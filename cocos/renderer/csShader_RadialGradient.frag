const char* csRadialGradient_frag = STRINGIFY(
                                              
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform float u_radiusX;
uniform float u_radiusY;
uniform vec3 u_lightColor;
uniform vec2 u_lightPosition;

void main()
{
    float distanceX = length( u_lightPosition.x - gl_FragCoord.x );
    float distanceY = length( u_lightPosition.y - gl_FragCoord.y );
\n#ifdef GL_ES\n
    lowp float quadIntensityX = 1.0 - (distanceX / u_radiusX);
    lowp float quadIntensity = quadIntensityX * (1.0 - (distanceY / u_radiusY));
\n#else\n
    float quadIntensityX = 1.0 - (distanceX / u_radiusX);
    float quadIntensity = quadIntensityX * (1.0 - (distanceY / u_radiusY));
\n#endif\n 
    gl_FragColor = vec4(v_fragmentColor.rgb * (u_lightColor * quadIntensity), 1.0);
}
);
