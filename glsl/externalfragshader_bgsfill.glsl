#extension GL_OES_EGL_image_external : require

varying vec2 tcoord;

uniform samplerExternalOES tex;
uniform sampler2D tex_previous;
uniform int nLayerNumber;
uniform int nEnableWeight;

void main(void) 
{
	if (nLayerNumber >= 3)
		{
		float f_prev_background = texture2D( tex_previous, tcoord ).a;
		
		if(nEnableWeight == 1)
			gl_FragColor.a = ((texture2D( tex_previous, tcoord ).r * 0.4) + (texture2D( tex_previous, tcoord ).g * 0.3) + (texture2D( tex_previous, tcoord ).b * 0.25) + f_prev_background * 0.05 );
		else
			gl_FragColor.a = (texture2D( tex_previous, tcoord ).r + texture2D( tex_previous, tcoord ).g + texture2D( tex_previous, tcoord ).b + f_prev_background) / 4.0;
			
		gl_FragColor.b = texture2D( tex_previous, tcoord).g;
		gl_FragColor.g = texture2D( tex_previous, tcoord).r;
		gl_FragColor.r = texture2D( tex, tcoord ).r;
			
		}
	else if (nLayerNumber == 0)
		{
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		}
	else if (nLayerNumber == 1)
		{
		gl_FragColor.g = texture2D( tex_previous, tcoord).r;
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		}
	else		// nLayerNumber == 2
		{
		gl_FragColor.b = texture2D( tex_previous, tcoord).g;
		gl_FragColor.g = texture2D( tex_previous, tcoord).r;
		gl_FragColor.r = texture2D( tex, tcoord ).r;
		
		if(nEnableWeight == 1)
			gl_FragColor.a = ((gl_FragColor.r * 0.5) + (gl_FragColor.g * 0.3) + (gl_FragColor.b * 0.2) );
		else
			gl_FragColor.a = (gl_FragColor.r + gl_FragColor.g + gl_FragColor.b) / 3.0;
		}
	
		
}
