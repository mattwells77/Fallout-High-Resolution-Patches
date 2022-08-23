float4x4 WorldViewProj	: WORLDVIEWPROJ;
//bool isGrayScale;
Texture ColorTexture;
Texture PaletteTexture;

sampler ColorTextureSampler;// = sampler_state { texture = <ColorTexture> ; MAGFILTER=POINT; MINFILTER=POINT; };//magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = wrap; AddressV = wrap; };
sampler PaletteTextureSampler = sampler_state { texture = <PaletteTexture> ; MAGFILTER=POINT; MINFILTER=POINT; };//magfilter = NONE; minfilter = NONE; mipfilter = NONE; AddressU = wrap; AddressV = wrap; };


struct a2v {
	float4 Position   	: POSITION0;
	float2 TexCoords  	: TEXCOORD0;
};

struct v2p {
	float4 Position		: POSITION0;
	float2 TexCoords   	: TEXCOORD0;
};

struct p2f {
	float4 Color 		: COLOR0;
};

void vs( in a2v IN, out v2p OUT ) {
	OUT.Position = mul( IN.Position, WorldViewProj );
	OUT.TexCoords = IN.TexCoords;
}

void ps( in v2p IN, out p2f OUT ) {
	float TextureColor = tex2D( ColorTextureSampler, IN.TexCoords );
    float p = clamp(TextureColor.r, 0, 0.999);
    OUT.Color = tex1D( PaletteTextureSampler, p );
	//if(isGrayScale) {
    //   OUT.Color = dot(OUT.Color, float3(0.40, 0.50, 0.10));
    //}
	///OUT.Color.a = 0.999;//OUT.Color.r;

}


technique PaletteTechnique {
    pass p0 {
        //vertexshader = compile vs_1_1 vs();
	    //pixelshader = compile ps_1_4 ps();
        vertexshader = compile vs_2_0 vs();
	    pixelshader = compile ps_2_0 ps();
    }
}
