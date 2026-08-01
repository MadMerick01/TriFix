// Version 0.05 diagnostic pattern. Coordinates are back-buffer pixel coordinates; no
// perspective, bezel compensation, image capture, or reprojection is performed here.
bool Glyph(float2 p, float2 origin, float scale, uint rows)
{
    int2 cell = int2(floor((p - origin) / scale));
    if (cell.x < 0 || cell.x >= 4 || cell.y < 0 || cell.y >= 7)
        return false;
    uint row = (rows >> (cell.y * 4)) & 15u;
    return (row & (8u >> cell.x)) != 0u;
}

bool Label(float2 p, float2 origin, float scale, uint region)
{
    // Four-by-seven bitmap capitals, stored top row in the low nibble.
    static const uint L = 0xF888888u, E = 0xF88E88Fu, F = 0x888E88Fu;
    static const uint T = 0x222222Fu, C = 0xF88888Fu, N = 0x9999DB9u;
    static const uint R = 0xA99E99Eu, I = 0xF22222Fu, G = 0xF9B889Fu;
    static const uint H = 0x999F999u;
    uint letters[6];
    uint count = 0;
    if (region == 0u) { letters[0]=L; letters[1]=E; letters[2]=F; letters[3]=T; count=4; }
    else if (region == 1u) { letters[0]=C; letters[1]=E; letters[2]=N; letters[3]=T; letters[4]=R; letters[5]=E; count=6; }
    else { letters[0]=R; letters[1]=I; letters[2]=G; letters[3]=H; letters[4]=T; count=5; }
    for (uint index = 0; index < count; ++index)
        if (Glyph(p, origin + float2(index * 6.0f * scale, 0.0f), scale, letters[index])) return true;
    return false;
}

bool Values(float2 p, float2 o, float s)
{
    static const uint D0=0x6999996u,D1=0xF222226u,D2=0xF124896u,D3=0x698116Fu;
    static const uint D4=0x111F99Au,D5=0xE99111Fu,D6=0x699E88Fu,D9=0x7117996u;
    static const uint X=0x9966999u,M=0x9999FF9u;
    // 2560 x 1440 pixels / 620 x 349 mm / 50 degrees, 520 mm eye, 6 mm bezel.
    uint glyphs[30] = {D2,D5,D6,D0,X,D1,D4,D4,D0,0,
                       D6,D2,D0,X,D3,D4,D9,M,M,0,
                       D5,D0,0,D5,D2,D0,0,D6,M,M};
    for (uint i=0; i<30; ++i)
    {
        float2 at = o + float2((i % 10) * 6.0f * s, (i / 10) * 9.0f * s);
        if (Glyph(p, at, s, glyphs[i])) return true;
    }
    return false;
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    float2 p = position.xy;
    uint region = min((uint)(p.x / 2560.0f), 2u);
    float2 q = float2(p.x - region * 2560.0f, p.y);
    float3 colour = float3(0.025f, 0.025f, 0.035f);

    bool minorGrid = (fmod(q.x, 160.0f) < 1.0f || fmod(q.y, 160.0f) < 1.0f);
    bool reference = abs(q.x - 1280.0f) < 2.0f || abs(q.y - 720.0f) < 2.0f;
    bool continuous = abs(p.y - (610.0f + p.x / 96.0f)) < 2.0f;
    bool boundary = abs(p.x - 2560.0f) < 4.0f || abs(p.x - 5120.0f) < 4.0f;
    bool crosshair = (abs(q.x - 1280.0f) < 3.0f && abs(q.y - 720.0f) < 80.0f) ||
                     (abs(q.y - 720.0f) < 3.0f && abs(q.x - 1280.0f) < 80.0f);
    float2 circleDelta = q - float2(700.0f, 1040.0f);
    bool circle = abs(length(circleDelta) - 200.0f) < 3.0f; // 400-pixel diameter
    float2 squareDelta = abs(q - float2(1860.0f, 1040.0f));
    bool square = max(squareDelta.x, squareDelta.y) >= 197.0f &&
                  max(squareDelta.x, squareDelta.y) <= 203.0f; // 400 x 400 pixels
    bool corners = ((q.x < 35.0f || q.x > 2525.0f) && (q.y < 4.0f || q.y > 1436.0f)) ||
                   ((q.y < 35.0f || q.y > 1405.0f) && (q.x < 4.0f || q.x > 2556.0f));

    if (minorGrid) colour = float3(0.12f, 0.12f, 0.16f);
    if (reference || circle || square) colour = float3(0.1f, 0.75f, 0.9f);
    if (continuous) colour = float3(1.0f, 0.75f, 0.1f);
    if (boundary || corners) colour = float3(1.0f, 0.15f, 0.15f);
    if (crosshair) colour = float3(0.2f, 1.0f, 0.25f);
    if (Label(p, float2(region * 2560.0f + 1050.0f, 100.0f), 12.0f, region))
        colour = float3(1.0f, 1.0f, 1.0f);

    // A compact, readable value key: W 2560, H 1440, physical 620x349 mm,
    // yaw 50 degrees, eye 520 mm, bezel 6 mm. The title and README decode the key.
    if (q.x >= 40.0f && q.x < 720.0f && q.y >= 1260.0f && q.y < 1380.0f)
        colour = float3(0.08f, 0.08f, 0.12f);
    if (Values(q, float2(60.0f, 1270.0f), 3.0f))
        colour = float3(1.0f, 1.0f, 1.0f);
    // Pixel-sized bar legend provides known dimensions even where text rasterisation scales.
    if (q.x >= 60.0f && q.x < 460.0f && q.y >= 1290.0f && q.y < 1294.0f)
        colour = float3(1.0f, 1.0f, 1.0f); // exactly 400 px
    return float4(colour, 1.0f);
}
