#version 430

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;
in vec3 vWorldPos;

uniform sampler2D stickerTex;

// Sticker projection plane (world space)
uniform vec3  stickerCenter;
uniform vec3  stickerRight;       // world-space right axis of the sticker
uniform vec3  stickerUp;          // world-space up axis of the sticker
uniform vec2  stickerHalfSize;    // half-extents in world units (w, h)

// Sticker transform
uniform float stickerRotation;    // radians, applied in UV space
uniform vec2  stickerOffset;      // UV translation
uniform vec2  stickerRepeat;      // tiling count (>= 1)
uniform float stickerBlend;       // opacity 0..1

out vec4 vFragColor;

const float Shininess = 128.0;

void main(void)
{
    // ── Planar projection ──────────────────────────────────────────
    vec3 delta = vWorldPos - stickerCenter;
    float u = dot(delta, stickerRight) / stickerHalfSize.x; // [-1, 1]
    float v = dot(delta, stickerUp)    / stickerHalfSize.y; // [-1, 1]

    // ── UV rotation ───────────────────────────────────────────────
    float cosR = cos(stickerRotation);
    float sinR = sin(stickerRotation);
    float ru = cosR * u - sinR * v;
    float rv = sinR * u + cosR * v;

    // ── Remap [-1,1] → [0,1], apply offset ───────────────────────
    ru = ru * 0.5 + 0.5 + stickerOffset.x;
    rv = rv * 0.5 + 0.5 + stickerOffset.y;

    // ── Tiling ────────────────────────────────────────────────────
    float tu = ru * stickerRepeat.x;
    float tv = rv * stickerRepeat.y;

    // Discard fragments outside the tiled sticker region
    if (tu < 0.0 || tu > stickerRepeat.x ||
        tv < 0.0 || tv > stickerRepeat.y)
        discard;

    // ── Sample texture ────────────────────────────────────────────
    // fract maps each tile to [0,1); V is already upright (stbi flipped on load)
    vec2 texUV = vec2(fract(tu), fract(tv));
    vec4 texColor = texture(stickerTex, texUV);

    if (texColor.a < 0.01) discard;

    // ── Phong lighting applied to sticker colour ──────────────────
    vec3 N = normalize(vVaryingNormal);
    vec3 L = normalize(vVaryingLightDir);
    float diff = max(0.0, dot(N, L));

    vec3 R = normalize(reflect(-L, N));
    float spec = (diff > 0.0) ? pow(max(0.0, dot(N, R)), Shininess) : 0.0;

    // ambient 0.15 + diffuse 0.85 + subtle specular
    vec3 litColor = texColor.rgb * (0.15 + 0.85 * diff) + 0.25 * spec;

    vFragColor = vec4(litColor, texColor.a * stickerBlend);
}
