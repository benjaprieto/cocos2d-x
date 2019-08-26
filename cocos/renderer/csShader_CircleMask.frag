const char* csCircleMask_frag = STRINGIFY(
                                              
#ifdef GL_ES
precision mediump float;               
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform int u_outline;


uniform vec4 u_size;
const highp vec2 center = vec2(0.5, 0.5);

void main()
{
    float radius = u_size.z;
	vec4 texColor = texture2D(CC_Texture0,v_texCoord);
	vec2 dist=v_texCoord - center;
	float d=length(dist* u_size.xy);
	float offset = radius - radius * 0.98;

	if (radius < 0.0001)
        discard;
	else if (d > radius + offset)
	    gl_FragColor = vec4(0.0);
	else if( d >= radius && u_outline > 0)
    {
	    gl_FragColor = mix(texColor, vec4(0.0), (d - radius) / offset);
    }
	else
    {
        gl_FragColor = texColor;
        
        float alphaVal = u_size.w/255.0;
        if(alphaVal > 0.0 )
        {
            gl_FragColor.r = gl_FragColor.r / alphaVal;
            gl_FragColor.b = gl_FragColor.b / alphaVal;
            gl_FragColor.g = gl_FragColor.g / alphaVal;
        }
        
        gl_FragColor.a = alphaVal;
    }
    
}
);
