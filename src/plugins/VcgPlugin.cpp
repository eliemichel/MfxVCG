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

OfxStatus VcgPlugin::Describe(OfxMeshEffectHandle descriptor) {
	AddInput(kOfxMeshMainInput);
	AddInput(kOfxMeshMainOutput);

    OfxParamSetHandle parameters;
    meshEffectSuite->getParamSet(descriptor, &parameters);
	vcgDescribe(parameters);

    return kOfxStatOK;
}

OfxStatus VcgPlugin::Cook(OfxMeshEffectHandle instance) {
	MfxMesh input = GetInput(kOfxMeshMainInput).GetMesh();

	MfxMeshProps inputProps;
	input.FetchProperties(inputProps);

	MfxAttributeProps pointPos, cornerPoint, faceLen;
	input.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(pointPos);
	input.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(cornerPoint);
	input.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(faceLen);

    // Transfer data to VCG mesh
    VcgMesh vcg_input_mesh, vcg_maybe_output_mesh;
	bool in_place;
	VcgMesh::VertexIterator vi = vcg::tri::Allocator<VcgMesh>::AddVertices(vcg_input_mesh, inputProps.pointCount);
	VcgMesh::FaceIterator fi = vcg::tri::Allocator<VcgMesh>::AddFaces(vcg_input_mesh, inputProps.faceCount);

	// Copy point data
	// TODO: is there a way to create a VCG mesh on existing data buffers without copying them?
	VcgMesh::VertexIterator vi0 = vi;
	char *data = pointPos.data;
	for (int i = 0 ; i < inputProps.pointCount; ++i) {
		float* P = (float*)data;
		vi->P() = VcgMesh::CoordType(P[0], P[1], P[2]);
		++vi;
		data += pointPos.stride;
	}
	vi = vi0;
	
	// Copy face data
	char* vertData = cornerPoint.data;
	char* faceData = faceLen.data;
	for (int i = 0 ; i < inputProps.faceCount ; ++i) {
		for (int k = 0; k < 3; ++k)
		{
			int ptnum = *(int*)(vertData + k * cornerPoint.stride);
			fi->V(k) = &*(vi + ptnum);
		}
		++fi;

		int faceVertCount = *(int*)faceData;
		vertData += faceVertCount * cornerPoint.stride;
		faceData += faceLen.stride;
	}

	// Call core cook function
	if (!vcgCook(vcg_input_mesh, vcg_maybe_output_mesh, &in_place)) {
		input.Release();
		return kOfxStatFailed;
	}
	VcgMesh & vcg_output_mesh = in_place ? vcg_input_mesh : vcg_maybe_output_mesh;

    // Allocate output mesh
	MfxMesh output = GetInput(kOfxMeshMainOutput).GetMesh();
    int pointCount = static_cast<int>(vcg_output_mesh.vert.size());
    int faceCount = static_cast<int>(vcg_output_mesh.face.size());
	output.Allocate(pointCount, 3 * faceCount, faceCount);

    // Get output mesh data
	output.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(pointPos);
	output.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(cornerPoint);
	output.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(faceLen);

    // Fill in output data
	data = pointPos.data;
	for (vi = vcg_output_mesh.vert.begin() ; vi != vcg_output_mesh.vert.end(); ++vi) {
		float* P = (float*)data;
		P[0] = vi->P().X();
		P[1] = vi->P().Y();
		P[2] = vi->P().Z();
		data += pointPos.stride;
	}

	vertData = cornerPoint.data;
	faceData = faceLen.data;
	for (fi = vcg_output_mesh.face.begin() ; fi != vcg_output_mesh.face.end() ; ++fi) {
		if (fi->IsD()) continue; // deleted face

		*(int*)faceData = 3;
		faceData += faceLen.stride;

		for (int k = 0; k < 3; ++k) {
			*(int*)vertData = static_cast<int>(vcg::tri::Index(vcg_output_mesh, fi->V(k)));
			vertData += cornerPoint.stride;
		}
	}

    // Release meshes
	input.Release();
	output.Release();

    return kOfxStatOK;
}

