//
// Created by IT DICE on 2024-08-12.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winSock2.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <algorithm>

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

// 닉네임을 사용자에 맞게 변경해 주세요.
#define NICKNAME "SEOUL21_IT_DICE_MODIFIED"

// 일타싸피 프로그램을 로컬에서 실행할 경우 변경하지 않습니다.
#define HOST "127.0.0.1"

// 일타싸피 프로그램과 통신할 때 사용하는 코드값으로 변경하지 않습니다.
#define PORT 1447
#define CODE_SEND 9901
#define CODE_REQUEST 9902
#define SIGNAL_ORDER 9908
#define SIGNAL_CLOSE 9909

// 커스텀 코드 부분

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

typedef struct block {
    Pos target;
    Pos pocket;
	float angle;
	float power;
    double score;
} Block;

// Linear GET Options
constexpr int LEFT = 1;
constexpr int RIGHT = 2;

// Linear Types
constexpr int NOMINAL = 0;
constexpr int ROW = 1;
constexpr int COL = 2;

constexpr double MAX_WIDTH = 254.0;
constexpr double MAX_HEIGHT = 127.0;

constexpr double DIAMETER = 5.73;
constexpr double CORRECTION_ANGLE = 45.0;
constexpr double CORRECTION_POCKET = 0.0;
constexpr double FRACTION_RATIO = 0.0;
constexpr double FRACTION_DIA = 0.0;
constexpr double NOMINAL_ANGLE = 20.0;
constexpr double CHECK_FRACTION_DIA = 0.2;

constexpr double ANGLE_GAP_RATIO = 10.0;
constexpr double DISTANCE_RATIO = 1.0;
constexpr double INTERCEPTION_RATIO = 100.0;

constexpr double POWER_RATIO = 0.3;

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
        Pos delta = {(DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y + delta.y};
        rightPoint = {start.x + delta.x, start.y - delta.y};
    }
    else if (angle == 90.0) {
        leftPoint = {start.x - (DIAMETER / 2.0), start.y};
        rightPoint = {start.x + (DIAMETER / 2.0), start.y};
    }
    else if (90.0 < angle && angle < 180.0) {
        double shiftAngle = 180.0 - angle;
        Pos delta = {(DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y - delta.y};
        rightPoint = {start.x + delta.x, start.y + delta.y};
    }
    else if (angle == 180.0) {
        leftPoint = {start.x, start.y - (DIAMETER / 2.0)};
        rightPoint = {start.x, start.y + (DIAMETER / 2.0)};
    }
    else if (180.0 < angle && angle < 270.0) {
        double shiftAngle = angle - 180.0;
        Pos delta = {(DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI)};
        leftPoint = {start.x - delta.x, start.y + delta.y};
        rightPoint = {start.x + delta.x, start.y - delta.y};
    }
    else if (angle == 270.0) {
        leftPoint = {start.x + (DIAMETER / 2.0), start.y};
        rightPoint = {start.x - (DIAMETER / 2.0), start.y};
    }
    else if (270.0 < angle && angle < 360.0) {
        double shiftAngle = 360.0 - angle;
        Pos delta = {(DIAMETER / 2.0) * sin(shiftAngle / 180.0 * M_PI),
                     (DIAMETER / 2.0) * cos(shiftAngle / 180.0 * M_PI)};
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
    // 교점 구하는 함수(내부 함수)
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

bool isCrossed(const Pos start, const Pos target, const double angle, const Pos circle) {
    // 이동 경로상에 circle 공이 걸리는 경우 확인하는 함수
    Linear left = getPathLinear(start, target, angle, LEFT);
    Linear right = getPathLinear(start, target, angle, RIGHT);

    printf("left : (%lf, %lf)\n", left.slope, left.draft);
    printf("right : (%lf, %lf)\n", right.slope, right.draft);

    Pos leftOfRange = {(start.x < target.x ? start.x : target.x), (start.y < target.y ? start.y : target.y)};
    Pos rightOfRange = {(start.x < target.x ? target.x : start.x), (start.y < target.y ? target.y : start.y)};

    leftOfRange.x -= DIAMETER / 2.0;
    rightOfRange.x += DIAMETER / 2.0;

    if (leftOfRange.y <= rightOfRange.y) {
        leftOfRange.y -= DIAMETER / 2.0;
        rightOfRange.y += DIAMETER / 2.0;
    }
    else {
        leftOfRange.y += DIAMETER / 2.0;
        rightOfRange.y -= DIAMETER / 2.0;
    }

    // 경로 사각형 범위 내에 들어오는 경우
    if (leftOfRange.x <= circle.x && circle.x <= rightOfRange.x &&
        leftOfRange.y <= circle.y && circle.y <= rightOfRange.y) {
        bool crossedFlag = false;

        const double c = circle.x, d = circle.y, r = DIAMETER / 2.0 + CHECK_FRACTION_DIA;
        if (left.type == ROW) {
            for (const auto& b: {left.draft, right.draft}) {
                const double checkSum = pow(-2 * c, 2.0)
                - 4 * (1 * (pow(c, 2.0) + pow(b - d, 2.0) - pow(r, 2.0)));
                if (checkSum >= 0.0)
                    crossedFlag = true;
            }
        }
        else if (left.type == COL) {
            for (const auto& b: {left.draft, right.draft}) {
                const double checkSum = pow(-2 * d, 2.0)
                - 4 * (1 * (pow(d, 2.0) + pow(b - c, 2.0) - pow(r, 2.0)));
                if (checkSum >= 0.0)
                    crossedFlag = true;
            }
        }
        else {
            for (const auto& path: {left, right}) {
                const double a = path.slope, b = path.draft;
                const double checkSum = pow((-2 * c) + (2 * a * b) + (-2 * a * d), 2.0)
                - 4 * ((pow(a, 2.0) + 1) * (pow(b - d, 2.0) + pow(c, 2.0) - pow(r, 2.0)));
                if (checkSum >= 0.0)
                    crossedFlag = true;
            }
        }

        if (crossedFlag)
            return true;
    }

    return false;
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

bool blockCompare(const Block &a, const Block &b) {
	// Block 비교 함수
	return a.score < b.score;
}

void ErrorHandling(const char* message);

int main()
{
	// 게임 환경에 대한 상수입니다.
	int TABLE_WIDTH = 254;
	int TABLE_HEIGHT = 127;
	int NUMBER_OF_BALLS = 6;
	int HOLES[6][2] = { {0, 0}, {127, 0}, {254, 0}, {0, 127}, {127, 127}, {254, 127} };

	float balls[6][2] = {
		0.0f,
	};
	int order = 0;

	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sockAddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup failure.");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (hSocket == INVALID_SOCKET)
		ErrorHandling("Socket Creating Failure.");

	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(HOST);
	sockAddr.sin_port = htons(PORT);

	printf("Trying Connect: %s:%d\n", HOST, PORT);
	if (connect(hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
		ErrorHandling("Connection Failure.");
	else
		printf("Connected: %s:%d\n", HOST, PORT);

	char sendData[1024];

	sprintf(sendData, "%d/%s/", CODE_SEND, NICKNAME);
	send(hSocket, (char*)&sendData, sizeof(sendData), 0);
	printf("Ready to play!\n--------------------");

	while (1)
	{
		// Receive Data
		char message[1024];
		int strLen;
		int recvCount = 0;
		strLen = recv(hSocket, message, sizeof(message) - 1, 0);
		// printf("Data Received: %s\n", message);

		// Read Game Data
		char* recvData;
		recvData = strtok(message, "/");
		float pos = atof(recvData);

		recvCount++;

		for (int i = 0; i < NUMBER_OF_BALLS; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				balls[i][j] = pos;
				recvData = strtok(NULL, "/");

				if (recvData != NULL)
				{
					pos = atof(recvData);

					recvCount++;
				}
			}
		}

		// Check Signal for Player Order or Close Connection
		if (balls[0][0] == SIGNAL_ORDER)
		{
			order = (int)balls[0][1];
			printf("\n\n* You will be the %s player. *\n\n", order == 1 ? "first" : "second");
			continue;
		}
		else if (balls[0][0] == SIGNAL_CLOSE)
		{
			break;
		}

		// Show Balls' Position
		for (int i = 0; i < NUMBER_OF_BALLS; i++)
		{
			printf("ball %d: %f, %f\n", i, balls[i][0], balls[i][1]);
		}

		float angle = 0.0f;
		float power = 0.0f;

		//////////////////////////////
		// 이 위는 일타싸피와 통신하여 데이터를 주고 받기 위해 작성된 부분이므로 수정하면 안됩니다.
		//
		// 모든 수신값은 변수, 배열에서 확인할 수 있습니다.
		//   - order: 1인 경우 선공, 2인 경우 후공을 의미
		//   - balls[][]: 일타싸피 정보를 수신해서 각 공의 좌표를 배열로 저장
		//     예) balls[0][0]: 흰 공의 X좌표
		//         balls[0][1]: 흰 공의 Y좌표
		//         balls[1][0]: 1번 공의 X좌표
		//         balls[4][0]: 4번 공의 X좌표
		//         balls[5][0]: 마지막 번호(8번) 공의 X좌표

		// 여기서부터 코드를 작성하세요.
		// 아래에 있는 것은 샘플로 작성된 코드이므로 자유롭게 변경할 수 있습니다.

		// Pocket 위치
		std::vector<Pos> pockets = { {254.0 - CORRECTION_POCKET, 127.0 - CORRECTION_POCKET},
			{254.0 - CORRECTION_POCKET, 0.0 + CORRECTION_POCKET},
			{127.0, 127.0 - CORRECTION_POCKET},
			{127.0, 0.0 + CORRECTION_POCKET},
			{0.0 + CORRECTION_POCKET, 127.0 - CORRECTION_POCKET},
			{0.0 + CORRECTION_POCKET, 0.0 + CORRECTION_POCKET} };

		// 공 위치 vector 변환
		std::vector<std::vector<int>> orderList = { {0, 0, 0}, {1, 3, 5}, {2, 4, 5} };
		std::vector<Pos> danggoos(6, { 0.0, 0.0 });
		for (int index = 0; index < 6; index++)
			danggoos[index] = { balls[index][0], balls[index][1] };

	    Pos white = danggoos[0];
	    Pos black = danggoos[5];

	    std::vector<Block> situationScores;  // 상황별 점수 기록용 vector

	    for (const auto& targetIndex: orderList[order]) {
	        Pos target = danggoos[targetIndex];
	    	// 없는 공인 경우
	    	if (target.x == -1.0 && target.y == -1.0)
	    		continue;

	    	// 칠 수 있는 공이 있는데 검은 공을 계산하려는 경우
	    	if (!situationScores.empty() && targetIndex == 5)
	    		break;


	    	ROA targetHittableAngle = hittableAngle(white, target);
	    	printf("targetHittableAngle : %lf to %lf\n", targetHittableAngle.start, targetHittableAngle.end);

	    	for (const auto& pocket: pockets) {
	    		double score = 0.0;  // 현재 상황에 대한 점수
	    		double targetToPocketAngle = angleToCenter(target, pocket);
	    		printf("targetToPocketAngle : %lf\n", targetToPocketAngle);

	    		if (isPossibleAngle(targetHittableAngle, targetToPocketAngle)) {
	    			// target을 칠 수 있는 각도 범위인 경우
	    			printf("Can Hit to this Pocket(%lf, %lf)\n", pocket.x, pocket.y);

	    			// 나아가야 하는 각도에 따른 hit 지점 계산하기
	    			Pos hitPoint = getHitPoint(target, targetToPocketAngle);
	    			double whiteToHitPointAngle = angleToCenter(white, hitPoint);
	    			printf("Hit Point is (%lf, %lf)\n", hitPoint.x, hitPoint.y);
	    			printf("Hit Angle is %lf\n", whiteToHitPointAngle);

	    			// 각도 차이에 따른 점수 추가
	    			score += abs(whiteToHitPointAngle - targetToPocketAngle) * ANGLE_GAP_RATIO;

	    			// 이동거리 계산하기
	    			double distanceWhiteToHitPoint = distanceToCenter(white, hitPoint);
	    			double distanceTargetToPocket = distanceToCenter(target, pocket);

	    			// 이동거리에 따른 점수 추가
	    			score += (distanceWhiteToHitPoint + distanceTargetToPocket) * DISTANCE_RATIO;

	    			for (const auto& otherBallIndex: orderList[(order == 1 ? 2 : 1)]) {
	    				Pos otherBall = danggoos[otherBallIndex];
	    				// 없는 공인 경우
	    				if (otherBall.x == -1.0 && otherBall.y == -1.0)
	    					continue;

	    				// 검은 공이 target인데 otherBall이 검은공이 될 수 없으므로
	    				if (otherBallIndex == 5 && targetIndex == 5)
	    					continue;

	    				// white 이동 path 상에 다른 공이 있는 경우 점수 추가
	    				if (isCrossed(white, hitPoint, whiteToHitPointAngle, otherBall))
	    					score += 1 * INTERCEPTION_RATIO;
	    			}

	    			printf("Shoot to (%lf, %lf) (%lf)\n", hitPoint.x, hitPoint.y, whiteToHitPointAngle);
	    			auto blockAngle = static_cast<float>(convertTogameAngle(whiteToHitPointAngle));
	    			auto blockPower = static_cast<float>((distanceWhiteToHitPoint + distanceTargetToPocket) * POWER_RATIO > 100.0 ? 100.0 :
	    				(distanceWhiteToHitPoint + distanceTargetToPocket) * POWER_RATIO);

	    			Block temp = {target, pocket, blockAngle, blockPower, score};
	    			situationScores.push_back(temp);
	    		}
	    		else {
	    			// target을 칠 수 없는 각도 범위인 경우
	    			printf("Can't Hit to this Pocket(%lf, %lf)\n", pocket.x, pocket.y);

	    			// 방향에 따른 임의 각도 설정
	    			double optionAngle = NOMINAL_ANGLE;
	    			std::vector<double> threadPointAngle = {
	    				(NOMINAL_ANGLE + 45.0 > 360.0 ? 360.0 - NOMINAL_ANGLE + 45.0 : NOMINAL_ANGLE + 45.0),
	    				(NOMINAL_ANGLE + 135.0 > 360.0 ? 360.0 - NOMINAL_ANGLE + 135.0 : NOMINAL_ANGLE + 135.0),
	    				(NOMINAL_ANGLE + 225.0 > 360.0 ? 360.0 - NOMINAL_ANGLE + 225.0 : NOMINAL_ANGLE + 225.0),
	    				(NOMINAL_ANGLE + 315.0 > 360.0 ? 360.0 - NOMINAL_ANGLE + 315.0 : NOMINAL_ANGLE + 315.0)};
	    			double whiteToTargetAngle = angleToCenter(white, target);
	    			if (isPossibleAngle({threadPointAngle[3], threadPointAngle[0]}, whiteToTargetAngle))
	    				optionAngle = threadPointAngle[0] - 45.0;
	    			else if (isPossibleAngle({threadPointAngle[0], threadPointAngle[1]}, whiteToTargetAngle))
	    				optionAngle = threadPointAngle[1] - 45.0;
	    			else if (isPossibleAngle({threadPointAngle[1], threadPointAngle[2]}, whiteToTargetAngle))
	    				optionAngle = threadPointAngle[2] - 45.0;
	    			else if (isPossibleAngle({threadPointAngle[2], threadPointAngle[3]}, whiteToTargetAngle))
	    				optionAngle = threadPointAngle[3] - 45.0;

	    			// 나아가야 하는 각도에 따른 hit 지점 계산하기
	    			Pos hitPoint = getHitPoint(target, optionAngle);
	    			double whiteToHitPointAngle = angleToCenter(white, hitPoint);
	    			printf("Hit Point is (%lf, %lf)\n", hitPoint.x, hitPoint.y);
	    			printf("Hit Angle is %lf\n", whiteToHitPointAngle);

	    			// 각도 차이에 따른 점수 추가
	    			score += abs(whiteToHitPointAngle - optionAngle) * ANGLE_GAP_RATIO;

	    			// 이동거리 계산하기
	    			double distanceWhiteToHitPoint = distanceToCenter(white, hitPoint);

	    			// 이동거리에 따른 점수 추가
	    			score += (distanceWhiteToHitPoint + MAX_WIDTH) * DISTANCE_RATIO;

	    			for (const auto& otherBallIndex: orderList[(order == 1 ? 2 : 1)]) {
	    				Pos otherBall = danggoos[otherBallIndex];
	    				// 없는 공인 경우
	    				if (otherBall.x == -1.0 && otherBall.y == -1.0)
	    					continue;

	    				// 검은 공이 target인데 otherBall이 검은공이 될 수 없으므로
	    				if (otherBallIndex == 5 && targetIndex == 5)
	    					continue;

	    				// white 이동 path 상에 다른 공이 있는 경우 점수 추가
	    				if (isCrossed(white, hitPoint, whiteToHitPointAngle, otherBall))
	    					score += 1 * INTERCEPTION_RATIO;
	    			}

	    			printf("Shoot to (%lf, %lf) (%lf)\n", hitPoint.x, hitPoint.y, whiteToHitPointAngle);
	    			auto blockAngle = static_cast<float>(convertTogameAngle(whiteToHitPointAngle));
	    			auto blockPower = static_cast<float>((distanceWhiteToHitPoint * 2.0) * POWER_RATIO > 100.0 ? 100.0 :
						(distanceWhiteToHitPoint * 2.0) * POWER_RATIO);

	    			Block temp = {target, pocket, blockAngle, blockPower, score};
	    			situationScores.push_back(temp);
	    		}
	    	}
	    }

		// 점수가 낮은 순으로 정렬 하기
		sort(situationScores.begin(), situationScores.end(), blockCompare);

		// 최종 결정
		printf("=======================================\n");
		printf("[Final]Shoot to target:(%lf, %lf), pocket:(%lf, %lf), angle:(%f), power:(%f)\n",
			situationScores[0].target.x, situationScores[0].target.y,
			situationScores[0].pocket.x, situationScores[0].pocket.y,
			situationScores[0].angle, situationScores[0].power);
		printf("=======================================\n");

		angle = situationScores[0].angle;
		power = situationScores[0].power;

		// 주어진 데이터(공의 좌표)를 활용하여 두 개의 값을 최종 결정하고 나면,
		// 나머지 코드에서 일타싸피로 값을 보내 자동으로 플레이를 진행하게 합니다.
		//   - angle: 흰 공을 때려서 보낼 방향(각도)
		//   - power: 흰 공을 때릴 힘의 세기
		//
		// 이 때 주의할 점은 power는 100을 초과할 수 없으며,
		// power = 0인 경우 힘이 제로(0)이므로 아무런 반응이 나타나지 않습니다.
		//
		// 아래는 일타싸피와 통신하는 나머지 부분이므로 수정하면 안됩니다.
		//////////////////////////////

		char mergedData[1024];
		sprintf(mergedData, "%f/%f/", angle, power);
		send(hSocket, (char*)&mergedData, sizeof(mergedData), 0);
		printf("Data Sent: %s\n", mergedData);
	}

	closesocket(hSocket);
	WSACleanup();

	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}