#include <filesystem>
#include <iostream>
#include "config_read.hpp"
#include <string>
#include "mesh/rectangular_channel_smesh.hpp"
#include "mesh/gmsh_conversion.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void gen_channel(double length, double width, double height) {
    std::cout << "Hello, CFD!" << '\n';
}

int main(){
    // Parse json file
    std::string json_path = (fs::path(PROJ_ROOT) / "include" / "config.json").string();
    ConfigReader config_reader = ConfigReader(json_path);

    // Read json file
    double length = config_reader.length(); 
    double width = config_reader.width();
    double height = config_reader.height();
    double res = config_reader.resolution();
    std::string name = config_reader.name();
    std::array<double, 3> center = config_reader.center();

    // Create channel based on json file
    RectangularChannel channel(length, width, height, center);


    RectangularChannelSMesh s_mesh;
    s_mesh.generate(res, channel);
    s_mesh.debug();

    std::cout << "Mesh vertices: " << s_mesh.points().size() << std::endl;
    std::cout << "Mesh triangles: " << s_mesh.triangles().size() << std::endl;

    std::cout << "Converting to gmsh surface..." << std::endl;

    // Gmsh
    gmsh::initialize();
    GmshConvertor convertor;
    convertor.convert_to_gmsh(s_mesh.triangles(), s_mesh.points());

    // Generate the 3D tetrahedral mesh
    gmsh::model::mesh::generate(3);

    // Save the mesh
    gmsh::write(PROJ_ROOT "/organ_on_chip.msh");

    gmsh::finalize();

    return 0;
}