const char* csBlurEdge_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision highp float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec3 vertices;
                                           
uniform sampler2D u_texture;
uniform sampler2D u_holder;
uniform vec2 u_size;
uniform int debug;

                                        
                                        
bool blank (vec4 coords)
{
    if(coords.g == 0.0 && coords.r == 0.0 && coords.b == 0.0 && coords.a == 0.0)
        return true;
    return false;
}
                                        
void main()
{
    vec4 texColor = texture2D(u_texture,v_texCoord);
    float offsetX = 1.0/u_size.x;
    float offsetY = 1.0/u_size.y;

    
    bool down = false;
    bool side = false;
    bool negSlope = false;
    bool posSlope = false;
    

//    float range = 5.0;
//    float range2 = 4.0;
    
    float range = 3.0;
    float range2 = 2.0;
    
    vec4 TL =  texture2D(u_texture,v_texCoord + vec2((-1.0 * range * offsetX), (range *offsetY) ));
    vec4 TR =    texture2D(u_texture,v_texCoord + vec2((range *offsetX), (range *offsetY) ));
    vec4 BL =  texture2D(u_texture,v_texCoord + vec2((-1.0 * range *offsetX), (-1.0 * range *offsetY) ));
    vec4 BR = texture2D(u_texture,v_texCoord + vec2((range *offsetX), (-1.0 * range *offsetY) ));

   
    
    
    bool TLhit = false;
    bool TRhit = false;
    bool BLhit = false;
    bool BRhit = false;
    int count = 0;
    
    if(blank(TL) ){
        count += 1;
        TLhit = true;
    }
    if(blank(TR)){
        count += 1;
        TRhit = true;
    }
    if(blank(BL)){
        BLhit = true;
        count += 1;
    }
    if(blank(BR)){
        BRhit = true;
        count += 1;
    }
    
    if(TLhit && BLhit)
        down = true;
    else if(TRhit && BRhit)
        down = true;
    else if(TLhit && TRhit)
        side = true;
    else if(BLhit && BRhit)
        side = true;
    else if(blank(texture2D(u_texture,v_texCoord + vec2((range2 *offsetX), (range2 *offsetY) ))))
    {
        negSlope = true;
        if(blank(texture2D(u_texture,v_texCoord + vec2((-1.0 * range2 *offsetX), (range2 *offsetY) )))   )
        {
            negSlope = false;
            side = true;
        }
    }
    else if(blank(texture2D(u_texture,v_texCoord + vec2((-1.0 * range2 *offsetX), (range2 *offsetY) )))   )
        posSlope = true;
    
//    if(TRhit)
//    {
//        negSlope = true;
//    }
//    
//    if(TRhit && BRhit)
//        negSlope = true;
    
//    if(BLhit && BRhit && TLhit && !TRhit){
//        negSlope = true;
//        side = false;
//        down = false;
//    } else if(TRhit && TLhit && BRhit && !BLhit){
//        negSlope = true;
//        side = false;
//        down = false;
//    }
//    else if(TRhit && TLhit && BLhit && !BRhit){
//        negSlope = true;
//        side = false;
//        down = false;
//    }
//    else if(TRhit && BLhit && BRhit && !TLhit){
//        negSlope = true;
//        side = false;
//        down = false;
//    }

    
//    else if(TRhit || BLhit /*|| BRhit || TLhit*/)
//    {
////        if(texColor.a < 0.99)
////        {
//            negSlope = true;
//            side = false;
//            down = false;
////        }
//    }

    if(count > 2)
    {
        negSlope = false;
        posSlope = false;
        side = false;
        down = false;
    }
    
//    if(TLhit && BLhit && BRhit && TRhit)
//    {
//        negSlope = false;
//        side = false;
//        down = false;
//    }

    
    
    
    
    if(negSlope)
    {
        vec4 color = texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y) );
        
        float countUp = 0.0;
        bool breakOut = false;
        float y = 0.0;
        while(!breakOut)
        {
            vec4 color2 = texture2D(u_texture,v_texCoord + vec2(0, (y * offsetY) ));
            y = y + 1.0;
            if(color2.g == 0.0 && color2.r == 0.0 && color2.b == 0.0 && color2.a < 0.02)
                breakOut = true;
            else
                countUp += 1.0;
        }
        
        breakOut = false;
        float countLeft = 0.0;
        y = 0.0;
        while(!breakOut)
        {
            vec4 color2 = texture2D(u_texture,v_texCoord + vec2((-1.0*offsetX), (y*offsetY) ));
            y = y + 1.0;
            if(color2.g == 0.0 && color2.r == 0.0 && color2.b == 0.0 && color2.a < 0.02)
                breakOut = true;
            else
                countLeft += 1.0;
        }
        
        
        if(countLeft == countUp )
        {
            countLeft += 1.0;
            countUp -= 1.0;  //2.0?
        }

//        countUp = countUp * 2.0;
//        countLeft = countLeft * 2.0;
        
        float diff = float(countLeft) - float(countUp);
        diff = diff + 1.0;
        float value = 1.0/diff;
        
//        countUp = countUp / 2.0;
        
//        value = 0.25;
    
        
        color.r = color.r / color.a;
        color.b = color.b / color.a;
        color.g = color.g / color.a;
        
        if(countLeft > 20.0)
        {
            gl_FragColor = color;
            return;
        }
        
        
        color.a = value * countUp;
        
        if(debug > 0){
            color.b = 1.0;
            color.a = 1.0;
        }
        
        
        gl_FragColor = color;
        return;
        
        
    }
    
    
    if(posSlope)
    {
        vec4 color = texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y) );
        
        float countUp = 0.0;
        bool breakOut = false;
        float y = 0.0;
        while(!breakOut)
        {
            vec4 color2 = texture2D(u_texture,v_texCoord + vec2(0, (y * offsetY) ));
            y = y + 1.0;
            if(color2.g == 0.0 && color2.r == 0.0 && color2.b == 0.0 && color2.a < 0.02)
                breakOut = true;
            else
                countUp += 1.0;
        }
        
        breakOut = false;
        float countRight = 0.0;
        y = 0.0;
        while(!breakOut)
        {
            vec4 color2 = texture2D(u_texture,v_texCoord + vec2((1.0*offsetX), (y*offsetY) ));
            y = y + 1.0;
            if(color2.g == 0.0 && color2.r == 0.0 && color2.b == 0.0 && color2.a < 0.02)
                breakOut = true;
            else
                countRight += 1.0;
        }
        
        
        if(countRight == countUp )
        {
            countRight += 1.0;
            countUp -= 1.0;  //2.0?
        }
        
        //        countUp = countUp * 2.0;
        //        countRight = countRight * 2.0;
        
        float diff = float(countRight) - float(countUp);
        diff = diff + 1.0;
        float value = 1.0/diff;
        
        //        countUp = countUp / 2.0;
        
        //        value = 0.25;
        
        
        color.r = color.r / color.a;
        color.b = color.b / color.a;
        color.g = color.g / color.a;
        
        if(countRight > 20.0)
        {
            gl_FragColor = color;
            return;
        }
        
        
        color.a = value * countUp;
        
        if(debug > 0){
            color.b = 1.0;
            color.r = 1.0;
            color.a = 1.0;
        }
        
        
        gl_FragColor = color;
        return;
        
        
    }
    
    
    
    
    
    if(down)
    {
        vec4 color = texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y) );
       
        
        vec4 up1 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * 1.0) ) );
        vec4 up2 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * 2.0) ) );
        vec4 up3 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * 3.0) ) );

        vec4 down1 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * -1.0) ) );
        vec4 down2 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * -2.0) ) );
        vec4 down3 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y+ (offsetY * -3.0) ) );

    //                    up2 = up2 /1.2;
    //                     up3 = up3 /1.4;
    //                    down2 = down2 /1.2;
    //                    down3 = down3 /1.4;

        color = color + up1 + up2 + up3 + down1+ down2+ down3;
        color = color/7.0;
        color.r = color.r / color.a;
        color.b = color.b / color.a;
        color.g = color.g / color.a;
        if(debug > 0){
            color.g = 1.0;
            color.a = 1.0;
        }
        gl_FragColor = color;
        return;
        

    }
    
    if(side)
    {
        vec4 color = texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y) );
      
        vec4 right1 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * 1.0) ,v_texCoord.y ) );
        vec4 right2 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * 2.0) ,v_texCoord.y ) );
        vec4 right3 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * 3.0) ,v_texCoord.y ) );
        
        vec4 left1 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * -1.0) ,v_texCoord.y ) );
        vec4 left2 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * -2.0) ,v_texCoord.y ) );
        vec4 left3 =texture2D(u_texture, vec2( v_texCoord.x+ (offsetX * -3.0) ,v_texCoord.y ) );


    //        right2 = right2 /1.2;
    //        right3 = right3 /1.4;
    //         left2 = left2 /1.2;
    //         left3 = left3 /1.4;

        color = color + right1 + right2 + right3 + left1 + left2 + left3;
        color = color/7.0;


        
        color.r = color.r / color.a;
        color.b = color.b / color.a;
        color.g = color.g / color.a;
        if(debug > 0){
           color.r = 1.0;
           color.a = 1.0;
        }
        gl_FragColor = color;
        return;
        

    }
//
//    if(negSlope)
//    {
//        
//       vec4 color = texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y) );
//        
//        vec4 right1 =texture2D(u_texture, vec2( v_texCoord.x ,v_texCoord.y +(offsetY * 1.0) ) );
////        vec4 right2 =texture2D(u_texture, vec2( v_texCoord.x+(offsetX * -2.0) ,v_texCoord.y +(offsetY * 2.0)) );
////        vec4 right3 =texture2D(u_texture, vec2( v_texCoord.x+(offsetX * -3.0)  ,v_texCoord.y +(offsetY * 3.0)) );
//
//        vec4 left1 =texture2D(u_texture, vec2( v_texCoord.x  ,v_texCoord.y +(offsetY * -1.0)) );
////        vec4 left2 =texture2D(u_texture, vec2( v_texCoord.x +(offsetX * 2.0) ,v_texCoord.y +(offsetY * -2.0)) );
////        vec4 left3 =texture2D(u_texture, vec2( v_texCoord.x +(offsetX * 3.0) ,v_texCoord.y +(offsetY * -3.0)) );
//
//    //                right2 = right2 /1.2;
//    //            right3 = right3 /1.4;
//    //                left2 = left2 /1.2;
//    //         left3 = left3 /1.4;
//
//        color = color + right1 + left1;
//        color = color/3.0;
//        
//        
//        color.r = color.r / color.a;
//        color.b = color.b / color.a;
//        color.g = color.g / color.a;
//    //                color.a = color.a * 0.75;
//        if(debug > 0){
//           color.b = 1.0;
//           color.a = 1.0;
//        }
//        gl_FragColor = color;
//        return;
//        
//    }
    
    texColor.r = texColor.r / texColor.a;
    texColor.b = texColor.b / texColor.a;
    texColor.g = texColor.g / texColor.a;
//    if(texColor.a == 0.0)
//        discard;
    gl_FragColor = texColor;
}
);