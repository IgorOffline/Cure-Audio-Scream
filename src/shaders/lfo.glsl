@vs common_vs
in vec4 position;
in vec2 coord;
out vec2 uv;

void main() {
    gl_Position = position;
    uv = coord;
}
@end

@fs vertical_grad_fs

in vec2 uv;
out vec4 frag_color;

layout(binding=0) uniform lfo_vertical_grad_uniforms {
    vec4 colour1;
    vec4 colour2;
    float buffer_len;
};

struct lfo_buffer_item {
    float y;
};

layout(binding=0) readonly buffer lfo_storage_buffer {
    lfo_buffer_item y_buffer[];
};

void main() {
    uint idx = uint(min(uv.x * buffer_len, buffer_len - 1));
    float buf_y = y_buffer[idx].y;

    vec4 interp_col = mix(colour2, colour1, uv.y);
    interp_col.a = mix(colour2.a, colour1.a, uv.y);

    frag_color = uv.y < buf_y ? interp_col : vec4(0);
}

@end

@program lfo_vertical_grad common_vs vertical_grad_fs