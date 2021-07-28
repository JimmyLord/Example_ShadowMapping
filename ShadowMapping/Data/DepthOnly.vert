attribute vec3 a_Position;

uniform mat4 u_WVPTransform;

void main()
{
    gl_Position = u_WVPTransform * vec4( a_Position, 1 );
}
