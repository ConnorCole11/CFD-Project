# CFD-Project (STILL IN PROGRESS)
An API for simulating a fluid's flow and drug transport in a given medium. This is part of a larger project to create an organ-on-chip digital twin with dose distributions of both radiopharmaceuticals and radiation. 



# Custom CFD Solver

A from-scratch computational fluid dynamics (CFD) solver written in C++. The long-term goal is to simulate pressure-driven flow through a 3D channel on an unstructured tetrahedral mesh.

> **Status: early development.** The geometry, configuration, and meshing pipeline are in place. The flow solver itself (`cfd/`) has not been implemented yet.

---

## Table of Contents

- [Overview](#overview)
- [Current Features](#current-features)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Building](#building)
- [Configuration](#configuration)
- [How the Meshing Pipeline Works](#how-the-meshing-pipeline-works)
- [Data Structures](#data-structures)
- [Known Limitations](#known-limitations)
- [Roadmap](#roadmap)
- [License](#license)

---

## Overview

The project currently takes a rectangular channel definition from a JSON file and produces a fully connected, solver-ready volume mesh:

```
config.json ──► RectangularChannel ──► structured surface triangulation
                                              │
                                              ▼
                                   Gmsh (classify + create volume)
                                              │
                                              ▼
                                   3D tetrahedral mesh (Gmsh)
                                              │
                                              ▼
                              VolumeMesh (nodes, tets, faces w/ owner/neighbor)
```

Gmsh is used only for tetrahedralizing the volume. The solver works on its own Gmsh-independent data structures, so the numerical code never depends on Gmsh tags or its API.

## Current Features

- **JSON-driven setup** for channel geometry, mesh resolution, boundary pressures, and (planned) time-stepping parameters.
- **Rectangular channel geometry** defined by length, width, height, and center position.
- **Structured surface mesh generator** that triangulates all six faces of the box:
  - Resolution is adjusted per axis so the cell count is an integer (`n = ceil(L / resolution)`), meaning the actual spacing may be slightly finer than requested.
  - Shared edge and corner points are de-duplicated so the six faces form a watertight surface.
  - Triangles are wound with outward-facing normals.
- **Gmsh integration**: the surface triangulation is imported as a discrete entity, classified into surface patches, converted to geometry, closed into a surface loop, and filled with a volume.
- **Tetrahedral volume meshing** via Gmsh's 3D mesher.
- **Solver-independent volume mesh** (`VolumeMesh`) with:
  - Nodes, 4-node tetrahedra, and triangular faces.
  - Face connectivity: each face stores its owner tetrahedron and (for interior faces) its neighbor.
  - Hash-based lookups by node ID, tetrahedron ID, or sorted face node triple.

## Project Structure

```
.
├── CMakeLists.txt
├── LICENSE
├── README.md
├── include/
│   ├── config.json                       # Simulation/mesh/geometry settings
│   ├── config_read.hpp                   # JSON config reader
│   ├── gmsh.h                            # Gmsh C++ API header
│   ├── cfd/                              # (reserved for solver headers)
│   ├── geometry/
│   │   └── rectangular_channel.hpp
│   └── mesh/
│       ├── mesh_data_structures.hpp      # Point3D, Triangle, Node, Face, Tetrahedron, VolumeMesh
│       ├── rectangular_channel_smesh.hpp # Surface mesh generator + Gmsh import
│       └── rectangular_channel_vmesh.hpp # Gmsh -> VolumeMesh conversion + lookups
├── src/
│   ├── main.cpp                          # Pipeline test driver (will become an example)
│   ├── config_read.cpp
│   ├── cfd/                              # (reserved for solver sources)
│   ├── geometry/
│   │   └── rectangular_channel.cpp
│   └── mesh/
│       ├── rectangular_channel_smesh.cpp
│       └── rectangular_channel_vmesh.cpp
└── tests/
```

## Dependencies

| Dependency | Purpose |
|---|---|
| C++20 compiler | Uses `<numbers>`, structured bindings, `std::optional`, `std::filesystem` |
| [CMake](https://cmake.org/) | Build system |
| [Gmsh](https://gmsh.info/) (C++ API / SDK) | Surface classification and tetrahedral volume meshing |
| [nlohmann/json](https://github.com/nlohmann/json) | Configuration parsing |

## Building

> Adjust paths to match your `CMakeLists.txt`. The code expects a `PROJ_ROOT` compile definition pointing at the repository root, which `main.cpp` uses to locate `include/config.json`.

```bash
git clone <your-repo-url>
cd <your-repo>

cmake -S . -B build
cmake --build build
```

If CMake cannot find Gmsh, point it at your Gmsh SDK installation (for example via `CMAKE_PREFIX_PATH` or the library/include paths your `CMakeLists.txt` defines).

Run the executable from the build directory:

```bash
./build/<executable-name>
```

The current driver generates the surface mesh, builds the Gmsh volume, tetrahedralizes it, converts it to a `VolumeMesh`, and builds the lookup tables, printing progress along the way.

## Configuration

Settings live in `include/config.json`:

```json
{
    "channel": {
        "length": 5.0,
        "width": 5.0,
        "height": 5.0,
        "center": { "x": 0, "y": 0, "z": 0 },
        "in_pressure": 1,
        "out_pressure": 0
    },
    "mesh": {
        "name": "channel",
        "resolution": 1.0
    },
    "CFD": {
        "timestep_size": 0.01,
        "timesteps": 1000
    }
}
```

| Key | Description |
|---|---|
| `channel.length / width / height` | Box dimensions along x, y, and z |
| `channel.center` | Center of the box |
| `channel.in_pressure / out_pressure` | Inlet and outlet pressure boundary values (parsed, not yet used) |
| `mesh.name` | Name of the Gmsh model |
| `mesh.resolution` | Target surface element edge length |
| `CFD.timestep_size / timesteps` | Time-stepping settings (present in the file, not yet read by `ConfigReader`) |

The channel axis runs along x: the `xmin` face is the inlet, the `xmax` face is the outlet, and the remaining four faces are walls.

## How the Meshing Pipeline Works

1. **Read config**: `ConfigReader` loads the JSON file.
2. **Build geometry**: a `RectangularChannel` stores the box dimensions and center.
3. **Surface mesh** (`RectangularChannelSMesh::generate_smesh`): computes grid counts and true spacing, generates de-duplicated boundary points, then triangulates each of the six faces.
4. **Import into Gmsh** (`generate_gmsh`): adds the points and triangles as a discrete surface, calls `classifySurfaces` and `createGeometry`, then creates a surface loop and volume.
5. **Volume mesh** (`RectangularChannelVMesh::gmsh_to_vmesh`): calls `gmsh::model::mesh::generate(3)`, reads nodes and 4-node tetrahedra, and builds faces by hashing each tetrahedron's sorted face node triple. The first tetrahedron to see a face becomes its owner; the second becomes its neighbor. Faces with no neighbor lie on the boundary.
6. **Lookups** (`build_lookups`): builds hash maps from node IDs, tetrahedron IDs, and face node triples to vector indices.

## Data Structures

Defined in `include/mesh/mesh_data_structures.hpp`:

| Type | Contents |
|---|---|
| `BoundaryTag` | `Inlet`, `Outlet`, `Wall` |
| `Point3D` | `x, y, z` |
| `Triangle` | 3 point indices + `BoundaryTag` (surface mesh) |
| `Node` | `x, y, z, id` |
| `Tetrahedron` | 4 node indices + `id` |
| `Face` | 3 node indices, owner tetrahedron, optional neighbor tetrahedron, optional `BoundaryTag` |
| `VolumeMesh` | `vector<Node>`, `vector<Tetrahedron>`, `vector<Face>` |

## Known Limitations

- No flow solver yet; the `cfd/` directories are placeholders.
- Only a rectangular channel geometry is supported.
- Boundary tags (inlet/outlet/wall) are defined but not yet assigned to surface triangles or to boundary faces in the `VolumeMesh`.
- `CFD` settings in `config.json` are not yet parsed.
- `tests/` is empty.
- Face keys are encoded as a single integer from the three sorted node indices, which limits the maximum mesh size (an overflow check is included).
- Units are not yet defined or definable. This will be applied with the CFD.

## Roadmap

- [ ] Assign `BoundaryTag`s (inlet, outlet, wall) to surface triangles and boundary faces
- [ ] Compute geometric quantities: cell volumes, face areas, normals, centroids
- [ ] Choose and implement the discretization (e.g., finite volume on tetrahedra)
- [ ] Implement governing equations and pressure-velocity coupling
- [ ] Apply boundary conditions from the config (inlet/outlet pressure, no-slip walls)
- [ ] Time stepping driven by `CFD.timestep_size` and `CFD.timesteps`
- [ ] Output results (e.g., VTK) for visualization
- [ ] Unit tests for geometry, meshing, and connectivity
- [ ] Validation against analytical or benchmark cases
- [ ] Turn `main.cpp` into a documented example

## License

See [LICENSE](LICENSE).