#define MIN_AMBIENT 0.1 // I really don't want to do this, but just in case

uniform sampler2D u_saoTex;
uniform sampler2D u_shadingTex;
uniform sampler2D u_colorLUT;

varying vec2 v_uvcoord;

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;
const float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 applyColorLUT(sampler2D lut, vec3 color) 
{

    float lutSize = float(textureSize(lut, 0).y);
    color = clamp(color, vec3(0.5 / lutSize), vec3(1.0 - 0.5 / lutSize));
    vec2 texcXY = vec2(color.r / lutSize, 1.0 - color.g);

    int frameZ = int(color.b * lutSize);
    float offsZ = fract(color.b * lutSize); 

    vec3 sample1 = texture(lut, texcXY + vec2(frameZ / lutSize, 0)).rgb;
    vec3 sample2 = texture(lut, texcXY + vec2( (frameZ + 1) / lutSize, 0)).rgb;

    //return mix(sample1, sample2, offsZ);
	return color;
}

// TODO: tweak effectcompositor to make this buffer the same as
// screen size instaad of render buffer size
void main()
{
	ivec2 ssCoord = ivec2(gl_FragCoord.xy);

	float ao = texelFetch(u_saoTex, ssCoord, 0).r;
	ao = (ao + MIN_AMBIENT) / (1.0 + MIN_AMBIENT);

	vec3 shading = texelFetch(u_shadingTex, ssCoord, 0).rgb;

	// consider if ao should go into hdr tonemap or not
	// ao = pow(ao, 1.0 / 2.2);

	vec3 result = ao * shading;

	// tone mapping
	float ExposureBias = 3.0;
    vec3 curr = Uncharted2Tonemap(ExposureBias * result);
    
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
    result = curr * whiteScale;

	// gamma correction
	result = pow(result, vec3(1.0 / 2.2));

	// color grading
	result = applyColorLUT(u_colorLUT, result);

	// vignette
	result *= 1.0 - smoothstep(0, 1, (length( (v_uvcoord - vec2(0.5, 0.5)) * vec2(1.2, 1.0)  ) - 0.22) ) * 1.0;


	gl_FragColor = vec4(result, 1);
	//gl_FragColor = vec4(vec3(ao), 1);
	//gl_FragColor = vec4(shading, 1);
}
