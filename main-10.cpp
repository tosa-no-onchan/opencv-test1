/*
*  main-10.cpp
*  Gazebo Turtlebot3 House の /map の保存画像の部屋の中心から、x軸方向に、障害物を探して、
*  障害物の重心を求める
*
* https://cvtech.cc/mask/
* http://opencv.jp/opencv-2svn/cpp/drawing_functions.html
*/
#include <opencv2/opencv.hpp>

int main() {
    cv::Mat rgb, gry, cny,bin_img;
    cv::namedWindow("Gray", cv::WINDOW_NORMAL);
    cv::namedWindow("BinReverse", cv::WINDOW_NORMAL);

    cv::namedWindow("result", cv::WINDOW_NORMAL);
    cv::namedWindow("result2", cv::WINDOW_NORMAL);

    //1. House /map 画像の取り込み
    //rgb = cv::imread("aaa.png");
    //cv::cvtColor(rgb, gry, cv::COLOR_BGR2GRAY);
    //gry = cv::imread("/home/nishi/map_builder.pgm",0);
    gry = cv::imread("../map_builder-house.pgm",0);
    cv::imshow("Gray", gry);
    //cv::Canny(gry, cny, 10, 100);
    //cv::Canny(gry, cny, 10, 100,3,true);

    std::cout << "gry.cols=" << gry.cols << " , gry.rows=" << gry.rows << std::endl;         // 421 x 421


    //2. 障害物を、2値画像 and 反転します。
	//int thresh = 120;
	//int thresh = 2;
	int thresh = 10;        // 障害物の値 0-100 / 未知領域:128  / 自由領域:255
	//threshold(gry, img_dst, thresh, 255, cv::THRESH_BINARY);
    // 障害物 < 10 だけを、白にします。
	cv::threshold(gry, bin_img, thresh, 255, cv::THRESH_BINARY_INV);


    //3.  障害物の2値画像 の 中心から、x 方向にずらしながら、障害物を探す処理をします。
    int x,y;

    // 部屋の中央あたりで、障害物が無い場所を、開始点にします。
    x=210-50;
    y=210-30;

    cv::Point center_p; // 円の中心位置
    int r =7;      // 円の半径  30 -> 1.5[M]      7 ->  0.35[M]

    cv::Mat result,result2,mask;

    bool blob_ok=false;

    while(y < gry.rows && x < gry.cols){

        center_p.x = x;
        center_p.y = y;

        // Mask画像 を作成
        mask = cv::Mat::zeros(gry.rows, gry.cols, CV_8UC1);

        //void circle(Mat& img, Point center, int radius, const Scalar& color, int thickness=1, int lineType=8, int shift=0)
        // 円のマスク  白がマスク
        //cv::circle(mask, cv::Point(300, 250), 100, cv::Scalar(255), 30, 4);
        //cv::circle(mask, cv::Point(220, 220), 30, cv::Scalar(255),-1);
        cv::circle(mask, center_p, r, cv::Scalar(255),-1);

        // 障害物 2値化画像に 円のマスクを実施
        bin_img.copyTo(result2,mask);

        // 白色領域の面積(ピクセル数)を計算する
	    int white_cnt = cv::countNonZero(result2);
        std::cout << "white_cnt=" << white_cnt << std::endl;
        if(white_cnt > 3){
        //if(white_cnt > 0){
            blob_ok=true;
            break;
        }
        x+=2;   // 2*5 = 10[cm] ずつ移動
    }

    if(blob_ok==true){
        // 4. 見つかった障害物の重心を求めます。
        cv::Moments m = cv::moments(result2,true);
        // 重心
        double x_g = m.m10 / m.m00;
        double y_g = m.m01 / m.m00;

        std::cout << "x_g=" << x_g <<" y_g="<< y_g << std::endl;

        // 入力画像に円のマスクを実施
        gry.copyTo(result,mask);

    }
    else{
        std::cout << "not found blog!!" << std::endl;
    }


    //imwrite("test.pgm", cny);  

    cv::imshow("BinReverse", bin_img);

    cv::imshow("result", result);
    cv::imshow("result2", result2);

    cv::waitKey(0);
    return 0;
}


