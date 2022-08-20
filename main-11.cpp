/*
*  main-11.cpp
*  Gazebo Turtlebot3 House の /map の保存画像の障害物を 2値画像として抽出し、障害物のブロブに分けて、
*  障害物のブロブ の外周に、一定間隔でアンカーを設置します。
*
* https://cvtech.cc/mask/
* http://opencv.jp/opencv-2svn/cpp/drawing_functions.html
*
* opencv blob end-point to end-point
* 手書きの end-point to end-point
* https://stackoverflow.com/questions/67143809/finding-the-end-points-of-a-hand-drawn-line-with-opencv
*
* https://learnopencv.com/contour-detection-using-opencv-python-c/
*/
#include <opencv2/opencv.hpp>
#include "Labeling.h"

int main() {
    cv::Mat rgb, gry, cny,bin_img;
    cv::namedWindow("Gray", cv::WINDOW_NORMAL);
    cv::namedWindow("BinReverse", cv::WINDOW_NORMAL);

    //cv::namedWindow("result", cv::WINDOW_NORMAL);
    //cv::namedWindow("result2", cv::WINDOW_NORMAL);

    cv::namedWindow("block", cv::WINDOW_NORMAL);
    cv::namedWindow("blob", cv::WINDOW_NORMAL);

    cv::namedWindow("anchor", cv::WINDOW_NORMAL);

    //1. House /map 画像の取り込み
    //rgb = cv::imread("aaa.png");
    //cv::cvtColor(rgb, gry, cv::COLOR_BGR2GRAY);
    gry = cv::imread("/home/nishi/map_builder.pgm",0);
    //gry = cv::imread("../map_builder-house.pgm",0);
    //gry = cv::imread("../map_builder-gass.pgm",0);
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

    // 非障害物 >= 10 だけを、白にします。
	//cv::threshold(gry, bin_img, thresh, 255, cv::THRESH_BINARY);



    cv::imshow("BinReverse", bin_img);

    //cv::waitKey(0);
    //return 0;

    //--------------------
    // 3. ここから、上記、2値画像 を、ロボットの回転円の直径のブロックに分割します。
    // 障害物のセルが1個でもあれば、そのブロックを障害物とします。
    //--------------------
    
    int   line_w_ = 5;  // ラインの幅 -> grid size [dot]  0.05[m] * 5 = 25[cm]
    //int   line_w_ = 4;  // ラインの幅 -> grid size [dot]  0.05[m] * 4 = 20[cm]

    int8_t d;
    int x_cur;
    int y_cur;
    int d_cur;

    u_int32_t height_ = bin_img.rows;
    u_int32_t width_ = bin_img.cols;

    int size_x_ = width_ / line_w_;
    int size_y_ = height_ / line_w_;

    if(width_ % line_w_)
        size_x_++;
    if(height_ % line_w_)     
        size_y_++;

    cv::Mat block = cv::Mat::zeros(size_y_,size_x_,CV_8U);

    // all raws
    for(int y=0;y < size_y_; y++){
        // all columuns of a raw 
        for(int x=0;x < size_x_; x++){
            d=0;
            //di=0;
            //cnt_i=0;
            // set up 1 grid
            for(int yy = 0; yy < line_w_ && d==0; yy++){
                for(int xx=0; xx < line_w_ && d==0; xx++){
                    y_cur = y*line_w_ + yy;
                    x_cur = x*line_w_ + xx;
                    if(y_cur < height_&& x_cur < width_){
                        // 障害物(白セル)です?
                        if(bin_img.data[y_cur * width_ + x_cur] == 0xff){
                            d++;
                        }
                    }
                }
            }
            // 半分は、障害物(白セル)です。
            if(d >= line_w_*line_w_/13){
                block.data[y*size_x_ + x] = 0xff;
            }
        }
    }

    cv::imshow("block", block);

    //cv::waitKey(0);
    //return 0;

    //----------------------
    // 4. 上でマークされたブロックをブロブ分割(ラベリング)します。
    // 今回は、Labeling.h を使います。
    // https://imura-lab.org/products/labeling/
    //-----------------------

    LabelingBS	labeling;

    int nlabel =0;  // 使用済ラベルの数(次割当ラベル番号)
    //int w = img.cols;
    int w = size_x_;
    //int h = img.rows;
    int h = size_y_;

    //cv::Mat mat_label = cv::Mat::zeros(h,w,CV_8U);
    //cv::Mat mat_label2 = cv::Mat::zeros(h,w,CV_8U);

	cv::Mat mat_blob;

	cv::Mat img_lab(h,w, CV_16SC1);

    //short *result = new short[ w * h ];

    labeling.Exec( block.data, (short *)img_lab.data, w, h, true, 0 );

    int n = labeling.GetNumOfResultRegions();   // ラベル総数

    std::cout << "n=" << n << std::endl;

    //----------------------
    //4. ラベリングされたブロブで大きなブロブを
    // 1つ選んで、それの重心を求めて、その場所を、ロボットの移動場所とします。
    //----------------------

    RegionInfoBS	*ri;
    ri = labeling.GetResultRegionInfo( 0 );


	cv::compare(img_lab, 1, mat_blob, cv::CMP_EQ); // ラベル番号1 を抽出


    #define TEST_MARK_G 
    #ifdef TEST_MARK_G

    float x_g,  y_g;
    ri->GetCenter(x_g,y_g);   // 重心を得る

	std::cout <<"x_g="<< x_g << " , y_g=" << y_g << std::endl;

    // 印を付ける  -> グレーの X 印
    mat_blob.data[((int)y_g-1)*w+(int)x_g-1] = 128;
    mat_blob.data[((int)y_g-1)*w+(int)x_g+1] = 128;
    mat_blob.data[(int)y_g*w+(int)x_g] = 128;
    mat_blob.data[((int)y_g+1)*w+(int)x_g-1] = 128;
    mat_blob.data[((int)y_g+1)*w+(int)x_g+1] = 128;
    #endif

    cv::imshow("blob", mat_blob);

    //cv::waitKey(0);
    //return 0;


    //------------------------
    // 5. 重心を全て求めてみます。
    //-----------------------
    std::cout <<"display all g center start" << std::endl;

    for(int i=0;i<n;i++){
        ri = labeling.GetResultRegionInfo( i );
        ri->GetCenter(x_g,y_g);   // 重心を得る

        std::cout <<"x_g="<< x_g << " , y_g=" << y_g << std::endl;
    }

    //cv::waitKey(0);
    //return 0;


    //------------------------
    // 6. ブロブを1つ選んで、その外周上に、一定間隔でアンカーを置きます。
    //-----------------------

    cv::Mat mat_blob2;
    int lv_no=0;
    ri = labeling.GetResultRegionInfo( lv_no );

	cv::compare(img_lab, lv_no+1, mat_blob2, cv::CMP_EQ); // ラベル番号1 を抽出

    cv::Mat mat_anchor=mat_blob2.clone();

    // 『OpenCVによる画像処理入門』(講談社) P157 周囲長 を求める
    // 始点の探索
    int height=img_lab.rows;
    int width=img_lab.cols;
    int ini_x,ini_y;
    bool ok_f=false;
    for(int y=0; y<height && ok_f==false;y++){
        for(int x=0; x < width;x++){
            if(mat_blob2.data[y*width + x]==255){
                ini_y=y;
                ini_x=x;
                ok_f=true;
                break;
            }
        }
    }

    // 4近傍 下右上左
    int rot_x[4] = {0, 1, 0, -1};
    int rot_y[4] = {1, 0, -1, 0};
    int rot=0; // 探索方向
    int perrimeter = 0; // 周囲長
    int now_x,now_y;
    int pre_x=ini_x;
    int pre_y=ini_y;

    int anchor=1;

    std::cout <<"anchor="<< anchor << std::endl;

    // 印を付ける  -> グレーの X 印
    mat_anchor.data[((int)pre_y-1)*width+(int)pre_x-1] = 128;
    mat_anchor.data[((int)pre_y-1)*width+(int)pre_x+1] = 128;
    mat_anchor.data[(int)pre_y*width+(int)pre_x] = 128;
    mat_anchor.data[((int)pre_y+1)*width+(int)pre_x-1] = 128;
    mat_anchor.data[((int)pre_y+1)*width+(int)pre_x+1] = 128;


    while(1){
        for(int i=0; i<4;i++){
            now_x = pre_x + rot_x[(rot+i)%4];
            now_y = pre_y + rot_y[(rot+i)%4];
            if(now_x < 0 || now_x > width-1 || now_y < 0 || now_y > height-1 ){
                continue;
            }
            if(mat_blob2.data[now_y * width + now_x] == 255){
                pre_x = now_x;
                pre_y = now_y;
                perrimeter++;
                if((perrimeter%10)==0){
                    anchor++;
                    std::cout <<"anchor="<< anchor << std::endl;

                    // 印を付ける  -> グレーの X 印
                    mat_anchor.data[((int)pre_y-1)*width+(int)pre_x-1] = 128;
                    mat_anchor.data[((int)pre_y-1)*width+(int)pre_x+1] = 128;
                    mat_anchor.data[(int)pre_y*width+(int)pre_x] = 128;
                    mat_anchor.data[((int)pre_y+1)*width+(int)pre_x-1] = 128;
                    mat_anchor.data[((int)pre_y+1)*width+(int)pre_x+1] = 128;

                }
                rot += i+3;
                break;
            }
        }
        if(pre_x == ini_x && pre_y == ini_y){
            break;
        }
    }

    cv::imshow("anchor", mat_anchor);

    cv::waitKey(0);
    return 0;


}


