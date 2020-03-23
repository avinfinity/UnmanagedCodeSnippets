

#include "iostream"
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


//
//int main( int argc , char **argv )
//{
//	QString dataPath = "G:/projects/Wallthickness/data/separated_part_7.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;
//
//	imt::volume::RawVolumeDataIO io;
//
//	imt::volume::VolumeInfo volInfo;
//
//	io.readUint16SCV(dataPath, volInfo);
//
//	imt::volume::HistogramFilter filter(&volInfo);
//
//	std::vector< long > histogram(std::numeric_limits< unsigned short >::max(), 0);
//
//	unsigned short *vData = (unsigned short*)volInfo.mVolumeData;
//
//	for (int zz = 0; zz <volInfo.mDepth; zz++)
//		for (int yy = 0; yy < volInfo.mHeight; yy++)
//			for (int xx = 0; xx < volInfo.mWidth; xx++)
//			{
//				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
//			}
//
//
//	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));
//
//	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
//	//filter.plotHistogram(histogram);
//
//
//	long iso50Th = filter.ISO50Threshold(histogram);
//
//	//imt::volume::MarchingCubes mc(volInfo, iso50Th);
//
//	//mc.compute();
//
//	imt::volume::DualMarchingCubes dmc(volInfo, iso50Th);
//	dmc.compute();
//
//
//
//	return 0;
//}


#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 
double valueAt( double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData )
{
	unsigned short interpolatedValue = 0;

	size_t zStep = width * height;
	size_t yStep = width;

	int lx = (int)x;
	int ly = (int)y;
	int lz = (int)z;

	int ux = (int)std::ceil(x);
	int uy = (int)std::ceil(y);
	int uz = (int)std::ceil(z);

	double xV = x - lx;
	double yV = y - ly;
	double zV = z - lz;

	double c000 = grayValue(lx, ly, lz);
	double c100 = grayValue(ux, ly, lz);
	double c010 = grayValue(lx, uy, lz);
	double c110 = grayValue(ux, uy, lz);
	double c001 = grayValue(lx, ly, uz);
	double c101 = grayValue(ux, ly, uz);
	double c011 = grayValue(lx, uy, uz);
	double c111 = grayValue(ux, uy, uz);

	double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
		c100 * xV * (1.0 - yV) * (1.0 - zV) +
		c010 * (1.0 - xV) * yV * (1.0 - zV) +
		c110 * xV * yV * (1.0 - zV) +
		c001 * (1.0 - xV) * (1.0 - yV) * zV +
		c101 * xV * (1.0 - yV) * zV +
		c011 * (1.0 - xV) * yV * zV +
		c111 * xV * yV * zV;

	return interpolatedValF;
}



template <class Scalar>
struct VolumeObject : dmc::object<Scalar>
{
public:
	typedef dmc::object<Scalar> base_type;

	using typename base_type::scalar_type;
	using typename base_type::vector_type;

	explicit VolumeObject( imt::volume::VolumeInfo& volume , double divisions = 5 )
		: _VolumeInfo(volume)
	{
		Eigen::Vector3f val(volume.mWidth , volume.mHeight ,
			volume.mDepth );

		radius_ = val.norm() / 2;

		invStep = 1.0 / volume.mVoxelStep(0); //std::max(std::max(volume.mWidth, volume.mHeight), volume.mDepth) / divisions;
	}

	virtual scalar_type value(const vector_type& p) const override
	{
		//auto abs_p = p.map([](auto x) { return std::abs(x); });

		//if (abs_p.x() < abs_p.y())
		//{
		//	if (abs_p.y() < abs_p.z())
		//		return radius_ - abs_p.z();
		//	else
		//		return radius_ - abs_p.y();
		//}
		//else
		//{
		//	if (abs_p.x() < abs_p.z())
		//		return radius_ - abs_p.z();
		//	else
		//		return radius_ - abs_p.x();
		//}

		double x = p.x() * invStep;
		double y = p.y() * invStep;
		double z = p.z() * invStep;

		if ( x > 1 && y > 1 && z > 1 && x < _VolumeInfo.mWidth - 2 
			 && y < _VolumeInfo.mHeight - 2 && z < _VolumeInfo.mDepth - 2 )
		{
			return valueAt(x , y, z, _VolumeInfo.mWidth, _VolumeInfo.mHeight, (unsigned short*)_VolumeInfo.mVolumeData);
		}
		else
		{
			return 0.0;
		}
	}


	virtual vector_type grad(const vector_type& p) const override
	{
		auto eps = 1.0e-6;

		return vector_type(
			value(p + vector_type(eps, 0.0, 0.0)) - value(p - vector_type(eps, 0.0, 0.0)),
			value(p + vector_type(0.0, eps, 0.0)) - value(p - vector_type(0.0, eps, 0.0)),
			value(p + vector_type(0.0, 0.0, eps)) - value(p - vector_type(0.0, 0.0, eps))) /
			(2.0 * eps);
	}

	virtual vector_type grad(const vector_type& p, int level) const override
	{

		double scale = std::pow(2.0, level);

		auto eps = _VolumeInfo.mVoxelStep(0) * scale;//1.0e-6;

		scalar_type vxp = value(p + vector_type(eps, 0.0, 0.0));
		scalar_type vxn = value(p - vector_type(eps, 0.0, 0.0));

		scalar_type vyp = value(p + vector_type(0, eps, 0.0));
		scalar_type vyn = value(p - vector_type(0, eps, 0.0));

		scalar_type vzp = value(p + vector_type(0, 0, eps));
		scalar_type vzn = value(p - vector_type(0, 0, eps));

		float multiplier = 0.5 / _VolumeInfo.mVoxelStep(0);// eps; ;//

		bool isBoundaryTransitionX = vxp > vxn ? (vxp > _VolumeInfo.mAirVoxelFilterVal && vxn < _VolumeInfo.mAirVoxelFilterVal) :
			(vxn > _VolumeInfo.mAirVoxelFilterVal && vxp < _VolumeInfo.mAirVoxelFilterVal);
		bool isBoundaryTransitionY = vyp > vyn ? (vyp > _VolumeInfo.mAirVoxelFilterVal && vyn < _VolumeInfo.mAirVoxelFilterVal) :
			(vyn > _VolumeInfo.mAirVoxelFilterVal && vyp < _VolumeInfo.mAirVoxelFilterVal);
		bool isBoundaryTransitionZ = vzp > vzn ? (vzp > _VolumeInfo.mAirVoxelFilterVal && vzn < _VolumeInfo.mAirVoxelFilterVal) :
			(vzn > _VolumeInfo.mAirVoxelFilterVal && vzp < _VolumeInfo.mAirVoxelFilterVal);

		if (isBoundaryTransitionX || isBoundaryTransitionY || isBoundaryTransitionZ)
		{
			return vector_type( (vxp - vxn) * multiplier, (vyp - vyn) * multiplier, (vzp - vzn) * multiplier);
		}
		else
		{ 
			return vector_type(0, 0, 0);
		}

	}

private:
	scalar_type radius_;
	imt::volume::VolumeInfo& _VolumeInfo;

	double invStep;
};

void view(std::vector<dmc::triangle3d>& triangle3d);

template <class Scalar>
struct test_object : dmc::object<Scalar>
{
public:
	typedef dmc::object<Scalar> base_type;

	using typename base_type::scalar_type;
	using typename base_type::vector_type;

	explicit test_object(scalar_type radius)
		: radius_(radius)
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

		double dist = sqrt(p.x() * p.x() + p.y() * p.y() + p.z() * p.z());

		return (dist - radius_);

#endif
	}

	virtual vector_type grad(const vector_type& p) const override
	{
		auto eps = 1.0e-6;

		return vector_type(
			value(p + vector_type(eps, 0.0, 0.0)) - value(p - vector_type(eps, 0.0, 0.0)),
			value(p + vector_type(0.0, eps, 0.0)) - value(p - vector_type(0.0, eps, 0.0)),
			value(p + vector_type(0.0, 0.0, eps)) - value(p - vector_type(0.0, 0.0, eps))) /
			(2.0 * eps);
	}

	virtual vector_type grad(const vector_type& p , int level) const override
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
};

int main(int /*argc*/, char* /*argv*/[])
{
		QString dataPath = "G:/projects/Wallthickness/data/separated_part_7.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;
	
		imt::volume::RawVolumeDataIO io;
	
		imt::volume::VolumeInfo volInfo;
	
		io.readUint16SCV(dataPath, volInfo);
	
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

		volInfo.mAirVoxelFilterVal = iso50Th;
	//
	//	//imt::volume::MarchingCubes mc(volInfo, iso50Th);
	//
	//	//mc.compute();
	//
	//	imt::volume::DualMarchingCubes dmc(volInfo, iso50Th);
	//	dmc.compute();
	//
	//
	//
	//	return 0;

		double divisions = 8.0;


		dmc::tree<double>::config_type config;

		config.maximum_depth = 3;

		std::cout << "maximum depth : " << config.maximum_depth << std::endl;

		config.grid_width = volInfo.mVoxelStep(0) * 6;
		//{ (double)volInfo.mWidth, (double)volInfo.mHeight , (double)volInfo.mDepth }
	    //dmc::tree<double> t({  0.0, 0.0, 0.0 }, { volInfo.mWidth * volInfo.mVoxelStep(0), 
		//volInfo.mHeight * volInfo.mVoxelStep(1) , volInfo.mDepth * volInfo.mVoxelStep(2) }, config);
	    
		//dmc::tree<double> t({ 0.0, 0.0, 0.0 }, { 1.0, 1.0 , 1.0 });
	 //   t.generate((test_object<double>(1.5f)));////;VolumeObject<double>(volInfo, divisions)); //

	dmc::tree<double> t({ -3.0, -3.0, -3.0 }, { 3.0, 3.0, 3.0 });
	t.generate(test_object<double>(1.5f));

		std::vector<Eigen::Vector3f> colors(t.mLeafPoints.size(), Eigen::Vector3f(1, 0, 0));
	
		tr::Display3DRoutines::displayPointSet(t.mLeafPoints, colors);
	std::vector<dmc::triangle3d> triangles;

	

	t.enumerate([&](const auto& t) 
	{
		triangles.push_back(t);
	});

	std::cout << "number of triangles : " << triangles.size() << std::endl;

	std::ofstream os("a.stl", std::ios::binary);

	//write_stl(os, triangles);

	view(triangles);
}



void view( std::vector<dmc::triangle3d>& triangle3d )
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