#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "math.h"
#include <chrono>
#include "omp.h"


using namespace std;
using namespace std::chrono;

const int X_SIZE = 1920;
const int Y_SIZE = 1080;
const int sizeOfXCell = 20;
const int sizeOfYCell = 20;
vector<int> steps = {-1,0,1};


typedef struct
{
    int x;
    int y;
    int status;
} VEC2;

int cntr;
vector<VEC2> points;
int xCells;
int yCells;
vector<VEC2> locals[1000][1000];

double rad(int x1, int y1, int x2, int y2)
{
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

int readFile(char * filename, vector<VEC2> &points, int &cntr)
{
    int count = 0;
    printf("opening a file ");
    //char * filename = "C:/WORKDIR/trash/cppContourProcessing/untitled/contours.txt";
    printf("%s \n", filename);

    FILE * points0 = fopen(filename, "r");
    char mystring0[256];

    while (fgets(mystring0,256,points0))
    {
        VEC2 cTmp;
        sscanf(mystring0,"%d %d", &cTmp.x, &cTmp.y);
        points.push_back(cTmp);
        count++;
    }

    fclose(points0);
    printf("INITIAL COUNT = %d\n\n", count);
    cntr = count;
    return 0;
}

int makeLocals(vector<VEC2>& points, const int& cntr, int & xCells, int & yCells)
{
    xCells = X_SIZE / sizeOfXCell;
    yCells = Y_SIZE / sizeOfYCell;
    for (int i = 1; i <= xCells; i++)
    {
        for (int j = 1; j <= yCells; j++)
        {
            for (int k = 0; k < cntr; k++)
            {
                if ((points[k].x < i*sizeOfXCell)&(points[k].x > (i-1)*sizeOfXCell)&
                        (points[k].y < j*sizeOfXCell)&(points[k].y > (j-1)*sizeOfXCell))
                {
                    locals[i][j].push_back(points[k]);
                }
            }
        }
    }
}

int writeFile(char * filename, vector<VEC2> &points, const int& counter)
{
    printf("writing a file ");
    printf("%s \n", filename);

    FILE * pointsWrite = fopen(filename, "w");

    for (int index = 0; index < counter; index++)
        fprintf(pointsWrite, "%d %d\n", points[index].x, points[index].y);

    printf("saved in %s \n", filename);

    fclose(pointsWrite);

    return 0;
}

int sphereFilter(vector<VEC2> &points, int &counter, double radi1, double radi2, int k)
{
    /**************************************************
 * FROM THE SORT_POINT_3D.CPP TIMEKILLER_PROJECT  *
 * FILTER WITH THE SPHERES                        *
 **************************************************/

    printf("MAKING TWO-SPHERE FILTRATION... \n\n");
    double r2;
    int correct = 0;
    int numberin1;
    int numberin2;
    VEC2 d;
    VEC2 pos;
    vector<VEC2> points_new;

    for (int j = 0; j < counter; j++)
    {
        points[j].status=0;
    }

    for (int j = 0; j < counter; j++)
    {
        pos = points[j];
        numberin1 = 0;
        numberin2 = 0;
        for (int i = 0; i < counter; i++)
        {
            //printf("%d %d %d \n", points[i].x, points[i].y, points[i].status);
            if (i != j)
            {
                d.x = pos.x-points[i].x;
                d.y = pos.y-points[i].y;

                r2 = d.x*d.x + d.y*d.y;//+delta;
                //printf("%f \n", r2);

                if (r2<radi1)
                {
                    numberin1++;
                }
                if (r2<radi2)
                {
                    numberin2++;
                }
                //printf(" numberin1 = %d      numberin2 = %d \n",   numberin1, numberin2);
            }
        }
        if ((numberin2 - numberin1 < k) & (points[j].status == 0))
        {
            points[j].status = 1;
            //printf("status = %d \n", points[j].status);
        }
        if(j%5000 == 0)
            printf("PROGRESS........... %1.f %\n", (double)j/(double)counter*100);
    }

    for (int j = 0; j < counter; j++)
    {
        if(points[j].status==0)
        {
            points_new.push_back(points[j]);
            correct++;
            //printf("corr = %d\n", correct);
        }
    }

    counter = correct;
    printf("after sphere count = %d \n\n", counter);
    points.clear();
    for (int i = 0; i < counter; i++)
        points.push_back(points_new[i]);

    writeFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours_sphere.txt", points, counter);
    return 0;
}

int localSphereFilter(vector<VEC2>& points, int& counter, double radi1, double radi2, int k, int xCells, int yCells)
{
    vector<VEC2> points_new;
    double r2;
    int correct = 0;
    int numberin1;
    int numberin2;
    VEC2 d;
    VEC2 pos;
    for (int j = 0; j < counter; j++)
    {
        points[j].status = 0;
    }
    makeLocals(points, counter, xCells, yCells);
    printf("MAKING LOCAL TWO-SPHERE FILTRATION... \n\n");
    for (int k = 1; k <= xCells; k++)
    {
        for (int l = 1; l <= yCells; l++)
        {
            for (int j = 0; j < locals[k][l].size(); j++)
            {
                pos = locals[k][l][j];
                numberin1 = 0;
                numberin2 = 0;
                for (const int& stepX : steps)
                {
                    for (const int& stepY : steps)
                    {
                        for (int i = 0; i < locals[k+stepX][l+stepY].size(); i++)
                        {
                            //printf("%d %d %d \n", points[i].x, points[i].y, points[i].status);
                            if ((stepX != 0)&&(stepY != 0)&&(k!=l))
                            {
                                d.x = pos.x-locals[k+stepX][l+stepY][i].x;
                                d.y = pos.y-locals[k+stepX][l+stepY][i].y;

                                r2 = d.x*d.x + d.y*d.y;//+delta;
                                //printf("%f \n", r2);

                                if (r2<radi1)
                                {
                                    numberin1++;
                                }
                                if (r2<radi2)
                                {
                                    numberin2++;
                                }
                                //printf(" numberin1 = %d      numberin2 = %d \n",   numberin1, numberin2);
                            }
                        }
                    }
                }
                if ((numberin2 - numberin1 < k) & (locals[k][l][j].status == 0))
                {
                    locals[k][l][j].status = 1;
                    //printf("status = %d \n", points[j].status);
                }

            }
            for (int j = 0; j < locals[k][l].size(); j++)
            {
                if(locals[k][l][j].status == 0)
                {
                    points_new.push_back(locals[k][l][j]);
                    correct++;
                    //printf("corr = %d\n", correct);
                }
            }
        }
        if (k%10 == 0)
            printf("PROGRESS........... %1.f %\n", (double)k/(double)xCells*100);
    }
    counter = correct;
    printf("after local sphere count = %d \n\n", counter);
    points.clear();
    for (int i = 0; i < counter; i++)
        points.push_back(points_new[i]);

    writeFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours_sphere.txt", points, counter);
    return 0;
}

int potentialFilter(vector<VEC2> &points, int &cntr, double thresh, double epsilon)
{
    /*************************************
 * BLOCK WITH THE POTENTIAL FILTER   *
 * FROM THE Optical Diagnostics of   *
 * the Variable Area Nozzle          *
 * Geometry paper                    *
 *************************************/
    printf("MAKING POTENTIAL FILTRATION... \n\n");

    vector<VEC2> points_new;
    double potential = 0.;
    int final_cntr = 0;
    for (int i = 0; i < cntr; i++)
    {
        for (int j = 0; j < cntr; j++)
        {
            if (j != i)
            {
                potential += 1/(rad(points[i].x, points[i].y, points[j].x, points[j].y) + epsilon);
            }
        }
        if (potential > thresh)
        {
            points_new.push_back(points[i]);
            final_cntr ++;
        }
        potential = 0.;
        if(i%5000 == 0)
            printf("PROGRESS........... %1.f %\n", (double)i/(double)cntr*100);
    }
    printf("final count = %d \n", final_cntr);
    cntr = final_cntr;
    points.clear();
    for (int i = 0; i < cntr; i++)
        points.push_back(points_new[i]);

    writeFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours_potential.txt", points, cntr);
    return 0;
}

int localPotentialFilter(vector<VEC2> &points, int &cntr, double thresh, double epsilon, int sizeOfXCell, int sizeOfYCell)
{
    vector<VEC2> points_new;
    double potential = 0.;
    int final_cntr = 0;
    makeLocals(points, cntr, xCells, yCells);
    printf("MAKING LOCAL POTENTIAL FILTRATION... \n\n");
    for (int i = 1; i <= xCells; i++)
    {
        for (int j = 1; j <= yCells; j++)
        {
            for (int k = 0; k < locals[i][j].size(); k++)
            {
                for (const int& stepX : steps)
                {
                    for (const int& stepY : steps)
                    {
                        if ((stepX + i > 0) && (stepY + j > 0) && (stepX + i < xCells + 1) && (stepY + j < yCells + 1))
                        {
                            for (int l = 0; l < locals[i+stepX][j+stepY].size(); l++)
                            {
                                if ((stepX != 0)&&(stepY != 0)&&(k!=l))
                                    potential += 1/(rad(locals[i][j][k].x, locals[i][j][k].y, locals[i+stepX][j+stepY][l].x,
                                        locals[i+stepX][j+stepY][l].y) + epsilon);
                            }
                        }
                    }
                }
                if (potential > thresh)
                {
                    points_new.push_back(locals[i][j][k]);
                    final_cntr ++;
                }
                potential = 0.;
            }
        }
        if (i%10 == 0)
            printf("PROGRESS........... %1.f %\n", (double)i/(double)xCells*100);
    }
    printf("final count = %d \n", final_cntr);
    cntr = final_cntr;
    points.clear();
    for (int i = 0; i < cntr; i++)
        points.push_back(points_new[i]);

    writeFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours_potential.txt", points, cntr);
    return 0;
}




int main()
{
    readFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours.txt", points, cntr);
    //readFile("C:/WORKDIR/trash/cppContourProcessing/untitled/resized.txt", points, cntr);

    time_point<system_clock> start, end;
    start = system_clock::now();
    sphereFilter(points, cntr, 200, 400, 25);
    //localSphereFilter(points, cntr, 200, 400, 5, 20, 20);
    //localPotentialFilter(points, cntr, 5., 0.001, 20, 20);
    potentialFilter(points, cntr, 200., 0.01);
    end = system_clock::now();
    int elapsed_seconds = duration_cast<seconds>
            (end-start).count();
    cout << "potential-filter-time: " << elapsed_seconds << "s\n";

    //writeFile("C:/WORKDIR/trash/cppContourProcessing/untitled/contours_new.txt", points, cntr);
    return 0;
}
