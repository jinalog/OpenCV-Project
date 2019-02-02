OpenCV project
===============

Dynamic image tracking using the OpenCV Library on c++ basis
-------------------------------------------------------------

#### * Get **real-time video** input.

#### * Detect objects **using HSV values** of specified color in the image.

```C++
void mouse_callback(int event, int x, int y, int flags, void *param) {
if (event == CV_EVENT_LBUTTONDOWN) {              //마우스 왼쪽 버튼
Vec3b color_pixel = img_color.at<Vec3b>(y, x);    //클릭한 위치 픽셀 값 읽어옴
Mat bgr_color = Mat(1, 1, CV_8UC3, color_pixel);  //cvtColor함수에 적용 위해 한 픽셀로 구성된 이미지로 바꿈
Mat hsv_color;
cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);    //BGR->HSV 색공간 변환

int hue = hsv_color.at<Vec3b>(0, 0)[0];           //hue값 정수 변수에 대입 
int saturation = hsv_color.at<Vec3b>(0, 0)[1];    //saturation값 정수 변수에 대입 
int value = hsv_color.at<Vec3b>(0, 0)[2];         //value값 정수 변수에 대입 
                      '''omission'''
//마우스 클릭 위치 픽셀에서 얻은 hue값과 유사한 색만 추출하기 위해 범위 정의.
if (hue < 10) {
lower_blue1 = Vec3b(hue - 10 + 180, threshold1, threshold1);
upper_blue1 = Vec3b(hue, 255, 255);
lower_blue2 = Vec3b(hue, threshold1, threshold1);
upper_blue2 = Vec3b(hue + 10, 255, 255);
}
                      '''omission'''
```

#### * To detect objects more clearly, apply the distance conversion function **"distanceTransform()"** using the color values detected.

```C++
Point getHandCenter(const Mat& mask, double& maxVal) {
Mat mask_img;                                             //거리 변환 행렬을 저장 변수
distanceTransform(mask, mask_img, CV_DIST_L2, 5);         //중심으로 부터 점점 옅어지는 영상(=중심의 픽셀값 가장 높음) 출력
minMaxIdx(mask_img, NULL, &maxVal, NULL, maxIdx, mask);   //최소값 사용하지 않음, maxIdx =max location(행, 열)

return Point(maxIdx[1], maxIdx[0]);                       //최대값 좌표의 행, 열 값 반환
}
```

#### * When an object enters the "ROI" area, it runs a simple game and the object acts like a mouse.
