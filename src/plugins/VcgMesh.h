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

#ifndef H_VCGMESH
#define H_VCGMESH

#include <vcg/complex/complex.h>

// VCG related routines

class VcgVertex; class VcgEdge; class VcgFace;
struct VcgUsedTypes : public vcg::UsedTypes<vcg::Use<VcgVertex>   ::AsVertexType,
                                           vcg::Use<VcgEdge>     ::AsEdgeType,
                                           vcg::Use<VcgFace>     ::AsFaceType>{};
class VcgVertex  : public vcg::Vertex< VcgUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::BitFlags  >{};
class VcgFace    : public vcg::Face<   VcgUsedTypes, vcg::face::FFAdj,  vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::BitFlags > {};
class VcgEdge    : public vcg::Edge<   VcgUsedTypes> {};
class VcgMesh    : public vcg::tri::TriMesh< std::vector<VcgVertex>, std::vector<VcgFace> , std::vector<VcgEdge>  > {};


#endif // H_VCGMESH
