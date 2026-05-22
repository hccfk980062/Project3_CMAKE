#version 430

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vertexNormal;

uniform MatVP{
	mat4 View;
	mat4 Projection;
};
uniform mat4 Model;

void main(void) 
{ 
	mat4 MVP = Projection * View * Model;
	mat4 MV = View * Model;
	
    // Don't forget to transform the geometry!
    gl_Position = MVP * vec4(vPosition, 1);
}
