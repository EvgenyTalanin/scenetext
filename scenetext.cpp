#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "opencv2/contrib/contrib.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

int GroundTruth(Mat& _originalImage, bool showImage = false)
{
    double t = (double)getTickCount();

    Mat originalImage(_originalImage.rows + 2, _originalImage.cols + 2, _originalImage.type());
    copyMakeBorder(_originalImage, originalImage, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(255, 255, 255));

    Mat bwImage(originalImage.size(), CV_8UC1);

    uchar thresholdValue = 100;
    uchar maxValue = 255;
    uchar middleValue = 192;
    uchar zeroValue = 0;
    Scalar middleScalar(middleValue);
    Scalar zeroScalar(zeroValue);

    static int neigborsCount = 4;
    static int dx[] = {-1,  0, 0, 1};
    static int dy[] = { 0, -1, 1, 0};
    int di, rx, ry;
    int perimeter;

    cvtColor(originalImage, bwImage, CV_RGB2GRAY);
    threshold(bwImage, bwImage, thresholdValue, maxValue, THRESH_BINARY_INV);

    int regionsCount = 0;
    int totalPixelCount = bwImage.rows * bwImage.cols;
    Point seedPoint;
    Rect rectFilled;
    int valuesSum, q1, q2, q3;
    bool p00, p10, p01, p11;

    for(int i = 0; i < totalPixelCount; i++)
    {
        if (bwImage.data[i] == maxValue)
        {
            seedPoint.x = i % bwImage.cols;
            seedPoint.y = i / bwImage.cols;

            if ((seedPoint.x == 0) || (seedPoint.y == 0) || (seedPoint.x == bwImage.cols - 1) || (seedPoint.y == bwImage.rows - 1))
            {
                continue;
            }

            regionsCount++;

            size_t pixelsFilled = floodFill(bwImage, seedPoint, middleScalar, &rectFilled);
            printf("New region: %d\n", regionsCount);
            // We use -1 here since image was expanded by 1 pixel
            printf("Start point: (%d; %d)\n", seedPoint.x - 1, seedPoint.y - 1);
            printf("Area: %d\n", (int)pixelsFilled);
            printf("Bounding box (%d; %d) + (%d; %d)\n", rectFilled.x - 1, rectFilled.y - 1, rectFilled.width, rectFilled.height);

            perimeter = 0;
            q1 = 0; q2 = 0; q3 = 0;

            int crossings[rectFilled.height];
            for(int j = 0; j < rectFilled.height; j++)
            {
                crossings[j] = 0;
            }

            for(ry = rectFilled.y - 1; ry <= rectFilled.y + rectFilled.height; ry++)
            {
                for(rx = rectFilled.x - 1; rx <= rectFilled.x + rectFilled.width; rx++)
                {
                    if ((rx > rectFilled.x) && (rx < rectFilled.x + rectFilled.width))
                    {
                        if ((bwImage.at<uint8_t>(ry, rx - 1) != bwImage.at<uint8_t>(ry, rx)) && (bwImage.at<uint8_t>(ry, rx - 1) + bwImage.at<uint8_t>(ry, rx) == middleValue + zeroValue))
                        {
                            crossings[ry - rectFilled.y]++;
                        }
                    }

                    if (bwImage.at<uint8_t>(ry, rx) == middleValue)
                    {
                        for(di = 0; di < neigborsCount; di++)
                        {
                            int xNew = rx + dx[di];
                            int yNew = ry + dy[di];

                            if (bwImage.at<uint8_t>(yNew, xNew) == zeroValue)
                            {
                                perimeter++;
                            }
                        }
                    }

                    // TODO: fix Euler number
                    p00 = bwImage.at<uint8_t>(ry, rx) == middleValue;
                    p01 = bwImage.at<uint8_t>(ry, rx + 1) == middleValue;
                    p10 = bwImage.at<uint8_t>(ry + 1, rx) == middleValue;
                    p11 = bwImage.at<uint8_t>(ry + 1, rx + 1) == middleValue;
                    valuesSum = p00 + p01 + p10 + p11;

                    if (valuesSum == 1) q1++; else
                    if (valuesSum == 3) q2++; else
                    if ((valuesSum == 2) && (p00 == p11)) q3++;
                }
            }

            q1 = q1 - q2 + 2 * q3;
            if (q1 % 4 != 0)
            {
                printf("Non-integer Euler number");
                exit(0);
            }
            q1 /= 4;

            printf("Perimeter: %d\n", (int)perimeter);
            printf("Euler number: %d\n", q1);
            printf("Crossings: ");
            for(int j = 0; j < rectFilled.height; j++)
            {
                printf("%d ", crossings[j]);
            }

            printf("\n=====\n\n");

            floodFill(bwImage, seedPoint, zeroScalar);

            rectangle(originalImage, rectFilled, zeroScalar);

            //break;
        }
    }

    t = (double)getTickCount() - t;
    printf("Working time: %g ms\n", t * 1000. / getTickFrequency());

    if (showImage)
    {
        imshow("Truth", originalImage);
        waitKey();
    }
}












class Region
{
private:
    Point start;
    Rect bounds;
    int area;
    int perimeter;
    /*
    int min_threshold;
    int max_threshold;
    int q1, q2, q3, euler;
    */

public:
    Region()
    {

    }

    Region(Point _p)
    {
        start = _p;
        bounds.x = _p.x;
        bounds.y = _p.y;
        bounds.width = 1;
        bounds.height = 1;
        area = 1;
        perimeter = 4;
    }

    // _dbl is "double border length"
    void Attach(Region _extra, int _dbl)
    {
        if (start != _extra.start)
        {
            bounds |= _extra.bounds;
            area += _extra.area;
            perimeter += _extra.perimeter - _dbl;
        }
    }

    Rect Bounds()
    {
        return bounds;
    }

    Point Start()
    {
        return start;
    }

    int Area()
    {
        return area;
    }

    int Perimeter()
    {
        return perimeter;
    }

    int BoundsArea()
    {
        return bounds.width * bounds.height;
    }
};

struct PointComp
{
    bool operator() (const Point& _one, const Point& _other) const
    {
        if (_one.x < _other.x) return true;
        if (_one.x > _other.x) return false;
        if (_one.y < _other.y) return true;
        if (_one.y > _other.y) return false;
        return false;
    }
};

void uf_MakeSet(Point _x, map<Point, Point, PointComp>* _parents, map<Point, int, PointComp>* _ranks)
{
    _parents->insert(pair<Point, Point>(_x, _x));
    _parents->insert(pair<Point, int>(_x, 0));
}

Point uf_Find(Point _x, map<Point, Point, PointComp>* _parents)
{
    map<Point, Point>::iterator parents_iterator = _parents->find(_x);

    if ((parents_iterator != _parents->end()) && (parents_iterator->second != _x))
    {
        return uf_Find(parents_iterator->second, _parents);
    }
    return _x;
}

void MatasLike(Mat& originalImage, bool showImage = false)
{
    double t = (double)getTickCount();

    Mat bwImage(originalImage.size(), CV_8UC1);

    map<int, vector<Point> > pointLevels;
    Point pc;

    static int neigborsCount = 4;
    static int dx[] = {-1,  0, 0, 1};
    static int dy[] = { 0, -1, 1, 0};
    int di;

    int i, j, k;

    cvtColor(originalImage, bwImage, CV_RGB2GRAY);

    // No Mat, will use stl for tests
    map<Point, int, PointComp> ranks;
    map<Point, Point, PointComp> parents;
    map<Point, Region, PointComp> regions;

    // Filling pointLevels
    for(i = 0; i < bwImage.cols; i++)
    {
        for(j = 0; j < bwImage.rows; j++)
        {
            pc.x = i;
            pc.y = j;
            pointLevels[bwImage.at<uint8_t>(pc)].push_back(pc);
        }
    }

    printf("pointLevels size: %d.\n", (int)pointLevels.size());

    static int thresh_start = 0;
    static int thresh_end = 101;
    static int thresh_step = 1;

    for(int thresh = thresh_start; thresh < thresh_end; thresh += thresh_step)
    {
        for(k = 0; k < pointLevels[thresh].size(); k++)
        {
            Point p0 = pointLevels[thresh][k];

            //printf("=== NEXT POI: %d %d\n", p0.x, p0.y);

            // Surely point when accessed for the first time is not in any region
            // Setting parent, rank, creating region (uf_makeset)
            parents[pointLevels[thresh][k]] = pointLevels[thresh][k];
            ranks[pointLevels[thresh][k]] = 0;

            Region _r(pointLevels[thresh][k]);
            regions[pointLevels[thresh][k]] = _r;
            // Surely find will be successful since region we are searching for was just created
            map<Point, Region>::iterator point_region = regions.find(pointLevels[thresh][k]);
            Point proot = pointLevels[thresh][k];

            bool changed = false;

            for(di = 0; di < neigborsCount; di++)
            {
                int x_new = pointLevels[thresh][k].x + dx[di];
                int y_new = pointLevels[thresh][k].y + dy[di];

                // TODO: implement corresponding function?
                if ((x_new < 0) || (y_new < 0) || (x_new >= originalImage.cols) || (y_new >= originalImage.rows))
                {
                    continue;
                }

                if (changed)
                {
                    proot = uf_Find(pointLevels[thresh][k], &parents);
                    point_region = regions.find(proot);
                    // point_region should exist!
                }

                Point p1(x_new, y_new);

                map<Point, Point, PointComp>::iterator p1pi = parents.find(p1);
                if (p1pi != parents.end())
                {
                    // Entering here means that p1 has some parent
                    // Will now find root
                    Point p1root = uf_Find(p1, &parents);

                    map<Point, Region, PointComp>::iterator neighbor_region = regions.find(p1root);
                    if (neighbor_region != regions.end())
                    {
                        // TODO: not always
                        changed = true;

                        // <12.12>
                        int doubleBorderLen = 0;
                        /*
                        Rect b = regions[proot].Bounds();
                        Rect b1 = regions[p1root].Bounds();
                        b.x--; b.y--; b.width += 2; b.height += 2;
                        b1.x--; b1.y--; b1.width += 2; b1.height += 2;
                        Rect roi = b1 & b;
                        if (regions[proot].Area() == 1)
                        {
                            roi = regions[proot].Bounds();
                        }
                        if (regions[p1root].Area() == 1)
                        {
                            roi = regions[p1root].Bounds();
                        }

                        for(int px = roi.x; px < roi.x + roi.width; px++)
                        {
                            for(int py = roi.y; py < roi.y + roi.height; py++)
                            {
                                Point pxyroot = uf_Find(Point(px, py), &parents);
                                for(int ddi = 0; ddi < neigborsCount; ddi++)
                                {
                                    Point pxydroot = uf_Find(Point(px + dx[ddi], py + dy[ddi]), &parents);
                                    if ( ((pxydroot == proot) && (pxyroot == p1root)) ||
                                         ((pxydroot == p1root) && (pxyroot == proot)) )
                                    {
                                        doubleBorderLen++;
                                    }
                                }
                            }
                        }
                        */
                        // </12.12>

                        // Need to union. Three cases: rank1>rank2, rank1<rank2, rank1=rank2
                        int point_rank = ranks[pointLevels[thresh][k]];
                        int neighbor_rank = ranks[p1root];

                        // uf_union
                        if (point_rank < neighbor_rank)
                        {
                            parents[proot] = p1root;
                            regions[p1root].Attach(regions[proot], doubleBorderLen);
                            if (proot != p1root)
                            {
                                // TODO: check if smth is really erased
                                regions.erase(proot);
                            }
                        }
                        else if (point_rank > neighbor_rank)
                        {
                            parents[p1root] = proot;
                            regions[proot].Attach(regions[p1root], doubleBorderLen);
                            if (proot != p1root)
                            {
                                // TODO: check if smth is really erased
                                regions.erase(p1root);
                            }
                        }
                        else
                        {
                            parents[p1root] = proot;
                            ranks[proot] = ranks[proot] + 1;
                            regions[proot].Attach(regions[p1root], doubleBorderLen);
                            if (proot != p1root)
                            {
                                // TODO: check if smth is really erased
                                regions.erase(p1root);
                            }
                        }
                    }
                    else
                    {
                        // Neighbor point not in region. Do nothing
                    }
                }
            }
        }
        /*
        printf("Threshold: %d. Regions count: %ld.\n", thresh, regions.size());
        for(map<Point, Region, PointComp>::iterator it = regions.begin(); it != regions.end(); it++)
        {
            if (it->second.Area2() > 100)
            {
                //printf("Region bounds: %d %d %d %d\n", it->second.Bounds().x, it->second.Bounds().y, it->second.Bounds().width, it->second.Bounds().height);
                rectangle(bwImage, it->second.Bounds(), Scalar(0, 0, 0));
            }
        }
        */
    }

    printf("Regions count: %ld.\n", regions.size());
    int regionsCount = 0;
    for(map<Point, Region, PointComp>::iterator it = regions.begin(); it != regions.end(); it++)
    {
        //if (it->second.Area2() > 100)
        {
            regionsCount++;

            rectangle(originalImage, it->second.Bounds(), Scalar(0, 0, 0));

            printf("New region: %d\n", regionsCount);
            printf("Area: %d\n", it->second.Area());
            printf("Bounding box (%d; %d) + (%d; %d)\n", it->second.Bounds().x, it->second.Bounds().y, it->second.Bounds().width, it->second.Bounds().height);
            printf("Perimeter: %d\n", it->second.Perimeter());
            //printf("\n");
            printf("=====\n\n");
        }
    }

    t = (double)getTickCount() - t;
    printf("Working time: %g ms\n", t * 1000. / getTickFrequency());

    if (showImage)
    {
        imshow("Approach", originalImage);
        waitKey();
    }
}










int main()
{
    string filename;
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/ontario_small.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/vilnius.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/lines.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/painting.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/road.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/floor.jpg";
    //filename = "/home/evgeny/argus/opencv_sandbox/test_images/campaign.jpg";
    filename = "/home/evgeny/argus/opencv_sandbox/test_images/incorrect640.jpg";
    Mat originalImage = imread(filename);
    Mat originalImage2 = imread(filename);

    GroundTruth(originalImage);
    MatasLike(originalImage2);

    return 0;
}
