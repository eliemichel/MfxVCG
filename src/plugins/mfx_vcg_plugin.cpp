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

#include <PluginSupport/MfxRegister>
#include "VcgPlugin.h"

#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/convex_hull.h>
#include <vcg/complex/algorithms/smooth.h>
#include <vcg/complex/algorithms/point_sampling.h>

///////////////////////////////////////////////////////////////////////////////
// Convex Hull Plugin

class ConvexHullPlugin : public VcgPlugin {
public:
	const char* GetName() override {
		return "Convex Hull";
	}

protected:
	void vcgDescribe(OfxParamSetHandle parameters) override {
	}
	
	bool vcgCook(VcgMesh & input_mesh,
			     VcgMesh & output_mesh,
			     bool *in_place) override {
		// in_place is used to signal back to the callee whether the filtered mesh
		// replaces the input_mesh (in_place = true) or if a new mesh has been
		// allocated and is returned in output_mesh (in_place = false).
		*in_place = false;

		UpdateNormal<VcgMesh>::PerFaceNormalized(input_mesh);
		return ConvexHull<VcgMesh,VcgMesh>::ComputeConvexHull(input_mesh, output_mesh);
	}

};

///////////////////////////////////////////////////////////////////////////////
// Laplacian Smooth Plugin

class LaplacianSmoothPlugin : public VcgPlugin {
public:
	const char* GetName() override {
		return "Laplacian Smooth";
	}

protected:
	void vcgDescribe(OfxParamSetHandle parameters) override {
		AddParam("Steps", 1);
		AddParam("Smooth Selected", false);
		AddParam("Cotangent Weight", false);
	}
	
	bool vcgCook(VcgMesh & input_mesh,
			     VcgMesh & output_mesh,
			     bool *in_place) override {
		*in_place = true;
	
		int steps = GetParam<int>("Steps").GetValue();
		bool smooth_selected = GetParam<bool>("Smooth Selected").GetValue();
		bool cotangent_weight = GetParam<bool>("Cotangent Weight").GetValue();

		Smooth<VcgMesh>::VertexCoordLaplacian(input_mesh, steps, smooth_selected, cotangent_weight);
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Poisson Surface Sampling

class PoissonSurfaceSampling : public VcgPlugin {
public:
	const char* GetName() override {
		return "Surface Sampling";
	}

protected:
	void vcgDescribe(OfxParamSetHandle parameters) override {
		AddParam("Sample Count", 100);
		AddParam("Distribute Regularily", true);
		AddParam("Use Custom Radius", false);
		AddParam("Custom Radius", 1.0)
			.Range(1e-6f, 1e6f);
	}

	bool vcgCook(VcgMesh& input_mesh,
		VcgMesh& output_mesh,
		bool* in_place) override {
		*in_place = false;

		int sampleCount = GetParam<int>("Sample Count").GetValue();
		bool regular = GetParam<bool>("Distribute Regularily").GetValue();
		bool use_custom_radius = GetParam<bool>("Use Custom Radius").GetValue();
		double custom_radius = GetParam<double>("Custom Radius").GetValue();

		//input_mesh.vert.EnableQuality(); // if using Ocf

		TrivialSampler<VcgMesh> ps;

		if (regular) {

			TrivialSampler<VcgMesh> ps0;

			SurfaceSampling<VcgMesh>::PoissonDiskParam pp;

			float radius =
				use_custom_radius
				? custom_radius
				: SurfaceSampling<VcgMesh>::ComputePoissonDiskRadius(input_mesh, sampleCount);

			SurfaceSampling<VcgMesh>::MontecarloPoisson(
				input_mesh,
				ps0,
				sampleCount
			);

			VcgMesh montecarlo_mesh;
			{
				auto samples = ps0.SampleVec();
				size_t offset = montecarlo_mesh.vert.size();
				Allocator<VcgMesh>::AddVertices(montecarlo_mesh, samples.size());
				for (size_t i = 0; i < samples.size(); ++i) {
					montecarlo_mesh.vert[offset + i].P() = samples[i];
				}
			}

			UpdateBounding<VcgMesh>::Box(input_mesh);

			SurfaceSampling<VcgMesh>::HierarchicalPoissonDisk(
				input_mesh,
				ps,
				montecarlo_mesh,
				radius//,
				//pp
			);
		}
		else {
			SurfaceSampling<VcgMesh>::Montecarlo(input_mesh, ps, sampleCount);
		}

		auto samples = ps.SampleVec();
		size_t offset = output_mesh.vert.size();
		Allocator<VcgMesh>::AddVertices(output_mesh, samples.size());
		for (size_t i = 0; i < samples.size(); ++i) {
			output_mesh.vert[offset + i].P() = samples[i];
		}
		std::cout << "Sampled " << samples.size() << " points in ps1" << std::endl;

		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

MfxRegister(
	ConvexHullPlugin,
	LaplacianSmoothPlugin,
	PoissonSurfaceSampling
);
