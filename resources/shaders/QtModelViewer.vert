// Vertex attributes
attribute highp vec4 vertexAttrib;
attribute highp vec3 normalAttrib;
attribute mediump vec4 colorAttrib;
attribute highp vec3 texCoordAttrib;
attribute highp float faceTexIndexAttrib;

// Attributes output to the fragment shader
varying highp vec3 texCoordAttribFrag;
varying mediump vec4 colorAttribFrag;
varying mediump vec3 baseVertexAttribFrag;
varying mediump vec3 baseNormalAttribFrag;

// Other outputs to the fragment shader
varying highp vec4 camVertexFrag;
varying highp vec3 camNormalFrag;
varying highp vec4 lightPositionFrag;

// Light position
uniform highp vec4 lightPosition;

// Transformation matrices
uniform highp mat4 perspectiveMatrix;
uniform highp mat4 modelMatrix;
uniform highp mat4 viewMatrix;
uniform highp mat3 normalMatrix;

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
