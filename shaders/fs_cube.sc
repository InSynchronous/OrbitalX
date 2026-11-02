$input v_texcoord

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
    gl_FragColor = texture2D(s_texColor, v_texcoord);
}
