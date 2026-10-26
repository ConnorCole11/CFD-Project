#include "mesh/rectangular_channel_smesh.hpp"
#include <cmath>
#include <algorithm>      // std::max
#include <stdexcept>      // std::invalid_argument
#include <iostream>

// RectangularChannelSMesh::RectangularChannelSMesh(){

// }
RectangularChannelSMesh::RectangularChannelSMesh() = default;

void RectangularChannelSMesh::generate_points(double target_resolution, const RectangularChannel channel){
    points_.clear();
    point_ids_.clear();
    
    // Create points, making sure they are on edges/corners as well to connect other faces directly
    auto addPoint = [&](int i, int j, int k) {


        // Unique key for this grid point
        std::size_t key =
            i + (nx_ + 1) * (j + (ny_ + 1) * k);

        // Skip if an edge/corner point was already added by another face
        if (point_ids_.find(key) != point_ids_.end()) {
            return;
        }

        points_.push_back(Point3D{
            xmin_ + i * true_res_x_,
            ymin_ + j * true_res_y_,
            zmin_ + k * true_res_z_
        });

        point_ids_[key] = points_.size() - 1;
    };

    // x-min and x-max faces: i is fixed
    for (int k = 0; k <= nz_; ++k) {
        for (int j = 0; j <= ny_; ++j) {
            addPoint(0,  j, k);  // xmin
            addPoint(nx_, j, k);  // xmax
        }
    }

    // y-min and y-max faces: j is fixed
    for (int k = 0; k <= nz_; ++k) {
        for (int i = 0; i <= nx_; ++i) {
            addPoint(i, 0,  k);  // ymin
            addPoint(i, ny_, k);  // ymax
        }
    }

    // z-min and z-max faces: k is fixed
    for (int j = 0; j <= ny_; ++j) {
        for (int i = 0; i <= nx_; ++i) {
            addPoint(i, j, 0);   // zmin
            addPoint(i, j, nz_);  // zmax
        }
    }
}

std::size_t RectangularChannelSMesh::grid_key(int i, int j, int k)
{
    return static_cast<std::size_t>(i)
         + static_cast<std::size_t>(nx_ + 1)
         * (static_cast<std::size_t>(j)
         + static_cast<std::size_t>(ny_ + 1) * k);
}

void RectangularChannelSMesh::generate_triangles(
    double target_resolution,
    const RectangularChannel channel)
{
    triangles_.clear();

    // Helper: retrieve the index of a structured-grid vertex.
    auto p = [&](int i, int j, int k) -> std::size_t {
        return point_ids_.at(grid_key(i, j, k));
    };

    // x-min face: i = 0, outward normal = (-1, 0, 0)
    for (int k = 0; k < nz_; ++k) {
        for (int j = 0; j < ny_; ++j) {

            auto p00 = p(0, j,     k);
            auto p10 = p(0, j + 1, k);
            auto p01 = p(0, j,     k + 1);
            auto p11 = p(0, j + 1, k + 1);

            triangles_.push_back({p00, p01, p11});
            triangles_.push_back({p00, p11, p10});
        }
    }

    // x-max face: i = nx_, outward normal = (+1, 0, 0)
    for (int k = 0; k < nz_; ++k) {
        for (int j = 0; j < ny_; ++j) {

            auto p00 = p(nx_, j,     k);
            auto p10 = p(nx_, j + 1, k);
            auto p01 = p(nx_, j,     k + 1);
            auto p11 = p(nx_, j + 1, k + 1);

            triangles_.push_back({p00, p10, p11});
            triangles_.push_back({p00, p11, p01});
        }
    }

    // y-min face: j = 0, outward normal = (0, -1, 0)
    for (int k = 0; k < nz_; ++k) {
        for (int i = 0; i < nx_; ++i) {

            auto p00 = p(i,     0, k);
            auto p10 = p(i + 1, 0, k);
            auto p01 = p(i,     0, k + 1);
            auto p11 = p(i + 1, 0, k + 1);

            triangles_.push_back({p00, p10, p11});
            triangles_.push_back({p00, p11, p01});
        }
    }

    // y-max face: j = ny_, outward normal = (0, +1, 0)
    for (int k = 0; k < nz_; ++k) {
        for (int i = 0; i < nx_; ++i) {

            auto p00 = p(i,     ny_, k);
            auto p10 = p(i + 1, ny_, k);
            auto p01 = p(i,     ny_, k + 1);
            auto p11 = p(i + 1, ny_, k + 1);

            triangles_.push_back({p00, p01, p11});
            triangles_.push_back({p00, p11, p10});
        }
    }

    // z-min face: k = 0, outward normal = (0, 0, -1)
    for (int j = 0; j < ny_; ++j) {
        for (int i = 0; i < nx_; ++i) {

            auto p00 = p(i,     j,     0);
            auto p10 = p(i + 1, j,     0);
            auto p01 = p(i,     j + 1, 0);
            auto p11 = p(i + 1, j + 1, 0);

            triangles_.push_back({p00, p01, p11});
            triangles_.push_back({p00, p11, p10});
        }
    }

    // z-max face: k = nz_, outward normal = (0, 0, +1)
    for (int j = 0; j < ny_; ++j) {
        for (int i = 0; i < nx_; ++i) {

            auto p00 = p(i,     j,     nz_);
            auto p10 = p(i + 1, j,     nz_);
            auto p01 = p(i,     j + 1, nz_);
            auto p11 = p(i + 1, j + 1, nz_);

            triangles_.push_back({p00, p10, p11});
            triangles_.push_back({p00, p11, p01});
        }
    }
}


void RectangularChannelSMesh::generate(double target_resolution, RectangularChannel channel){
    // Get channel dimensions
    double length = channel.length();
    double width = channel.width();
    double height = channel.height();

    // Define positional bounds
    std::array<double, 3> center = channel.center();
    xmin_ = center[0] - length*0.5;
    xmax_ = center[0] + length*0.5;
    ymin_ = center[1] - width*0.5;
    ymax_ = center[1] + width*0.5;
    zmin_ = center[2] - height*0.5;
    zmax_ = center[2] + height*0.5;

    // Get number of points per dimension
    nx_ = static_cast<int>(std::ceil((xmax_ - xmin_)/target_resolution));
    ny_ = static_cast<int>(std::ceil((ymax_ - ymin_)/target_resolution));
    nz_ = static_cast<int>(std::ceil((zmax_ - zmin_)/target_resolution));

    true_res_x_ = (xmax_ - xmin_) / nx_;
    true_res_y_ = (ymax_ - ymin_) / ny_;
    true_res_z_ = (zmax_ - zmin_) / nz_;

    // Generate the mesh using the parameters
    generate_points(target_resolution, channel);
    generate_triangles(target_resolution, channel);


}

const void RectangularChannelSMesh::debug() const {
    std::cout << "nx: " << nx_
            << ", ny: " << ny_
            << ", nz: " << nz_ << '\n';

    std::cout << "actual spacing: "
            << true_res_x_ << ", "
            << true_res_y_ << ", "
            << true_res_z_ << '\n';
}

const std::vector<Point3D>& RectangularChannelSMesh::points() const {
    return points_;
}

const std::vector<Triangle>& RectangularChannelSMesh::triangles() const {
    return triangles_;
}