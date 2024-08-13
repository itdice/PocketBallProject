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

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

// 닉네임을 사용자에 맞게 변경해 주세요.
#define NICKNAME "SEOUL21_IT_DICE"

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
constexpr double NOMAL_ANGLE = 20.0;

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

		std::vector<Pos> pockets = { {254.0 - CORRECTION_POCKET, 127.0 - CORRECTION_POCKET},
			{254.0 - CORRECTION_POCKET, 0.0 + CORRECTION_POCKET},
			{127.0, 127.0 - CORRECTION_POCKET},
			{127.0, 0.0 + CORRECTION_POCKET},
			{0.0 + CORRECTION_POCKET, 127.0 - CORRECTION_POCKET},
			{0.0 + CORRECTION_POCKET, 0.0 + CORRECTION_POCKET} };

		std::vector<std::vector<int>> orderList = { {0, 0, 0}, {1, 3 , 5}, {2, 4, 5} };
		std::vector<Pos> danggoos(6, { 0.0, 0.0 });
		for (int index = 0; index < 6; index++)
			danggoos[index] = { balls[index][0], balls[index][1] };

		Pos white = danggoos[0];
		for (const auto& targetIndex : orderList[order]) {
			Pos target = danggoos[targetIndex];
			if (target.x == -1.0 && target.y == -1.0)
				continue;

			ROA hitAngle = hittableAngle(white, target);
			printf("hitAngle : %lf to %lf\n", hitAngle.start, hitAngle.end);

			for (const auto& pocket : pockets) {
				double targetToHallAngle = angleToCenter(target, pocket);
				if (isPossibleAngle(hitAngle, targetToHallAngle)) {
					// 목표 홀까지 칠 수 있는 각도인 경우
					Pos hitPoint = getHitPoint(target, targetToHallAngle);
					printf("movementAngle : %lf\n", targetToHallAngle);
					double pointAngle = angleToCenter(white, hitPoint);

					// 보정 과정
					double fractionAngle = abs(targetToHallAngle - pointAngle) / 45.0 * FRACTION_RATIO;
					if (0.0 <= targetToHallAngle && targetToHallAngle < 90.0 ||
						270.0 <= targetToHallAngle && targetToHallAngle < 360.0)
						targetToHallAngle -= fractionAngle;
					else if (90.0 <= targetToHallAngle && targetToHallAngle < 180.0 ||
						180.0 <= targetToHallAngle && targetToHallAngle < 270.0)
						targetToHallAngle += fractionAngle;

					hitPoint = getHitPoint(target, targetToHallAngle);
					pointAngle = angleToCenter(white, hitPoint);

					double distanceToTaget = distanceToCenter(white, hitPoint);
					double distanceToHall = distanceToCenter(target, pocket);

					printf("Shot to (%lf, %lf) (%lf)\n", hitPoint.x, hitPoint.y, pointAngle);

					angle = static_cast<float>(convertTogameAngle(pointAngle));
					power = static_cast<float>((distanceToTaget + distanceToHall) * 0.3 > 100.0 ? 100.0 :
						(distanceToTaget + distanceToHall) * 0.5);

					break;
				}
			}

			if (angle == 0.0f && power == 0.0f) {
				Pos hitPoint = getHitPoint(target, NOMAL_ANGLE);
				printf("movementAngle : %lf\n", NOMAL_ANGLE);
				double pointAngle = angleToCenter(white, hitPoint);

				double distanceToTaget = distanceToCenter(white, hitPoint);

				printf("Shot to (%lf, %lf) (%lf)\n", hitPoint.x, hitPoint.y, pointAngle);

				angle = static_cast<float>(convertTogameAngle(pointAngle));
				power = static_cast<float>((distanceToTaget * 2.0) * 0.3 > 100.0 ? 100.0 :
					(distanceToTaget * 2.0) * 0.5);
			}

			break;
		}

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