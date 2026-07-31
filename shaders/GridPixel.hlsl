// SV_POSITION is expressed in physical back-buffer pixels. Generating the pattern here
// makes every line exactly one pixel wide and automatically covers any resized viewport.
float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    static const uint spacing = 32;
    const uint2 pixel = uint2(position.xy);
    if ((pixel.x % spacing) != 0 && (pixel.y % spacing) != 0)
    {
        discard;
    }

    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
