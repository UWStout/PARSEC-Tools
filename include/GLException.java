package edu.uwstout.berriers.PSHelper.Model;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL20.GL_SHADING_LANGUAGE_VERSION;
import static org.lwjgl.opengl.GL20.GL_MAX_VARYING_FLOATS;
import static org.lwjgl.opengl.GL20.GL_MAX_VERTEX_ATTRIBS;

import java.util.StringTokenizer;

import com.trolltech.qt.opengl.*;

// Exceptions to support the openGL error checking
public class GLException extends Exception {
	private static final long serialVersionUID = 1L;
	
	public GLException(String message) { super(message); }
	
	public static void reportOpenGLInfo(QGLContext context) {
    	// Examine what kind of visual we requested vs what we got
    	QGLFormat req = context.requestedFormat();
    	QGLFormat fmt = context.format();

    	// Compare all basic format options side-by-side
    	System.out.println("************ QGLContext Info ************");
    	System.out.println("*           |   Requested |      Actual *");
    	System.out.println("*****************************************");
    	System.out.printf ("* GL Ver    |         %d.%d |         %d.%d *\n",
    			req.majorVersion(), req.minorVersion(),
    			fmt.majorVersion(), fmt.minorVersion());
    	System.out.printf ("* Profile   | %11s | %11s *\n",
    			req.profile().toString(), fmt.profile().toString());
    	System.out.printf ("* DirectRen | %11s | %11s *\n",
    			req.directRendering(), fmt.directRendering());
    	System.out.printf ("* Overlay   | %11s | %11s *\n",
    			req.hasOverlay(), fmt.hasOverlay());
    	System.out.printf ("* Plane     | %11d | %11d *\n",
    			req.plane(), fmt.plane());
    	System.out.println("**************** Buffers ****************");
    	System.out.printf ("* Paging    | %11s | %11s *\n",
    			(req.stereo()?"Quad":(req.doubleBuffer()?"Double":"single")),
    			(fmt.stereo()?"Quad":(fmt.doubleBuffer()?"Double":"single")));
    	System.out.printf ("* DepthBuf  | %11s | %11s *\n",
    			bufferToString(req.depth(), req.depthBufferSize()),
    			bufferToString(fmt.depth(), fmt.depthBufferSize()));
    	System.out.printf ("* Color     | %11s | %11s *\n",
    			req.rgba()?"RGBA":"Indexed", fmt.rgba()?"RGBA":"Indexed");
    	System.out.printf ("* AlphaBuf  | %11s | %11s *\n",
    			bufferToString(req.alpha(), req.alphaBufferSize()),
    			bufferToString(fmt.alpha(), fmt.alphaBufferSize()));
    	System.out.printf ("* AccumBuf  | %11s | %11s *\n",
    			bufferToString(req.accum(), req.accumBufferSize()),
    			bufferToString(fmt.accum(), fmt.accumBufferSize()));
    	System.out.printf ("* StencBuf  | %11s | %11s *\n",
    			bufferToString(req.stencil(), req.stencilBufferSize()),
    			bufferToString(fmt.stencil(), fmt.stencilBufferSize()));
    	System.out.printf ("* SampleBuf | %11s | %11s *\n",
    			bufferToString(req.sampleBuffers(), req.samples()),
    			bufferToString(fmt.sampleBuffers(), fmt.samples()));
    	System.out.println("*****************************************");
    	
    	try {
    		// Print out actual OpenGL info (sometimes better than what's reported by Qt)
	        System.out.println("\n**** OpenGL Info ****");
	        System.out.println("* GL_VERSION   : " + glGetString(GL_VERSION)); GLException.openGLErrorCheck();
	        System.out.println("* GL_VENDOR    : " + glGetString(GL_VENDOR)); GLException.openGLErrorCheck();
	        System.out.println("* GL_RENDERER  : " + glGetString(GL_RENDERER)); GLException.openGLErrorCheck();
	        System.out.println("* GLSL_VERSION : " + glGetString(GL_SHADING_LANGUAGE_VERSION)); GLException.openGLErrorCheck();
	
	        System.out.println("**** GLSL Limits ****");
	        System.out.println("* Max varying floats : " + glGetInteger(GL_MAX_VARYING_FLOATS));
	        System.out.println("* Max attributes     : " + glGetInteger(GL_MAX_VERTEX_ATTRIBS));
	        
	        // Print out all supported extensions (mostly cause we can)
	        System.out.println("**** Extensions *****");
	        String extensions = glGetString(GL_EXTENSIONS);
	        StringTokenizer extensionList = new StringTokenizer(extensions, " ");
	        while(extensionList.hasMoreTokens()) {
	            String extension = extensionList.nextToken();
	            GLException.openGLErrorCheck();
	            System.out.println("* " + extension);
	        }
	        
	        System.out.println("*********************\n");
    	} catch(Exception e) {
    		System.err.println("Exception occured while reading OpenGL info:\n");
    		e.printStackTrace();
    	}    		
	}
	
	private static String bufferToString(boolean enabled, int size)
	{
		if(!enabled) { return "Disabled"; }
		else {
			if(size <= 0) { return "Enabled"; }
			return (new Integer(size)).toString();
		}
	}

    // Checking of OpenGL Error state
    public static boolean openGLErrorCheckAndOutput(StackTraceElement trace[]) {
		try { openGLErrorCheck(); }
		catch(GLException e) {
	    	System.err.printf("OpenGL Error: (%s:%d) %s\n",
					trace[1].getFileName(), trace[1].getLineNumber(), e.getMessage());    	
	    	return true;
		}
		
		return false;
    }
    
    public static void openGLErrorCheck() throws GLException
    {
		int error = glGetError();
		switch (error)
		{
			case GL_NO_ERROR: return;
			case GL_INVALID_ENUM: throw new GLInvalidEnumException();
			case GL_INVALID_VALUE: throw new GLInvalidValueException();
			case GL_INVALID_OPERATION: throw new GLInvalidOperationException();
			case GL_OUT_OF_MEMORY: throw new GLOutOfMemoryException();
			case GL_STACK_UNDERFLOW: throw new GLStackUnderflowException();
			case GL_STACK_OVERFLOW: throw new GLStackOverflowException();
			default: throw new GLException("Unrecognized OpenGL Exception.");
		}
    }
}
