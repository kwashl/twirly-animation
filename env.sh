test "$?BASH_VERSION" = "0" || eval 'setenv() { export "$1=$2"; }'

setenv A4_DEPS $PWD/deps/
setenv GLEW_DIR $A4_DEPS/glew
setenv GLFW_DIR $A4_DEPS/glfw
setenv GLM_INCLUDE_DIR $A4_DEPS/glm/
