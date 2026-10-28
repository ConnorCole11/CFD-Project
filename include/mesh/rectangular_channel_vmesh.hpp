#include <gmsh.h>
#include "mesh/mesh_data_structures.hpp"
#include <vector>
#include <array>
#include <unordered_map>

class RectangularChannelVMesh {
public:
    RectangularChannelVMesh();
    void gmsh_to_vmesh();

    void build_lookups();
    Node get_node(std::size_t id);
    Tetrahedron get_tetra(std::size_t id);
    Face get_face(std::array<std::size_t, 3> id);
    VolumeMesh get_vmesh();



private:
    VolumeMesh v_mesh_;
    std::unordered_map<std::size_t, std::size_t> node_lookup_;
    std::unordered_map<std::size_t, std::size_t> tetrahedron_lookup_;
    std::unordered_map<std::size_t, std::size_t> face_lookup_;
};