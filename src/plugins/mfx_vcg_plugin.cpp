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

#include "VcgPlugin.h"

// When increasing PLUGIN_COUNT, Ctrl+F for all the occurences of "PLUGIN_COUNT"
// in this file to find comment blocks explaining what must be changed
// consequently.
#define PLUGIN_COUNT 2

///////////////////////////////////////////////////////////////////////////////
// Convex Hull Plugin

class ConvexHullPlugin : public VcgPlugin {
public:
	ConvexHullPlugin() {
		ofxPlugin.pluginIdentifier = "ConvexHull";
	}

	void vcgDescribe(OfxParamSetHandle parameters) override {
	}
	
	bool vcgCook(VcgMesh & input_mesh,
			     VcgMesh & output_mesh,
			     OfxParamSetHandle parameters,
			     bool *in_place) override {
		// in_place is used to signal back to the callee whether the filtered mesh
		// replaces the input_mesh (in_place = true) or if a new mesh has been
		// allocated and is returned in output_mesh (in_place = false).
		*in_place = false;

		vcg::tri::UpdateNormal<VcgMesh>::PerFaceNormalized(input_mesh);
		return vcg::tri::ConvexHull<VcgMesh,VcgMesh>::ComputeConvexHull(input_mesh, output_mesh);
	}

};

///////////////////////////////////////////////////////////////////////////////
// Laplacian Smooth Plugin

class LaplacianSmoothPlugin : public VcgPlugin {
public:
	LaplacianSmoothPlugin() {
		ofxPlugin.pluginIdentifier = "LaplacianSmooth";
	}

	void vcgDescribe(OfxParamSetHandle parameters) override {
		parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "Steps", NULL);
		parameterSuite->paramDefine(parameters, kOfxParamTypeBoolean, "Smooth Selected", NULL);
		parameterSuite->paramDefine(parameters, kOfxParamTypeBoolean, "Cotangent Weight", NULL);
	}
	
	bool vcgCook(VcgMesh & input_mesh,
			     VcgMesh & output_mesh,
			     OfxParamSetHandle parameters,
			     bool *in_place) override {
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
};

///////////////////////////////////////////////////////////////////////////////
// Closures
// TODO: find a cleaner design for this..

ConvexHullPlugin plugin0;
LaplacianSmoothPlugin plugin1;
VcgPlugin* gRuntimePool[] = {
	&plugin0,
	&plugin1,
	// There must be a number of exactly PLUGIN_COUNT plugins in this array
};

#define MAKE_PLUGIN_CLOSURES(nth) \
static void plugin ## nth ## _setHost(OfxHost *host) { \
	gRuntimePool[nth]->setHost(host); \
} \
static OfxStatus plugin ## nth ## _mainEntry(const char *action, \
	                                         const void *handle, \
	                                         OfxPropertySetHandle inArgs, \
	                                         OfxPropertySetHandle outArgs) { \
	return gRuntimePool[nth]->mainEntry(action, handle, inArgs, outArgs); \
}

#define REGISTER_PLUGIN_CLOSURE(index) \
gRuntimePool[index]->ofxPlugin.mainEntry = plugin ## index ## _mainEntry; \
gRuntimePool[index]->ofxPlugin.setHost = plugin ## index ## _setHost;

MAKE_PLUGIN_CLOSURES(0)
MAKE_PLUGIN_CLOSURES(1)
// Add such a line for each n < PLUGIN_COUNT. (This is needed to create
// individual C entry points of the form pluginX_mainEntry for each plug-in)

OfxExport int OfxGetNumberOfPlugins(void) {
	REGISTER_PLUGIN_CLOSURE(0)
	REGISTER_PLUGIN_CLOSURE(1)
	// Define here plug-ins for each n < PLUGIN_COUNT, providing an identifier
	// and pointers to the cooking and description functions. These functions
	// are defined above. Follow the examples of the other plug-ins.
    return PLUGIN_COUNT;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
	return &gRuntimePool[nth]->ofxPlugin;
}
