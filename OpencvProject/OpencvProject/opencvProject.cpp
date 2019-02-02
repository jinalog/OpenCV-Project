/*
* 프로그램명: edugame.cpp
* 설명: 특정 물체(시연 영상에서는 손)를 인식하여 그 물체의 중심값과 좌표를 이용하여 작성된 O/X 퀴즈 또는 그림찾기 프로그램
*/

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "Image.h"

using namespace cv;
using namespace std;

Image in;

int maxIdx[2];			//getHandCenter()적용 후 거리 변환 행렬의 값이 가장 큰 픽셀의 좌표값 저장(행, 열 순)
int threshold1 = 80;	//saturation, value 값의 최소값 지정
Vec3b lower_blue1, upper_blue1, lower_blue2, upper_blue2;	//hue 범위
Mat img_color, dst;	//img_color=설정 이미지, dst=사용자 이미지

void trackbar(int, void*) {}	//트랙바 콜백

								//손바닥의 중심점 반환
Point getHandCenter(const Mat& mask, double& maxVal) {
	Mat mask_img; //거리 변환 행렬을 저장할 변수
	distanceTransform(mask, mask_img, CV_DIST_L2, 5);		//중심으로 부터 점점 옅어지는 영상(=중심의 픽셀값 가장 높음) 출력
	minMaxIdx(mask_img, NULL, &maxVal, NULL, maxIdx, mask); //최소값은 사용하지 않음, maxIdx =max location(행, 열)

	return Point(maxIdx[1], maxIdx[0]);	//최대값 좌표의 행, 열 값 반환
}

//마우스 콜백 함수, 마우스 클릭시 좌표의 픽셀값 가져옴
void mouse_callback(int event, int x, int y, int flags, void *param) {
	if (event == CV_EVENT_LBUTTONDOWN) {	//마우스 왼쪽 버튼
		Vec3b color_pixel = img_color.at<Vec3b>(y, x);		//클릭한 위치 픽셀 값 읽어옴
		Mat bgr_color = Mat(1, 1, CV_8UC3, color_pixel);	//cvtColor함수에 적용 위해 한 픽셀로 구성된 이미지로 바꿈
		Mat hsv_color;
		cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);		//BGR->HSV 색공간 변환

		int hue = hsv_color.at<Vec3b>(0, 0)[0];			//hue값 정수 변수에 대입 
		int saturation = hsv_color.at<Vec3b>(0, 0)[1];	//saturation값 정수 변수에 대입 
		int value = hsv_color.at<Vec3b>(0, 0)[2];		//value값 정수 변수에 대입 

														//마우스 클릭 위치 픽셀에서 얻은 hue값과 유사한 색만 추출하기 위해 범위 정의.
		if (hue < 10) {
			lower_blue1 = Vec3b(hue - 10 + 180, threshold1, threshold1);
			upper_blue1 = Vec3b(hue, 255, 255);
			lower_blue2 = Vec3b(hue, threshold1, threshold1);
			upper_blue2 = Vec3b(hue + 10, 255, 255);
		}
		else if (hue > 170) {
			lower_blue1 = Vec3b(hue - 10, threshold1, threshold1);
			upper_blue1 = Vec3b(hue, 255, 255);
			lower_blue2 = Vec3b(hue, threshold1, threshold1);
			upper_blue2 = Vec3b(hue + 10 - 180, 255, 255);
		}
		else {
			lower_blue1 = Vec3b(hue - 10, threshold1, threshold1);
			upper_blue1 = Vec3b(hue, 255, 255);
			lower_blue2 = Vec3b(hue, threshold1, threshold1);
			upper_blue2 = Vec3b(hue + 10, 255, 255);
		}
	}
}

//물체를 인식한 mask영상과 배경을 합성
void imageProcessing(Mat image) {
	threshold1 = getTrackbarPos("threshold", "img_color");	//트랙바 값 적용

	Mat img_hsv;
	cvtColor(img_color, img_hsv, COLOR_BGR2HSV);	// 이미지 색공간 변환 

													//inRange()는 이미지에서 범위내에 있는 픽셀은 흰색으로 그 외의 이미지는 검은색으로 만든다
	Mat  img_mask1, img_mask2, img_mask;
	inRange(img_hsv, lower_blue1, upper_blue1, img_mask1);	// 마우스 클릭시 만들었던 범위를 이용해 hsv 이미지에서 mask 이미지 생성
	inRange(img_hsv, lower_blue2, upper_blue2, img_mask2);
	img_mask = img_mask1 | img_mask2;	//img_mask => 손 영역 흰색 마스크 설정

										//img_mask에 대한 침식, 팽창 연산
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5), Point(2, 2));	//morphologyEx에 전달 될 수있는 구조 요소를 생성하고 반환, MORPH_RECT=직사각형구조
	morphologyEx(img_mask, img_mask, MORPH_OPEN, element);	//바이너리 이미지에서 흰색 영역으로 검색되는 노이즈 제거
	morphologyEx(img_mask, img_mask, MORPH_CLOSE, element);	//흰색 영역 내부에 생기는 검정 영역 메움

															//img_mask에서의 마스크 영역(img_result)
	Mat img_result;
	bitwise_and(img_color, img_color, img_result, img_mask);

	//배경합성
	double alpha = 1, beta = 1;
	add(image, img_result, dst);

	//영상 좌우 반전
	flip(img_color, img_color, 1);
	flip(img_mask, img_mask, 1);
	flip(dst, dst, 1);

	//손바닥 중심점 찾고 중심 그리기, center = Point 자료형
	double maxVal;
	Point center = getHandCenter(img_mask, maxVal);
	circle(dst, center, 4, Scalar(0, 0, 255), -1);
}

//"O, X" 관심 영역 지정 및 정해진 이미지 합성
void imageROI(int range, int range2, int range3, int range4) {
	Mat T_i_th, F_i_th, masks1[3], masks2[3], foreground1, foreground2, background1, background2, th_dst1, th_dst2;

	threshold(in.T_image, T_i_th, 70, 255, THRESH_BINARY); //지정 영역에 특정 값 지정 -> 70이상 = 255(흰색) 
	split(T_i_th, masks1); //채널 분리
	bitwise_not(masks1[0], masks1[1]);

	threshold(in.F_image, F_i_th, 70, 255, THRESH_BINARY);
	split(F_i_th, masks2);
	bitwise_not(masks2[2], masks2[0]);

	//관심영역 지정 및 이미지 합성
	Rect roi(range, range2, 100, 100);
	bitwise_and(in.T_image, in.T_image, foreground1, masks1[0]);
	bitwise_and(dst(roi), dst(roi), background1, masks1[1]);
	add(background1, foreground1, th_dst1);
	th_dst1.copyTo(dst(roi));

	Rect roi2(range3, range4, 100, 100);
	bitwise_and(in.F_image, in.F_image, foreground2, masks2[2]);
	bitwise_and(dst(roi2), dst(roi2), background2, masks2[0]);
	add(background2, foreground2, th_dst2);
	th_dst2.copyTo(dst(roi2));
}

//"그림" 관심 영역 지정 및 정해진 이미지 합성
void imageROI2(int range, int range2, int range3, int range4, Mat image1, Mat image2) {
	Mat T_i_th, F_i_th, masks1[5], masks2[5], foreground1, foreground2, background1, background2, th_dst1, th_dst2;

	threshold(image1, T_i_th, 70, 255, THRESH_BINARY);
	split(T_i_th, masks1);
	bitwise_or(masks1[0], masks1[1], masks1[3]);
	bitwise_or(masks1[2], masks1[3], masks1[3]);
	bitwise_not(masks1[3], masks1[4]);

	threshold(image2, F_i_th, 70, 255, THRESH_BINARY);
	split(F_i_th, masks2);
	bitwise_or(masks2[0], masks2[1], masks2[3]);
	bitwise_or(masks2[2], masks2[3], masks2[3]);
	bitwise_not(masks2[3], masks2[4]);

	//관심영역 지정 및 이미지 합성
	Rect roi(range, range2, 100, 100);
	bitwise_and(image1, image1, foreground1, masks1[3]);
	bitwise_and(dst(roi), dst(roi), background1, masks1[4]);
	add(background1, foreground1, th_dst1);
	th_dst1.copyTo(dst(roi));

	Rect roi2(range3, range4, 100, 100);
	bitwise_and(image2, image2, foreground2, masks2[3]);
	bitwise_and(dst(roi2), dst(roi2), background2, masks2[4]);
	add(background2, foreground2, th_dst2);
	th_dst2.copyTo(dst(roi2));
}

int main() {
	//퀴즈 문제 배열
	String quizArr[7];
	quizArr[0] = "Is apple a fruit?";					//답 : O
	quizArr[1] = "The Sun is in the sky";				//답 : 0
	quizArr[2] = "A polar bear take a winter sleep";	//답 : X
	quizArr[3] = "Rome is a city in the USA.";			//답 : X
	quizArr[4] = " Birds have teeth.";					//답 : X
	quizArr[5] = "Is tomato a fruit?";					//답 : X
	quizArr[6] = "The iPhone was made by Apple.";		//답 : 0

	String quizArr2[5];
	quizArr2[0] = "Which is an apple?";
	quizArr2[1] = "Which is an giraffe?";
	quizArr2[2] = "Which is an grape?";
	quizArr2[3] = "Which is an rabbit?";
	quizArr2[4] = "Which is an watermelon?";

	//관심영역 좌표 배열(x1, y1), (x2, y2)
	int x1[10] = { 100, 200, 100, 200, 100, 200, 100, 100, 200, 100 };
	int y1[10] = { 100, 300, 370, 100, 250, 100, 300, 100, 300, 370 };
	int x2[10] = { 400, 500, 400, 500, 400, 500, 400, 400, 500, 400 };
	int y2[10] = { 100, 150, 200, 300, 100, 300, 100, 100, 150, 200 };

	namedWindow("img_color");	//윈도우 생성
	setMouseCallback("img_color", mouse_callback);	//마우스 이벤트 콜백함수 윈도우에 등록
	createTrackbar("threshold", "img_color", &threshold1, 255, trackbar);	//trackbar생성
	setTrackbarPos("threshold", "img_color", 81);	//트랙바 초기값 설정


	VideoCapture cap(0);	// 카메라 연결 위한 VideoCapture 객체 생성
	if (!cap.isOpened()) {
		cout << "카메라를 열 수 없습니다." << endl;
		return -1;
	}

	//게임 시작
	int gameSelect = 0;
	while (1) {
		cap.read(img_color);		//VideoCapture 객체에 read함수 사용해 이미지 가져옴 
		imageProcessing(in.image1);	//imageProcessing() 호출 (parameter는 이미지 변수)
		Rect roi(83, 293, 132, 88);	//roi / roi2의 관심영역 지정
		Rect roi2(300, 292, 133, 91);

		if (maxIdx[1] >= 83 && maxIdx[1] <= 215 && maxIdx[0] >= 293 && maxIdx[0] <= 381) {
			gameSelect = 1;
			break;	//관심영역에 물체의 중심점 들어오면 break
		}
		if (maxIdx[1] >= 300 && maxIdx[1] <= 433 && maxIdx[0] >= 292 && maxIdx[0] <= 383) {
			gameSelect = 2;
			break;
		}

		imshow("img_color", img_color);
		imshow("dst", dst);

		if (waitKey(10) == 27) break;
	}

	//카운트다운
	int time = 0;
	for (int i = 0; i < 3; i++) {
		while (1) {
			cap.read(img_color);
			imageProcessing(in.countdown[i]);	//imageProcessing() 호출 (parameter는 이미지 배열)
			time++;
			if (time == 30) break;	//초가 지나면 다음 이미지 출력

			imshow("img_color", img_color);
			imshow("dst", dst);

			if (waitKey(10) == 27) break;
		}
		time = 0;
	}

	//O, X 퀴즈 시작
	int score = 0, num = 0, delay = 0;	//점수, 사용자 선택 정답 판별, 딜레이값
	if (gameSelect == 1) {
		for (int i = 0; i < 7; i++) {
			while (1) {
				cap.read(img_color);
				imageProcessing(in.image2);
				imageROI(x1[i], y1[i], x2[i], y2[i]);	//imageROI() 호출 (parameter는 좌표값)

				putText(dst, quizArr[i], Point(60, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);			//영상 화면에 문제 출력
				putText(dst, to_string(score), Point(550, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);	//영상 화면에 점수 출력

				if (maxIdx[1] >= x1[i] && maxIdx[1] <= x1[i] + 100 && maxIdx[0] >= y1[i] && maxIdx[0] <= y1[i] + 100) {	//O 영역에 물체의 중심점이 들어왔을 때
					num = 1;
					delay++;
					if (delay >= 40) break;
				}
				if (maxIdx[1] >= x2[i] && maxIdx[1] <= x2[i] + 100 && maxIdx[0] >= y2[i] && maxIdx[0] <= y2[i] + 100) {	//X 영역에 물체의 중심점이 들어왔을 때
					num = 2;
					delay++;
					if (delay >= 40) break;
				}

				imshow("img_color", img_color);
				imshow("dst", dst);

				if (waitKey(10) == 27) break;
			}

			switch (i) {	//점수 계산
			case 0:
			case 1:
			case 6:	//0, 1, 6번 문제에서 사용자가 O를 정답으로 선택했을 때
				if (num == 1) score += 10;	//10점 획득
				else score -= 10;			//X를 선택했을 때 10점 감점
				break;
			case 2:
			case 3:
			case 4:
			case 5:	//2, 3, 4, 5번 문제에서 사용자가 X를 정답으로 선택했을 때
				if (num == 2) score += 10;	//10점 획득
				else score -= 10;			//O를 선택했을 때 10점 감점
				break;
			}
			num = 0;	//정답 선택 값 초기화
			delay = 0;	//delay 값 초기화
		}
	}

	//그림찾기 퀴즈 시작
	int j = 0;
	if (gameSelect == 2) {
		for (int i = 0; i < 9; i += 2) {
			while (1) {
				cap.read(img_color);
				imageProcessing(in.image2);
				imageROI2(x1[i], y1[i], x2[i], y2[i], in.selectgame[i], in.selectgame[i + 1]);	//imageROI() 호출 (parameter는 좌표값)
				putText(dst, quizArr2[j], Point(60, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);			//영상 화면에 문제 출력
				putText(dst, to_string(score), Point(550, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);	//영상 화면에 점수 출력

				if (maxIdx[1] >= x1[i] && maxIdx[1] <= x1[i] + 100 && maxIdx[0] >= y1[i] && maxIdx[0] <= y1[i] + 100) {	//O 영역에 물체의 중심점이 들어왔을 때
					num = 1;
					delay++;
					if (delay >= 40) break;
				}
				if (maxIdx[1] >= x2[i] && maxIdx[1] <= x2[i] + 100 && maxIdx[0] >= y2[i] && maxIdx[0] <= y2[i] + 100) {	//X 영역에 물체의 중심점이 들어왔을 때
					num = 2;
					delay++;
					if (delay >= 40) break;
				}

				imshow("img_color", img_color);
				imshow("dst", dst);

				if (waitKey(10) == 27) break;
			}

			switch (i) {	//점수 계산
			case 0:
			case 4:
			case 6:
				if (num == 1) score += 10;
				else score -= 10;
				break;
			case 2:
			case 8:
				if (num == 2) score += 10;
				else score -= 10;
				break;
			}
			num = 0;	//정답 선택 값 초기화
			delay = 0;	//delay 값 초기화
			j++;
		}
	}

	//게임 종료
	while (1) {
		cap.read(img_color);
		int x1 = 100, y1 = 200, x2 = 560, y2 = 300;	//게임을 종료 관심 영역 지정
		imageProcessing(in.image3);
		Rect roi(490, 420, 120, 35);	//게임 종료 관심 영역 지정

		if (maxIdx[1] >= 490 && maxIdx[1] <= 610 && maxIdx[0] >= 420 && maxIdx[0] <= 455) break;

		putText(dst, to_string(score), Point(300, 250), 0, 2.0, Scalar(0, 0, 0), 3);	//최종 점수 출력

		imshow("img_color", img_color);
		imshow("dst", dst);

		if (waitKey(10) == 27) break;
	}
}