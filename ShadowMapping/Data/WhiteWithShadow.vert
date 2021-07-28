attribute vec3 a_Position;

uniform mat4 u_WVPTransform;
uniform mat4 u_ShadowLightTransform;

varying vec3 v_LightSpace_VertexPosition;

void main()
{
    gl_Position = u_WVPTransform * vec4( a_Position, 1 );

    // Project the vertex using the light's view&proj matrices.
    vec4 lightClipSpace = u_ShadowLightTransform * vec4( a_Position, 1 );
    lightClipSpace.xyz = lightClipSpace.xyz * 0.5 + 0.5;
    v_LightSpace_VertexPosition = lightClipSpace.xyz / lightClipSpace.w;
}
