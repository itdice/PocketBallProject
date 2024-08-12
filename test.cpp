//
// Created by IT DICE on 2024-08-12.
//
#include <iostream>
#include <cmath>
#include <vector>

typedef struct pos {
    double x;
    double y;
} Pos;

typedef struct linear {
    int type;
    double slope;
    double draft;
} Linear;

typedef struct rangeOfAngle {
    double start;
    double end;
} ROA;

// Linear GET Options
constexpr int LEFT = 1;
constexpr int RIGHT = 2;

// Linear Types
constexpr int NOMINAL = 0;
constexpr int ROW = 1;
constexpr int COL = 2;

constexpr double DIAMETER = 5.73;
constexpr double CORRECTION_ANGLE = 45.0;
constexpr double CORRECTION_POCKET = 0.0;
constexpr double FRACTION_RATIO = 0.0;
constexpr double FRACTION_DIA = 0.0;
constexpr double NOMINAL_ANGLE = 20.0;

double distanceToCenter(const Pos start, const Pos target) {
    // start -  target 간의 거리
    const Pos delta = { abs(start.x - target.x), abs(start.y - target.y) };
    return pow(pow(delta.x, 2.0f) + pow(delta.y, 2.0f), 0.5f);
}

double angleToCenter(const Pos start, const Pos target) {
    // start - target 간의 절대 각도
    const Pos delta = { abs(start.x - target.x), abs(start.y - target.y) };
    const double ratio = delta.x / pow(pow(delta.x, 2.0) + pow(delta.y, 2.0), 0.5);
    double angle;

    if (start.x <= target.x)
        angle = acos(ratio) / M_PI * 180.0;
    else
        angle = acos(-1 * ratio) / M_PI * 180.0;

    if (start.y > target.y)
        angle = 180.0 + (180. - angle);
    return angle;
}

Linear getLinear(const Pos start, const Pos target, const int option = LEFT) {
    // 공통내접선 구하는 함수 (내부 함수)
    const double distance = distanceToCenter(start, target);
    const double alphaAngle = angleToCenter(start, target);
    const double betaAngle = acos(DIAMETER / distance) / M_PI * 180.0;

    const Pos center = { (start.x + target.x) / 2.0, (start.y + target.y) / 2.0 };

    double linearAngle = 45.0;
    if (option == LEFT)
        linearAngle = 90.0 + alphaAngle - betaAngle;
    else if (option == RIGHT)
        linearAngle = -90.0 + alphaAngle + betaAngle;

    Linear result = { NOMINAL, 1.0, 0.0 };
    if (linearAngle == 0.0 || linearAngle == 180.0) {
        result.type = ROW;
        result.slope = 0.0;
        result.draft = center.y;
    }
    else if (linearAngle == 90.0 || linearAngle == 270.0) {
        result.type = COL;
        result.slope = 0.0;
        result.draft = center.x;
    }
    else {
        result.type = NOMINAL;
        result.slope = tan(linearAngle / 180.0f * M_PI);
        result.draft = center.y - (result.slope * center.x);
    }

    return result;
}

Linear getPathLinear(const Pos start, const Pos target, const double angle, const int option = LEFT) {
    // 이동 경로 직선 함수 구하는 것 (내부 함수)
    Pos leftPoint = {0.0, 0.0}, rightPoint = {0.0, 0.0};

    if (angle == 0.0) {
        leftPoint = {start.x, start.y + (DIAMETER / 2.0)};
        rightPoint = {start.x, start.y - (DIAMETER / 2.0)};
    }
    else if (0.0 < angle && angle < 90.0) {
        double shiftAngle = angle;
        Pos delta = {(DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y + delta.y};
        rightPoint = {start.x + delta.x, start.y - delta.y};
    }
    else if (angle == 90.0) {
        leftPoint = {start.x - (DIAMETER / 2.0), start.y};
        rightPoint = {start.x + (DIAMETER / 2.0), start.y};
    }
    else if (90.0 < angle && angle < 180.0) {
        double shiftAngle = 180.0 - angle;
        Pos delta = {(DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y - delta.y};
        rightPoint = {start.x + delta.x, start.y + delta.y};
    }
    else if (angle == 180.0) {
        leftPoint = {start.x, start.y - (DIAMETER / 2.0)};
        rightPoint = {start.x, start.y + (DIAMETER / 2.0)};
    }
    else if (180.0 < angle && angle < 270.0) {
        double shiftAngle = angle - 180.0;
        Pos delta = {(DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y + delta.y};
        rightPoint = {start.x + delta.x, start.y - delta.y};
    }
    else if (angle == 270.0) {
        leftPoint = {start.x + (DIAMETER / 2.0), start.y};
        rightPoint = {start.x - (DIAMETER / 2.0), start.y};
    }
    else if (270.0 < angle && angle < 360.0) {
        double shiftAngle = 360.0 - angle;
        Pos delta = {(DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y - delta.y};
        rightPoint = {start.x + delta.x, start.y + delta.y};
    }

    Linear result = {NOMINAL, 1.0, 0.0};
    if (angle == 0.0  || angle == 180.0) {
        result.type = ROW;
        result.slope = 0.0;
    }
    else if (angle == 90.0 || angle == 270.0) {
        result.type = COL;
        result.slope = 0.0;
    }
    else {
        result.type = NOMINAL;
        result.slope = tan(angle / 180.0 * M_PI);
    }

    if (option == LEFT) {
        if (angle == 0.0 || angle == 180.0)
            result.draft = leftPoint.y;
        else if (angle == 90.0 || angle == 270.0)
            result.draft = leftPoint.x;
        else
            result.draft = leftPoint.y - (result.slope * leftPoint.x);
    }
    else if (option == RIGHT) {
        if (angle == 0.0 || angle == 180.0)
            result.draft = rightPoint.y;
        else if (angle == 90.0 || angle == 270.0)
            result.draft = rightPoint.x;
        else
            result.draft = rightPoint.y - (result.slope * rightPoint.x);
    }

    return result;
}

Pos getPoint(const Pos circle, const Linear data) {
    // 교점 구하는 거(내부 함수)
    Pos result = { 0.0f, 0.0f };
    if (data.type == ROW) {
        result.x = circle.x;
    }
    else if (data.type == COL) {
        result.y = circle.y;
    }
    else {
        const double a = data.slope, b = data.draft;
        const double c = circle.x, d = circle.y;

        result.x = (-1 * (2 * a * b - 2 * a * d - 2 * c)) / (2 * (a * a + 1));
        result.y = a * result.x + b;
    }

    return result;
}

bool isCrossed(const Pos start, const Pos target, const Pos circle, const Linear left, const Linear right) {
    Pos begin = {(start.x < target.x ? start.x : target.x), (start.y < target.y ? start.y : target.y)};
    Pos end = {(start.x < target.x ? target.x : start.x), (start.y < target.y ? target.y : start.y)};

    if (left.type == ROW && right.type == ROW) {
        if (begin.x - DIAMETER / 2.0 <= circle.x && circle.x <= end.x + DIAMETER / 2.0 &&
        begin.y - DIAMETER <= circle.y && circle.y <= begin.y + DIAMETER)
    }
    else if (left.type == COL && right.type == COL) {

    }
    else {
        if (left.slope > 0.0) {

        }
        else if (left.slope < 0.0) {

        }
    }
}

ROA hittableAngle(const Pos start, const Pos target) {
    // 칠수 있는 각도 범위 도출
    const Linear leftLinear = getLinear(start, target, LEFT);
    printf("left : (slope : %f, draft : %f)\n", leftLinear.slope, leftLinear.draft);
    const Linear rightLinear = getLinear(start, target, RIGHT);
    printf("right : (slope : %f, draft : %f)\n", rightLinear.slope, rightLinear.draft);

    const Pos startPoint = getPoint(target, leftLinear);
    const Pos endPoint = getPoint(target, rightLinear);

    const double startAngle = angleToCenter(startPoint, target);
    const double endAngle = angleToCenter(endPoint, target);
    const ROA result = { startAngle + CORRECTION_ANGLE, endAngle - CORRECTION_ANGLE };

    return result;
}

Pos getHitPoint(const Pos target, const double angle) {
    // target과 그 target이 진행해야할 절대 각도를 이용해서 처야하는 지점 구하는 함수
    Pos result = { target.x, target.y };

    if (angle == 0.0) {
        result.x = target.x - (DIAMETER + FRACTION_DIA);
        result.y = target.y;
    }
    else if (0.0 < angle && angle < 90.0) {
        const Pos delta = { (DIAMETER + FRACTION_DIA) * cos(angle / 180.0 * M_PI),
                            (DIAMETER + FRACTION_DIA) * sin(angle / 180.0 * M_PI) };
        result.x = target.x - delta.x;
        result.y = target.y - delta.y;
    }
    else if (angle == 90.0) {
        result.x = target.x;
        result.y = target.y - (DIAMETER + FRACTION_DIA);
    }
    else if (90.0 < angle && angle < 180.0) {
        const Pos delta = { (DIAMETER + FRACTION_DIA) * cos((180.0 - angle) / 180.0 * M_PI),
                            (DIAMETER + FRACTION_DIA) * sin((180.0 - angle) / 180.0 * M_PI) };
        result.x = target.x + delta.x;
        result.y = target.y - delta.y;
    }
    else if (angle == 180.0) {
        result.x = target.x + (DIAMETER + FRACTION_DIA);
        result.y = target.y;
    }
    else if (180.0 < angle && angle < 270.0) {
        const Pos delta = { (DIAMETER + FRACTION_DIA) * cos((angle - 180.0) / 180.0 * M_PI),
                            (DIAMETER + FRACTION_DIA) * sin((angle - 180.0) / 180.0 * M_PI) };
        result.x = target.x + delta.x;
        result.y = target.y + delta.y;
    }
    else if (angle == 270.0) {
        result.x = target.x;
        result.y = target.y + (DIAMETER + FRACTION_DIA);
    }
    else if (270.0 < angle && angle < 360.0) {
        const Pos delta = { (DIAMETER + FRACTION_DIA) * cos((angle - 270.0) / 180.0 * M_PI),
                            (DIAMETER + FRACTION_DIA) * sin((angle - 270.0) / 180.0 * M_PI) };
        result.x = target.x - delta.x;
        result.y = target.y + delta.y;
    }

    return result;
}

double convertTogameAngle(const double angle) {
    // 게임 상의 각도로 변환 해주는 함수
    double gameAngle = 0.0f;

    if (90.0f - angle >= 0.0f)
        gameAngle = 90.0 - angle;
    else
        gameAngle = 360.0 + (90.0 - angle);

    return gameAngle;
}

bool isPossibleAngle(const ROA range, const double angle) {
    // range 범위 내에 angle이 들어오는지 확인하는 함수
    if (range.start <= range.end)
        return (range.start <= angle && angle <= range.end);
    else
        return (range.start <= angle && angle < 360.0 || 0.0 <= angle && angle <= range.end);
}

int main(int argc, char** argv) {


    return 0;
}