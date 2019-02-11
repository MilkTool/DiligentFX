"#include \"AtmosphereShadersCommon.fxh\"\n"
"\n"
"cbuffer cbParticipatingMediaScatteringParams\n"
"{\n"
"    AirScatteringAttribs g_MediaParams;\n"
"}\n"
"\n"
"#include \"LookUpTables.fxh\"\n"
"\n"
"Texture2D<float3> g_tex2DSphereRandomSampling;\n"
"\n"
"Texture3D<float3> g_tex3DMultipleSctrLUT;\n"
"SamplerState g_tex3DMultipleSctrLUT_sampler;\n"
"\n"
"\n"
"void PrecomputeAmbientSkyLightPS(ScreenSizeQuadVSOutput VSOut,\n"
"                                 // IMPORTANT: non-system generated pixel shader input\n"
"                                 // arguments must have the exact same name as vertex shader \n"
"                                 // outputs and must go in the same order.\n"
"\n"
"                                 out float4 f4SkyLight : SV_Target)\n"
"{\n"
"    float fU = NormalizedDeviceXYToTexUV(VSOut.f2NormalizedXY).x;\n"
"    float3 f3RayStart = float3(0.0, 20.0, 0.0);\n"
"    float3 f3EarthCentre =  -float3(0.0, 1.0, 0.0) * EARTH_RADIUS;\n"
"    float fCosZenithAngle = clamp(fU * 2.0 - 1.0, -1.0, +1.0);\n"
"    float3 f3DirOnLight = float3(sqrt(saturate(1.0 - fCosZenithAngle*fCosZenithAngle)), fCosZenithAngle, 0.0);\n"
"    f4SkyLight = F4ZERO;\n"
"    // Go through a number of random directions on the sphere\n"
"    for(int iSample = 0; iSample < NUM_RANDOM_SPHERE_SAMPLES; ++iSample)\n"
"    {\n"
"        // Get random direction\n"
"        float3 f3RandomDir = normalize( g_tex2DSphereRandomSampling.Load(int3(iSample,0,0)) );\n"
"        // Reflect directions from the lower hemisphere\n"
"        f3RandomDir.y = abs(f3RandomDir.y);\n"
"        // Get multiple scattered light radiance when looking in direction f3RandomDir (the light thus goes in direction -f3RandomDir)\n"
"        float4 f4UVWQ = -F4ONE;\n"
"        float3 f3Sctr = LookUpPrecomputedScattering(f3RayStart, f3RandomDir, f3EarthCentre, f3DirOnLight.xyz, g_tex3DMultipleSctrLUT, g_tex3DMultipleSctrLUT_sampler, f4UVWQ); \n"
"        // Accumulate ambient irradiance through the horizontal plane\n"
"        f4SkyLight.rgb += f3Sctr * dot(f3RandomDir, float3(0.0, 1.0, 0.0));\n"
"    }\n"
"    // Each sample covers 2 * PI / NUM_RANDOM_SPHERE_SAMPLES solid angle (integration is performed over\n"
"    // upper hemisphere)\n"
"    f4SkyLight.rgb *= 2.0 * PI / float(NUM_RANDOM_SPHERE_SAMPLES);\n"
"}\n"
