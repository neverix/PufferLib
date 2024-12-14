#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

const vec2 size = vec2(800, 450);   // Framebuffer size
const float samples = 5.0;          // Pixels per axis; higher = bigger glow, worse performance
const float quality = 2.5;          // Defines size factor: Lower = smaller glow, better quality

void main() {
    // vec4 sum = vec4(0);
    // vec2 sizeFactor = vec2(1) / size * quality;

    // // Texel color fetching from texture sampler
    // vec4 source = texture(texture0, fragTexCoord);

    // const int range = 2;            // should be = (samples - 1)/2;

    // for(int x = -range; x <= range; x++) {
    //     for(int y = -range; y <= range; y++) {
    //         sum += texture(texture0, fragTexCoord + vec2(x, y) * sizeFactor);
    //     }
    // }

    // // Calculate final fragment color
    // finalColor = ((sum / (samples * samples)) + source) * colDiffuse / range;
    // finalColor = gl_FragDepth > 10 ? texture(texture0, fragTexCoord) : 0;
    // finalColor = texture(texture0, fragTexCoord) * (clamp(length(fragPosition) / 1000.0, 0.0, 1.0));
    // finalColor = texture(texture0, fragTexCoord) * (clamp(length(fragPosition) / 1000.0, 0.0, 1.0));
    // finalColor = texture(texture0, fragTexCoord) * (fragDepth / 100.0);
    // float fd = clamp(fragDepth / 100.0, 0, 1);
    // float fd = gl_FragCoord.z / 10.0;
    // float fd = gl_FragDepth / 10.0;
    // finalColor = vec4(fd, fd, fd, 1.0);
    // float fd = -fragNormal.z;
    // float fd = gl_FragCoord.z;
    // finalColor = vec4(fd, fd, fd, 1.0);
    // finalColor = fragColor;
    // finalColor = texture(texture0, fragTexCoord) * 0.8;
    // float fd = max(0.0, dot(fragNormal, normalize(vec3(0, 0.3, -1))));
    // fd = clamp(fd, 0.2, 0.8);
    // float fd = fragNormal.z;
    // finalColor = vec4(fd, fd, fd, 1.0);
    // finalColor = texture(texture0, fragTexCoord) * (fragNormal.x + 0.5);
    finalColor = fragColor;
}