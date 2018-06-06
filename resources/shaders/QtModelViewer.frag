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
varying mediump vec3 baseVertexAttribFrag;
varying mediump vec3 baseNormalAttribFrag;
varying mediump vec4 colorAttribFrag;
varying highp vec3 texCoordAttribFrag;

// Other outputs from vertex used for lighting
varying highp vec4 camVertexFrag;
varying highp vec3 camNormalFrag;
varying highp vec4 lightPositionFrag;

// Uniforms set from the API
uniform highp vec4 colorUniform;

uniform sampler2D colorTex;
uniform sampler2D colorTex1;
uniform sampler2D colorTex2;
uniform sampler2D colorTex3;
uniform int renderMode;

void main(void)
{
    // Basic color rendering modes
    if(renderMode == RENDER_SINGLE_COLOR)
    {
        gl_FragColor = colorUniform;
    }
    else if(renderMode == RENDER_VERTEX_COLOR)
    {
        gl_FragColor = colorAttribFrag;
    }
    else if(renderMode == RENDER_TEXTURE_COLOR)
    {
    	if(texCoordAttribFrag.z > 2.5) {
	        gl_FragColor = texture2D(colorTex3, texCoordAttribFrag.xy);
	    } else if(texCoordAttribFrag.z > 1.5) {
	        gl_FragColor = texture2D(colorTex2, texCoordAttribFrag.xy);
	    } else if(texCoordAttribFrag.z > 0.5) {
	        gl_FragColor = texture2D(colorTex1, texCoordAttribFrag.xy);
	    } else {
	        gl_FragColor = texture2D(colorTex, texCoordAttribFrag.xy);
	    }
    }

    // False color visualization modes
    else if(renderMode == RENDER_VERTEX_DATA)
    {
        highp vec3 vertexColor = (baseVertexAttribFrag + 1.0)/2.0;
        gl_FragColor = vec4(vertexColor, 1.0);
    }
    else if(renderMode == RENDER_NORMS_DATA)
    {
        highp vec3 normColor = (baseNormalAttribFrag + 1.0)/2.0;
        gl_FragColor = vec4(normColor, 1.0);
    }
    else if(renderMode == RENDER_UV_DATA)
    {
        gl_FragColor = vec4(texCoordAttribFrag.xy, 0.0, 1.0);
    }
    else if(renderMode == RENDER_TEXNUM_DATA)
    {
    	if(texCoordAttribFrag.z > 5.5) {
	        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);	// [5.5+] = White
    	} else if(texCoordAttribFrag.z > 4.5) {
        	gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);	// (4.5 5.5] = Cyan
    	} else if(texCoordAttribFrag.z > 3.5) {
        	gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);	// (3.5 4.5] = Magenta
    	} else if(texCoordAttribFrag.z > 2.5) {
       		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);	// (2.5 3.5] = Yellow
   		} else if(texCoordAttribFrag.z > 1.5) {
	        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);	// (1.5 2.5] = Blue
	    } else if(texCoordAttribFrag.z > 0.5) {
	        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);	// (0.5 1.5] = Green
	    } else {
	        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);	// [0.5-] = Red
	    }
    }
    
    // All the shaded modes
    else
    {
        highp vec3 L = normalize(vec3(lightPositionFrag - camVertexFrag));
        highp vec3 N = normalize(camNormalFrag);
        highp float lambert = max(dot(L, N), 0.0);

        if(renderMode == RENDER_VERTEX_SHADED)
        {
            gl_FragColor = vec4(colorAttribFrag.rgb*lambert, 1.0);
        }
        else if(renderMode == RENDER_TEXTURE_SHADED)
        {
        	highp vec4 texColor;
	    	if(texCoordAttribFrag.z > 2.5) {
		        texColor = texture2D(colorTex3, texCoordAttribFrag.xy);
		    } else if(texCoordAttribFrag.z > 1.5) {
		        texColor = texture2D(colorTex2, texCoordAttribFrag.xy);
		    } else if(texCoordAttribFrag.z > 0.5) {
		        texColor = texture2D(colorTex1, texCoordAttribFrag.xy);
		    } else {
		        texColor = texture2D(colorTex, texCoordAttribFrag.xy);
		    }
        
            gl_FragColor = vec4(texColor.rgb*lambert, 1.0);
        }
        else
        {
            gl_FragColor = vec4(colorUniform.rgb*lambert, 1.0);;
        }
    }
}

