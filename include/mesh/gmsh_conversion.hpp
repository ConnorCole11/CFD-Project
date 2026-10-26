#include <gmsh.h>
#include <mesh/mesh_data_structures.hpp>

class GmshConvertor {
public:
    
    GmshConvertor();
    
    void convert_to_gmsh(
        std::vector<Triangle> triangles, 
        std::vector<Point3D> points
    );

private:

};