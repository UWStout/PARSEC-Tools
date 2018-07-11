#version 330

// Vertex attributes
in vec4 vertexAttrib;
in vec3 normalAttrib;
in vec4 colorAttrib;
in vec3 texCoordAttrib;
in float faceTexIndexAttrib;

// Attributes output to the fragment shader
out vec3 texCoordAttribFrag;
out vec4 colorAttribFrag;
out vec3 baseVertexAttribFrag;
out vec3 baseNormalAttribFrag;

// Other outputs to the fragment shader
out vec4 camVertexFrag;
out vec3 camNormalFrag;
out vec4 lightPositionFrag;

// Light position
uniform vec4 lightPosition;

// Transformation matrices
uniform mat4 perspectiveMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    // Pass through unmodified for visualization
    baseVertexAttribFrag = vertexAttrib.xyz;
    baseNormalAttribFrag = normalAttrib;
    
    // Pass through for interpolation
    colorAttribFrag = colorAttrib;
    texCoordAttribFrag = texCoordAttrib;
    
    // Transform and pass through for lighting
    lightPositionFrag = viewMatrix * lightPosition;
    camNormalFrag = normalMatrix * normalAttrib;
    camVertexFrag = viewMatrix * modelMatrix * vertexAttrib;
    gl_Position = perspectiveMatrix * camVertexFrag;
}
