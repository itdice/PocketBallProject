#include <iostream>
#include <cmath>

constexpr int LEFT = 1;
constexpr int RIGHT = 2;
constexpr int LEFT_DRAFT = 3;
constexpr int RIGHT_DRAFT = 4;

constexpr int NOMINAL = 0;
constexpr int ROW = 1;
constexpr int COL = 2;

using namespace std;

typedef struct pos {
    double y;
    double x;
} Pos;

typedef struct linear {
    double slope;
    double draft;
    int option;
} Linear;

typedef struct rangeOfAngle {
    double start;
    double end;
} ROA;

double distanceToCenter(Pos start, Pos target) {
    Pos delta = {abs(start.y - target.y), abs(start.x - target.x)};
    return pow(pow(delta.y, 2) + pow(delta.x, 2), 0.5);
}

double angleToCenter(Pos start, Pos target) {
    Pos delta = {abs(start.y - target.y), abs(start.x - target.x)};
    double ratio = delta.x / (pow(pow(delta.y, 2) + pow(delta.x, 2), 0.5));
    double angle;

    if (start.x <= target.x)
        angle = acos(ratio) / M_PI * 180.0;
    else
        angle = acos(-1 * ratio) / M_PI * 180.0;

    if (start.y <= target.y)
        return angle;
    else
        return 180.0 + (180.0 - angle);
}

double betaAngleToCenter(Pos start, Pos target, double diameter) {
    double distance = distanceToCenter(start, target);
    double ratio = (diameter * 2) / (distance);
    return acos(ratio) / M_PI * 180.0;
}

Linear getLinear(Pos start, Pos target, double diameter, int option = LEFT) {
    Linear result = {1.0, 0.0};
    double alphaAngle = angleToCenter(start, target);
    double betaAngle = betaAngleToCenter(start, target, diameter);

    Pos centerOfDistance = {(start.y + target.y) / 2.0, (start.x + target.x) / 2.0};
    double linearAngle = 45.0;
    double addDraft = diameter;

    if (option == LEFT || option == LEFT_DRAFT)
        linearAngle = 90.0 - betaAngle + alphaAngle;
    else if (option == RIGHT || option == RIGHT_DRAFT)
        linearAngle = -90.0 + betaAngle + alphaAngle;

    if (linearAngle == 0.0 || linearAngle == 180.0) {
        result.option = ROW;
        result.slope = 0.0;
        result.draft = centerOfDistance.y;
    }
    else if (linearAngle == 90.0 || linearAngle == 270.0) {
        result.option = COL;
        result.slope = 0.0;
        result.draft = centerOfDistance.x;
    }
    else {
        result.option = NOMINAL;
        result.slope = tan(linearAngle / 180.0 * M_PI);
        result.draft = centerOfDistance.y - (result.slope * centerOfDistance.x);
    }



    double addDraft = 2 * (diameter / cos(linearAngle));

    return result;
}

ROA hittableAngle(Pos start, Pos target, double diameter) {

}

int main(int argc, char** argv) {
    Pos ball, hall = {10.0, 10.0};
    cin >> ball.y >> ball.x;

    cout << angleToCenter(ball, hall) << "°";

    return 0;
}
