"#include \"AtmosphereShadersCommon.fxh\"\n"
"\n"
"\n"
"cbuffer cbParticipatingMediaScatteringParams\n"
"{\n"
"    AirScatteringAttribs g_MediaParams;\n"
"}\n"
"\n"
"float2 IntegrateParticleDensity(in float3 f3Start, \n"
"                                in float3 f3End,\n"
"                                in float3 f3EarthCentre,\n"
"                                float fNumSteps )\n"
"{\n"
"    float3 f3Step = (f3End - f3Start) / fNumSteps;\n"
"    float fStepLen = length(f3Step);\n"
"        \n"
"    float fStartHeightAboveSurface = abs( length(f3Start - f3EarthCentre) - g_MediaParams.fEarthRadius );\n"
"    float2 f2PrevParticleDensity = exp( -fStartHeightAboveSurface / g_MediaParams.f2ParticleScaleHeight );\n"
"\n"
"    float2 f2ParticleNetDensity = float2(0.0, 0.0);\n"
"    for(float fStepNum = 1.0; fStepNum <= fNumSteps; fStepNum += 1.0)\n"
"    {\n"
"        float3 f3CurrPos = f3Start + f3Step * fStepNum;\n"
"        float fHeightAboveSurface = abs( length(f3CurrPos - f3EarthCentre) - g_MediaParams.fEarthRadius );\n"
"        float2 f2ParticleDensity = exp( -fHeightAboveSurface / g_MediaParams.f2ParticleScaleHeight );\n"
"        f2ParticleNetDensity += (f2ParticleDensity + f2PrevParticleDensity) * fStepLen / 2.0;\n"
"        f2PrevParticleDensity = f2ParticleDensity;\n"
"    }\n"
"    return f2ParticleNetDensity;\n"
"}\n"
"\n"
"float2 IntegrateParticleDensityAlongRay(in float3 f3Pos, \n"
"                                        in float3 f3RayDir,\n"
"                                        float3 f3EarthCentre, \n"
"                                        const float fNumSteps,\n"
"                                        const bool bOccludeByEarth)\n"
"{\n"
"    if( bOccludeByEarth )\n"
"    {\n"
"        // If the ray intersects the Earth, return huge optical depth\n"
"        float2 f2RayEarthIsecs; \n"
"        GetRaySphereIntersection(f3Pos, f3RayDir, f3EarthCentre, g_MediaParams.fEarthRadius, f2RayEarthIsecs);\n"
"        if( f2RayEarthIsecs.x > 0.0 )\n"
"            return float2(1e+20, 1e+20);\n"
"    }\n"
"\n"
"    // Get intersection with the top of the atmosphere (the start point must always be under the top of it)\n"
"    //      \n"
"    //                     /\n"
"    //                .   /  . \n"
"    //      .  \'         /\\         \'  .\n"
"    //                  /  f2RayAtmTopIsecs.y > 0\n"
"    //                 *\n"
"    //                   f2RayAtmTopIsecs.x < 0\n"
"    //                  /\n"
"    //      \n"
"    float2 f2RayAtmTopIsecs;\n"
"    GetRaySphereIntersection(f3Pos, f3RayDir, f3EarthCentre, g_MediaParams.fAtmTopRadius, f2RayAtmTopIsecs);\n"
"    float fIntegrationDist = f2RayAtmTopIsecs.y;\n"
"\n"
"    float3 f3RayEnd = f3Pos + f3RayDir * fIntegrationDist;\n"
"\n"
"    return IntegrateParticleDensity(f3Pos, f3RayEnd, f3EarthCentre, fNumSteps);\n"
"}\n"
"\n"
"void PrecomputeNetDensityToAtmTopPS( FullScreenTriangleVSOutput VSOut,\n"
"                                     out float2 f2Density : SV_Target0 )\n"
"{\n"
"    float2 f2UV = NormalizedDeviceXYToTexUV(VSOut.f2NormalizedXY);\n"
"    // Do not allow start point be at the Earth surface and on the top of the atmosphere\n"
"    float fStartHeight = clamp( lerp(0.0, g_MediaParams.fAtmTopHeight, f2UV.x), 10.0, g_MediaParams.fAtmTopHeight-10.0 );\n"
"\n"
"    float fCosTheta = f2UV.y * 2.0 - 1.0;\n"
"    float fSinTheta = sqrt( saturate(1.0 - fCosTheta*fCosTheta) );\n"
"    float3 f3RayStart = float3(0.0, 0.0, fStartHeight);\n"
"    float3 f3RayDir = float3(fSinTheta, 0.0, fCosTheta);\n"
"    \n"
"    float3 f3EarthCentre = float3(0.0, 0.0, -g_MediaParams.fEarthRadius);\n"
"\n"
"    const float fNumSteps = 200.0;\n"
"    f2Density = IntegrateParticleDensityAlongRay(f3RayStart, f3RayDir, f3EarthCentre, fNumSteps, true);\n"
"}\n"
