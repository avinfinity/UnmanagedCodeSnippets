#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>

#include <fstream> 
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h> 
#include "eigenincludes.h"

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;
// c2t3
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;
typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3;
typedef GT::FT FT;
typedef FT(*Function)(Point_3);
typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;
FT sphere_function(Point_3 p) {
	const FT x2 = p.x()*p.x(), y2 = p.y()*p.y(), z2 = p.z()*p.z();
	const FT x2_p1 = ( p.x() - 1 )*( p.x() - 1 ),  x2_m1 = (p.x() + 1 )* ( p.x() + 1 );
	//if (p.z() < 0)
	//return x2 + y2 + z2 - 1;
	//else if ( p.z() > 0 && p.z() < 0.0001 && x2 + y2 < 1)
	//	return 0;
	//else
	//	return -1;

	//center1 ( 1 , 0 , 0 ) , ( -1 , 0 , 0 )


	if (std::abs(p.x() - 1) > std::abs(p.x() + 1))
	{
		return x2_m1 + y2 + z2 - 2;
	}
	else
	{
		return x2_p1 + y2 + z2 - 2;
	}
}
int main() {
	Tr tr;            // 3D-Delaunay triangulation
	C2t3 c2t3(tr);   // 2D-complex in 3D-Delaunay triangulation
	// defining the surface
	Surface_3 surface(sphere_function,             // pointer to function
		Sphere_3(CGAL::ORIGIN, 2.)); // bounding sphere
	// Note that "2." above is the *squared* radius of the bounding sphere!
	// defining meshing criteria
	CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30.,  // angular bound
		0.1,  // radius bound
		0.1); // distance bound
	// meshing surface
	CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());
	std::cout << "Final number of points: " << tr.number_of_vertices() << "\n";

	std::ofstream out("out.off");
	CGAL::output_surface_facets_to_off(out, c2t3);



	return 0;
}