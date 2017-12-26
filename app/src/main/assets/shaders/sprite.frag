precision mediump float;

uniform sampler2D Texture;

varying vec2 _texcoord;

void main()
{
    gl_FragColor = texture2D(Texture, _texcoord);
}
