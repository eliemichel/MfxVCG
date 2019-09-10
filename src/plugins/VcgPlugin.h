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

#ifndef H_VCGPLUGIN
#define H_VCGPLUGIN

#include "ofxCore.h"
#include "ofxMeshEffect.h"

#include "VcgMesh.h"

class VcgPlugin {
public:
	VcgPlugin();

	OfxStatus load();
	OfxStatus unload();
	OfxStatus describe(OfxMeshEffectHandle descriptor);
	OfxStatus createInstance(OfxMeshEffectHandle instance);
	OfxStatus destroyInstance(OfxMeshEffectHandle instance);
	OfxStatus cook(OfxMeshEffectHandle instance);
	
	OfxStatus mainEntry(const char *action,
	                    const void *handle,
	                    OfxPropertySetHandle inArgs,
	                    OfxPropertySetHandle outArgs);

	void setHost(OfxHost *_host);

	virtual void vcgDescribe(OfxParamSetHandle parameters) = 0;
	virtual bool vcgCook(VcgMesh & input_mesh,
			             VcgMesh & output_mesh,
			             OfxParamSetHandle parameters,
			             bool *in_place) = 0;

public:
	OfxPlugin ofxPlugin;
	OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
};

#endif // H_VCGPLUGIN