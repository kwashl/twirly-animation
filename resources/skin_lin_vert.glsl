#version 120

attribute vec3 aPos;
attribute vec3 aNor;
attribute vec4 w0;
attribute vec4 w1;
attribute vec4 w2;
attribute vec4 w3;
attribute vec4 w4;

// TODO Add matrix uniforms
uniform int selBone;

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

void main(){
   vColor = aNor;
   vec4 newPos = vec4(0.0,0.0,0.0,0.0);
   // TODO Compute new position (newPos)

   // Set final position
   gl_Position = P * (MV * newPos);

   vec3 diffusedColor;

   // Check selection of bone.
   if(selBone<0 || selBone>=18) diffusedColor = 0.5*aNor+0.5; 
   else{
      // TODO Bone is selected. Compute diffused color to display weights.
      // HINT Use getColorFromWeight function in ShapeSkin class for reference.
   }
   
   vColor = phongShading(vec3(newPos), diffusedColor, specColor);
}
