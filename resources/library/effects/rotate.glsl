#property description Rotate the screen

void main(void) {
    vec2 normCoord = (uv - 0.5) * aspectCorrection;

    float r = iIntensity;
    r *= (sawtooth(iTime * iFrequency / 2.0, 0.5) - 0.5) * 2;
    float s = sin(r * M_PI);
    float c = cos(r * M_PI);
    mat2 rot = mat2(c, -s, s, c);

    vec2 newUV = normCoord * rot / aspectCorrection + 0.5;

    fragColor = texture(iInput, newUV);
    fragColor *= box(newUV);
}
