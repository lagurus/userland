varying vec2 tcoord;
uniform sampler2D tex;
uniform sampler2D tex_previous;
uniform int nLayerNumber;
uniform int nEnableWeight;

void main(void) 
{
	// Convert to grayscale using NTSC conversion weights
	//float gray = dot( texture2D( tex, tcoord ).rgb, vec3(0.299, 0.587, 0.114));
	
	//gl_FragColor = texture2D( tex, tcoord );
	//gl_FragColor.a = 1.0;
	
	//gl_FragColor = vec4(gray, gray, gray, texture2D( tex, tcoord ).a );
	
	if (nLayerNumber >= 2)
		{
		gl_FragColor.b = texture2D( tex_previous, tcoord).g;
		gl_FragColor.g = texture2D( tex_previous, tcoord).r;
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		
		float f_prev_background = texture2D( tex_previous, tcoord ).a;
		
		//gl_FragColor.a = 1.0;
		
		// now have in all r,g,b different frame data
		
		// in gl_FragColor.a is background
		
		if(nEnableWeight == 1)
			gl_FragColor.a = ((gl_FragColor.r * 0.4) + (gl_FragColor.g * 0.25) + (gl_FragColor.b * 0.25) + f_prev_background * 0.1 );
		else
			gl_FragColor.a = (gl_FragColor.r + gl_FragColor.g + gl_FragColor.b + f_prev_background) / 4.0;
		}
	
	else if (nLayerNumber == 0)
		{
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		}
	else
		{
		gl_FragColor.g = texture2D( tex_previous, tcoord).r;
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		}
}
