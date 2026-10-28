#pragma once
#include <optional>
#include <array>
#include <cstddef>
#include <vector>

enum struct BoundaryTag {
    Inlet,
    Outlet,
    Wall
};

struct Point3D{
    double x;
    double y;
    double z;
};

// Surface structures
struct Triangle{
    std::array<std::size_t, 3> points;
    BoundaryTag boundary;
};


// Volume structures

struct Node{
    double x;
    double y;
    double z;
    std::size_t id;
};

struct Face {
    std::array<std::size_t, 3> nodes;
    std::size_t tetra_owner; // id of tetrahedron
    std::optional<std::size_t> tetra_neighbor; // Second tetrahedron id. For surface faces, this is a sentinel
    std::optional<BoundaryTag> boundary; // Empty for non-boundary faces
};

struct Tetrahedron {
    std::array<std::size_t, 4> nodes;
    std::size_t id;
};

struct VolumeMesh {
    std::vector<Node> nodes;
    std::vector<Tetrahedron> tetrahedra;
    std::vector<Face> faces;
};