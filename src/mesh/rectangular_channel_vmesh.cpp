#include "mesh/rectangular_channel_vmesh.hpp"
#include <iostream>
#include <algorithm>
#include <limits>
#include <stdexcept>

RectangularChannelVMesh::RectangularChannelVMesh() {};

void RectangularChannelVMesh::gmsh_to_vmesh(){
    // Generate the 3D mesh
    std::cout << "Generating gmsh volume mesh..." << std::endl;
    gmsh::model::mesh::generate(3);

    // temp
    std::vector<int> element_types;
    gmsh::model::mesh::getElementTypes(element_types);

    std::cout << "Element types found:";

    for (int type : element_types) {
        std::vector<std::size_t> tags;
        std::vector<std::size_t> connectivity;

        gmsh::model::mesh::getElementsByType(
            type, tags, connectivity
        );

        std::cout << "\nType " << type
                << ": " << tags.size() << " elements";
    }

    std::cout << '\n';
    // temp

    // Get node tags, coords, and coords relative to mesh
    std::cout << "Getting nodes..." << std::endl;
    std::vector<std::size_t> node_tags;
    std::vector<double> coords;
    std::vector<double> parametric_coords;

    gmsh::model::mesh::getNodes(
    node_tags,
    coords,
    parametric_coords
    );

    // Get element tags and coords
    std::cout << "Getting elements..." << std::endl;
    std::vector<std::size_t> element_tags;
    std::vector<std::size_t> connectivity;
    gmsh::model::mesh::getElementsByType(
        4,
        element_tags,
        connectivity
    );

    // Convert data to VolumeMesh
    // 1. Convert to node structure: {x, y, z, id}
    std::cout << "Converting node data to Nodes structure..." << std::endl;
    std::vector<Node> nodes;
    nodes.reserve(node_tags.size());
    for (std::size_t i = 0; i < node_tags.size(); i++){
        // Convert to node structure
        Node node {coords[3*i], coords[3*i+1], coords[3*i+2], node_tags[i]};
        nodes.push_back(node);
    }

    // Node lookup table
    std::unordered_map<std::size_t, std::size_t> nodeIndex; // {index in vector, index in gmsh}
    nodeIndex.reserve(node_tags.size());
    for (std::size_t i = 0; i < node_tags.size(); ++i) {
        nodeIndex.emplace(node_tags[i], i);
    }

    // 2. Assign nodes to tetrahedra
    std::cout << "Converting element data to Tetrahedra structure..." << std::endl;
    std::vector<Tetrahedron> tetrahedra;
    tetrahedra.reserve(element_tags.size());
    for (std::size_t i = 0; i < element_tags.size(); i++){
        std::array<std::size_t, 4> node_ids;
        for (std::size_t j = 0; j < 4; ++j) {
            auto it = nodeIndex.find(connectivity[4*i + j]);

            if (it == nodeIndex.end()) {
                throw std::runtime_error(
                    "Tetrahedron references an unknown node tag"
                );
            }

            node_ids[j] = it->second;
        }

        Tetrahedron tetrahedron {
            node_ids, 
            element_tags[i]
        };

        tetrahedra.push_back(tetrahedron);
    }

    // 3. Create faces: {std::array<std::size_t, 3> nodes, <std::size_t> tetra_owner, <std::size_t> tetra_neighbor, <BoundaryTag> boundary}
    // Node combos: 012, 013, 023, 123
    std::vector<Face> faces;
    std::unordered_map<std::size_t, std::size_t> face_lookup;

    const std::size_t n_nodes = nodes.size();

    // Check that M^3 - 1 fits in std::size_t, so the key cannot overflow.
    if (n_nodes > std::cbrt(
            static_cast<double>(std::numeric_limits<std::size_t>::max()))) {
        throw std::overflow_error("Mesh is too large for face key encoding");
    }

    face_lookup.reserve(4 * tetrahedra.size());
    faces.reserve(4 * tetrahedra.size());

    for (std::size_t i = 0; i < tetrahedra.size(); ++i) {

        // The four triangular faces of this tetrahedron
        const std::array<std::array<std::size_t, 3>, 4> face_nodes = {{
            {tetrahedra[i].nodes[0], tetrahedra[i].nodes[1], tetrahedra[i].nodes[2]},
            {tetrahedra[i].nodes[0], tetrahedra[i].nodes[1], tetrahedra[i].nodes[3]},
            {tetrahedra[i].nodes[0], tetrahedra[i].nodes[2], tetrahedra[i].nodes[3]},
            {tetrahedra[i].nodes[1], tetrahedra[i].nodes[2], tetrahedra[i].nodes[3]}
        }};

        for (const auto& face_node : face_nodes) {

            // Sort a COPY to make the key independent of node ordering.
            auto sorted_nodes = face_node;
            std::sort(sorted_nodes.begin(), sorted_nodes.end());

            // Encode the three node indices into one integer.
            const std::size_t key =
                sorted_nodes[0]
                + n_nodes * sorted_nodes[1]
                + n_nodes * n_nodes * sorted_nodes[2];

            // Has this face already been encountered?
            auto it = face_lookup.find(key);

            if (it == face_lookup.end()) {

                // New face: it belongs to this tetrahedron so far.
                Face face{};
                face.nodes = face_node;  // Preserve original ordering
                face.tetra_owner = i;    // Tetrahedron vector index
                face.tetra_neighbor = std::nullopt;
                face.boundary = std::nullopt;

                faces.push_back(face);

                // Store the index of the newly added face.
                face_lookup.emplace(key, faces.size() - 1);

            } else {

                // Existing face: this tetrahedron is its neighbor.
                Face& existing_face = faces[it->second];

                // A face should belong to at most two tetrahedra.
                if (existing_face.tetra_neighbor.has_value()) {
                    throw std::runtime_error(
                        "A face belongs to more than two tetrahedra"
                    );
                }

                existing_face.tetra_neighbor = i;
            }
        }
    }


    

    // 4. Create VolumeMesh object
    std::cout << "Assembling data into VolumeMesh structure..." << std::endl;

    v_mesh_.nodes = nodes;
    v_mesh_.tetrahedra = tetrahedra;
    v_mesh_.faces = faces;



    // temporary
    std::cout << "Gmsh node tags: " << node_tags.size() << '\n';
    std::cout << "Element tags: " << element_tags.size() << '\n';
    std::cout << "Connectivity entries: " << connectivity.size() << '\n';
}

void RectangularChannelVMesh::build_lookups(){
    // lookup will be {node/tetra id: idx in v_mesh_.nodes}
    std::size_t n_nodes = v_mesh_.nodes.size();
    node_lookup_.reserve(n_nodes);
    for (std::size_t i = 0; i < n_nodes; i++){
        std::size_t node_id = v_mesh_.nodes[i].id;
        node_lookup_.emplace(node_id, i);
    }

    // tetrahedron
    std::size_t n_tetra = v_mesh_.tetrahedra.size();
    tetrahedron_lookup_.reserve(n_tetra);
    for (std::size_t i=0; i < n_tetra; i++){
        std::size_t tetra_id = v_mesh_.tetrahedra[i].id;
        tetrahedron_lookup_.emplace(tetra_id, i);
    }

    // face
    std::size_t n_faces = v_mesh_.faces.size();
    face_lookup_.reserve(n_faces);
    for (std::size_t i=0; i < n_faces; i++){
        std::array<std::size_t, 3> sorted_ids = v_mesh_.faces[i].nodes;
        std::sort(sorted_ids.begin(), sorted_ids.end());
        const std::size_t key =
            sorted_ids[0]
            + n_nodes * sorted_ids[1]
            + n_nodes * n_nodes * sorted_ids[2];
        face_lookup_.emplace(key, i);
        std::cout << "Face key: " << key << std::endl;
    }
}

Node RectangularChannelVMesh::get_node(std::size_t id) {
    auto it = node_lookup_.find(id);

    if (it == node_lookup_.end()) {
        throw std::out_of_range("Node ID not found");
    }

    return v_mesh_.nodes[it->second];
}

Tetrahedron RectangularChannelVMesh::get_tetra(std::size_t id){
    auto it = tetrahedron_lookup_.find(id);

    if (it == tetrahedron_lookup_.end()) {
        throw std::out_of_range("tetra ID not found");
    }

    return v_mesh_.tetrahedra[it->second];
}

Face RectangularChannelVMesh::get_face(std::array<std::size_t, 3> ids){
    // Already gmsh-independent
    std::size_t n_nodes = v_mesh_.nodes.size();
    std::sort(ids.begin(), ids.end());
    const std::size_t key =
        ids[0]
        + n_nodes * ids[1]
        + n_nodes * n_nodes * ids[2];

    auto it = face_lookup_.find(key);

    if (it == face_lookup_.end()) {
        throw std::out_of_range("Face key not found");
    }

    return v_mesh_.faces[it->second];
}

VolumeMesh RectangularChannelVMesh::get_vmesh(){
    return v_mesh_;
}

