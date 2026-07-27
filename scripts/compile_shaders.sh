#!/bin/bash
set -e

shader_name=$1

mkdir -p build/bin/shaders
slangc assets/shaders/"$shader_name".slang \
       -target spirv \
       -profile glsl_450 \
       -entry vertexMain \
       -stage vertex \
       -o build/bin/shaders/"$shader_name".vert.spv

echo "Compiled Vertex Shader"

slangc assets/shaders/"$shader_name".slang \
       -target spirv \
       -profile glsl_450 \
       -entry fragmentMain \
       -stage fragment \
       -o build/bin/shaders/"$shader_name".frag.spv

echo "Compiled Fragment Shader"
echo "Shaders compiled successfully at 'build/bin/shaders/'"
