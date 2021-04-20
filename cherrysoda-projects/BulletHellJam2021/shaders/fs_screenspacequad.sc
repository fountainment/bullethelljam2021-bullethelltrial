$input v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <cherrysoda_uniforms.sh>

SAMPLER2D(s_tex, 0);

uniform vec4 u_data;
#define u_center u_data.xy
#define u_radius u_data.z

void main()
{
	if (v_texcoord0.x < 0. || v_texcoord0.x > 1. || v_texcoord0.y < 0. || v_texcoord0.y > 1.) {
		discard;
	}
    vec4 color = v_color0 * texture2D(s_tex, v_texcoord0);
    if (color.w < 1./255.) {
        discard;
    }
    if (length(v_texcoord0.xy - u_center) < u_radius) {
		color.xyz = 1. - color.xyz;
    }
    gl_FragColor = color;
}
