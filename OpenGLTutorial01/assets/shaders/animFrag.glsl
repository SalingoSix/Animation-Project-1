#version 400

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 fTangent;		// For bump (or normal) mapping
in vec3 fBitangent;

out vec4 FragColor;

uniform int activeBoy;

void main()
{	
	if (activeBoy == 0)
	{	//Inactive boy is yellow
		FragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
	else
	{	//Active boy is blue
		FragColor = vec4(70.0 / 255.0, 130.0 / 255.0, 180.0 / 255.0, 1.0);
	}
}