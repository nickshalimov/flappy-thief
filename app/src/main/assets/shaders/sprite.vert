uniform mat3 Matrix;

attribute vec2 y;
attribute vec2 texcoord;

varying vec2 _texcoord;

void main()
{
    _texcoord = texcoord;
    gl_Position = vec4((Matrix * vec3(y.xy, 1)).xy, 0, 1);
}
