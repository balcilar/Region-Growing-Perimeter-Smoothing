#include <stdio.h>
#include <opencv2/opencv.hpp>
#include<vector>

class ImageProc
{

public:

	cv::Mat region;
	cv::Mat image;
	cv::Mat boundary;
	std::vector<cv::Point2f> BoundaryPoints;
	std::vector<cv::Point2f> SmoothedBoundaryPoints;
	int th = 5;
	int syc=0;
	

	cv::Mat& findRegion(cv::Mat &mat, int x, int y, int thh)
	{
		th = thh;
		image = mat;
		region = cv::Mat::zeros(image.rows, image.cols, CV_8U);
		findRegionInner(x, y);
		return region;
	}

	cv::Mat& findPerimeter(cv::Mat &mat)
	{

		boundary = mat.clone();
		const int channels = mat.channels();

		if (channels != 1)
		{
			std::cout << "Given Region image has to be grayscale" << std::endl;
			return boundary;
		}

		// remove inner holes		
		cv::Mat holes = mat.clone();
		cv::floodFill(holes,cv::Point(0, 0), cv::Scalar(255));
		holes=255-holes;
		mat=mat+holes;
		

		// apply erosion for ones(3,3)
		uchar *p;
		for (int i = 1; i < mat.rows - 1; ++i)
		{
			p = boundary.ptr<uchar>(i);
			for (int j = 1; j < mat.cols - 1; ++j)
			{
				int tmp = 0;
				for (int ii = -1;ii<2;ii++)
					for (int jj = -1;jj<2;jj++)
						tmp += (int)mat.ptr<uchar>(i + ii)[j + jj];
				if (tmp == 9 * 255)
					p[j] = 0;
			}
		}
		return boundary;
	}

	void displayImage()
	{
		cv::namedWindow("image", cv::WINDOW_AUTOSIZE);
		cv::imshow("image", image);
		cv::waitKey(0);
	}

	void displayPixels(bool isRegion)
	{

		if (isRegion)
		{
			cv::namedWindow("region", cv::WINDOW_AUTOSIZE);
			cv::imshow("region", region);
		}
		else
		{
			cv::Mat tmp=image.clone();
			cv::namedWindow("boundary", cv::WINDOW_AUTOSIZE);			
			for (int i = 0;i<BoundaryPoints.size() - 1;i++)
				cv::line(tmp, BoundaryPoints[i], BoundaryPoints[i + 1], cv::Scalar(255, 255, 255), 4);
			cv::imshow("boundary", tmp);
			cv::waitKey(0);
			tmp.release();

		}

		cv::waitKey(0);
	}

	void savePixels(std::string  filename, bool isRegion)
	{
		if (isRegion)
		{
			cv::imwrite(filename, region);
		}
		else
		{
			cv::Mat tmp = image.clone();			
			for (int i = 0;i<BoundaryPoints.size() - 1;i++)
				cv::line(tmp, BoundaryPoints[i], BoundaryPoints[i + 1], cv::Scalar(255, 255, 255), 4);
			cv::imwrite(filename, tmp);
			tmp.release();

		}
	}

	void smoothPerimeter(float smoothParam)
	{
		// if given params is out of range return unsmoothed version of perimeter
		if (smoothParam >= 1 || smoothParam <=0)
		{
			return;
		}
			
		std::vector<cv::Point2f> splinePoints;
		double stepsize=1/smoothParam;	
		float length= BoundaryPoints.size() * smoothParam;

		for (int i = 0; i < length - 3; i++)
		{
			cv::Point2f p0= BoundaryPoints[(int)(i/ smoothParam)]; 
			cv::Point2f p1 = BoundaryPoints[(int)((i+1) / smoothParam)];
			cv::Point2f p2 = BoundaryPoints[(int)((i+2) / smoothParam)];
			cv::Point2f p3 = BoundaryPoints[(int)((i+3) / smoothParam)];

			for (int j = 0; j < (int)(2*stepsize); j++)
			{
				//std::cout << j << " " ;
				cv::Point2f pnt= pointOnCurve(p0,  p1,  p2, p3, smoothParam/2*j);
				SmoothedBoundaryPoints.push_back(pnt);
			}
		}
		cv::Point2d first = SmoothedBoundaryPoints[0];
		SmoothedBoundaryPoints.push_back(first);

		BoundaryPoints.clear();		
		BoundaryPoints=SmoothedBoundaryPoints;
		
	}


	void getContinousBoundaryPoints(cv::Mat &InputImage )//, std::vector<Point2D>& BoundaryPoints)
	{

	/*
	This function was found on internet and modified according to our requirements
	https://www.codeproject.com/Articles/1105045/Tracing-Boundary-in-D-Image-Using-Moore-Neighborho
	
	*/
		//std::vector<cv::Point2d> BoundaryPoints;
		BoundaryPoints.clear();

		
		if (InputImage.data)
		{
			int Offset[8][2] = {
				{ -1, -1 },       //  +----------+----------+----------+
				{ 0, -1 },        //  |          |          |          |
				{ 1, -1 },        //  |(x-1,y-1) | (x,y-1)  |(x+1,y-1) |
				{ 1, 0 },         //  +----------+----------+----------+
				{ 1, 1 },         //  |(x-1,y)   |  (x,y)   |(x+1,y)   |
				{ 0, 1 },         //  |          |          |          |
				{ -1, 1 },        //  +----------+----------+----------+
				{ -1, 0 }         //  |          | (x,y+1)  |(x+1,y+1) |
			};                    //  |(x-1,y+1) |          |          |
								  //  +----------+----------+----------+
			const int NEIGHBOR_COUNT = 8;
			
			cv::Point2d BoundaryPixelCord;
			cv::Point2d BoundaryStartingPixelCord;
			cv::Point2d BacktrackedPixelCord;
			int BackTrackedPixelOffset[1][2] = { { 0,0 } };
			bool bIsBoundaryFound = false;
			bool bIsStartingBoundaryPixelFound = false;

			for (int i = 1; i < InputImage.rows - 1; ++i)
			{				
				for (int j = 1; j < InputImage.cols - 1; ++j)
				{
					if (InputImage.ptr<uchar>(i)[j]==255)
					{
						BoundaryPixelCord.x = j;
						BoundaryPixelCord.y = i;
						BoundaryStartingPixelCord = BoundaryPixelCord;
						BacktrackedPixelCord.x = j-1;
						BacktrackedPixelCord.y = i;
						BackTrackedPixelOffset[0][0] = BacktrackedPixelCord.x - BoundaryPixelCord.x;
						BackTrackedPixelOffset[0][1] = BacktrackedPixelCord.y - BoundaryPixelCord.y;
						BoundaryPoints.push_back(BoundaryPixelCord);
						bIsStartingBoundaryPixelFound = true;
						break;
					}
				}
				if (bIsStartingBoundaryPixelFound)
					break;
			}

			
			cv::Point2d CurrentBoundaryCheckingPixelCord;
			cv::Point2d PrevBoundaryCheckingPixxelCord;
			if (!bIsStartingBoundaryPixelFound)
			{
				BoundaryPoints.pop_back();
			}
			while (true && bIsStartingBoundaryPixelFound)
			{
				int CurrentBackTrackedPixelOffsetInd = -1;
				for (int Ind = 0; Ind < NEIGHBOR_COUNT; ++Ind)
				{
					if (BackTrackedPixelOffset[0][0] == Offset[Ind][0] &&
						BackTrackedPixelOffset[0][1] == Offset[Ind][1])
					{
						CurrentBackTrackedPixelOffsetInd = Ind;// Finding the bracktracked 
															   // pixel's offset index
						break;
					}
				}
				int Loop = 0;
				while (Loop < (NEIGHBOR_COUNT - 1) && CurrentBackTrackedPixelOffsetInd != -1)
				{
					int OffsetIndex = (CurrentBackTrackedPixelOffsetInd + 1) % NEIGHBOR_COUNT;
					CurrentBoundaryCheckingPixelCord.x = BoundaryPixelCord.x + Offset[OffsetIndex][0];
					CurrentBoundaryCheckingPixelCord.y = BoundaryPixelCord.y + Offset[OffsetIndex][1];
					int ImageIndex = CurrentBoundaryCheckingPixelCord.y * InputImage.cols +	CurrentBoundaryCheckingPixelCord.x;
					int i= CurrentBoundaryCheckingPixelCord.x;
					if (0 != InputImage.ptr<uchar>(CurrentBoundaryCheckingPixelCord.y)[i])// finding the next boundary pixel
					{
						BoundaryPixelCord = CurrentBoundaryCheckingPixelCord;
						BacktrackedPixelCord = PrevBoundaryCheckingPixxelCord;
						BackTrackedPixelOffset[0][0] = BacktrackedPixelCord.x - BoundaryPixelCord.x;
						BackTrackedPixelOffset[0][1] = BacktrackedPixelCord.y - BoundaryPixelCord.y;
						BoundaryPoints.push_back(BoundaryPixelCord);
						break;
					}
					PrevBoundaryCheckingPixxelCord = CurrentBoundaryCheckingPixelCord;
					CurrentBackTrackedPixelOffsetInd += 1;
					Loop++;
				}
				if (BoundaryPixelCord.x == BoundaryStartingPixelCord.x &&
					BoundaryPixelCord.y == BoundaryStartingPixelCord.y) // if the current pixel = 
																		// starting pixel
				{
					BoundaryPoints.pop_back();
					bIsBoundaryFound = true;
					break;
				}
			}
			if (!bIsBoundaryFound) // If there is no connected boundary clear the list
			{
				BoundaryPoints.clear();
			}
			else
			{
				cv::Point2d first= BoundaryPoints[0];
				BoundaryPoints.push_back(first);
			}
		}
	}
	


private:


	cv::Point2f pointOnCurve(cv::Point2f p0, cv::Point2f p1, cv::Point2f p2, cv::Point2f p3, float t)
	{
		cv::Point2f ret;

		float t2 = t * t;
		float t3 = t2 * t;

		ret.x = 0.5f * ((2.0f * p1.x) +
			(-p0.x + p2.x) * t +
			(2.0f * p0.x - 5.0f * p1.x + 4 * p2.x - p3.x) * t2 +
			(-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

		ret.y = 0.5f * ((2.0f * p1.y) +
			(-p0.y + p2.y) * t +
			(2.0f * p0.y - 5.0f * p1.y + 4 * p2.y - p3.y) * t2 +
			(-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

		return ret;
	}

	void findRegionInner(int x, int y)
	{
		region.ptr<uchar>(y)[x] = 255;		

		if (y>region.rows - 2 || x>region.cols - 2 || y<1 || x < 1)
			return;

		cv::Vec3b *pI, *pIm1, *pIp1;

		pI = image.ptr<cv::Vec3b>(y);
		pIm1 = image.ptr<cv::Vec3b>(y - 1);
		pIp1 = image.ptr<cv::Vec3b>(y + 1);

		cv::Vec3b rgb = 1.0*pI[x];
		cv::Vec3f up, down, left, right;

		up = (cv::Vec3f)pIm1[x] - (cv::Vec3f)rgb;
		down = (cv::Vec3f)pIp1[x] - (cv::Vec3f)rgb;
		left = (cv::Vec3f)pI[x - 1] - (cv::Vec3f)rgb;
		right = (cv::Vec3f)pI[x + 1] - (cv::Vec3f)rgb;


		//uchar rup, rdown, rleft, rright;

		
		if (region.ptr<uchar>(y - 1)[x] != 255 && up[0] * up[0] + up[1] * up[1] + up[2] * up[2] <th*th)
			findRegionInner(x, y - 1);


		
		if (region.ptr<uchar>(y + 1)[x] != 255 && down[0] * down[0] + down[1] * down[1] + down[2] * down[2] <th*th)
			findRegionInner(x, y + 1);

		
		if (region.ptr<uchar>(y)[x - 1] != 255 && left[0] * left[0] + left[1] * left[1] + left[2] * left[2] <th*th)
			findRegionInner(x - 1, y);

		
		if (region.ptr<uchar>(y)[x + 1] != 255 && right[0] * right[0] + right[1] * right[1] + right[2] * right[2] <th*th)
			findRegionInner(x + 1, y);

		return;
	}
};

