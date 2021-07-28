uniform vec4 u_Color;
uniform sampler2D u_DepthTexture;

varying vec3 v_LightSpace_VertexPosition;

void main()
{
	gl_FragColor = u_Color;

	// Sample the depth texture at the UV projected from the light pov.
	vec4 depthValue = texture2D( u_DepthTexture, v_LightSpace_VertexPosition.xy );

	// Determine if the fragment depth is further than the depth sample from the texture.
	float bias = 0.002;
	if( depthValue.r < v_LightSpace_VertexPosition.z - bias )
		gl_FragColor = vec4(0,0,0,1);
}
