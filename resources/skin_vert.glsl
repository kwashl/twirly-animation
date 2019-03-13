#version 120

attribute vec3 aPos;
attribute vec3 aNor;
attribute vec3 aCol;

uniform mat4 P;
uniform mat4 MV;

varying vec3 vColor;

const vec3 lightPos = vec3(1.0, 1.0, 1.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

// Compute shading based on position, normal, and colors.
vec3 phongShading(vec3 pos, vec3 dCol, vec3 sCol){
   vec3 normal = vec3(MV * vec4(aNor, 0.0));
   vec4 vertPos4 = MV * vec4(pos, 1.0);
   vec3 vertPos = vec3(vertPos4) / vertPos4.w;
   vec3 lightDir = normalize(lightPos - vertPos);
   vec3 reflectDir = reflect(-lightDir, normal);
   vec3 viewDir = normalize(-vertPos);
   
   float lambertian = max(dot(lightDir,normal), 0.0);
   float specular = 0.0;
  
   if(lambertian > 0.0) {
      float specAngle = max(dot(reflectDir, viewDir), 0.0);
      specular = pow(specAngle, 4.0);
      specular *= lambertian;
   }
   return lambertian*dCol+ 0.25*specular*sCol;
}

void main() {
	gl_Position = P * (MV * vec4(aPos, 1.0));
   
   vec3 diffusedColor = aCol; 

   // Using normal for diffused color
   vColor = phongShading(aPos, diffusedColor, specColor);
}
