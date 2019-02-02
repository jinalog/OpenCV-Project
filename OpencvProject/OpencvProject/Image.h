#pragma once
#pragma once
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class Image {
public:
	Mat T_image = imread("../image/T_image.jpg", IMREAD_COLOR);	//O 
	Mat F_image = imread("../image/F_image.jpg", IMREAD_COLOR);	//X 
	Mat image1 = imread("../image/image1.jpg", IMREAD_COLOR);	//시작 
	Mat image2 = imread("../image/image2.jpg", IMREAD_COLOR);	//게임 
	Mat image3 = imread("../image/image3.jpg", IMREAD_COLOR);	//종료 
	Mat image4 = imread("../image/three.jpg", IMREAD_COLOR);	//3 
	Mat image5 = imread("../image/two.jpg", IMREAD_COLOR);		//2 
	Mat image6 = imread("../image/one.jpg", IMREAD_COLOR);		//1 
	Mat countdown[3] = { image4, image5, image6 };

	Mat selectImage1 = imread("../image/apple.jpg", IMREAD_COLOR);
	Mat selectImage2 = imread("../image/banana.jpg", IMREAD_COLOR);
	Mat selectImage3 = imread("../image/elephant.jpg", IMREAD_COLOR);
	Mat selectImage4 = imread("../image/giraffe.jpg", IMREAD_COLOR);
	Mat selectImage5 = imread("../image/grape.jpg", IMREAD_COLOR);
	Mat selectImage6 = imread("../image/lion.jpg", IMREAD_COLOR);
	Mat selectImage7 = imread("../image/rabbit.jpg", IMREAD_COLOR);
	Mat selectImage8 = imread("../image/snake.jpg", IMREAD_COLOR);
	Mat selectImage9 = imread("../image/strawberry.jpg", IMREAD_COLOR);
	Mat selectImage10 = imread("../image/watermelon.jpg", IMREAD_COLOR);
	Mat selectgame[10] = { selectImage1, selectImage2, selectImage3, selectImage4, selectImage5, selectImage6, selectImage7, selectImage8, selectImage9, selectImage10 };
};