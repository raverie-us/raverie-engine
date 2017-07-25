String LoadingShaderVertex =
"#version 150\n\
uniform mat4x4 Transform;\n\
uniform mat3x3 UvTransform;\n\
in vec3 attLocalPosition;\n\
in vec2 attUv;\n\
out vec2 psInUv;\n\
void main(void)\n\
{\n\
  psInUv = (vec3(attUv, 1.0) * UvTransform).xy;\n\
  gl_Position = vec4(attLocalPosition, 1.0) * Transform;\n\
}";

String LoadingShaderPixel =
"#version 150\n\
uniform sampler2D Texture;\n\
uniform float Alpha;\n\
in vec2 psInUv;\n\
void main(void)\n\
{\n\
  vec2 uv = vec2(psInUv.x, 1.0 - psInUv.y);\n\
  gl_FragColor = texture(Texture, uv);\n\
  gl_FragColor.xyz *= Alpha;\n\
}";
