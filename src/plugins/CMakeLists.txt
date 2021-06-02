# MfxVCG is an Open Mesh Effect plug-in providing VCGlib-based filters.
# Copyright (C) 2019  Élie Michel
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set(INC
  .
)

set(SRC
  VcgMesh.h
  VcgPlugin.h
  VcgPlugin.cpp
  mfx_vcg_plugin.cpp
)

set(LIB
  vcglib
  CppPluginSupport
)

add_library(mfx_vcg_plugin SHARED ${SRC})

target_include_directories(mfx_vcg_plugin PRIVATE ${INC})
target_link_libraries(mfx_vcg_plugin PRIVATE ${LIB})
set_target_properties(mfx_vcg_plugin PROPERTIES SUFFIX ".ofx")
