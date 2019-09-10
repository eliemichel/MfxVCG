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

#include "VcgPlugin.h"

VcgPlugin::VcgPlugin() {
	ofxPlugin.pluginApi = kOfxMeshEffectPluginApi;
	ofxPlugin.apiVersion = kOfxMeshEffectPluginApiVersion;
	ofxPlugin.pluginVersionMajor = 1;
	ofxPlugin.pluginVersionMinor = 0;
}

OfxStatus VcgPlugin::load() {
    return kOfxStatOK;
}

OfxStatus VcgPlugin::unload() {
    return kOfxStatOK;
}

OfxStatus VcgPlugin::describe(OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == propertySuite ||
        NULL == parameterSuite ||
        NULL == meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    OfxParamSetHandle parameters;
    meshEffectSuite->getParamSet(descriptor, &parameters);
	vcgDescribe(parameters);

    return kOfxStatOK;
}

OfxStatus VcgPlugin::createInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

OfxStatus VcgPlugin::destroyInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

OfxStatus VcgPlugin::cook(OfxMeshEffectHandle instance) {
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
	if (!vcgCook(vcg_input_mesh, vcg_maybe_output_mesh, parameters, &in_place)) {
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

OfxStatus VcgPlugin::mainEntry(const char *action,
	                const void *handle,
	                OfxPropertySetHandle inArgs,
	                OfxPropertySetHandle outArgs) {
	if (0 == strcmp(action, kOfxActionLoad)) {
		return load();
	}
	if (0 == strcmp(action, kOfxActionUnload)) {
		return unload();
	}
	if (0 == strcmp(action, kOfxActionDescribe)) {
		return describe((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionCreateInstance)) {
		return createInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionDestroyInstance)) {
		return destroyInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
		return cook((OfxMeshEffectHandle)handle);
	}
	return kOfxStatReplyDefault;
}

void VcgPlugin::setHost(OfxHost *_host) {
	host = _host;
	if (NULL != host) {
		propertySuite = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
		parameterSuite = (OfxParameterSuiteV1*)host->fetchSuite(host->host, kOfxParameterSuite, 1);
		meshEffectSuite = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
	}
}

