#version 120

attribute vec3 aPos;
attribute vec3 aNor;
attribute vec4 w0;
attribute vec4 w1;
attribute vec4 w2;
attribute vec4 w3;
attribute vec4 w4;
uniform mat4 M[18];

// TODO Add matrix uniforms
uniform int selBone;

uniform mat4 P;
uniform mat4 MV;

varying vec3 vColor;

const vec3 lightPos = vec3(1.0, -3.0, 1.0);
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
   float w[18];

   w[0] = w0.x;
   w[1] = w0.y;
   w[2] = w0.z;
   w[3] = w0.w;
   w[4] = w1.x;
   w[5] = w1.y;
   w[6] = w1.z;
   w[7] = w1.w;
   w[8] = w2.x;
   w[9] = w2.y;
   w[10] = w2.z;
   w[11] = w2.w;
   w[12] = w3.x;
   w[13] = w3.y;
   w[14] = w3.z;
   w[15] = w3.w;
   w[16] = w4.x;
   w[17] = w4.y;
   vColor = aNor;

   vec4 newPos = vec4(0.0,0.0,0.0,0);
   // TODO: compute the new skinned vertex location
 for (int j=0; j<18; j++) {
     // TODO:  replace the following line
   //   newPos = vec4(aPos,1.0);    // leave the vertex position unchanged
     newPos += w[j] * M[j] * vec4(aPos,1.0); // CHANGED CODE
  }

   // Set final position
   gl_Position = P * (MV * newPos);   // multiply by Projection and ModelView matrices

   vec3 diffusedColor;

   // Check selection of bone.
   if(selBone<0 || selBone>=18) diffusedColor = 0.5*aNor+0.5; 
   else{
     // could set the bone colour according to the weights; not required
   }
   
   vColor = phongShading(vec3(newPos), diffusedColor, specColor);
}
