varying vec2 tcoord;
uniform sampler2D tex;
void main(void) 
{
	// Convert to grayscale using NTSC conversion weights
	float gray = dot( texture2D( tex, tcoord ).rgb, vec3(0.299, 0.587, 0.114));
	
	//gl_FragColor = texture2D( tex, tcoord );
	gl_FragColor = vec4(gray, gray, gray, texture2D( tex, tcoord ).a );
    
}
