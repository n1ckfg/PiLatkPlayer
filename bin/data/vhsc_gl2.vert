#version 120

varying vec2 varyingtexcoord;

void main() {
    varyingtexcoord = gl_MultiTexCoord0.xy;

    // send the vertices to the fragment shader
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;;
}