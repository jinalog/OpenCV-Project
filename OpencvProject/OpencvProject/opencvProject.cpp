/*
* ���α׷���: edugame.cpp
* ����: Ư�� ��ü(�ÿ� ���󿡼��� ��)�� �ν��Ͽ� �� ��ü�� �߽ɰ��� ��ǥ�� �̿��Ͽ� �ۼ��� O/X ���� �Ǵ� �׸�ã�� ���α׷�
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

int maxIdx[2];			//getHandCenter()���� �� �Ÿ� ��ȯ ����� ���� ���� ū �ȼ��� ��ǥ�� ����(��, �� ��)
int threshold1 = 80;	//saturation, value ���� �ּҰ� ����
Vec3b lower_blue1, upper_blue1, lower_blue2, upper_blue2;	//hue ����
Mat img_color, dst;	//img_color=���� �̹���, dst=����� �̹���

void trackbar(int, void*) {}	//Ʈ���� �ݹ�

								//�չٴ��� �߽��� ��ȯ
Point getHandCenter(const Mat& mask, double& maxVal) {
	Mat mask_img; //�Ÿ� ��ȯ ����� ������ ����
	distanceTransform(mask, mask_img, CV_DIST_L2, 5);		//�߽����� ���� ���� �������� ����(=�߽��� �ȼ��� ���� ����) ���
	minMaxIdx(mask_img, NULL, &maxVal, NULL, maxIdx, mask); //�ּҰ��� ������� ����, maxIdx =max location(��, ��)

	return Point(maxIdx[1], maxIdx[0]);	//�ִ밪 ��ǥ�� ��, �� �� ��ȯ
}

//���콺 �ݹ� �Լ�, ���콺 Ŭ���� ��ǥ�� �ȼ��� ������
void mouse_callback(int event, int x, int y, int flags, void *param) {
	if (event == CV_EVENT_LBUTTONDOWN) {	//���콺 ���� ��ư
		Vec3b color_pixel = img_color.at<Vec3b>(y, x);		//Ŭ���� ��ġ �ȼ� �� �о��
		Mat bgr_color = Mat(1, 1, CV_8UC3, color_pixel);	//cvtColor�Լ��� ���� ���� �� �ȼ��� ������ �̹����� �ٲ�
		Mat hsv_color;
		cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);		//BGR->HSV ������ ��ȯ

		int hue = hsv_color.at<Vec3b>(0, 0)[0];			//hue�� ���� ������ ���� 
		int saturation = hsv_color.at<Vec3b>(0, 0)[1];	//saturation�� ���� ������ ���� 
		int value = hsv_color.at<Vec3b>(0, 0)[2];		//value�� ���� ������ ���� 

														//���콺 Ŭ�� ��ġ �ȼ����� ���� hue���� ������ ���� �����ϱ� ���� ���� ����.
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

//��ü�� �ν��� mask����� ����� �ռ�
void imageProcessing(Mat image) {
	threshold1 = getTrackbarPos("threshold", "img_color");	//Ʈ���� �� ����

	Mat img_hsv;
	cvtColor(img_color, img_hsv, COLOR_BGR2HSV);	// �̹��� ������ ��ȯ 

													//inRange()�� �̹������� �������� �ִ� �ȼ��� ������� �� ���� �̹����� ���������� �����
	Mat  img_mask1, img_mask2, img_mask;
	inRange(img_hsv, lower_blue1, upper_blue1, img_mask1);	// ���콺 Ŭ���� ������� ������ �̿��� hsv �̹������� mask �̹��� ����
	inRange(img_hsv, lower_blue2, upper_blue2, img_mask2);
	img_mask = img_mask1 | img_mask2;	//img_mask => �� ���� ��� ����ũ ����

										//img_mask�� ���� ħ��, ��â ����
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5), Point(2, 2));	//morphologyEx�� ���� �� ���ִ� ���� ��Ҹ� �����ϰ� ��ȯ, MORPH_RECT=���簢������
	morphologyEx(img_mask, img_mask, MORPH_OPEN, element);	//���̳ʸ� �̹������� ��� �������� �˻��Ǵ� ������ ����
	morphologyEx(img_mask, img_mask, MORPH_CLOSE, element);	//��� ���� ���ο� ����� ���� ���� �޿�

															//img_mask������ ����ũ ����(img_result)
	Mat img_result;
	bitwise_and(img_color, img_color, img_result, img_mask);

	//����ռ�
	double alpha = 1, beta = 1;
	add(image, img_result, dst);

	//���� �¿� ����
	flip(img_color, img_color, 1);
	flip(img_mask, img_mask, 1);
	flip(dst, dst, 1);

	//�չٴ� �߽��� ã�� �߽� �׸���, center = Point �ڷ���
	double maxVal;
	Point center = getHandCenter(img_mask, maxVal);
	circle(dst, center, 4, Scalar(0, 0, 255), -1);
}

//"O, X" ���� ���� ���� �� ������ �̹��� �ռ�
void imageROI(int range, int range2, int range3, int range4) {
	Mat T_i_th, F_i_th, masks1[3], masks2[3], foreground1, foreground2, background1, background2, th_dst1, th_dst2;

	threshold(in.T_image, T_i_th, 70, 255, THRESH_BINARY); //���� ������ Ư�� �� ���� -> 70�̻� = 255(���) 
	split(T_i_th, masks1); //ä�� �и�
	bitwise_not(masks1[0], masks1[1]);

	threshold(in.F_image, F_i_th, 70, 255, THRESH_BINARY);
	split(F_i_th, masks2);
	bitwise_not(masks2[2], masks2[0]);

	//���ɿ��� ���� �� �̹��� �ռ�
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

//"�׸�" ���� ���� ���� �� ������ �̹��� �ռ�
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

	//���ɿ��� ���� �� �̹��� �ռ�
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
	//���� ���� �迭
	String quizArr[7];
	quizArr[0] = "Is apple a fruit?";					//�� : O
	quizArr[1] = "The Sun is in the sky";				//�� : 0
	quizArr[2] = "A polar bear take a winter sleep";	//�� : X
	quizArr[3] = "Rome is a city in the USA.";			//�� : X
	quizArr[4] = " Birds have teeth.";					//�� : X
	quizArr[5] = "Is tomato a fruit?";					//�� : X
	quizArr[6] = "The iPhone was made by Apple.";		//�� : 0

	String quizArr2[5];
	quizArr2[0] = "Which is an apple?";
	quizArr2[1] = "Which is an giraffe?";
	quizArr2[2] = "Which is an grape?";
	quizArr2[3] = "Which is an rabbit?";
	quizArr2[4] = "Which is an watermelon?";

	//���ɿ��� ��ǥ �迭(x1, y1), (x2, y2)
	int x1[10] = { 100, 200, 100, 200, 100, 200, 100, 100, 200, 100 };
	int y1[10] = { 100, 300, 370, 100, 250, 100, 300, 100, 300, 370 };
	int x2[10] = { 400, 500, 400, 500, 400, 500, 400, 400, 500, 400 };
	int y2[10] = { 100, 150, 200, 300, 100, 300, 100, 100, 150, 200 };

	namedWindow("img_color");	//������ ����
	setMouseCallback("img_color", mouse_callback);	//���콺 �̺�Ʈ �ݹ��Լ� �����쿡 ���
	createTrackbar("threshold", "img_color", &threshold1, 255, trackbar);	//trackbar����
	setTrackbarPos("threshold", "img_color", 81);	//Ʈ���� �ʱⰪ ����


	VideoCapture cap(0);	// ī�޶� ���� ���� VideoCapture ��ü ����
	if (!cap.isOpened()) {
		cout << "ī�޶� �� �� �����ϴ�." << endl;
		return -1;
	}

	//���� ����
	int gameSelect = 0;
	while (1) {
		cap.read(img_color);		//VideoCapture ��ü�� read�Լ� ����� �̹��� ������ 
		imageProcessing(in.image1);	//imageProcessing() ȣ�� (parameter�� �̹��� ����)
		Rect roi(83, 293, 132, 88);	//roi / roi2�� ���ɿ��� ����
		Rect roi2(300, 292, 133, 91);

		if (maxIdx[1] >= 83 && maxIdx[1] <= 215 && maxIdx[0] >= 293 && maxIdx[0] <= 381) {
			gameSelect = 1;
			break;	//���ɿ����� ��ü�� �߽��� ������ break
		}
		if (maxIdx[1] >= 300 && maxIdx[1] <= 433 && maxIdx[0] >= 292 && maxIdx[0] <= 383) {
			gameSelect = 2;
			break;
		}

		imshow("img_color", img_color);
		imshow("dst", dst);

		if (waitKey(10) == 27) break;
	}

	//ī��Ʈ�ٿ�
	int time = 0;
	for (int i = 0; i < 3; i++) {
		while (1) {
			cap.read(img_color);
			imageProcessing(in.countdown[i]);	//imageProcessing() ȣ�� (parameter�� �̹��� �迭)
			time++;
			if (time == 30) break;	//�ʰ� ������ ���� �̹��� ���

			imshow("img_color", img_color);
			imshow("dst", dst);

			if (waitKey(10) == 27) break;
		}
		time = 0;
	}

	//O, X ���� ����
	int score = 0, num = 0, delay = 0;	//����, ����� ���� ���� �Ǻ�, �����̰�
	if (gameSelect == 1) {
		for (int i = 0; i < 7; i++) {
			while (1) {
				cap.read(img_color);
				imageProcessing(in.image2);
				imageROI(x1[i], y1[i], x2[i], y2[i]);	//imageROI() ȣ�� (parameter�� ��ǥ��)

				putText(dst, quizArr[i], Point(60, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);			//���� ȭ�鿡 ���� ���
				putText(dst, to_string(score), Point(550, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);	//���� ȭ�鿡 ���� ���

				if (maxIdx[1] >= x1[i] && maxIdx[1] <= x1[i] + 100 && maxIdx[0] >= y1[i] && maxIdx[0] <= y1[i] + 100) {	//O ������ ��ü�� �߽����� ������ ��
					num = 1;
					delay++;
					if (delay >= 40) break;
				}
				if (maxIdx[1] >= x2[i] && maxIdx[1] <= x2[i] + 100 && maxIdx[0] >= y2[i] && maxIdx[0] <= y2[i] + 100) {	//X ������ ��ü�� �߽����� ������ ��
					num = 2;
					delay++;
					if (delay >= 40) break;
				}

				imshow("img_color", img_color);
				imshow("dst", dst);

				if (waitKey(10) == 27) break;
			}

			switch (i) {	//���� ���
			case 0:
			case 1:
			case 6:	//0, 1, 6�� �������� ����ڰ� O�� �������� �������� ��
				if (num == 1) score += 10;	//10�� ȹ��
				else score -= 10;			//X�� �������� �� 10�� ����
				break;
			case 2:
			case 3:
			case 4:
			case 5:	//2, 3, 4, 5�� �������� ����ڰ� X�� �������� �������� ��
				if (num == 2) score += 10;	//10�� ȹ��
				else score -= 10;			//O�� �������� �� 10�� ����
				break;
			}
			num = 0;	//���� ���� �� �ʱ�ȭ
			delay = 0;	//delay �� �ʱ�ȭ
		}
	}

	//�׸�ã�� ���� ����
	int j = 0;
	if (gameSelect == 2) {
		for (int i = 0; i < 9; i += 2) {
			while (1) {
				cap.read(img_color);
				imageProcessing(in.image2);
				imageROI2(x1[i], y1[i], x2[i], y2[i], in.selectgame[i], in.selectgame[i + 1]);	//imageROI() ȣ�� (parameter�� ��ǥ��)
				putText(dst, quizArr2[j], Point(60, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);			//���� ȭ�鿡 ���� ���
				putText(dst, to_string(score), Point(550, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);	//���� ȭ�鿡 ���� ���

				if (maxIdx[1] >= x1[i] && maxIdx[1] <= x1[i] + 100 && maxIdx[0] >= y1[i] && maxIdx[0] <= y1[i] + 100) {	//O ������ ��ü�� �߽����� ������ ��
					num = 1;
					delay++;
					if (delay >= 40) break;
				}
				if (maxIdx[1] >= x2[i] && maxIdx[1] <= x2[i] + 100 && maxIdx[0] >= y2[i] && maxIdx[0] <= y2[i] + 100) {	//X ������ ��ü�� �߽����� ������ ��
					num = 2;
					delay++;
					if (delay >= 40) break;
				}

				imshow("img_color", img_color);
				imshow("dst", dst);

				if (waitKey(10) == 27) break;
			}

			switch (i) {	//���� ���
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
			num = 0;	//���� ���� �� �ʱ�ȭ
			delay = 0;	//delay �� �ʱ�ȭ
			j++;
		}
	}

	//���� ����
	while (1) {
		cap.read(img_color);
		int x1 = 100, y1 = 200, x2 = 560, y2 = 300;	//������ ���� ���� ���� ����
		imageProcessing(in.image3);
		Rect roi(490, 420, 120, 35);	//���� ���� ���� ���� ����

		if (maxIdx[1] >= 490 && maxIdx[1] <= 610 && maxIdx[0] >= 420 && maxIdx[0] <= 455) break;

		putText(dst, to_string(score), Point(300, 250), 0, 2.0, Scalar(0, 0, 0), 3);	//���� ���� ���

		imshow("img_color", img_color);
		imshow("dst", dst);

		if (waitKey(10) == 27) break;
	}
}