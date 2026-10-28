#include <string>
#include "mesh/mesh_data_structures.hpp"
#include "geometry/rectangular_channel.hpp"
#include <cstddef>
#include <vector>
#include <array>
#include <unordered_map>
#include <gmsh.h>

class RectangularChannelSMesh {
public:

    enum struct Boundaries{
        xmin, // inlet
        xmax, // outlet
        ymin, 
        ymax, 
        zmin, 
        zmax
    };

    RectangularChannelSMesh();

    void generate_smesh(
        double resolution, 
        RectangularChannel channel
    );
    void generate_gmsh(
        const std::vector<Triangle>& triangles, 
        const std::vector<Point3D>& points
    );

    const std::vector<Point3D>& points() const;
    const std::vector<Triangle>& triangles() const;
    const void debug() const;

private:
    std::vector<Point3D> points_;
    std::unordered_map<std::size_t, std::size_t> point_ids_;
    std::vector<Triangle> triangles_;
    int nx_, ny_, nz_;
    double xmin_, xmax_, ymin_, ymax_, zmin_, zmax_;
    double true_res_x_, true_res_y_, true_res_z_;

    void generate_points(
        double resolution, 
        const RectangularChannel channel
    );

    std::size_t grid_key(int i, int j, int k);


    void generate_triangles(
        double target_resolution, 
        const RectangularChannel channel
    );

};


