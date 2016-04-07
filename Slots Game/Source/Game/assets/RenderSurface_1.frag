#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D texFramebuffer;
uniform float time = 0;
uniform float mag = 0;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(float x, float y){
	return rand(vec2(x, y));
}

void main() {
	// get pixels
	vec2 r = Texcoord;
	r *= 64;
	r -= mod(r, 1);
	r /= 64;

	vec2 p = r;
	
	// distort uvs
	r.y += cos(time*200*(mag))*4/64.f * mag;

	// original
	vec4 orig = vec4(texture(texFramebuffer, Texcoord) );

	// new
	vec4 col = vec4(texture(texFramebuffer, r) );
	
	// combined
	outColor = orig;
	outColor += col*mag;
}
