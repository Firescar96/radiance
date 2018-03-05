#property description Cycle quickly through the rainbow according to lightness, giving things a rainbow sheen

void main(void) {
    fragColor = texture(iInput, uv);
    vec3 hsv = rgb2hsv(fragColor.rgb);
    float phase = hsv.x + hsv.z * iIntensity * 30. * (1. - hsv.z) + iIntensityIntegral * iFrequency;
    hsv.x = mod(phase, 1.);
    fragColor.rgb = hsv2rgb(hsv);
}
