// Original implementation: integer prescale followed by a one-destination-pixel transition.
// uPar = inverse source size; uFoco = integer scale, supplied by C.
// No derivatives; no-highp GLES2 compiles only the bilinear fallback.
#define NV_SHARP_BILINEAR_SHADER \
"void main() {\n" \
"#if !defined(GL_ES) || (defined(GL_FRAGMENT_PRECISION_HIGH) && !defined(NV_SGSR_MEDIUMP))\n" \
"  highp vec2 uv = vec2(vUv.x,1.0-vUv.y);\n" \
"  highp vec2 pixel = uv/uPar;\n" \
"  highp vec2 seam = floor(pixel+0.5);\n" \
"  highp vec2 f = clamp((pixel-seam)*uFoco,-0.5,0.5);\n" \
"  vec3 rgb = texture2D(uTex,(seam+f)*uPar).rgb;\n" \
"  gl_FragColor = vec4(rgb,texture2D(uTex,uv).a);\n" \
"#else\n" \
"  gl_FragColor = texture2D(uTex,vec2(vUv.x,1.0-vUv.y));\n" \
"#endif\n" \
"}\n"
