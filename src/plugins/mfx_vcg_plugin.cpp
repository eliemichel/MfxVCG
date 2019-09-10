/*
 * MfxVCG is an Open Mesh Effect plug-in providing VCGlib-based filters.
 * Copyright (C) 2019  Élie Michel
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <string.h>

#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/convex_hull.h>
#include <vcg/complex/algorithms/smooth.h>

#include "ofxCore.h"
#include "ofxMeshEffect.h"

///////////////////////////////////////////////////////////////////////////////
// VCG related routines

class VcgVertex; class VcgEdge; class VcgFace;
struct VcgUsedTypes : public vcg::UsedTypes<vcg::Use<VcgVertex>   ::AsVertexType,
                                           vcg::Use<VcgEdge>     ::AsEdgeType,
                                           vcg::Use<VcgFace>     ::AsFaceType>{};
class VcgVertex  : public vcg::Vertex< VcgUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class VcgFace    : public vcg::Face<   VcgUsedTypes, vcg::face::FFAdj,  vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::BitFlags > {};
class VcgEdge    : public vcg::Edge<   VcgUsedTypes> {};
class VcgMesh    : public vcg::tri::TriMesh< std::vector<VcgVertex>, std::vector<VcgFace> , std::vector<VcgEdge>  > {};

///////////////////////////////////////////////////////////////////////////////
// Plugin runtime

typedef bool (*VcgCookFunc)(VcgMesh&, VcgMesh&, OfxParamSetHandle, OfxParameterSuiteV1*, bool*);
typedef void (*VcgDescribeFunc)(OfxParamSetHandle, OfxParameterSuiteV1*);

typedef struct PluginRuntime {
	OfxPlugin ofxPlugin;
	OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
	VcgCookFunc vcgCook;
	VcgDescribeFunc vcgDescribe;
} PluginRuntime;

// When increasing MAX_PLUGINS, Ctrl+F for all the occurences of "MAX_PLUGINS"
// in this file to find comment blocks explaining what must be changed
// consequently.
#define MAX_PLUGINS 2
PluginRuntime gRuntimePool[MAX_PLUGINS];

static void runtime_init(PluginRuntime *runtime, const char *plugin_identifier, VcgCookFunc vcg_cook, VcgDescribeFunc vcg_describe) {
	runtime->ofxPlugin.pluginApi = kOfxMeshEffectPluginApi;
	runtime->ofxPlugin.apiVersion = kOfxMeshEffectPluginApiVersion;
	runtime->ofxPlugin.pluginIdentifier = plugin_identifier;
	runtime->ofxPlugin.pluginVersionMajor = 1;
	runtime->ofxPlugin.pluginVersionMinor = 0;
	runtime->vcgCook = vcg_cook;
	runtime->vcgDescribe = vcg_describe;
}

///////////////////////////////////////////////////////////////////////////////
// Core cook functions

void vcg_describe_convex_hull(OfxParamSetHandle parameters, OfxParameterSuiteV1 *parameterSuite) {
}

bool vcg_cook_convex_hull(VcgMesh & input_mesh,
	                      VcgMesh & output_mesh,
	                      OfxParamSetHandle parameters,
	                      OfxParameterSuiteV1 *parameterSuite,
	                      bool *in_place) {
	// in_place is used to signal back to the callee whether the filtered mesh
	// replaces the input_mesh (in_place = true) or if a new mesh has been
	// allocated and is returned in output_mesh (in_place = false).
	*in_place = false;

	vcg::tri::UpdateNormal<VcgMesh>::PerFaceNormalized(input_mesh);
	return vcg::tri::ConvexHull<VcgMesh,VcgMesh>::ComputeConvexHull(input_mesh, output_mesh);
}

void vcg_describe_laplacian_smooth(OfxParamSetHandle parameters, OfxParameterSuiteV1 *parameterSuite) {
    parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "Steps", NULL);
	parameterSuite->paramDefine(parameters, kOfxParamTypeBoolean, "Smooth Selected", NULL);
	parameterSuite->paramDefine(parameters, kOfxParamTypeBoolean, "Cotangent Weight", NULL);
}

bool vcg_cook_laplacian_smooth(VcgMesh & input_mesh,
	                           VcgMesh & output_mesh,
	                           OfxParamSetHandle parameters,
	                           OfxParameterSuiteV1 *parameterSuite,
	                           bool *in_place) {
	*in_place = true;
	
	OfxParamHandle param;
	int steps;
    bool smooth_selected, cotangent_weight;
	
	parameterSuite->paramGetHandle(parameters, "Steps", &param, NULL);
    parameterSuite->paramGetValue(param, &steps);

	parameterSuite->paramGetHandle(parameters, "Smooth Selected", &param, NULL);
    parameterSuite->paramGetValue(param, &smooth_selected);

	parameterSuite->paramGetHandle(parameters, "Cotangent Weight", &param, NULL);
    parameterSuite->paramGetValue(param, &cotangent_weight);

	vcg::tri::Smooth<VcgMesh>::VertexCoordLaplacian(input_mesh, steps, smooth_selected, cotangent_weight);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Open Mesh Effect plugins

static OfxStatus load(PluginRuntime *runtime) {
    return kOfxStatOK;
}

static OfxStatus unload(PluginRuntime *runtime) {
    return kOfxStatOK;
}

static OfxStatus describe(PluginRuntime *runtime, OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == runtime->propertySuite ||
        NULL == runtime->parameterSuite ||
        NULL == runtime->meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }
    OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;
    OfxParameterSuiteV1 *parameterSuite = runtime->parameterSuite;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    OfxParamSetHandle parameters;
    meshEffectSuite->getParamSet(descriptor, &parameters);
	runtime->vcgDescribe(parameters, parameterSuite);

    return kOfxStatOK;
}

static OfxStatus createInstance(PluginRuntime *runtime, OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus destroyInstance(PluginRuntime *runtime, OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus cook(PluginRuntime *runtime, OfxMeshEffectHandle instance) {
    OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;
    OfxParameterSuiteV1 *parameterSuite = runtime->parameterSuite;
    OfxTime time = 0;

    // Get input/output
    OfxMeshInputHandle input, output;
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, NULL);
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainOutput, &output, NULL);

	// Get parameters
	OfxParamSetHandle parameters;
	meshEffectSuite->getParamSet(instance, &parameters);

    // Get meshes
    OfxPropertySetHandle input_mesh, output_mesh;
    meshEffectSuite->inputGetMesh(input, time, &input_mesh);
    meshEffectSuite->inputGetMesh(output, time, &output_mesh);

    // Get input mesh data
    int input_point_count = 0, input_vertex_count = 0, input_face_count = 0;
    float *input_points;
    int *input_vertices, *input_faces;
    propertySuite->propGetInt(input_mesh, kOfxMeshPropPointCount,
                              0, &input_point_count);
    propertySuite->propGetInt(input_mesh, kOfxMeshPropVertexCount,
                              0, &input_vertex_count);
    propertySuite->propGetInt(input_mesh, kOfxMeshPropFaceCount,
                              0, &input_face_count);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropPointData,
                                  0, (void**)&input_points);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropVertexData,
                                  0, (void**)&input_vertices);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropFaceData,
                                  0, (void**)&input_faces);

    // Transfer data to VCG mesh
    VcgMesh vcg_input_mesh, vcg_maybe_output_mesh;
	bool in_place;
	VcgMesh::VertexIterator vi = vcg::tri::Allocator<VcgMesh>::AddVertices(vcg_input_mesh, input_point_count);
	VcgMesh::FaceIterator fi = vcg::tri::Allocator<VcgMesh>::AddFaces(vcg_input_mesh, input_face_count);

	// Copy point data
	// TODO: is there a way to create a VCG mesh on existing data buffers without copying them?
	VcgMesh::VertexIterator vi0 = vi;
	float *data = input_points;
	for (int i = 0 ; i < input_point_count ; ++i) {
		vi->P() = VcgMesh::CoordType(data[0], data[1], data[2]);
		++vi;
		data += 3; 
	}
	vi = vi0;
	int offset = 0;
	for (int i = 0 ; i < input_face_count ; ++i) {
		fi->V(0) = &*(vi + input_vertices[offset + 0]);
		fi->V(1) = &*(vi + input_vertices[offset + 1]);
		fi->V(2) = &*(vi + input_vertices[offset + 2]);
		++fi;
		offset += input_faces[i];
	}

	// Call core cook function
	if (!runtime->vcgCook(vcg_input_mesh, vcg_maybe_output_mesh, parameters, parameterSuite, &in_place)) {
		meshEffectSuite->inputReleaseMesh(input_mesh);
		return kOfxStatFailed;
	}
	VcgMesh & vcg_output_mesh = in_place ? vcg_input_mesh : vcg_maybe_output_mesh;

    // Allocate output mesh
    int output_point_count = static_cast<int>(vcg_output_mesh.vert.size());
    int output_face_count = static_cast<int>(vcg_output_mesh.face.size());
    int output_vertex_count = 3 * output_face_count ;
    meshEffectSuite->meshAlloc(output_mesh,
                               output_point_count,
                               output_vertex_count,
                               output_face_count);

    // Get output mesh data
    float *output_points;
    int *output_vertices, *output_faces;
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropPointData,
                                  0, (void**)&output_points);
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropVertexData,
                                  0, (void**)&output_vertices);
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropFaceData,
                                  0, (void**)&output_faces);

    // Fill in output data
	size_t i = 0;
	for (vi = vcg_output_mesh.vert.begin() ; vi != vcg_output_mesh.vert.end(); ++vi, ++i) {
		output_points[3 * i + 0] = vi->P().X();
		output_points[3 * i + 1] = vi->P().Y();
		output_points[3 * i + 2] = vi->P().Z();
	}
	i = 0;
	for (fi = vcg_output_mesh.face.begin() ; fi != vcg_output_mesh.face.end() ; ++fi, ++i) {
		if (fi->IsD()) continue; // deleted face
		output_faces[i] = 3;
		output_vertices[3 * i + 0] = static_cast<int>(vcg::tri::Index(vcg_output_mesh, fi->V(0)));
		output_vertices[3 * i + 1] = static_cast<int>(vcg::tri::Index(vcg_output_mesh, fi->V(1)));
		output_vertices[3 * i + 2] = static_cast<int>(vcg::tri::Index(vcg_output_mesh, fi->V(2)));
	}

    // Release meshes
    meshEffectSuite->inputReleaseMesh(input_mesh);
    meshEffectSuite->inputReleaseMesh(output_mesh);

    return kOfxStatOK;
}

static OfxStatus mainEntry(PluginRuntime *runtime,
	                       const char *action,
                           const void *handle,
                           OfxPropertySetHandle inArgs,
                           OfxPropertySetHandle outArgs) {
    if (0 == strcmp(action, kOfxActionLoad)) {
        return load(runtime);
    }
    if (0 == strcmp(action, kOfxActionUnload)) {
        return unload(runtime);
    }
    if (0 == strcmp(action, kOfxActionDescribe)) {
        return describe(runtime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionCreateInstance)) {
        return createInstance(runtime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionDestroyInstance)) {
        return destroyInstance(runtime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
        return cook(runtime, (OfxMeshEffectHandle)handle);
    }
    return kOfxStatReplyDefault;
}

static void setHost(PluginRuntime *runtime, OfxHost *host) {
    runtime->host = host;
    if (NULL != host) {
      runtime->propertySuite = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
      runtime->parameterSuite = (OfxParameterSuiteV1*)host->fetchSuite(host->host, kOfxParameterSuite, 1);
      runtime->meshEffectSuite = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
    }
}

#define MAKE_PLUGIN_CLOSURES(nth) \
static void plugin ## nth ## _setHost(OfxHost *host) { \
	setHost(&gRuntimePool[nth], host); \
} \
static OfxStatus plugin ## nth ## _mainEntry(const char *action, \
	                                         const void *handle, \
	                                         OfxPropertySetHandle inArgs, \
	                                         OfxPropertySetHandle outArgs) { \
	return mainEntry(&gRuntimePool[nth], action, handle, inArgs, outArgs); \
}

#define REGISTER_PLUGIN_CLOSURE(index, plugin_identifier, vcg_cook, vcg_describe) \
gRuntimePool[index].ofxPlugin.mainEntry = plugin ## index ## _mainEntry; \
gRuntimePool[index].ofxPlugin.setHost = plugin ## index ## _setHost; \
runtime_init(&gRuntimePool[index], plugin_identifier, vcg_cook, vcg_describe);

MAKE_PLUGIN_CLOSURES(0)
MAKE_PLUGIN_CLOSURES(1)
// Add such a line for each n < MAX_PLUGINS. (This is needed to create
// individual C entry points of the form pluginX_mainEntry for each plug-in)

OfxExport int OfxGetNumberOfPlugins(void) {
	REGISTER_PLUGIN_CLOSURE(0, "ConvexHull", vcg_cook_convex_hull, vcg_describe_convex_hull)
	REGISTER_PLUGIN_CLOSURE(1, "LaplacianSmooth", vcg_cook_laplacian_smooth, vcg_describe_laplacian_smooth)
	// Define here plug-ins for each n < MAX_PLUGINS, providing an identifier
	// and pointers to the cooking and description functions. These functions
	// are defined above. Follow the examples of the other plug-ins.
    return 2;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
	return &gRuntimePool[nth].ofxPlugin;
}
