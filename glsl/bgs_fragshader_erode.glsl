varying vec2 tcoord;
uniform sampler2D tex;
uniform vec2 fTexelSize;

void main(void) 
{
	// -----------------------------------------------------------------------------------------
	
	// make dillate
	//
	float fCol = 1.0;
	
	for(int xoffset = -1; xoffset <= 1; xoffset++)
		{
		for(int yoffset = -1; yoffset <= 1; yoffset++)
			{
			vec2 offset = vec2(xoffset,yoffset);
			
			fCol = min( fCol, texture2D(tex,tcoord+offset * fTexelSize ).r );	// in r is strored result of previous shader
			}
		}
		
	gl_FragColor.r = clamp( fCol, 0.0,  1.0 );
    
}
