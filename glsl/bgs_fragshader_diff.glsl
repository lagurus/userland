varying vec2 tcoord;
uniform sampler2D tex;
uniform sampler2D tex_orig;
uniform vec2 fThreshold;
uniform vec2 fTexelSize;

void main(void) 
{
	// in tex.a is result weight of last 3 pictures = background
	// in tex_orig is original texture - all rgba layers should be same
	//
	// in gl_FragColor.r is then stored foreground
	// in gl_FragColor.g is stored threshold of foreground
	//
	
	// ------------------------------------------------------------------------------------------------
	
	gl_FragColor.g 	= abs( texture2D( tex, tcoord ).a - texture2D( tex_orig, tcoord ).r );
	
	// ------------------------------------------------------------------------------------------------
	
	/*if (gl_FragColor.r >= fThreshold.x)	// solution A - what is faster?
		{
		gl_FragColor.g = 1.0;
		}
	else
		{
		gl_FragColor.g = 0.0;
		}*/
	
	vec2		src_r 	= vec2( gl_FragColor.g, 0 );		// solution B - what is faster?
	gl_FragColor.r	= vec2( greaterThanEqual( src_r, fThreshold ) ).r;
		
	
    
}
