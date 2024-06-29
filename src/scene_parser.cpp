#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "scene_parser.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "material.hpp"
#include "object3d.hpp"
#include "group.hpp"
// #include "lightGroup.hpp"
#include "mesh.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "transform.hpp"
#include "texture.hpp"
#include "quad.hpp"
#include "brdf.hpp"
#include "curve.hpp"
#include "revsurface.hpp"
#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

SceneParser::SceneParser(const char *filename)
{

    // initialize some reasonable default values
    // cout << filename << endl;
    group = nullptr;
    lightGroup = new LightGroup(0);
    camera = nullptr;
    background_color = Vector3f(0.5, 0.5, 0.5);
    num_lights = 0;
    lights = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0)
    {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr)
    {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    if (num_lights == 0)
    {
        printf("WARNING:    No lights specified\n");
    }
}

SceneParser::~SceneParser()
{

    delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++)
    {
        delete materials[i];
    }
    delete[] materials;
    for (i = 0; i < num_lights; i++)
    {
        delete lights[i];
    }
    delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile()
{
    //
    // at the top level, the scene can have a camera,
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token))
    {
        if (!strcmp(token, "PerspectiveCamera"))
        {
            parsePerspectiveCamera();
        }
        else if (!strcmp(token, "Background"))
        {
            parseBackground();
        }
        else if (!strcmp(token, "Lights"))
        {
            parseLights();
        }
        else if (!strcmp(token, "Materials"))
        {
            parseMaterials();
        }
        else if (!strcmp(token, "Group"))
        {
            group = parseGroup();
            for (int i = 0; i < group->all_object.size(); i++)
            {
                if (group->all_object[i]->material->isALight())
                {
                    // cout << i << endl;
                    lightGroup->addLight(group->all_object[i]);
                }
            }
        }
        else
        {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert(!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert(!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert(!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert(!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert(!strcmp(token, "defocusAngle"));
    float defocusAngle = readFloat();
    getToken(token);
    assert(!strcmp(token, "focusDist"));
    float focusDist = readFloat();
    getToken(token);
    assert(!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, width, height, angle_radians, defocusAngle, focusDist);
}

void SceneParser::parseBackground()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true)
    {
        getToken(token);
        if (!strcmp(token, "}"))
        {
            break;
        }
        else if (!strcmp(token, "color"))
        {
            background_color = readVector3f();
        }
        else
        {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light *[num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count)
    {
        getToken(token);
        if (strcmp(token, "DirectionalLight") == 0)
        {
            lights[count] = parseDirectionalLight();
        }
        else if (strcmp(token, "PointLight") == 0)
        {
            lights[count] = parsePointLight();
        }
        else
        {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}

Light *SceneParser::parseDirectionalLight()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert(!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light *SceneParser::parsePointLight()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert(!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    return new PointLight(position, color);
}
// ====================================================================
// ====================================================================

void SceneParser::parseMaterials()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material *[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count)
    {
        getToken(token);
        // cout << token << endl;
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial"))
        {
            materials[count] = parseMaterial();
        }
        else
        {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert(!strcmp(token, "}"));
}

int getTokenInFile(char token[MAX_PARSER_TOKEN_LENGTH], FILE *file)
{
    // for simplicity, tokens must be separated by whitespace
    assert(file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF)
    {
        token[0] = '\0';
        return 0;
    }
    return 1;
}
Vector3f readVector3fInFile(FILE *file)
{
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3)
    {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert(0);
    }
    return Vector3f(x, y, z);
}

float readFloatInFile(FILE *file)
{
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1)
    {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}

int readIntInFile(FILE *file)
{
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1)
    {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}
Texture *SceneParser::parseTexture(char *filename)
{
    Texture *ans;
    char token[MAX_PARSER_TOKEN_LENGTH];
    FILE *textureFile = fopen(filename, "r");
    getTokenInFile(token, textureFile);
    // cout << token << endl;
    assert(!strcmp(token, "{"));
    getTokenInFile(token, textureFile);
    if (strcmp(token, "checkerTexture") == 0)
    {
        float scale = 0;
        Vector3f colorEven;
        Vector3f colorOdd;
        while (true)
        {
            getTokenInFile(token, textureFile);
            if (strcmp(token, "scale") == 0)
            {
                scale = readFloatInFile(textureFile);
            }
            else if (strcmp(token, "colorEven") == 0)
            {
                colorEven = readVector3fInFile(textureFile);
            }
            else if (strcmp(token, "colorOdd") == 0)
            {
                colorOdd = readVector3fInFile(textureFile);
            }
            else
            {
                assert(!strcmp(token, "}"));
                break;
            }
        }
        ans = new CheckerTexture(scale, colorEven, colorOdd);
    }
    else if (strcmp(token, "imageTexture") == 0)
    {
        char filename[MAX_PARSER_TOKEN_LENGTH];
        while (true)
        {
            getTokenInFile(token, textureFile);
            if (strcmp(token, "imagefile") == 0)
            {
                getTokenInFile(filename, textureFile);
                // filename = std::string(token);
            }
            else
            {
                assert(!strcmp(token, "}"));
                break;
            }
        }
        // cout << filename << endl;
        ans = new ImageTexture(filename);
    }
    else if (strcmp(token, "nTexture") == 0)
    {
        char filename[MAX_PARSER_TOKEN_LENGTH];
        while (true)
        {
            getTokenInFile(token, textureFile);
            if (strcmp(token, "imagefile") == 0)
            {
                getTokenInFile(filename, textureFile);
                // filename = std::string(token);
            }
            else
            {
                assert(!strcmp(token, "}"));
                break;
            }
        }
        // cout << filename << endl;
        ans = new NTexture(filename);
    }
    fclose(textureFile);
    textureFile = nullptr;
    return ans;
}

Material *SceneParser::parseMaterial()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0), lightColor(1, 1, 1);
    float shininess = 0;
    bool enableReflect = false;
    bool enableRefract = false;
    float perDiffuse = 0;
    float perReflect = 0;
    float perRefract = 0;
    float refractRate = 0;
    float fuzz = 0;
    bool hasTexture = false;
    bool isLight = false;
    float boost = 1;
    bool hasNtexture = false;
    Texture *texture;
    Texture *nTexture;
    BaseBrdf *brdf;
    getToken(token);
    assert(!strcmp(token, "{"));
    while (true)
    {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0)
        {
            diffuseColor = readVector3f();
        }
        else if (strcmp(token, "specularColor") == 0)
        {
            specularColor = readVector3f();
        }
        else if (strcmp(token, "shininess") == 0)
        {
            shininess = readFloat();
        }
        else if (strcmp(token, "enableReflect") == 0)
        {
            // cout << "sadsad" << endl;
            enableReflect = bool(readInt());
            // cout << enableReflect << endl;
        }
        else if (strcmp(token, "enableRefract") == 0)
        {
            enableRefract = bool(readInt());
        }
        else if (strcmp(token, "refractRate") == 0)
        {
            refractRate = readFloat();
        }
        else if (strcmp(token, "perDiffuse") == 0)
        {
            perDiffuse = readFloat();
        }
        else if (strcmp(token, "perReflect") == 0)
        {
            perReflect = readFloat();
        }
        else if (strcmp(token, "perRefract") == 0)
        {
            perRefract = readFloat();
        }
        else if (strcmp(token, "fuzz") == 0)
        {
            fuzz = readFloat();
        }
        else if (strcmp(token, "texture") == 0)
        {
            // Optional: read in texture and draw it.
            // cout << "sada" << endl;
            getToken(filename);
            // cout << filename << endl;
            texture = parseTexture(filename);
            hasTexture = true;
        }
        else if (strcmp(token, "nTexture") == 0)
        {
            getToken(filename);
            nTexture = parseTexture(filename);
            hasNtexture = true;
        }
        else if (strcmp(token, "lightColor") == 0)
        {
            isLight = true;
            lightColor = readVector3f();
        }
        else if (strcmp(token, "boost") == 0)
        {
            boost = readFloat();
        }
        else if (strcmp(token, "cookTorrence") == 0)
        {
            Vector3f paras = readVector3f();
            float alpha = paras.x();
            float F0 = paras.y();
            brdf = new cookTorrance(alpha, F0);
        }
        else
        {
            assert(!strcmp(token, "}"));
            break;
        }
    }
    Material *answer;
    if (!isLight)
    {
        answer = new Material(diffuseColor, specularColor, shininess, bool(enableReflect), bool(enableRefract), refractRate, perDiffuse, perReflect, perRefract, fuzz);
        answer->setBrdf(brdf);
    }
    else
    {
        answer = new LightMaterial(diffuseColor, specularColor, shininess, bool(enableReflect), bool(enableRefract), refractRate, perDiffuse, perReflect, perRefract, fuzz);
        answer->setLightColor(lightColor);
        answer->boost = boost;
    }
    if (hasTexture)
    {
        answer->setTexture(texture);
    }
    if (hasNtexture)
    {
        answer->setNTexture(nTexture);
    }
    return answer;
}
Curve *SceneParser::parseBezierCurve()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "controls"));
    vector<Vector3f> controls;
    while (true)
    {
        getToken(token);
        if (!strcmp(token, "["))
        {
            controls.push_back(readVector3f());
            getToken(token);
            assert(!strcmp(token, "]"));
        }
        else if (!strcmp(token, "}"))
        {
            break;
        }
        else
        {
            printf("Incorrect format for BezierCurve!\n");
            exit(0);
        }
    }
    Curve *answer = new BezierCurve(controls);
    return answer;
}

Curve *SceneParser::parseBsplineCurve()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "controls"));
    vector<Vector3f> controls;
    while (true)
    {
        getToken(token);
        if (!strcmp(token, "["))
        {
            controls.push_back(readVector3f());
            getToken(token);
            assert(!strcmp(token, "]"));
        }
        else if (!strcmp(token, "}"))
        {
            break;
        }
        else
        {
            printf("Incorrect format for BsplineCurve!\n");
            exit(0);
        }
    }
    Curve *answer = new BsplineCurve(controls);
    return answer;
}

// ====================================================================
// ====================================================================
RevSurface *SceneParser::parseRevSurface()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "axis"));
    Vector3f axis = readVector3f();
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert(!strcmp(token, "profile"));
    Curve *profile;
    getToken(token);
    if (!strcmp(token, "BezierCurve"))
    {
        profile = parseBezierCurve();
    }
    else if (!strcmp(token, "BsplineCurve"))
    {
        profile = parseBsplineCurve();
    }
    else
    {
        printf("Unknown profile type in parseRevSurface: '%s'\n", token);
        exit(0);
    }
    getToken(token);
    assert(!strcmp(token, "}"));
    auto *answer = new RevSurface(profile, current_material);
    answer->setAxis(axis);
    answer->setCenter(center);
    return answer;
}

Object3D *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH])
{
    Object3D *answer = nullptr;
    if (!strcmp(token, "Group"))
    {
        answer = (Object3D *)parseGroup();
    }
    else if (!strcmp(token, "Sphere"))
    {
        answer = (Object3D *)parseSphere();
    }
    else if (!strcmp(token, "Plane"))
    {
        answer = (Object3D *)parsePlane();
    }
    else if (!strcmp(token, "Triangle"))
    {
        answer = (Object3D *)parseTriangle();
    }
    else if (!strcmp(token, "TriangleMesh"))
    {
        answer = (Object3D *)parseTriangleMesh();
    }
    else if (!strcmp(token, "Transform"))
    {
        answer = (Object3D *)parseTransform();
    }
    else if (!strcmp(token, "Quad"))
    {
        answer = (Object3D *)parseQuad();
    }
    else if (!strcmp(token, "RevSurface"))
    {
        answer = (Object3D *)parseRevSurface();
    }
    else
    {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================
Group *SceneParser::parseGroup()
{
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert(!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count)
    {
        getToken(token);
        if (!strcmp(token, "MaterialIndex"))
        {
            // change the current material
            int index = readInt();
            assert(index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        }
        else
        {
            Object3D *object = parseObject(token);
            assert(object != nullptr);
            answer->addObject(count, object);
            count++;
        }
    }
    getToken(token);
    assert(!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================
Quad *SceneParser::parseQuad()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "origin"));
    Vector3f origin = readVector3f();
    getToken(token);
    assert(!strcmp(token, "u"));
    Vector3f u = readVector3f();
    getToken(token);
    assert(!strcmp(token, "v"));
    Vector3f v = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    return new Quad(origin, u, v, current_material);
}
Sphere *SceneParser::parseSphere()
{
    bool isMoving = false;
    Vector3f moveVector;
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert(!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    // cout << token << endl;
    if (strcmp(token, "moveVector") == 0)
    {
        isMoving = true;
        moveVector = readVector3f();
    }
    if (isMoving)
    {
        getToken(token);
    }
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    Sphere *ans = new Sphere(center, radius, current_material);
    ans->setMoveVector(moveVector);
    return ans;
}

Plane *SceneParser::parsePlane()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert(!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    return new Plane(normal, offset, current_material);
}

Triangle *SceneParser::parseTriangle()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert(!strcmp(token, "}"));
    assert(current_material != nullptr);
    return new Triangle(v0, v1, v2, current_material);
}

Mesh *SceneParser::parseTriangleMesh()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert(!strcmp(token, "{"));
    getToken(token);
    assert(!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token);
    assert(!strcmp(token, "}"));
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    Mesh *answer = new Mesh(filename, current_material);

    return answer;
}

Transform *SceneParser::parseTransform()
{
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D *object = nullptr;
    getToken(token);
    assert(!strcmp(token, "{"));
    // read in transformations:
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true)
    {
        if (!strcmp(token, "Scale"))
        {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        }
        else if (!strcmp(token, "UniformScale"))
        {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        }
        else if (!strcmp(token, "Translate"))
        {
            matrix = matrix * Matrix4f::translation(readVector3f());
        }
        else if (!strcmp(token, "XRotate"))
        {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "YRotate"))
        {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "ZRotate"))
        {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        }
        else if (!strcmp(token, "Rotate"))
        {
            getToken(token);
            assert(!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert(!strcmp(token, "}"));
        }
        else if (!strcmp(token, "Matrix4f"))
        {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert(!strcmp(token, "{"));
            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert(!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        }
        else
        {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }

    assert(object != nullptr);
    getToken(token);
    assert(!strcmp(token, "}"));
    return new Transform(matrix, object);
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH])
{
    // for simplicity, tokens must be separated by whitespace
    assert(file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF)
    {
        token[0] = '\0';
        return 0;
    }
    return 1;
}

Vector3f SceneParser::readVector3f()
{
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3)
    {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert(0);
    }
    return Vector3f(x, y, z);
}

float SceneParser::readFloat()
{
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1)
    {
        printf("Error trying to read 1 float\n");
        assert(0);
    }
    return answer;
}

int SceneParser::readInt()
{
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1)
    {
        printf("Error trying to read 1 int\n");
        assert(0);
    }
    return answer;
}
