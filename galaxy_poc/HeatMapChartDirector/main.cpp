#include <stdio.h>
#include "chartdir.h"
#include <math.h>
#include <vector>

void createChart(int chartIndex, const char *filename)
{
    // The x and y coordinates of the grid
    double dataX[] = {-4, -3, -2, -1, 0, 1, 2, 3, 4};
    double dataY[] = {-4, -3, -2, -1, 0, 1, 2, 3, 4};

    // The values at the grid points. In this example, we will compute the values using the formula
    // z = Sin(x * pi / 3) * Sin(y * pi / 3).
    double dataZ[(int)(sizeof(dataX) / sizeof(dataX[0])) * (int)(sizeof(dataY) / sizeof(dataY[0]))];
    for(int yIndex = 0; yIndex < (int)(sizeof(dataY) / sizeof(dataY[0])); ++yIndex) {
        double y = dataY[yIndex];
        for(int xIndex = 0; xIndex < (int)(sizeof(dataX) / sizeof(dataX[0])); ++xIndex) {
            double x = dataX[xIndex];
            dataZ[yIndex * (int)(sizeof(dataX) / sizeof(dataX[0])) + xIndex] = sin(x * 3.1416 / 3) *
                sin(y * 3.1416 / 3);
        }
    }

    // Create a XYChart object of size 360 x 360 pixels
    XYChart *c = new XYChart(360, 360);

    // Set the plotarea at (30, 25) and of size 300 x 300 pixels. Use semi-transparent black
    // (c0000000) for both horizontal and vertical grid lines
    c->setPlotArea(30, 25, 300, 300, -1, -1, -1, 0xc0000000, -1);

    // Add a contour layer using the given data
    ContourLayer *layer = c->addContourLayer(DoubleArray(dataX, (int)(sizeof(dataX) / sizeof(dataX[0
        ]))), DoubleArray(dataY, (int)(sizeof(dataY) / sizeof(dataY[0]))), DoubleArray(dataZ, (int)(
        sizeof(dataZ) / sizeof(dataZ[0]))));

    // Set the x-axis and y-axis scale
    c->xAxis()->setLinearScale(-4, 4, 1);
    c->yAxis()->setLinearScale(-4, 4, 1);

    if (chartIndex == 0) {
        // Discrete coloring, spline surface interpolation
        c->addTitle("Spline Surface - Discrete Coloring", "arialbi.ttf", 12);
    } else if (chartIndex == 1) {
        // Discrete coloring, linear surface interpolation
        c->addTitle("Linear Surface - Discrete Coloring", "arialbi.ttf", 12);
        layer->setSmoothInterpolation(false);
    } else if (chartIndex == 2) {
        // Smooth coloring, spline surface interpolation
        c->addTitle("Spline Surface - Continuous Coloring", "arialbi.ttf", 12);
        layer->setContourColor(Chart::Transparent);
        layer->colorAxis()->setColorGradient(true);
    } else {
        // Discrete coloring, linear surface interpolation
        c->addTitle("Linear Surface - Continuous Coloring", "arialbi.ttf", 12);
        layer->setSmoothInterpolation(false);
        layer->setContourColor(Chart::Transparent);
        layer->colorAxis()->setColorGradient(true);
    }

    // Output the chart
    c->makeChart(filename);

    //free up resources
    delete c;
}

void CreateHeatmapChartImage(const char * fileName)
{
    // The number of cells in the x and y direction, and the cell size in pixels
    int xCount = 20;
    int yCount = 20;
    int cellSize = 20;

    // Some random data
    std::vector<double> dataZ(xCount * yCount);
    for(int yIndex = 0; yIndex < yCount; ++yIndex) {
        for(int xIndex = 0; xIndex < xCount; ++xIndex) {
            dataZ[yIndex * xCount + xIndex] = xIndex * sin((double)yIndex) + yIndex * sin((double)xIndex);
        }
    }

    // Set the chatr size and plot area size based on the number of cells and cell size
    XYChart *c = new XYChart((xCount + 1) * cellSize + 180, (yCount + 1) * cellSize + 70);
    c->setPlotArea(50, 30, (xCount + 1) * cellSize, (yCount + 1) * cellSize, -1, -1, -1, Chart::Transparent, -1);

    // Set up the x-axis and y-axis
    c->xAxis()->setLinearScale(1, xCount, 1);
    c->yAxis()->setLinearScale(1, yCount, 1);
    c->xAxis()->setMargin(cellSize, cellSize);
    c->yAxis()->setMargin(cellSize, cellSize);
    c->xAxis()->setLabelStyle("arialbd.ttf");
    c->yAxis()->setLabelStyle("arialbd.ttf");

    // Create a dummy contour layer to use the color axis
    ContourLayer *layer = c->addContourLayer(DoubleArray(0, 0), DoubleArray(0, 0), DoubleArray(0, 0));
    ColorAxis *cAxis = layer->setColorAxis(c->getPlotArea()->getLeftX() + c->getPlotArea()->getWidth() + 30,
        c->getPlotArea()->getTopY(), Chart::TopLeft, c->getPlotArea()->getHeight(), Chart::Right);

    // Specify the color and scale used and the threshold position
    int colors[] = { 0x0000ff, 0x008080, 0x00ff00, 0xcccc00, 0xff0000 };
    cAxis->setColorGradient(true, IntArray(colors, (int)(sizeof(colors) / sizeof(colors[0]))));
    cAxis->setLinearScale(0, 20, 20);
    cAxis->addMark(15, 0x000000, "Threshold", "arialbd.ttf", 8);

    // Set color axis labels to use Arial Bold font
    cAxis->setLabelStyle("arialbd.ttf");
    cAxis->setLabelFormat("{value}%");

    // Create a dummy chart with the color axis and call BaseChart::layout so that the
    // Axis.getColor can be used.
    XYChart *c2 = new XYChart(10, 10);
    cAxis = c2->addContourLayer(DoubleArray(0, 0), DoubleArray(0, 0), DoubleArray(0, 0))->colorAxis();
    cAxis->setColorGradient(true, IntArray(colors, (int)(sizeof(colors) / sizeof(colors[0]))));
    cAxis->setLinearScale(0, 20, 5);
    c2->layout();

    // Add the cells as scatter symbols
//    double symbolSize = 1;
    for (int yIndex = 0; yIndex < yCount; ++yIndex) {
        int yOffset = yIndex * xCount;
        for (int xIndex = 0; xIndex < xCount; ++xIndex) {
            double xCoor = xIndex + 1;
            double yCoor = yIndex + 1;
            int color = cAxis->getColor(dataZ[yOffset + xIndex]);

            c->addScatterLayer(DoubleArray(&xCoor, 1), DoubleArray(&yCoor, 1), "", Chart::SquareSymbol,
                cellSize, color, 0x888888)->addExtraField(DoubleArray(&(dataZ[yOffset + xIndex]), 1));
        }
    }

    // Output the chart
    c->makeChart(fileName);

    //free up resources
    delete c;
    delete c2;
}

int main(int /*argc*/, char */*argv*/[])
{
//    createChart(0, "contourinterpolate0.jpg");
//    createChart(1, "contourinterpolate1.jpg");
//    createChart(2, "contourinterpolate2.jpg");
//    createChart(3, "contourinterpolate3.jpg");

    CreateHeatmapChartImage("heatMap.png");
    return 0;
}
