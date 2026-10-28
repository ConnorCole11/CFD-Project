#include <filesystem>
#include <iostream>
#include "config_read.hpp"
#include <string>
#include "mesh/rectangular_channel_smesh.hpp"
#include "mesh/rectangular_channel_vmesh.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

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
    RectangularChannelSMesh s_mesher;
    RectangularChannelVMesh v_mesher;

    s_mesher.generate_smesh(res, channel);

    // Gmsh ----------------
    gmsh::initialize();
    gmsh::model::add(name);

    // Convert surface data to gmsh structure
    s_mesher.generate_gmsh(s_mesher.triangles(), s_mesher.points());
    std::cout << "Returned to main()" << std::endl;

    // Generate the gmsh surface to 3D tetrahedral mesh and convert to VolumeMesh data stucture
    v_mesher.gmsh_to_vmesh();
    gmsh::finalize();

    // Use local datastructures
    v_mesher.build_lookups();
    VolumeMesh v_mesh = v_mesher.get_vmesh();

    std::cout << "End of main" << std::endl;
    return 0;
}