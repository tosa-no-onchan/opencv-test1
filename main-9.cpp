/*
* main9.cpp
* /map を pgm ファイルに落とした画像を取り込んで
* 未知領域に接する、自由領域のセルのみで、ブロブを作ります。
* 上記ブロブの中で一番大きな領域を選んで、そのブロブの重心を算出します。
* -1 に 4近傍接する白をピックアップする。
*
* 下記がベースです。Special Thanks!!
* 「OpenCVによる画像処理入門」(講談社)
* P167 ラベル C言語版
* P161 重心 C++ 版
* https://imura-lab.org/products/labeling/
*/
#include <opencv2/opencv.hpp>
#include "Labeling.h"

//#define USE_MAP_SAVER 
// map_server/map_saver の画像を使う時は、こちらを使って下さい。
#ifdef USE_MAP_SAVER
    #define FREE_AREA 254
    #define UNKNOWN_AREA 255

// オンちゃん独自の  保管画像を使う時は、こちら
#else
    #define FREE_AREA 0xff
    #define UNKNOWN_AREA 0x80
#endif


int compare_int(const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}

#define COLOR_1 40


int main() {
    cv::Mat rgb, gry, thres,reverse,neg,dst2;

    //cv::namedWindow("gry", cv::WINDOW_NORMAL);
    //cv::namedWindow("thres", cv::WINDOW_NORMAL);
    //cv::namedWindow("reverse", cv::WINDOW_NORMAL);


    //cv::namedWindow("img", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("img", cv::WINDOW_NORMAL);
    //cv::namedWindow("thres", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("thres", cv::WINDOW_NORMAL);
    cv::namedWindow("unknown", cv::WINDOW_NORMAL);
    cv::namedWindow("block", cv::WINDOW_NORMAL);

    cv::namedWindow("blob", cv::WINDOW_NORMAL);


    //rgb = cv::imread("aaa.png");
    //cv::cvtColor(rgb, gry, cv::COLOR_BGR2GRAY);
    //gry = cv::imread("/home/nishi/map_builder.pgm",0);
    gry = cv::imread("../map_builder-house.pgm",0);
    //gry = cv::imread("../map_builder-gass.pgm",0);
    //gry = cv::imread("../Grid_save.pgm",0);


    //cv::imshow("gry", gry);


    //cv::waitKey(0);
    //return 0;


    int thresh=100;
    //int thresh=130;
    //int thresh=250;

    //cv::threshold(gry,thres,thresh,255,cv::THRESH_BINARY);

    //cv::imshow("thres", thres);


    // 白黒反転
    //thres.convertTo(reverse,thres.type(),-1.0,255.0);
    //cv::imshow("reverse", reverse);

    cv::Mat img=gry;
    
    //cv::Mat img = (cv::Mat_<u_int8_t>(6,6) << 
    //            0, 128, 0, 128, 128, 0,
    //            0, 255, 0, 255, 255, 0,
    //            128, 255, 255, 255, 0, 0,
    //            128, 255, 0, 0, 0, 0, 
    //            0,255, 0, 255, 255, 0,
    //            0, 0, 0, 128, 128, 0
    //            );  


    cv::imshow("img", img);

    std::cout << "img.rows=" << img.rows << std::endl;
    std::cout << "img.cols=" << img.cols << std::endl;
    std::cout << "img.type()=" << img.type() << std::endl;
    std::cout << "img.step=" << img.step << std::endl;


    //cv::waitKey(0);
    //return 0;


    //cv::Mat dst = cv::Mat::zeros(img.rows,img.cols,CV_16U);
    cv::Mat unknown = cv::Mat::zeros(img.rows,img.cols,CV_8U);

    std::cout << "unknown.rows=" << unknown.rows << std::endl;
    std::cout << "unknown.cols=" << unknown.cols << std::endl;
    std::cout << "unknown.channels()=" << unknown.channels() << std::endl;
    std::cout << "unknown.type()=" << unknown.type() << std::endl;
    std::cout << "unknown.step=" << unknown.step << std::endl;
    

    //int thresh=100;
    //int thresh=130;
    //int thresh=250;

    //cv::threshold(img,thres,thresh,255,cv::THRESH_BINARY);

    //cv::imshow("thres", thres);

    //std::cout << (int)thres.at<u_int8_t>(0,0) << "," << (int)thres.at<u_int8_t>(0,1) << "," << (int)thres.at<u_int8_t>(0,2) << std::endl;
    //std::cout << (int)thres.at<u_int8_t>(1,0) << "," << (int)thres.at<u_int8_t>(1,1) << "," << (int)thres.at<u_int8_t>(1,2) << std::endl;
    //std::cout << (int)thres.at<u_int8_t>(2,0) << "," << (int)thres.at<u_int8_t>(2,1) << "," << (int)thres.at<u_int8_t>(2,2) << std::endl;


    //int nlabel =0;  // ラベルの数
    int w = img.cols;
    int h = img.rows;

    //const int TABLESIZE = 1024;
    //static int table[TABLESIZE];        // label 使用番号 管理テーブル
    //table[0]=0;

    int unknown_t[img.rows*img.cols];
    for(int y=0;y<img.rows;y++){
        for(int x=0;x < img.cols;x++){
            unknown_t[y*img.cols + x] = 0;
        }
    }
 

    //----------------
    // 1. 4近傍に 未知領域(-1) のある、自由領域(白) をピックアップして、unknown_t[]へ入れる
    //----------------
    for(int y = 0 ; y < h; y++){
        for (int x=0;x < w; x++){
            //std::cout << "y=" << y << ",x=" << x << std::endl;
            // 注目画素は、自由領域 でない(黒)
            if(img.data[y*w+x] != FREE_AREA){
               unknown_t[y * w + x] = 0;
            }
            // 注目画素は、自由領域(白 : 0xff) だ!!
            else{
                // 近傍8画素をチェック
                //const int N = 8;
                //const int dy[N] = { 1, 1,  1,  0, 0, -1, -1, -1};
                //const int dx[N] = {-1, 0,  1, -1, 1, -1 , 0 , 1};

                // 近傍4画素をチェック 上、下、左、右
                const int N = 4;
                const int dy[N] = { 1,  0, 0, -1};
                const int dx[N] = { 0, -1, 1,  0};

                int list[N];
                int count =0;
                for(int k = 0; k < N; k++){
                    int xdx = x + dx[k];
                    int ydy = y + dy[k];
                    int m = ydy * w + xdx;
                    // はみ出していない and  Unknown -1(0x80) セル
                    if(xdx >= 0 && ydy >= 0 && xdx < w && ydy < h && img.data[m] == UNKNOWN_AREA){
                        unknown_t[y*w+x] = 0xff;
                        unknown.data[y*w+x] = 0xff;
                        break;
                    }
                }
            }
        }
    }

    cv::imshow("unknown", unknown);

    //cv::waitKey(0);
    //return 0;

    //--------------------
    // 2. ここから、上記、unknown_t を、ロボットの回転円の直径のブロックに分割します。
    // 未知領域(-1=0x80)に隣接する自由領域セル(0=0xff) が1個でもあれば、そのブロックをマーキングします。
    //--------------------

    int   line_w_ = 5;  // ラインの幅 -> grid size [dot]  0.05[m] * 5 = 25[cm]

    int8_t d;
    int x_cur;
    int y_cur;
    int d_cur;

    u_int32_t height_ = img.rows;
    u_int32_t width_ = img.cols;

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
                        // unknown セルに接している?
                        if(unknown_t[y_cur * width_ + x_cur] == 0xff){
                            d=0xff;
                        }
                    }
                }
            }
            if(d!=0){
                block.data[y*size_x_ + x] = 0xff;
            }
        }
    }

    cv::imshow("block", block);

    //cv::waitKey(0);
    //return 0;

    //----------------------
    // 3. 上でマークされたブロックをブロブ分割(ラベリング)します。
    // 今回は、Labeling.h を使います。
    // https://imura-lab.org/products/labeling/
    //-----------------------

    LabelingBS	labeling;

    int nlabel =0;  // 使用済ラベルの数(次割当ラベル番号)
    //int w = img.cols;
    w = size_x_;
    //int h = img.rows;
    h = size_y_;

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

    cv::waitKey(0);
    return 0;


}


