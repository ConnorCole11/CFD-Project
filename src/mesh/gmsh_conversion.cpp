#include <mesh/gmsh_conversion.hpp>
#include <vector>

GmshConvertor::GmshConvertor(){}

void GmshConvertor::convert_to_gmsh(
    std::vector<Triangle> triangles, 
    std::vector<Point3D> points) 
    {
        // Define gmsh model
        gmsh::model::add("organ on chip");

        // Define gmsh boundary tag
        const int surface_tag = gmsh::model::addDiscreteEntity(2);

        // Define gmsh node tags and coords
        std::vector<std::size_t> node_tags;
        std::vector<double> node_coords;

        node_tags.reserve(points.size());
        node_coords.reserve(points.size()*3);

        // Convert points into gmsh's expected data structure
        for (std::size_t i = 0; i < points.size(); i++){
            node_tags.push_back(i+1); // gmsh usually starts with 1 I guess)

            node_coords.push_back(points[i].x);
            node_coords.push_back(points[i].y);
            node_coords.push_back(points[i].z);
        }
        
        // Add that data structure to gmsh mesh
        gmsh::model::mesh::addNodes(
            2, surface_tag, node_tags, node_coords
        );

        // Define gmsh triangle tags and coords
        std::vector<std::size_t> element_tags;
        std::vector<std::size_t> connectivity;

        element_tags.reserve(triangles.size());
        connectivity.reserve(triangles.size()*3);

        // convert triangle data to gmsh's expected data structure
        for (std::size_t i = 0; i < triangles.size(); i++) {
            element_tags.push_back(i+1);

            connectivity.push_back(triangles[i].points[0] + 1);
            connectivity.push_back(triangles[i].points[0] + 1);
            connectivity.push_back(triangles[i].points[0] + 1);
        }
        
        // Add new data structure to gmsh's model
        gmsh::model::mesh::addElementsByType(
            surface_tag, 2, element_tags, connectivity
        );
}  