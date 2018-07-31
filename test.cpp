// written by Muhammet Balcilar, France
// All rights reserved

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "ImageProc.h"

using namespace cv;


int main(int argc, char **argv) {



	if (argc < 6) {
		printf("usage: part2 <Image_Path> <startx> <starty> <threshold> <smoothcoeff> [--showimage] [--showregion] [--showperimeter] [--saveregion <regionfilename>] [--saveperimeter <perimeterfilename>]\n");
		return -1;
	}
	
	Mat image = imread(argv[1], 1);

	if (!image.data)                             
	{
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	int x=atoi( argv[2]);
	int y=atoi( argv[3]);
	int th = atoi(argv[4]);
	float smoothparam = atof(argv[5]);

	ImageProc p;

	Mat reg=p.findRegion(image,x,y,th);
	Mat bnd = p.findPerimeter(reg);
	p.getContinousBoundaryPoints(bnd);
	
	// if we find enough number of boundary point to smooth
	if (p.BoundaryPoints.size()>50)
		p.smoothPerimeter(smoothparam);
		


	for (int i = 6; i < argc;i++)
	{
		if (strcmp(argv[i], "--showimage") == 0)
		{
			p.displayImage();
		}
		else if (strcmp(argv[i], "--showregion") == 0)
		{
			p.displayPixels(true);
		}
		else if (strcmp(argv[i], "--showperimeter") == 0)
		{
			p.displayPixels(false);
		}

		if (strcmp(argv[i], "--saveperimeter") == 0)
		{
			if (argc>i)
				p.savePixels(argv[i+1], false);
			else
			{
				std::cout << "Parameter Error, give created perimeter image file name" << std::endl;
			}
		}
		else if (strcmp(argv[i], "--saveregion") == 0)
		{
			if (argc>i)
				p.savePixels(argv[i + 1], true);
			else
			{
				std::cout << "Parameter Error, give created region image file name" << std::endl;
			}

		}
	}

	return 0;
}
