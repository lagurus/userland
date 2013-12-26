attribute vec2 vertex;
varying vec2 tcoord;

void main(void)
{
	tcoord = 0.5 * (vertex + 1.0);
	gl_Position = vec4(vertex, 0.0, 1.0);
}

