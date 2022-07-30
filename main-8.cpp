/*
* main8.cpp
* /map を pgm ファイルに落とした画像を取り込んで
* 未知領域に接する、自由領域のセルのみで、ブロブを作ります。
* 上記ブロブの中で一番大きな領域を選んで、そのブロブの重心を算出します。
* -1 に 4近傍接する白をピックアップする。
*
* 下記がベースです。Special Thanks!!
* 「OpenCVによる画像処理入門」(講談社)
* P167 ラベル C言語版
* P161 重心 C++ 版
* 
*/
#include <opencv2/opencv.hpp>


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
    cv::namedWindow("label", cv::WINDOW_NORMAL);
    cv::namedWindow("blob", cv::WINDOW_NORMAL);


    //rgb = cv::imread("aaa.png");
    //cv::cvtColor(rgb, gry, cv::COLOR_BGR2GRAY);
    gry = cv::imread("../map_builder.pgm",0);
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
    //-----------------------

    int nlabel =0;  // 使用済ラベルの数(次割当ラベル番号)
    //int w = img.cols;
    w = size_x_;
    //int h = img.rows;
    h = size_y_;

    cv::Mat mat_label = cv::Mat::zeros(h,w,CV_8U);
    cv::Mat mat_label2 = cv::Mat::zeros(h,w,CV_8U);

    const int TABLESIZE = 1024;
    static int l_no[TABLESIZE];        // pass1 label 使用番号 管理テーブル
    l_no[0]=0;

    int img_l[w*h];             // 読み込み 画像 の 有意画素に、ラベルを割り当てた画素テーブル
    for(int y=0;y<h;y++){
        for(int x=0;x < w;x++){
            img_l[y*w + x] = 0;
        }
    }
 

    // 1回目の走査
    // 結果は、 label_t[][] に入れる。
    for(int y = 0 ; y < h; y++){
        for (int x=0;x < w; x++){
            //std::cout << "y=" << y << ",x=" << x << std::endl;
            // カレント画素は、ターゲットでない(黒)
            if(block.data[y*w+x] == 0){
               img_l[y * w + x] = 0;
            }
            // カレント画素は、ターゲット(白)
            else{
                // 近傍4画素(上3画素と左1画素) をチェック
                const int N = 4;
                //const int dy[N] = {-1, 0, 1, -1};
                const int dy[N] = {-1, 0, -1, -1};
                //const int dx[N] = {-1, -1, -1, 0};
                const int dx[N] = {-1, -1, 0, 1};

                int n4_l[N];    // 近傍4画素の、使用中ラベル番号を保管
                int count =0;
                for(int k = 0; k < N; k++){
                    int xdx = x + dx[k];
                    int ydy = y + dy[k];
                    int m = ydy * w + xdx;
                    // はみ出していない and 近傍4画素 img_t[m] は、ラベル割りあて済
                    if(xdx >= 0 && ydy >= 0 && xdx < w && ydy < h && img_l[m] != 0){
                        //std::cout << "m=" << m << std::endl;
                        if(m > h*w){
                            std::cout << "error " << std::endl;
                        }
                        std::cout << "label[" << m << "]=" << img_l[m] << std::endl;

                        // 近傍4画素の使用中ラベル番号を記録
                        n4_l[count] = l_no[img_l[m]];
                        //n4_l[count] = img_t[m];
                        count++;
                    }
                }

                // 近傍4画素には、ラベル割りあて済が無い。
                if(count == 0){
                    // 4画素いずれもラベルがなかった場合
                    if(nlabel < TABLESIZE -1){
                        nlabel++;   // 割当ラベル番号の更新
                        img_l[y * w +x] = nlabel;
                        l_no[nlabel] = nlabel;
                    }
                }
                else{
                    // n4_l[] : 近傍4画素のラベル番号の ソート
                    // sort(list,list + count)
                    // http://www.cc.kyoto-su.ac.jp/~yamada/ap/qsort.html
                    qsort(n4_l, count, sizeof(int), compare_int);

                    // n4_l[]から重複を省いたものを n4_l2[]へ
                    int n4_l2[N];   // 近傍4画素の 使用中ラベル番号2 
                    int uniq = 1;
                    n4_l2[0] = n4_l[0]; // ラベルの最小値をセット

                    for(int k=1; k< count;k++){
                        if(n4_l[k] != n4_l2[uniq - 1]){
                            n4_l2[uniq] = n4_l[k];
                            uniq++;
                        }
                    }
                    // カレント画素のラベル値 に 近傍4画素 のうち最小ラベル値を セット
                    img_l[y * w +x] = n4_l2[0];
                    // ラベル値が2以上の場合に以下を実行
                    for(int k=1; k < uniq; k++){
                        l_no[n4_l2[k]] = n4_l2[0];  // 重複している、ラベル番号は、最小番号に変更。
                                                            // 但し、img_t[x] は、そのまま残る。
                                                            // 古いラベル番号は、l_number のエントリー番号が該当
                    }
                }
            }
        }
    }


    std::cout << "img_l[0]= " << (int)img_l[0] << std::endl;


    // 連結しているラベルを結合し、ラベル値の中抜けがないように変換表をつくる
    static u_char l_n2[TABLESIZE];
    int k2 = 0;

    #define MAXVALUE 255

    // pass1 割当済ラベル　管理テーブル の中で、
    // 下記状況は、ありえるの?
    // l_no[] = {0, 1, 2, 3, 1, 5, 4, 4 }
    // l_no[] = {0, 1, 2, 3, 1, 5, 1, 1 }
    for(int k=0; k <= nlabel; k++){
        if(l_no[k] == k){
            // ラベル値の対応が一致している場合:中抜けの無い新たな対応表を作る
            if(k2 <= MAXVALUE){
                l_n2[k] = k2;
                k2 ++;
            }
            else{
                // MAXVALUE より大きなラベル値は、捨てる
                l_n2[k] = 0;
            }
        }
        else{
            // pass1 ラベル値の対応が一致していない場合:対応するラベル値の最小値を探す
            int kk =k;
            do {
                kk = l_no[kk];
            } while(l_no[kk] != kk);
            l_no[k] = kk;
        }
    }

    std::vector<int> blob_cnt(10,0);

    // 2回目の走査
    for(int y = 0; y < h;y++){
        for(int x=0;x < w;x++){
            int cur = y * w +x;
            if(l_n2[l_no[img_l[cur]]] == 0){
                mat_label.data[cur] = 0xff;
            }
            else{
                // 表示は下記が良いでしょう
                mat_label.data[cur] = COLOR_1 + l_n2[l_no[img_l[cur]]] * 10;

                // l_n2[l_no[img_l[y * w +x]]] が、該当素子のラベル番号なので
                // ラベル番号 毎の、その件数を記録すると共に、
                // ラベル番号毎に、該当ブロックを Vector に保管が必要です。
                // または、mat_label2.data を使って処理するか。
                mat_label2.data[cur] = l_n2[l_no[img_l[cur]]];
                if(l_n2[l_no[img_l[cur]]] < blob_cnt.size()){
                    blob_cnt[l_n2[l_no[img_l[cur]]]]++;
                }
                else{
                    blob_cnt.push_back(1);
                }
            }
        }
    }
    cv::imshow("label", mat_label);

    //cv::waitKey(0);
    //return 0;


    //----------------------
    //4. ラベリングされたブロブで大きなブロブを
    // 1つ選んで、それの重心を求めて、その場所を、ロボットの移動場所とします。
    //----------------------

    cv::Mat mat_blob = cv::Mat::zeros(h,w,CV_8U);

    int max_blob_no=1;
    int max_cnt=0;

    // 一番大きいブロブ番後を得る
    for(int i=1;i<blob_cnt.size();i++){
        //std::cout << "blob_cnt[" << i << "]=" << blob_cnt[i] << std::endl;
        if(blob_cnt[i] > max_cnt){
            max_cnt = blob_cnt[i];
            max_blob_no=i;
        }
    }

    std::cout << "max_blob_no=" << max_blob_no << " , max_cnt=" << max_cnt << std::endl;

    //該当ブロブだけを抽出します
    for(int i =0;i < h*w;i++){
        if(mat_label2.data[i] == max_blob_no){
            mat_blob.data[i] = 0xff;    // 白
        }
    }

    // 重心を求めます。
    cv::Moments m = cv::moments(mat_blob, true);
    double x_g = m.m10 / m.m00;
    double y_g = m.m01 / m.m00;

	std::cout <<"w="<< w << ", h=" << h << std::endl;

    //ここが、ロボットの移動先
    // 実際使うには、 x_g,y_g を、標準座標に変換しないといけない。
	std::cout <<"x_g="<< x_g << " , y_g=" << y_g << std::endl;

    // 印を付ける  -> グレーの X 印
    mat_blob.data[((int)y_g-1)*w+(int)x_g-1] = 128;
    mat_blob.data[((int)y_g-1)*w+(int)x_g+1] = 128;
    mat_blob.data[(int)y_g*w+(int)x_g] = 128;
    mat_blob.data[((int)y_g+1)*w+(int)x_g-1] = 128;
    mat_blob.data[((int)y_g+1)*w+(int)x_g+1] = 128;
    
    cv::imshow("blob", mat_blob);



    cv::waitKey(0);
    return 0;
}


