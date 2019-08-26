const char* csScissor_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n
varying vec4 v_fragmentColor;
varying vec4 v_position;
varying vec2 v_texCoord;
uniform vec2 u_size;
uniform vec2 u_origin;

void main()
{
    if(v_position.r >= u_origin.x && v_position.r <= u_size.x && v_position.g >= u_origin.y && v_position.g <= u_size.y )
    {
        gl_FragColor = vec4(v_fragmentColor.r,v_fragmentColor.g,v_fragmentColor.b,0.0);
    }
    else
    {
        gl_FragColor = v_fragmentColor;
    }
}
);
