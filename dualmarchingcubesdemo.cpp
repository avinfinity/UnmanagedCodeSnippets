
#include <iostream>
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include "volumesegmenter.h"
#include "display3droutines.h"
#include "histogramfilter.h"
#include "marchingcubes.h"
#include "dualmarchingcubes.h"
#include "stdio.h"
#include <dmc/dmc.hpp>
#include <fstream>
#include "meshviewer.h"
#include "volumesigneddistancefunction.h"



void angleAxisRotationDemo()
{
	Eigen::Vector3f vector( 0 , 0, 1); //z Axis

	Eigen::Vector3f position( 0 , 0 , 0);
	Eigen::Vector3f direction = vector;

	double length = 1;
	double shaftRadius = 0.01;
	double arrowRadius = 0.02;

	int shaftResolution = 30;

	std::vector<Eigen::Vector3f> arrowPoints;
	std::vector<Eigen::Vector3f> arrowColors;
	std::vector<unsigned int> arrowIndices;

	vc::createArrow(position, direction, length, shaftRadius, arrowRadius , shaftResolution, arrowPoints, arrowIndices);

	arrowColors.resize(arrowPoints.size(), Eigen::Vector3f(1, 0, 0));

	


	//now generate rays
	
	double angleStart = -M_PI / 4;
	double angleEnd = M_PI / 4;
	double angleStep = M_PI / 12;

	

	for (double theta1 = angleStart; theta1 < angleEnd; theta1 += angleStep)
		for (double theta2 = angleStart; theta2 < angleEnd; theta2 += angleStep)
		{
			if ( std::abs(theta1) < 0.0001 && std::abs(theta2) < 0.0001)
				continue;

			double cos1 = cos(theta1);
			double cos2 = cos(theta2);
			double theta3 = acos( 1 - cos1 * cos1 - cos2 * cos2 );

			Eigen::Matrix3f m;
			m = Eigen::AngleAxisf(theta1, Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(theta2, Eigen::Vector3f::UnitY())
				* Eigen::AngleAxisf(theta3, Eigen::Vector3f::UnitZ());

			std::cout << m << std::endl << "is unitary: " << m.isUnitary() << std::endl;

			Eigen::Vector3f newDir = m * direction;


			std::vector<Eigen::Vector3f> newArrowPoints;
			std::vector<unsigned int> newArrowIndices;

			vc::createArrow(position, newDir, length, shaftRadius, arrowRadius, shaftResolution, newArrowPoints, newArrowIndices);

			unsigned int offset = arrowPoints.size();

			//std::for_each(newArrowIndices.begin(), newArrowIndices.end(), [](unsigned int &n  ) { n += offset; });

			for (auto& id : newArrowIndices)
			{
				id += offset;
			}

			arrowPoints.insert(arrowPoints.end(), newArrowPoints.begin(), newArrowPoints.end());
			arrowIndices.insert(arrowIndices.end(), newArrowIndices.begin(), newArrowIndices.end());

			arrowColors.resize(arrowColors.size() + arrowPoints.size(), Eigen::Vector3f(0, 1, 0));

		}


	tr::Display3DRoutines::displayMesh(arrowPoints, arrowColors, arrowIndices);
}

void generateMeshVolumeSDF(imt::volume::VolumeInfo& volume, std::vector<Eigen::Vector3f>& surfacePoints, int isoThreshold);

void view(std::vector<dmc::triangle3d>& triangle3d);

int main(int argc, char **argv)
{

	//angleAxisRotationDemo();


	//return 0;





	std::cout << "dual marching cubes demo : " << std::endl;
	QString dataPath = "G:/projects/Wallthickness/data/separated_part_7.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(dataPath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;	

	volInfo.mVolumeDataU16 = (unsigned short*)volInfo.mVolumeData;

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< long > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

	for (int zz = 0; zz <volInfo.mDepth; zz++)
		for (int yy = 0; yy < volInfo.mHeight; yy++)
			for (int xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
	//filter.plotHistogram(histogram);


	long iso50Th = filter.ISO50Threshold(histogram);

	std::cout << " iso threshold for the volume : " << iso50Th << std::endl;

	imt::volume::MarchingCubes mc(volInfo , iso50Th);

	auto surface = mc.compute();

	std::cout << " number of points on the surface : " << surface.size() << std::endl;

	generateMeshVolumeSDF( volInfo , surface , iso50Th );

	//tr::Display3DRoutines::displayPointSet(surface, std::vector<Eigen::Vector3f>(surface.size(), Eigen::Vector3f(1, 0, 0)));

	std::cout << "computing dual marching cubes " << std::endl;

	imt::volume::DualMarchingCubes dmc( volInfo, iso50Th , surface);

	dmc.compute();

	return 0;
}



template <class Scalar>
struct VolumeSDFObject : dmc::object<Scalar>
{
public:
	typedef dmc::object<Scalar> base_type;

	using typename base_type::scalar_type;
	using typename base_type::vector_type;

	explicit VolumeSDFObject(scalar_type radius , imt::volume::VolumeSignedDistanceFunction& vsdf )
		: radius_(radius) , mVsdf(vsdf)
	{
	}

	virtual scalar_type value(const vector_type& p) const override
	{

#if 0
		auto abs_p = p.map([](auto x) { return std::abs(x); });

		if (abs_p.x() < abs_p.y())
		{
			if (abs_p.y() < abs_p.z())
				return radius_ - abs_p.z();
			else
				return radius_ - abs_p.y();
		}
		else
		{
			if (abs_p.x() < abs_p.z())
				return radius_ - abs_p.z();
			else
				return radius_ - abs_p.x();
		}
#else

		//double dist = sqrt(p.x() * p.x() + p.y() * p.y() + p.z() * p.z());

		//return (dist - radius_);

		Eigen::Vector3f pt(p.x(), p.y(), p.z());

		return mVsdf.signedDistance(pt);

#endif
	}

	virtual vector_type grad(const vector_type& p) const override
	{
		//auto eps = mVsdf.voxelStep();

		//return vector_type(
		//	value(p + vector_type(eps, 0.0, 0.0)) - value(p - vector_type(eps, 0.0, 0.0)),
		//	value(p + vector_type(0.0, eps, 0.0)) - value(p - vector_type(0.0, eps, 0.0)),
		//	value(p + vector_type(0.0, 0.0, eps)) - value(p - vector_type(0.0, 0.0, eps))) /
		//	(2.0 * eps);

		Eigen::Vector3f pt(p.x(), p.y(), p.z());

		Eigen::Vector3f gradf = mVsdf.gradient(pt); 
	
		return vector_type(gradf(0), gradf(1), gradf(2));
	}

	virtual vector_type grad(const vector_type& p, int level) const override
	{
		auto eps = 1.0e-6;

		return vector_type(
			value(p + vector_type(eps, 0.0, 0.0)) - value(p - vector_type(eps, 0.0, 0.0)),
			value(p + vector_type(0.0, eps, 0.0)) - value(p - vector_type(0.0, eps, 0.0)),
			value(p + vector_type(0.0, 0.0, eps)) - value(p - vector_type(0.0, 0.0, eps))) /
			(2.0 * eps);
	}



private:
	scalar_type radius_;

	imt::volume::VolumeSignedDistanceFunction& mVsdf;
};



void generateMeshVolumeSDF( imt::volume::VolumeInfo& volume , std::vector<Eigen::Vector3f>& surfacePoints  , int isoThreshold )
{
	imt::volume::VolumeSignedDistanceFunction vsdf( volume , surfacePoints , isoThreshold );

	
	dmc::tree<double>::config_type config;

	config.grid_width = volume.mVoxelStep(0) * volume.mDepth;

	config.tolerance = 0.5;

	//config.maximum_depth = 3;

	//std::cout << "maximum depth : " << config.maximum_depth << std::endl;

	//config.grid_width = volInfo.mVoxelStep(0) * 6;
	//{ (double)volInfo.mWidth, (double)volInfo.mHeight , (double)volInfo.mDepth }
	//dmc::tree<double> t({  0.0, 0.0, 0.0 }, { volInfo.mWidth * volInfo.mVoxelStep(0), 
	//volInfo.mHeight * volInfo.mVoxelStep(1) , volInfo.mDepth * volInfo.mVoxelStep(2) }, config);

	//dmc::tree<double> t({ 0.0, 0.0, 0.0 }, { 1.0, 1.0 , 1.0 });
	//   t.generate((test_object<double>(1.5f)));////;VolumeObject<double>(volInfo, divisions)); //

	dmc::tree<double> t( {0 , 0 , 0} , { volume.mDepth * volume.mVoxelStep(0), 
		                                 volume.mDepth * volume.mVoxelStep(1),
		                                 volume.mDepth * volume.mVoxelStep(2) } , config );


	t.generate(VolumeSDFObject<double>(1.5f , vsdf));

	//std::vector<Eigen::Vector3f> colors(t.mLeafPoints.size(), Eigen::Vector3f(1, 0, 0));

	//tr::Display3DRoutines::displayPointSet(t.mLeafPoints, colors);
	std::vector<dmc::triangle3d> triangles;



	t.enumerate([&](const auto& t)
	{
		triangles.push_back(t);
	});
	

	view(triangles);
}



void view(std::vector<dmc::triangle3d>& triangle3d)
{
	int nTriangles = triangle3d.size();

	std::vector<Eigen::Vector3f> vertices(nTriangles * 3);
	std::vector<unsigned int> indices(nTriangles * 3);

	for (int tt = 0; tt < nTriangles; tt++)
	{
		vertices[3 * tt](0) = triangle3d[tt].p1()[0];
		vertices[3 * tt](1) = triangle3d[tt].p1()[1];
		vertices[3 * tt](2) = triangle3d[tt].p1()[2];

		vertices[3 * tt + 1](0) = triangle3d[tt].p2()[0];
		vertices[3 * tt + 1](1) = triangle3d[tt].p2()[1];
		vertices[3 * tt + 1](2) = triangle3d[tt].p2()[2];

		vertices[3 * tt + 2](0) = triangle3d[tt].p3()[0];
		vertices[3 * tt + 2](1) = triangle3d[tt].p3()[1];
		vertices[3 * tt + 2](2) = triangle3d[tt].p3()[2];

		indices[3 * tt] = 3 * tt;
		indices[3 * tt + 1] = 3 * tt + 1;
		indices[3 * tt + 2] = 3 * tt + 2;
	}


	tr::Display3DRoutines::displayMesh(vertices, indices);

}