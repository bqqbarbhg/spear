#include "EnvLightSystem.h"

#include "client/LightSystem.h"
#include "client/MiscTexture.h"

#include "sp/Renderer.h"

#include "game/shader/GameShaders.h"
#include "game/shader/EnvmapBlend.h"
#include "game/shader2/GameShaders2.h"

#include "sf/Random.h"

namespace cl {

static sf::Vec3 sampleSphereUniform(const sf::Vec2 &uv)
{
	float theta = uv.x * sf::F_2PI;
	float y = uv.y * 2.0f - 1.0f;
	float r = sf::sqrt(sf::max(0.0f, 1.0f - y*y));
	return sf::Vec3(cosf(theta) * r, y, sinf(theta) * r);
}

// misc/halton_sequence_2d.py
static const sf::Vec2 halton2D[] = {
	{ 0.000000f, 0.000000f },
	{ 0.200000f, 0.142857f },
	{ 0.400000f, 0.285714f },
	{ 0.600000f, 0.428571f },
	{ 0.800000f, 0.571429f },
	{ 0.040000f, 0.714286f },
	{ 0.240000f, 0.857143f },
	{ 0.440000f, 0.020408f },
	{ 0.640000f, 0.163265f },
	{ 0.840000f, 0.306122f },
	{ 0.080000f, 0.448980f },
	{ 0.280000f, 0.591837f },
	{ 0.480000f, 0.734694f },
	{ 0.680000f, 0.877551f },
	{ 0.880000f, 0.040816f },
	{ 0.120000f, 0.183673f },
	{ 0.320000f, 0.326531f },
	{ 0.520000f, 0.469388f },
	{ 0.720000f, 0.612245f },
	{ 0.920000f, 0.755102f },
	{ 0.160000f, 0.897959f },
	{ 0.360000f, 0.061224f },
	{ 0.560000f, 0.204082f },
	{ 0.760000f, 0.346939f },
	{ 0.960000f, 0.489796f },
	{ 0.008000f, 0.632653f },
	{ 0.208000f, 0.775510f },
	{ 0.408000f, 0.918367f },
	{ 0.608000f, 0.081633f },
	{ 0.808000f, 0.224490f },
	{ 0.048000f, 0.367347f },
	{ 0.248000f, 0.510204f },
	{ 0.448000f, 0.653061f },
	{ 0.648000f, 0.795918f },
	{ 0.848000f, 0.938776f },
	{ 0.088000f, 0.102041f },
	{ 0.288000f, 0.244898f },
	{ 0.488000f, 0.387755f },
	{ 0.688000f, 0.530612f },
	{ 0.888000f, 0.673469f },
	{ 0.128000f, 0.816327f },
	{ 0.328000f, 0.959184f },
	{ 0.528000f, 0.122449f },
	{ 0.728000f, 0.265306f },
	{ 0.928000f, 0.408163f },
	{ 0.168000f, 0.551020f },
	{ 0.368000f, 0.693878f },
	{ 0.568000f, 0.836735f },
	{ 0.768000f, 0.979592f },
	{ 0.968000f, 0.002915f },
	{ 0.016000f, 0.145773f },
	{ 0.216000f, 0.288630f },
	{ 0.416000f, 0.431487f },
	{ 0.616000f, 0.574344f },
	{ 0.816000f, 0.717201f },
	{ 0.056000f, 0.860058f },
	{ 0.256000f, 0.023324f },
	{ 0.456000f, 0.166181f },
	{ 0.656000f, 0.309038f },
	{ 0.856000f, 0.451895f },
	{ 0.096000f, 0.594752f },
	{ 0.296000f, 0.737609f },
	{ 0.496000f, 0.880466f },
	{ 0.696000f, 0.043732f },
	{ 0.896000f, 0.186589f },
	{ 0.136000f, 0.329446f },
	{ 0.336000f, 0.472303f },
	{ 0.536000f, 0.615160f },
	{ 0.736000f, 0.758017f },
	{ 0.936000f, 0.900875f },
	{ 0.176000f, 0.064140f },
	{ 0.376000f, 0.206997f },
	{ 0.576000f, 0.349854f },
	{ 0.776000f, 0.492711f },
	{ 0.976000f, 0.635569f },
	{ 0.024000f, 0.778426f },
	{ 0.224000f, 0.921283f },
	{ 0.424000f, 0.084548f },
	{ 0.624000f, 0.227405f },
	{ 0.824000f, 0.370262f },
	{ 0.064000f, 0.513120f },
	{ 0.264000f, 0.655977f },
	{ 0.464000f, 0.798834f },
	{ 0.664000f, 0.941691f },
	{ 0.864000f, 0.104956f },
	{ 0.104000f, 0.247813f },
	{ 0.304000f, 0.390671f },
	{ 0.504000f, 0.533528f },
	{ 0.704000f, 0.676385f },
	{ 0.904000f, 0.819242f },
	{ 0.144000f, 0.962099f },
	{ 0.344000f, 0.125364f },
	{ 0.544000f, 0.268222f },
	{ 0.744000f, 0.411079f },
	{ 0.944000f, 0.553936f },
	{ 0.184000f, 0.696793f },
	{ 0.384000f, 0.839650f },
	{ 0.584000f, 0.982507f },
	{ 0.784000f, 0.005831f },
	{ 0.984000f, 0.148688f },
	{ 0.032000f, 0.291545f },
	{ 0.232000f, 0.434402f },
	{ 0.432000f, 0.577259f },
	{ 0.632000f, 0.720117f },
	{ 0.832000f, 0.862974f },
	{ 0.072000f, 0.026239f },
	{ 0.272000f, 0.169096f },
	{ 0.472000f, 0.311953f },
	{ 0.672000f, 0.454810f },
	{ 0.872000f, 0.597668f },
	{ 0.112000f, 0.740525f },
	{ 0.312000f, 0.883382f },
	{ 0.512000f, 0.046647f },
	{ 0.712000f, 0.189504f },
	{ 0.912000f, 0.332362f },
	{ 0.152000f, 0.475219f },
	{ 0.352000f, 0.618076f },
	{ 0.552000f, 0.760933f },
	{ 0.752000f, 0.903790f },
	{ 0.952000f, 0.067055f },
	{ 0.192000f, 0.209913f },
	{ 0.392000f, 0.352770f },
	{ 0.592000f, 0.495627f },
	{ 0.792000f, 0.638484f },
	{ 0.992000f, 0.781341f },
	{ 0.001600f, 0.924198f },
	{ 0.201600f, 0.087464f },
	{ 0.401600f, 0.230321f },
	{ 0.601600f, 0.373178f },
	{ 0.801600f, 0.516035f },
	{ 0.041600f, 0.658892f },
	{ 0.241600f, 0.801749f },
	{ 0.441600f, 0.944606f },
	{ 0.641600f, 0.107872f },
	{ 0.841600f, 0.250729f },
	{ 0.081600f, 0.393586f },
	{ 0.281600f, 0.536443f },
	{ 0.481600f, 0.679300f },
	{ 0.681600f, 0.822157f },
	{ 0.881600f, 0.965015f },
	{ 0.121600f, 0.128280f },
	{ 0.321600f, 0.271137f },
	{ 0.521600f, 0.413994f },
	{ 0.721600f, 0.556851f },
	{ 0.921600f, 0.699708f },
	{ 0.161600f, 0.842566f },
	{ 0.361600f, 0.985423f },
	{ 0.561600f, 0.008746f },
	{ 0.761600f, 0.151603f },
	{ 0.961600f, 0.294461f },
	{ 0.009600f, 0.437318f },
	{ 0.209600f, 0.580175f },
	{ 0.409600f, 0.723032f },
	{ 0.609600f, 0.865889f },
	{ 0.809600f, 0.029155f },
	{ 0.049600f, 0.172012f },
	{ 0.249600f, 0.314869f },
	{ 0.449600f, 0.457726f },
	{ 0.649600f, 0.600583f },
	{ 0.849600f, 0.743440f },
	{ 0.089600f, 0.886297f },
	{ 0.289600f, 0.049563f },
	{ 0.489600f, 0.192420f },
	{ 0.689600f, 0.335277f },
	{ 0.889600f, 0.478134f },
	{ 0.129600f, 0.620991f },
	{ 0.329600f, 0.763848f },
	{ 0.529600f, 0.906706f },
	{ 0.729600f, 0.069971f },
	{ 0.929600f, 0.212828f },
	{ 0.169600f, 0.355685f },
	{ 0.369600f, 0.498542f },
	{ 0.569600f, 0.641399f },
	{ 0.769600f, 0.784257f },
	{ 0.969600f, 0.927114f },
	{ 0.017600f, 0.090379f },
	{ 0.217600f, 0.233236f },
	{ 0.417600f, 0.376093f },
	{ 0.617600f, 0.518950f },
	{ 0.817600f, 0.661808f },
	{ 0.057600f, 0.804665f },
	{ 0.257600f, 0.947522f },
	{ 0.457600f, 0.110787f },
	{ 0.657600f, 0.253644f },
	{ 0.857600f, 0.396501f },
	{ 0.097600f, 0.539359f },
	{ 0.297600f, 0.682216f },
	{ 0.497600f, 0.825073f },
	{ 0.697600f, 0.967930f },
	{ 0.897600f, 0.131195f },
	{ 0.137600f, 0.274052f },
	{ 0.337600f, 0.416910f },
	{ 0.537600f, 0.559767f },
	{ 0.737600f, 0.702624f },
	{ 0.937600f, 0.845481f },
	{ 0.177600f, 0.988338f },
	{ 0.377600f, 0.011662f },
	{ 0.577600f, 0.154519f },
	{ 0.777600f, 0.297376f },
	{ 0.977600f, 0.440233f },
	{ 0.025600f, 0.583090f },
	{ 0.225600f, 0.725948f },
	{ 0.425600f, 0.868805f },
	{ 0.625600f, 0.032070f },
	{ 0.825600f, 0.174927f },
	{ 0.065600f, 0.317784f },
	{ 0.265600f, 0.460641f },
	{ 0.465600f, 0.603499f },
	{ 0.665600f, 0.746356f },
	{ 0.865600f, 0.889213f },
	{ 0.105600f, 0.052478f },
	{ 0.305600f, 0.195335f },
	{ 0.505600f, 0.338192f },
	{ 0.705600f, 0.481050f },
	{ 0.905600f, 0.623907f },
	{ 0.145600f, 0.766764f },
	{ 0.345600f, 0.909621f },
	{ 0.545600f, 0.072886f },
	{ 0.745600f, 0.215743f },
	{ 0.945600f, 0.358601f },
	{ 0.185600f, 0.501458f },
	{ 0.385600f, 0.644315f },
	{ 0.585600f, 0.787172f },
	{ 0.785600f, 0.930029f },
	{ 0.985600f, 0.093294f },
	{ 0.033600f, 0.236152f },
	{ 0.233600f, 0.379009f },
	{ 0.433600f, 0.521866f },
	{ 0.633600f, 0.664723f },
	{ 0.833600f, 0.807580f },
	{ 0.073600f, 0.950437f },
	{ 0.273600f, 0.113703f },
	{ 0.473600f, 0.256560f },
	{ 0.673600f, 0.399417f },
	{ 0.873600f, 0.542274f },
	{ 0.113600f, 0.685131f },
	{ 0.313600f, 0.827988f },
	{ 0.513600f, 0.970845f },
	{ 0.713600f, 0.134111f },
	{ 0.913600f, 0.276968f },
	{ 0.153600f, 0.419825f },
	{ 0.353600f, 0.562682f },
	{ 0.553600f, 0.705539f },
	{ 0.753600f, 0.848397f },
	{ 0.953600f, 0.991254f },
	{ 0.193600f, 0.014577f },
	{ 0.393600f, 0.157434f },
	{ 0.593600f, 0.300292f },
	{ 0.793600f, 0.443149f },
	{ 0.993600f, 0.586006f },
	{ 0.003200f, 0.728863f },
	{ 0.203200f, 0.871720f },
	{ 0.403200f, 0.034985f },
	{ 0.603200f, 0.177843f },
	{ 0.803200f, 0.320700f },
	{ 0.043200f, 0.463557f },
	{ 0.243200f, 0.606414f },
	{ 0.443200f, 0.749271f },
	{ 0.643200f, 0.892128f },
	{ 0.843200f, 0.055394f },
	{ 0.083200f, 0.198251f },
	{ 0.283200f, 0.341108f },
	{ 0.483200f, 0.483965f },
	{ 0.683200f, 0.626822f },
	{ 0.883200f, 0.769679f },
	{ 0.123200f, 0.912536f },
	{ 0.323200f, 0.075802f },
	{ 0.523200f, 0.218659f },
	{ 0.723200f, 0.361516f },
	{ 0.923200f, 0.504373f },
	{ 0.163200f, 0.647230f },
	{ 0.363200f, 0.790087f },
	{ 0.563200f, 0.932945f },
	{ 0.763200f, 0.096210f },
	{ 0.963200f, 0.239067f },
	{ 0.011200f, 0.381924f },
	{ 0.211200f, 0.524781f },
	{ 0.411200f, 0.667638f },
	{ 0.611200f, 0.810496f },
	{ 0.811200f, 0.953353f },
	{ 0.051200f, 0.116618f },
	{ 0.251200f, 0.259475f },
	{ 0.451200f, 0.402332f },
	{ 0.651200f, 0.545190f },
	{ 0.851200f, 0.688047f },
	{ 0.091200f, 0.830904f },
	{ 0.291200f, 0.973761f },
	{ 0.491200f, 0.137026f },
	{ 0.691200f, 0.279883f },
	{ 0.891200f, 0.422741f },
	{ 0.131200f, 0.565598f },
	{ 0.331200f, 0.708455f },
	{ 0.531200f, 0.851312f },
	{ 0.731200f, 0.994169f },
	{ 0.931200f, 0.017493f },
	{ 0.171200f, 0.160350f },
	{ 0.371200f, 0.303207f },
	{ 0.571200f, 0.446064f },
	{ 0.771200f, 0.588921f },
	{ 0.971200f, 0.731778f },
	{ 0.019200f, 0.874636f },
	{ 0.219200f, 0.037901f },
	{ 0.419200f, 0.180758f },
	{ 0.619200f, 0.323615f },
	{ 0.819200f, 0.466472f },
	{ 0.059200f, 0.609329f },
	{ 0.259200f, 0.752187f },
	{ 0.459200f, 0.895044f },
	{ 0.659200f, 0.058309f },
	{ 0.859200f, 0.201166f },
	{ 0.099200f, 0.344023f },
	{ 0.299200f, 0.486880f },
	{ 0.499200f, 0.629738f },
	{ 0.699200f, 0.772595f },
	{ 0.899200f, 0.915452f },
	{ 0.139200f, 0.078717f },
	{ 0.339200f, 0.221574f },
	{ 0.539200f, 0.364431f },
	{ 0.739200f, 0.507289f },
	{ 0.939200f, 0.650146f },
	{ 0.179200f, 0.793003f },
	{ 0.379200f, 0.935860f },
	{ 0.579200f, 0.099125f },
	{ 0.779200f, 0.241983f },
	{ 0.979200f, 0.384840f },
	{ 0.027200f, 0.527697f },
	{ 0.227200f, 0.670554f },
	{ 0.427200f, 0.813411f },
	{ 0.627200f, 0.956268f },
	{ 0.827200f, 0.119534f },
	{ 0.067200f, 0.262391f },
	{ 0.267200f, 0.405248f },
	{ 0.467200f, 0.548105f },
	{ 0.667200f, 0.690962f },
	{ 0.867200f, 0.833819f },
	{ 0.107200f, 0.976676f },
	{ 0.307200f, 0.139942f },
	{ 0.507200f, 0.282799f },
	{ 0.707200f, 0.425656f },
	{ 0.907200f, 0.568513f },
	{ 0.147200f, 0.711370f },
	{ 0.347200f, 0.854227f },
	{ 0.547200f, 0.997085f },
	{ 0.747200f, 0.000416f },
	{ 0.947200f, 0.143274f },
	{ 0.187200f, 0.286131f },
	{ 0.387200f, 0.428988f },
	{ 0.587200f, 0.571845f },
	{ 0.787200f, 0.714702f },
	{ 0.987200f, 0.857559f },
	{ 0.035200f, 0.020825f },
	{ 0.235200f, 0.163682f },
	{ 0.435200f, 0.306539f },
	{ 0.635200f, 0.449396f },
	{ 0.835200f, 0.592253f },
	{ 0.075200f, 0.735110f },
	{ 0.275200f, 0.877968f },
	{ 0.475200f, 0.041233f },
	{ 0.675200f, 0.184090f },
	{ 0.875200f, 0.326947f },
	{ 0.115200f, 0.469804f },
	{ 0.315200f, 0.612661f },
	{ 0.515200f, 0.755519f },
	{ 0.715200f, 0.898376f },
	{ 0.915200f, 0.061641f },
	{ 0.155200f, 0.204498f },
	{ 0.355200f, 0.347355f },
	{ 0.555200f, 0.490212f },
	{ 0.755200f, 0.633070f },
	{ 0.955200f, 0.775927f },
	{ 0.195200f, 0.918784f },
	{ 0.395200f, 0.082049f },
	{ 0.595200f, 0.224906f },
	{ 0.795200f, 0.367763f },
	{ 0.995200f, 0.510621f },
	{ 0.004800f, 0.653478f },
	{ 0.204800f, 0.796335f },
	{ 0.404800f, 0.939192f },
	{ 0.604800f, 0.102457f },
	{ 0.804800f, 0.245314f },
	{ 0.044800f, 0.388172f },
	{ 0.244800f, 0.531029f },
	{ 0.444800f, 0.673886f },
	{ 0.644800f, 0.816743f },
	{ 0.844800f, 0.959600f },
	{ 0.084800f, 0.122865f },
	{ 0.284800f, 0.265723f },
	{ 0.484800f, 0.408580f },
	{ 0.684800f, 0.551437f },
	{ 0.884800f, 0.694294f },
	{ 0.124800f, 0.837151f },
	{ 0.324800f, 0.980008f },
	{ 0.524800f, 0.003332f },
	{ 0.724800f, 0.146189f },
	{ 0.924800f, 0.289046f },
	{ 0.164800f, 0.431903f },
	{ 0.364800f, 0.574761f },
	{ 0.564800f, 0.717618f },
	{ 0.764800f, 0.860475f },
	{ 0.964800f, 0.023740f },
	{ 0.012800f, 0.166597f },
	{ 0.212800f, 0.309454f },
	{ 0.412800f, 0.452312f },
	{ 0.612800f, 0.595169f },
	{ 0.812800f, 0.738026f },
	{ 0.052800f, 0.880883f },
	{ 0.252800f, 0.044148f },
	{ 0.452800f, 0.187005f },
	{ 0.652800f, 0.329863f },
	{ 0.852800f, 0.472720f },
	{ 0.092800f, 0.615577f },
	{ 0.292800f, 0.758434f },
	{ 0.492800f, 0.901291f },
	{ 0.692800f, 0.064556f },
	{ 0.892800f, 0.207414f },
	{ 0.132800f, 0.350271f },
	{ 0.332800f, 0.493128f },
	{ 0.532800f, 0.635985f },
	{ 0.732800f, 0.778842f },
	{ 0.932800f, 0.921699f },
	{ 0.172800f, 0.084965f },
	{ 0.372800f, 0.227822f },
	{ 0.572800f, 0.370679f },
	{ 0.772800f, 0.513536f },
	{ 0.972800f, 0.656393f },
	{ 0.020800f, 0.799250f },
	{ 0.220800f, 0.942107f },
	{ 0.420800f, 0.105373f },
	{ 0.620800f, 0.248230f },
	{ 0.820800f, 0.391087f },
	{ 0.060800f, 0.533944f },
	{ 0.260800f, 0.676801f },
	{ 0.460800f, 0.819658f },
	{ 0.660800f, 0.962516f },
	{ 0.860800f, 0.125781f },
	{ 0.100800f, 0.268638f },
	{ 0.300800f, 0.411495f },
	{ 0.500800f, 0.554352f },
	{ 0.700800f, 0.697209f },
	{ 0.900800f, 0.840067f },
	{ 0.140800f, 0.982924f },
	{ 0.340800f, 0.006247f },
	{ 0.540800f, 0.149105f },
	{ 0.740800f, 0.291962f },
	{ 0.940800f, 0.434819f },
	{ 0.180800f, 0.577676f },
	{ 0.380800f, 0.720533f },
	{ 0.580800f, 0.863390f },
	{ 0.780800f, 0.026656f },
	{ 0.980800f, 0.169513f },
	{ 0.028800f, 0.312370f },
	{ 0.228800f, 0.455227f },
	{ 0.428800f, 0.598084f },
	{ 0.628800f, 0.740941f },
	{ 0.828800f, 0.883798f },
	{ 0.068800f, 0.047064f },
	{ 0.268800f, 0.189921f },
	{ 0.468800f, 0.332778f },
	{ 0.668800f, 0.475635f },
	{ 0.868800f, 0.618492f },
	{ 0.108800f, 0.761349f },
	{ 0.308800f, 0.904207f },
	{ 0.508800f, 0.067472f },
	{ 0.708800f, 0.210329f },
	{ 0.908800f, 0.353186f },
	{ 0.148800f, 0.496043f },
	{ 0.348800f, 0.638900f },
	{ 0.548800f, 0.781758f },
	{ 0.748800f, 0.924615f },
	{ 0.948800f, 0.087880f },
	{ 0.188800f, 0.230737f },
	{ 0.388800f, 0.373594f },
	{ 0.588800f, 0.516451f },
	{ 0.788800f, 0.659309f },
	{ 0.988800f, 0.802166f },
	{ 0.036800f, 0.945023f },
	{ 0.236800f, 0.108288f },
	{ 0.436800f, 0.251145f },
	{ 0.636800f, 0.394002f },
	{ 0.836800f, 0.536860f },
	{ 0.076800f, 0.679717f },
	{ 0.276800f, 0.822574f },
	{ 0.476800f, 0.965431f },
	{ 0.676800f, 0.128696f },
	{ 0.876800f, 0.271554f },
	{ 0.116800f, 0.414411f },
	{ 0.316800f, 0.557268f },
	{ 0.516800f, 0.700125f },
	{ 0.716800f, 0.842982f },
	{ 0.916800f, 0.985839f },
	{ 0.156800f, 0.009163f },
	{ 0.356800f, 0.152020f },
	{ 0.556800f, 0.294877f },
	{ 0.756800f, 0.437734f },
	{ 0.956800f, 0.580591f },
	{ 0.196800f, 0.723449f },
	{ 0.396800f, 0.866306f },
	{ 0.596800f, 0.029571f },
	{ 0.796800f, 0.172428f },
	{ 0.996800f, 0.315285f },
	{ 0.006400f, 0.458142f },
	{ 0.206400f, 0.601000f },
	{ 0.406400f, 0.743857f },
	{ 0.606400f, 0.886714f },
	{ 0.806400f, 0.049979f },
	{ 0.046400f, 0.192836f },
	{ 0.246400f, 0.335693f },
	{ 0.446400f, 0.478551f },
	{ 0.646400f, 0.621408f },
	{ 0.846400f, 0.764265f },
	{ 0.086400f, 0.907122f },
	{ 0.286400f, 0.070387f },
	{ 0.486400f, 0.213244f },
	{ 0.686400f, 0.356102f },
	{ 0.886400f, 0.498959f },
	{ 0.126400f, 0.641816f },
	{ 0.326400f, 0.784673f },
	{ 0.526400f, 0.927530f },
	{ 0.726400f, 0.090796f },
	{ 0.926400f, 0.233653f },
	{ 0.166400f, 0.376510f },
	{ 0.366400f, 0.519367f },
	{ 0.566400f, 0.662224f },
	{ 0.766400f, 0.805081f },
	{ 0.966400f, 0.947938f },
	{ 0.014400f, 0.111204f },
	{ 0.214400f, 0.254061f },
	{ 0.414400f, 0.396918f },
	{ 0.614400f, 0.539775f },
	{ 0.814400f, 0.682632f },
	{ 0.054400f, 0.825489f },
	{ 0.254400f, 0.968347f },
	{ 0.454400f, 0.131612f },
	{ 0.654400f, 0.274469f },
	{ 0.854400f, 0.417326f },
	{ 0.094400f, 0.560183f },
	{ 0.294400f, 0.703040f },
	{ 0.494400f, 0.845898f },
	{ 0.694400f, 0.988755f },
	{ 0.894400f, 0.012078f },
	{ 0.134400f, 0.154935f },
	{ 0.334400f, 0.297793f },
	{ 0.534400f, 0.440650f },
	{ 0.734400f, 0.583507f },
	{ 0.934400f, 0.726364f },
	{ 0.174400f, 0.869221f },
	{ 0.374400f, 0.032486f },
	{ 0.574400f, 0.175344f },
	{ 0.774400f, 0.318201f },
	{ 0.974400f, 0.461058f },
	{ 0.022400f, 0.603915f },
	{ 0.222400f, 0.746772f },
	{ 0.422400f, 0.889629f },
	{ 0.622400f, 0.052895f },
	{ 0.822400f, 0.195752f },
	{ 0.062400f, 0.338609f },
	{ 0.262400f, 0.481466f },
	{ 0.462400f, 0.624323f },
	{ 0.662400f, 0.767180f },
	{ 0.862400f, 0.910037f },
	{ 0.102400f, 0.073303f },
	{ 0.302400f, 0.216160f },
	{ 0.502400f, 0.359017f },
	{ 0.702400f, 0.501874f },
	{ 0.902400f, 0.644731f },
	{ 0.142400f, 0.787589f },
	{ 0.342400f, 0.930446f },
	{ 0.542400f, 0.093711f },
	{ 0.742400f, 0.236568f },
	{ 0.942400f, 0.379425f },
	{ 0.182400f, 0.522282f },
	{ 0.382400f, 0.665140f },
	{ 0.582400f, 0.807997f },
	{ 0.782400f, 0.950854f },
	{ 0.982400f, 0.114119f },
	{ 0.030400f, 0.256976f },
	{ 0.230400f, 0.399833f },
	{ 0.430400f, 0.542691f },
	{ 0.630400f, 0.685548f },
	{ 0.830400f, 0.828405f },
	{ 0.070400f, 0.971262f },
	{ 0.270400f, 0.134527f },
	{ 0.470400f, 0.277384f },
	{ 0.670400f, 0.420242f },
	{ 0.870400f, 0.563099f },
	{ 0.110400f, 0.705956f },
	{ 0.310400f, 0.848813f },
	{ 0.510400f, 0.991670f },
	{ 0.710400f, 0.014994f },
	{ 0.910400f, 0.157851f },
	{ 0.150400f, 0.300708f },
	{ 0.350400f, 0.443565f },
	{ 0.550400f, 0.586422f },
	{ 0.750400f, 0.729279f },
	{ 0.950400f, 0.872137f },
	{ 0.190400f, 0.035402f },
	{ 0.390400f, 0.178259f },
	{ 0.590400f, 0.321116f },
	{ 0.790400f, 0.463973f },
	{ 0.990400f, 0.606830f },
};

static const sf::Vec3 cubeDirs[] = {
	{ +1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
	{ 0.0f, +1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, +1.0f }, { 0.0f, 0.0f, -1.0f },
};

static const float sliceHeight[] = {
	0.05f, 0.75f, 1.5f,
};

struct EnvLightSystemImp final : EnvLightSystem
{
	static const uint32_t DepthSlices = 3;
	static const uint32_t UpdateCount = 3;

	struct UpdateState
	{
		sf::Vec3 rayDir;
		sf::Array<PointLight> pointLights;
		RenderArgs renderArgs;
		float depthToDistance;
	};

	uint32_t envmapResolution = 256;
	uint32_t renderResolution = 128;

	sp::RenderTarget gbufferTarget[2];
	sp::RenderTarget gbufferDepthTarget;
	sp::RenderPass gbufferPass;

	sp::RenderTarget lightingTarget;
	sp::RenderPass lightingPass;

	sp::Pipeline envmapBlendPipe;

	Shader2 lightingShader;
	sp::Pipeline lightingPipe;
	sf::Random rng;

	bool firstUpdate = true;
	uint32_t envDiffuseIndex = 0;
	sp::Texture envDiffuse[2];
	sp::RenderPass envBlendPass[2];

	MiscTextureRef blueNoiseTex;

	uint32_t haltonIx = 0;

	UpdateState updateStates[UpdateCount][DepthSlices];

	void initTargets()
	{
		sf::Vec2i res = sf::Vec2i(renderResolution * DepthSlices, renderResolution * UpdateCount);

		gbufferTarget[0].init("envmap gbuffer0", res, SG_PIXELFORMAT_RGBA8);
		gbufferTarget[1].init("envmap gbuffer1", res, SG_PIXELFORMAT_RGBA8);
		gbufferDepthTarget.init("envmap gbufferDepth", res, SG_PIXELFORMAT_DEPTH);

		{
			sg_image_desc d = { };
			d.min_filter = d.mag_filter = SG_FILTER_NEAREST;
			lightingTarget.init("envmap lighting", res, SG_PIXELFORMAT_RGBA32F, 1, d);
		}

		gbufferPass.init("envmap gbuffer", gbufferTarget[0], gbufferTarget[1], gbufferDepthTarget);
		lightingPass.init("envmap lighting", lightingTarget);

		for (uint32_t swapI = 0; swapI < 2; swapI++) {
			sf::SmallStringBuf<64> label;
			label.format("envDiffuse%u", swapI);
			sg_image_desc d = { };
			d.render_target = true;
			d.label = label.data;
			d.pixel_format = SG_PIXELFORMAT_RGBA32F;
			d.type = SG_IMAGETYPE_3D;
			d.width = envmapResolution * 6;
			d.height = envmapResolution;
			d.depth = DepthSlices;
			d.num_mipmaps = 1;
			d.min_filter = d.mag_filter = SG_FILTER_LINEAR;
			d.wrap_u = d.wrap_v = d.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
			envDiffuse[swapI].init(d);

			sp::FramebufferDesc fbDesc;
			fbDesc.colorFormat = SG_PIXELFORMAT_RGBA32F;
			fbDesc.depthFormat = SG_PIXELFORMAT_NONE;

			sg_pass_desc passDesc = { };
			sf::SmallStringBuf<128> name;
			name.format("envBlend%u", swapI);
			passDesc.label = name.data;
			for (uint32_t i = 0; i < DepthSlices; i++) {
				passDesc.color_attachments[i].image = envDiffuse[swapI].image;
				passDesc.color_attachments[i].slice = i;
			}
			envBlendPass[swapI].init(passDesc, sf::Vec2i((int32_t)envmapResolution), fbDesc);
		}
	}

	// API

	EnvLightSystemImp()
	{
		initTargets();

		uint8_t permutation[SP_NUM_PERMUTATIONS] = { };
		#if CL_SHADOWCACHE_USE_ARRAY
			permutation[SP_SHADOWGRID_USE_ARRAY] = 1;
		#else
			permutation[SP_SHADOWGRID_USE_ARRAY] = 0;
		#endif
		lightingShader = getShader2(SpShader_EnvmapLighting, permutation);

		{
			MiscTextureProps props;
			props.minFilter = props.magFilter = SG_FILTER_NEAREST;
			blueNoiseTex.load(sf::Symbol("Assets/Misc/Misc_RGBA/BlueNoise_64.png"), props);
		}

		lightingPipe.init(lightingShader.handle, sp::PipeVertexFloat2);

		{
			sg_pipeline_desc &d = envmapBlendPipe.init(gameShaders.envmapBlend, sp::PipeVertexFloat2);
			d.blend.color_attachment_count = DepthSlices;
		}
	}

	void renderEnvmap(Systems &systems) override
	{
		if (!blueNoiseTex.isLoaded()) return;

		if (firstUpdate) {
			sg_pass_action action = { };
			action.colors[0].action = SG_ACTION_CLEAR;
			sp::beginPass(envBlendPass[0], &action);
			sp::endPass();
		}

		EnvLightAltas prevEnvAtlas = getEnvLightAtlas();

		uint32_t writeSwap = envDiffuseIndex ^ 1;
		uint32_t readSwap = envDiffuseIndex;
		envDiffuseIndex = writeSwap;

		sf::Vec3 rayDirs[UpdateCount];

		for (uint32_t i = 0; i < UpdateCount; i++) {
			sf::Vec2 sample = halton2D[haltonIx++];
			if (haltonIx >= sf_arraysize(halton2D)) haltonIx = 0;

			sample += rng.nextVec2() * 0.01f;
			if (sample.x >= 1.0) sample.x -= 1.0f;
			if (sample.y >= 1.0) sample.y -= 1.0f;

			rayDirs[i] = sampleSphereUniform(sample);
		}

		// Pass 1: Render G-buffers
		{
			sg_pass_action action = { };
			action.colors[0].action = SG_ACTION_CLEAR;
			action.colors[1].action = SG_ACTION_CLEAR;
			action.depth.action = SG_ACTION_CLEAR;
			action.depth.val = 1.0f;
			sp::beginPass(gbufferPass, &action);

			for (uint32_t rayI = 0; rayI < UpdateCount; rayI++)
			for (uint32_t sliceI = 0; sliceI < DepthSlices; sliceI++) {
				UpdateState &updateState = updateStates[rayI][sliceI];
				sf::Vec3 rayDir = rayDirs[rayI];

				updateState.rayDir = rayDir;

				sf::Vec3 eye = sf::Vec3(0.0f, sliceHeight[sliceI], 0.0f);
				float extent = (float)renderResolution * 0.5f;
				float range = 30.0f;
				float attenuation = 40.0f;

				sf::Vec3 dir, up;
				sf::Vec2 skew;

				float tanY = sf::max(sf::abs(rayDir.y), 0.01f);
				float far = tanY * range;
				if (rayDir.y > 0.0f) {
					dir = sf::Vec3(0.0f, +1.0f, 0.0f);
					up = sf::Vec3(0.0f, 0.0f, +1.0f);
					skew = sf::Vec2(rayDir.x / -tanY, rayDir.z / -tanY);
				} else {
					dir = sf::Vec3(0.0f, -1.0f, 0.0f);
					up = sf::Vec3(0.0f, 0.0f, -1.0f);
					skew = sf::Vec2(rayDir.x / tanY, rayDir.z / tanY);
				}

				updateState.depthToDistance = range / tanY / attenuation;

				RenderArgs &renderArgs = updateState.renderArgs;
				renderArgs.cameraPosition = eye;
				renderArgs.worldToView = sf::mat::look(eye, dir, up);
				renderArgs.viewToClip = sf::mat::orthoSkewedD3D(sf::Vec2(extent), skew, 0.0f, far);
				renderArgs.worldToClip = renderArgs.viewToClip * renderArgs.worldToView;
				renderArgs.frustum = sf::Frustum(renderArgs.worldToClip, sp::getClipNearW());

				sg_apply_viewport((int)(renderResolution * sliceI), (int)renderResolution * rayI, (int)renderResolution, (int)renderResolution, true);

				systems.renderEnvmapGBuffer(renderArgs);

				updateState.pointLights.clear();
				systems.light->queryVisiblePointLights(systems.envmapAreas, updateState.pointLights);
			}

			sp::endPass();
		}

		const uint32_t maxLights = 32;

		// Pass 2: Evaluate lighting
		{
			sg_pass_action action = { };
			action.colors[0].action = SG_ACTION_CLEAR;
			action.depth.action = SG_ACTION_CLEAR;
			action.depth.val = 1.0f;
			sp::beginPass(lightingPass, &action);

			for (uint32_t rayI = 0; rayI < UpdateCount; rayI++)
			for (uint32_t sliceI = 0; sliceI < DepthSlices; sliceI++) {
				UpdateState &updateState = updateStates[rayI][sliceI];
				sf::Vec3 rayDir = updateState.rayDir;

				// TODO(profile): Do this with geometry instead?
				sg_apply_viewport((int)(renderResolution * sliceI), (int)renderResolution * rayI, (int)renderResolution, (int)renderResolution, true);

				sg_bindings binds = { };

				// TODO: Multiple passes
				if (updateState.pointLights.size > maxLights) {
					updateState.pointLights.resizeUninit(maxLights);
				}

				UBO_EnvmapVertex vu;
				vu.flipY = rayDir.y > 0.0f ? -1.0f : 1.0f;

				UBO_EnvmapPixel pu;
				pu.clipToWorld = sf::inverse(updateState.renderArgs.worldToClip);
				pu.numLightsF = (float)updateState.pointLights.size;
				pu.uvMad = sf::Vec4(1.0f / 3.0f, 1.0f / 3.0f, (float)sliceI / 3.0f, (float)rayI / 3.0f);
				pu.rayDir = rayDir;
				pu.diffuseEnvmapMad = prevEnvAtlas.worldMad;
				sf::Vec4 *dst = pu.pointLightData;
				for (PointLight &light : updateState.pointLights) {
					light.writeShader(dst);
				}

				lightingPipe.bind();

				binds.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;

				bindUniformVS(lightingShader, vu);
				bindUniformFS(lightingShader, pu);

				bindImageFS(lightingShader, binds, CL_SHADOWCACHE_TEX, systems.light->getShadowTexture());
				bindImageFS(lightingShader, binds, TEX_diffuseEnvmapAtlas, prevEnvAtlas.image);
				bindImageFS(lightingShader, binds, TEX_gbuffer0, gbufferTarget[0].image);
				bindImageFS(lightingShader, binds, TEX_gbuffer1, gbufferTarget[1].image);

				sg_apply_bindings(&binds);

				sg_draw(0, 3, 1);
			}

			sp::endPass();
		}

		// Pass 3: Update the atlas
		{
			sg_pass_action action = { };
			action.colors[0].action = SG_ACTION_CLEAR;
			action.depth.action = SG_ACTION_CLEAR;
			action.depth.val = 1.0f;
			sp::beginPass(envBlendPass[writeSwap], &action);

			sg_bindings binds = { };
			binds.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			// binds.fs_images[SLOT_EnvmapBlend_blueNoise] = blueNoiseTex->image;
			binds.fs_images[SLOT_EnvmapBlend_lighting] = lightingTarget.image;
			binds.fs_images[SLOT_EnvmapBlend_envmapPrev] = envDiffuse[readSwap].image;

			EnvmapBlend_Pixel_t pu = { };

			float uvToNoise = (float)envmapResolution / 64.0f;
			sf::Vec2 blueNoiseOffset = rng.nextVec2();
			pu.uvToBlueNoiseMad = sf::Vec4(uvToNoise, uvToNoise, blueNoiseOffset.x, blueNoiseOffset.y);
			pu.uvToLightMad = sf::Vec4(2.0f, 2.0f, -0.5f, -0.5f);
			pu.prevShift = sf::Vec2();

			for (uint32_t i = 0; i < DepthSlices; i++) {
				pu.rayDirs[i] = sf::Vec4(rayDirs[i], 0.0f);
			}
			pu.prevShift = sf::Vec2();
			pu.uvToLightMad = sf::Vec4(2.0f, 2.0f, -0.5f, -0.5f);

			if (envmapBlendPipe.bind()) {
				sg_apply_bindings(&binds);
			}

			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_EnvmapBlend_Pixel, &pu, sizeof(pu));

			sg_draw(0, 3, 1);

			sp::endPass();
		}

	firstUpdate = false;
	}

	EnvLightAltas getEnvLightAtlas() const override
	{
		EnvLightAltas atlas = { };
		atlas.image = envDiffuse[envDiffuseIndex].image;
		atlas.worldMad = sf::Vec4(1.0f / (float)envmapResolution, 1.0f / (float)envmapResolution, 0.5f, 0.5f);
		return atlas;
	}

	sg_image getDebugLightingImage() const override
	{
		return lightingTarget.image;
	}
};

sf::Box<EnvLightSystem> EnvLightSystem::create() { return sf::box<EnvLightSystemImp>(); }


}
