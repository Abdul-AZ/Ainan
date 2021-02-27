set(GLSL_SHADERS
    -v "${GLSL_SHADERS_DIR}/CircleOutline.vert" -f "${GLSL_SHADERS_DIR}/CircleOutline.frag" -o "${GLSL_SHADERS_DIR}/CircleOutline.cso"
    -v "${GLSL_SHADERS_DIR}/FlatColor.vert" -f "${GLSL_SHADERS_DIR}/FlatColor.frag" -o "${GLSL_SHADERS_DIR}/FlatColor.cso"
    -v "${GLSL_SHADERS_DIR}/Gizmo.vert" -f "${GLSL_SHADERS_DIR}/Gizmo.frag" -o "${GLSL_SHADERS_DIR}/Gizmo.cso"
    -v "${GLSL_SHADERS_DIR}/Grid.vert" -f "${GLSL_SHADERS_DIR}/Grid.frag" -o "${GLSL_SHADERS_DIR}/Grid.cso"
    -v "${GLSL_SHADERS_DIR}/Image.vert" -f "${GLSL_SHADERS_DIR}/Image.frag" -o "${GLSL_SHADERS_DIR}/Image.cso"
    -v "${GLSL_SHADERS_DIR}/Image.vert" -f "${GLSL_SHADERS_DIR}/Blur.frag" -o "${GLSL_SHADERS_DIR}/Blur.cso"
    -v "${GLSL_SHADERS_DIR}/LitSprite.vert" -f "${GLSL_SHADERS_DIR}/LitSprite.frag" -o "${GLSL_SHADERS_DIR}/LitSprite.cso"
    -v "${GLSL_SHADERS_DIR}/QuadBatch.vert" -f "${GLSL_SHADERS_DIR}/QuadBatch.frag" -o "${GLSL_SHADERS_DIR}/QuadBatch.cso"
    -v "${GLSL_SHADERS_DIR}/3DAmbient.vert" -f "${GLSL_SHADERS_DIR}/3DAmbient.frag" -o "${GLSL_SHADERS_DIR}/3DAmbient.cso"
    )