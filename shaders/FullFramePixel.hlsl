// TriFix 0.07: inverse map every physical output pixel into one procedural reference canvas.
static const float2 Resolution = float2(2560.0f, 1440.0f);
static const float2 Visible = float2(0.620f, 0.349f);
static const float Bezel = 0.006f;
static const float Depth = 0.520f;
static const float2 ReferenceSize = float2(12.0f, 4.6f);

float3 PanelRight(float yaw) { return float3(cos(yaw), 0.0f, -sin(yaw)); }

float3 PanelCentre(uint panel)
{
    if (panel == 1u) return float3(0.0f, 0.0f, Depth);
    float side = panel == 0u ? -1.0f : 1.0f;
    float hinge = Visible.x * 0.5f + Bezel;
    float yaw = side * radians(50.0f); // accepted left-negative/right-positive convention
    return float3(side * hinge, 0.0f, Depth) + side * hinge * PanelRight(yaw);
}

float2 InverseMap(float2 localPixel, uint panel, out float3 physicalPoint)
{
    float yaw = panel == 0u ? -radians(50.0f) : (panel == 2u ? radians(50.0f) : 0.0f);
    float2 localMetres = float2((localPixel.x / Resolution.x - 0.5f) * Visible.x,
                                (0.5f - localPixel.y / Resolution.y) * Visible.y);
    physicalPoint = PanelCentre(panel) + PanelRight(yaw) * localMetres.x +
                    float3(0.0f, localMetres.y, 0.0f);
    // Intersect the eye ray with z=Depth. This is exactly one perspective division.
    float scale = Depth / physicalPoint.z;
    float2 referenceMetres = physicalPoint.xy * scale;
    return float2(referenceMetres.x / ReferenceSize.x + 0.5f,
                  0.5f - referenceMetres.y / ReferenceSize.y);
}

float3 ReferenceImage(float2 uv)
{
    float2 p = float2(uv.x * 3.0f, uv.y);
    uint band = min((uint)p.x, 2u);
    float3 colour = band == 0u ? float3(0.055f,0.09f,0.14f) :
                    (band == 1u ? float3(0.08f,0.055f,0.12f) : float3(0.14f,0.075f,0.045f));
    // Equal physical increments on the reference plane make genuinely square grid cells.
    float2 grid = frac(uv * ReferenceSize / 0.20f);
    if (grid.x < 0.025f || grid.y < 0.025f) colour = float3(0.23f,0.27f,0.32f);
    float stroke = 0.004f;
    if (abs(uv.y - 0.5f) < stroke) colour = float3(1.0f,0.78f,0.08f); // horizon
    if (abs(uv.x - 0.5f) < stroke / 3.0f || abs(uv.y - 0.5f) < stroke / 3.0f)
        colour = float3(0.2f,1.0f,0.3f);
    if (abs(uv.y - uv.x) < stroke || abs(uv.y - (1.0f - uv.x)) < stroke)
        colour = float3(0.95f,0.25f,0.75f);
    // Large coherent shapes, including circles and squares straddling nominal thirds.
    float2 aspect = float2(ReferenceSize.x / ReferenceSize.y, 1.0f);
    float circles = min(abs(length((uv-float2(1.0f/3.0f,0.32f))*aspect)-0.18f),
                        abs(length((uv-float2(2.0f/3.0f,0.68f))*aspect)-0.18f));
    if (circles < 0.009f) colour = float3(0.1f,0.8f,1.0f);
    float2 square = abs((uv-float2(0.5f,0.5f))*aspect);
    if (abs(max(square.x,square.y)-0.28f) < 0.009f) colour = float3(1.0f,0.35f,0.15f);
    return colour;
}

float4 Render(float4 position, bool diagnostic) : SV_TARGET
{
    uint panel = min((uint)(position.x / Resolution.x), 2u);
    float2 local = float2(position.x - panel * Resolution.x, position.y);
    float3 physicalPoint;
    float2 uv = InverseMap(local, panel, physicalPoint);
    bool valid = all(uv >= 0.0f) && all(uv <= 1.0f);
    float3 colour = valid ? ReferenceImage(uv) : float3(0.8f,0.0f,0.8f);
    if (diagnostic)
    {
        colour = valid ? float3(uv, 0.2f) : float3(1.0f,0.0f,0.0f);
        if (local.x < 5.0f || local.x > Resolution.x-5.0f ||
            local.y < 5.0f || local.y > Resolution.y-5.0f) colour = float3(1.0f,1.0f,1.0f);
        if (length(local-Resolution*0.5f) < 18.0f) colour = float3(0.0f,1.0f,0.2f);
        // Bars encode 620x349 mm, yaw 50, eye 520 mm, bezel 6 mm without font assets.
        if (local.x > 32.0f && local.x < 652.0f && local.y > 1320.0f && local.y < 1328.0f)
            colour = float3(1.0f,0.8f,0.0f);
    }
    return float4(colour, 1.0f);
}

#ifdef TRIFIX_DIAGNOSTIC
float4 main(float4 position : SV_POSITION) : SV_TARGET { return Render(position, true); }
#else
float4 main(float4 position : SV_POSITION) : SV_TARGET { return Render(position, false); }
#endif
