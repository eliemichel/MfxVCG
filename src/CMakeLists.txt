# MfxVCG is an Openfx plug-in providing VCGlib-based filters.
# Copyright (C) 2019-2021  Élie Michel
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

add_subdirectory(OpenMfx)

add_library(vcglib INTERFACE)
target_include_directories(vcglib INTERFACE vcglib)

add_subdirectory(plugins)
