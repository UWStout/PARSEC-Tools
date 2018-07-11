#version 330

// These constants need to match the enum in QtModelViewerWidget
#define RENDER_SINGLE_COLOR     0
#define RENDER_VERTEX_COLOR     1
#define RENDER_TEXTURE_COLOR    2

#define RENDER_SHADED           3
#define RENDER_VERTEX_SHADED    4
#define RENDER_TEXTURE_SHADED   5

#define RENDER_VERTEX_DATA      6
#define RENDER_NORMS_DATA       7
#define RENDER_UV_DATA          8
#define RENDER_TEXNUM_DATA      9

// Attributes Passed in from the vertex shader
in vec3 baseVertexAttribFrag;
in vec3 baseNormalAttribFrag;
in vec4 colorAttribFrag;
in vec3 texCoordAttribFrag;

// Other outputs from vertex used for lighting
in vec4 camVertexFrag;
in vec3 camNormalFrag;
in vec4 lightPositionFrag;

// Uniforms set from the API
uniform vec4 colorUniform;

uniform sampler2D colorTex;
uniform sampler2D colorTex1;
uniform sampler2D colorTex2;
uniform sampler2D colorTex3;
uniform int renderMode;

// Final output color
out vec4 fragColor;

void main(void) {
    // Basic color rendering modes
    if(renderMode == RENDER_SINGLE_COLOR) {
        fragColor = colorUniform;
    } else if(renderMode == RENDER_VERTEX_COLOR) {
        fragColor = colorAttribFrag;
    } else if(renderMode == RENDER_TEXTURE_COLOR) {
        if(texCoordAttribFrag.z > 2.5) {
            fragColor = texture(colorTex3, texCoordAttribFrag.xy);
        } else if(texCoordAttribFrag.z > 1.5) {
            fragColor = texture(colorTex2, texCoordAttribFrag.xy);
        } else if(texCoordAttribFrag.z > 0.5) {
            fragColor = texture(colorTex1, texCoordAttribFrag.xy);
        } else {
            fragColor = texture(colorTex, texCoordAttribFrag.xy);
        }
    }

    // False color visualization modes
    else if(renderMode == RENDER_VERTEX_DATA) {
        vec3 vertexColor = (baseVertexAttribFrag + 1.0)/2.0;
        fragColor = vec4(vertexColor, 1.0);
    } else if(renderMode == RENDER_NORMS_DATA) {
        vec3 normColor = (baseNormalAttribFrag + 1.0)/2.0;
        fragColor = vec4(normColor, 1.0);
    } else if(renderMode == RENDER_UV_DATA) {
        fragColor = vec4(texCoordAttribFrag.xy, 0.0, 1.0);
    } else if(renderMode == RENDER_TEXNUM_DATA) {
        if(texCoordAttribFrag.z > 5.5) {
            fragColor = vec4(1.0, 1.0, 1.0, 1.0);	// [5.5+] = White
        } else if(texCoordAttribFrag.z > 4.5) {
            fragColor = vec4(0.0, 1.0, 1.0, 1.0);	// (4.5 5.5] = Cyan
        } else if(texCoordAttribFrag.z > 3.5) {
            fragColor = vec4(1.0, 0.0, 1.0, 1.0);	// (3.5 4.5] = Magenta
        } else if(texCoordAttribFrag.z > 2.5) {
            fragColor = vec4(1.0, 1.0, 0.0, 1.0);	// (2.5 3.5] = Yellow
        } else if(texCoordAttribFrag.z > 1.5) {
            fragColor = vec4(0.0, 0.0, 1.0, 1.0);	// (1.5 2.5] = Blue
        } else if(texCoordAttribFrag.z > 0.5) {
            fragColor = vec4(0.0, 1.0, 0.0, 1.0);	// (0.5 1.5] = Green
        } else {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);	// [0.5-] = Red
        }
    }
    
    // All the shaded modes
    else {
        vec3 L = normalize(vec3(lightPositionFrag - camVertexFrag));
        vec3 N = normalize(camNormalFrag);
        float lambert = max(dot(L, N), 0.0);

        if(renderMode == RENDER_VERTEX_SHADED) {
            fragColor = vec4(colorAttribFrag.rgb*lambert, 1.0);
        } else if(renderMode == RENDER_TEXTURE_SHADED) {
            vec4 texColor;
            if(texCoordAttribFrag.z > 2.5) {
                texColor = texture(colorTex3, texCoordAttribFrag.xy);
            } else if(texCoordAttribFrag.z > 1.5) {
                texColor = texture(colorTex2, texCoordAttribFrag.xy);
            } else if(texCoordAttribFrag.z > 0.5) {
                texColor = texture(colorTex1, texCoordAttribFrag.xy);
            } else {
                texColor = texture(colorTex, texCoordAttribFrag.xy);
            }
        
            fragColor = vec4(texColor.rgb*lambert, 1.0);
        } else {
            fragColor = vec4(colorUniform.rgb*lambert, 1.0);
        }
    }
}

