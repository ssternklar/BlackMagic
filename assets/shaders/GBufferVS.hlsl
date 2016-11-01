

struct Vertex
{ 
	float3 position		: POSITION;  
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( Vertex input )
{
	VertexToPixel output;

	output.position = input.position;

	return output;
}