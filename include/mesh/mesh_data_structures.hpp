#pragma once

#include <array>
#include <cstddef>
#include <vector>

struct Point3D{
    double x;
    double y;
    double z;
};

enum struct BoundaryTag {
    Inlet,
    Outlet,
    Wall
};

struct Triangle{
    std::array<std::size_t, 3> points;
    BoundaryTag boundary;
};

struct Tetrahedron {
    std::array<std::size_t, 4> points;
};

struct VolumeMesh {
    std::vector<Point3D> points;
    std::vector<Tetrahedron> tetrahedra;
    std::vector<Triangle> boundaryFaces;
};