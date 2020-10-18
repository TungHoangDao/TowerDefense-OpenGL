/*

The vertex shader transforms the vertex by both modelview and projection matrices. This is what allows you to move objects with glTranslatef and also applies the perspective projection from gluPerspective.

The fragment program will colour each pixel rendered red. gl_FragColor is the main output from fragment programs.

Run the program and make sure the geometry draws with its colour being generated in the fragment shader. Change the color to green to check it works as expected.
*/
#define M_PI		3.14159265358979323846

uniform float Time;
uniform int WaveDim;

const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;

const vec3 lEC = vec3(0.0, 0.0, 1.0);//Light position
const vec3 Ld = vec3(1.0);
const vec3 Md = vec3(0.8);
const vec3 Ls = vec3(1.0);
const vec3 Ms = vec3(1.0);

const float shininess = 50.0;

varying vec4 Color;

void ComputeLightning()
{
  vec4 ambient, diffuse, specular;
  vec3 nEC = gl_Normal;
  float NdotL;

  // Ambient color
  ambient = gl_FrontMaterial.ambient * (gl_LightModel.ambient + gl_LightSource[0].ambient);
  Color = ambient;

  float dp = dot(nEC, lEC);

  if (dp > 0.0)
  {
    // Calculate diffuse contribution
    nEC = normalize(nEC);
    float NDotL = dot(nEC, lEC);

    diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    diffuse*= NDotL;
    Color+= diffuse;

    // specularlightning
    vec3 vEC = vec3(0.0, 0.0, 1.0);
    vec3 H = vec3(lEC + vEC);
    H = normalize(H);

    float nDotH = dot(nEC, H);

    if (nDotH < 0.0)
    {
      nDotH = 0.0;
    }

    vec3 specular = vec3(Ls * Ms * pow(nDotH, shininess));
    Color+= vec4(specular, 1);
    //    Color*= vec4(vec3(0.0,1.0,1.0),1);
  }
}

void main()
{
  float h;
  vec3 n; // Normal vector
  float x = gl_Vertex[0];
  float z = gl_Vertex[2];

    h = A1 * sin(k1 * gl_Vertex.x + w1 * Time) + A2 * sin(k2 * gl_Vertex.z + w2 * Time);

    // Calculate normal again.
    n.x = - A1 * k1 * cos(k1 * gl_Vertex.x + w1 * Time);
    n.y = 1.0;
    n.z = - A2 * k2 * cos(k2 * gl_Vertex.z + w2 * Time);

  vec4 osVert = gl_Vertex + vec4(vec3(0,h,0),1);
  vec4 esVert = gl_ModelViewMatrix * osVert;
  vec4 csVert = gl_ProjectionMatrix * esVert;

//  LightIntensity = ComputeLightning;

  // Transform the new normal
  gl_Normal = gl_NormalMatrix *  normalize(n);
  gl_Position = csVert;

  // Compute the lightning then pass to frag shader.
  ComputeLightning();
}
