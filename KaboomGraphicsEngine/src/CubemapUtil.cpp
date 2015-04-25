#include "CubemapUtil.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

CubemapUtil::CubemapUtil()
{
	_cubeMapImages.resize(6);
}

void CubemapUtil::allocCubeImages(int singleImageDim)
{
	for (int i = 0; i < 6; i++)
	{
		_cubeMapImages[i] = new osg::Image;

		// HDR
		_cubeMapImages[i]->allocateImage(singleImageDim, singleImageDim, 1, GL_RGBA16F_ARB, GL_FLOAT);
	}
}


osg::ref_ptr<osg::Image> CubemapUtil::getImage(enum osg::TextureCubeMap::Face f)
{
	// TODO: hope osg don't change this cubemap enum
	return _cubeMapImages[(int)f];
}

bool CubemapUtil::loadVerticalCross(const std::string &path)
{
	// Method referenced from CubeMapGen
	// u, v offsets of each face image in the cross, 
	// in units of faces (cross is layed out in a 3x4 grid)
	//      ___
	//     |Z+ |
	//  ___|___|___
	// |X- |Y+ |X+ |
	// |___|___|___|
	//     |Z- |
	//     |___|
	//     |Y- |
	//     |___|

	osg::ref_ptr<osg::Image> cubemap = osgDB::readImageFile(path);
	if (cubemap == NULL)
	{
		OSG_WARN << "Cubemap " + path + " read failed..." << std::endl;
		return false;
	}

	//int crossOffsets[6][2] = {
	//	{ 2, 1 },  //X+ face
	//	{ 0, 1 },  //X- face
	//	{ 1, 1 },  //Y+ face
	//	{ 1, 3 },  //Y- face
	//	{ 1, 0 },  //Z+ face
	//	{ 1, 2 },  //Z- face	
	//};

	int crossOffsets[6][2] = {
		{ 2, 2 },  //X+ face
		{ 0, 2 },  //X- face
		{ 1, 2 },  //Y+ face
		{ 1, 0 },  //Y- face
		{ 1, 3 },  //Z+ face
		{ 1, 1 },  //Z- face	
	};

	//make sure cross image is 3:4 ratio
	if ((cubemap->s() / 3) != (cubemap->t() / 4))
	{
		//texture width/height ratio must be 3/4 for cube map cross
		fprintf(stderr, "Error: texture width:height ratio must be 3:4 for cube map cross." //no comma, string continues on next line
			" Image Contained in %s is %dx%d. \n", path, cubemap->s(), cubemap->t());
		return false;
	}

	int cubeMapSize = cubemap->s() / 3;

	for (int i = 0; i < 6; i++)
	{
		int left = crossOffsets[i][0] * cubeMapSize;
		int right = (crossOffsets[i][0] + 1) * cubeMapSize;
		int top = crossOffsets[i][1] * cubeMapSize;
		int bottom = (crossOffsets[i][1] + 1) * cubeMapSize;

		osg::ref_ptr<osg::Image> img = NULL;

		if (i == 0 || i == 1)
		{
			img = createSubImage(cubemap, left, top, right, bottom, true);
			if (i == 0)
			{
				img->flipHorizontal();
			}
			else if (i == 1)
			{
				img->flipVertical();
			}
		}
		else
		{
			img = createSubImage(cubemap, left, top, right, bottom, false);
			if (i == 5)
			{
				img->flipVertical();
				img->flipHorizontal();
			}
		}

		if (img == NULL)
		{
			return false;
		}

		_cubeMapImages[i] = img;
	}

	return true;
}

void CubemapUtil::saveImageToFile(const std::string &path)
{
	for (int i = 0; i < 6; i++)
	{
		std::string filePath = path + "\\map_" + std::to_string(i) + ".png";
		std::cout << "CubemapUtil: Writing file " + path << "...  ";
		bool res = osgDB::writeImageFile(*_cubeMapImages[i], filePath);
		if (res) std::cout << " successfully" << std::endl;
		else std::cout << " failed..." << std::endl;
	}
}

//http://www.jotschi.de/Technik/2009/10/18/openscenegraph-osgimage-getting-a-subimage-from-an-image.html
osg::ref_ptr<osg::Image> CubemapUtil::createSubImage(osg::Image *sourceImage, int startX, int startY, int stopX, int stopY, bool rotate)
{
	int subImageWidth = stopX - startX;
	int subImageHeight = stopY - startY;
	if (subImageWidth <= 0 || subImageHeight <= 0)
	{
		osg::notify(osg::ALWAYS) << "Error the subimage size is not correct. Aborting.";
		return NULL;
	}

	std::cout << "Size: " << sourceImage->s() << "x" << sourceImage->s()
		<< std::endl;
	std::cout << "SubImageSize: " << subImageWidth << "x" << subImageHeight
		<< std::endl;
	std::cout << "Channels: " << sourceImage->r() << std::endl;

	const unsigned char *sourceData = sourceImage->data();
	struct pixelStruct {
		unsigned char r, g, b;
	};

	osg::Image* subImage = new osg::Image();

	// allocate memory for image data
	const unsigned char *targetData = new unsigned char[subImageWidth
		* subImageHeight * 3];

	struct pixelStruct *pPixelSource = (struct pixelStruct*) (sourceData);
	struct pixelStruct *pPixelTarget = (struct pixelStruct*) (targetData);
	struct pixelStruct *pCurrentPixelSource = NULL;
	struct pixelStruct *pCurrentPixelTarget = NULL;

	int subX = 0;
	int subY = 0;
	for (int x = startX; x < stopX; x++) {
		for (int y = startY; y < stopY; y++) {
			/*std::cout << "Source Pixel [" << x << "][" << y << "]" << std::endl;
			std::cout << "Target Pixel [" << subX << "][" << subY << "]"
			<< std::endl;*/
			pCurrentPixelSource = &pPixelSource[y * sourceImage->s() + x];
			if (rotate)
			{
				pCurrentPixelTarget = &pPixelTarget[subX * subImageHeight + subY];
			}
			else
			{
				pCurrentPixelTarget = &pPixelTarget[subY * subImageWidth + subX];
			}
			pCurrentPixelTarget->r = pCurrentPixelSource->r;
			pCurrentPixelTarget->g = pCurrentPixelSource->g;
			pCurrentPixelTarget->b = pCurrentPixelSource->b;
			subY++;
		}
		subY = 0;
		subX++;
	}

	subImage->setImage(subImageWidth, subImageHeight, sourceImage->r(),
		sourceImage->getInternalTextureFormat(),
		sourceImage->getPixelFormat(), 5121, (unsigned char*)targetData,
		osg::Image::NO_DELETE);

	return subImage;
}