#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;


uniform vec4 ratio;

void main()
{
	FragColor =  ratio.x*texture(texture0,TexCoord)+ratio.y*texture(texture1,TexCoord)+
				 ratio.z*texture(texture2,TexCoord);
}
