# 일타싸피 서울 21반 전략

## 전략 1 : 공통내접선 이용하기

- White와 Target의 **공통내접선**을 구하고, 그 **공통내접선**의 교점을 통해 White로 Traget을 칠 수 있는 각도 범위가 어떻게 되는지 구한다.
- 즉, Target과 Pocket 간의 각도가 해당 각도 범위 내에 들어가야 한다.

![Strategy01.jpeg](https://github.com/itdice/PocketBallProject/blob/main/Strategy01.jpeg?raw=true)

## 전략 2 : 칠 수 없는 경우 임의의 각도

- 어떤 Pocket도 넣을 수 없는 상황인 경우, **Target을 지정된 각도로 치게 만들어**서 추후에 Pocket에 넣을 수 있는 상황을 기대한다.

## 전략 3 : Target하기 쉬운 공을 선택하기 (Score Base)

- 실제 포켓볼에서 현재 White의 위치를 고려하여 가장 넣기 쉬운 목표 공을 선택하는 것과 같은 원리입니다.
- 목표 공을 성공적으로 넣을 확률을 높이기 위해 다음 세 가지 요소를 고려하여 점수를 산출하였습니다.
    - White와 목표 공 간의 거리와 목표 공과 Pocket 간의 거리
    - White에서 목표 공을 타격하기 위한 각도와 목표 공에서 Pocket으로 향하는 각도의 차이
    - White를 타격한 후 이동 경로에 위치한 상대방 공의 수
- 각 요소의 특성을 반영하여 적절한 계수를 곱해 점수를 산출하고, 최종적으로 점수가 가장 낮은 경우에 해당하는 각도와 힘을 선택하였습니다.

![Strategy02.jpeg](https://github.com/itdice/PocketBallProject/blob/main/Strategy02.jpeg?raw=true)
![Strategy03.jpeg](https://github.com/itdice/PocketBallProject/blob/main/Strategy03.jpeg?raw=true)

## 전략 4 : Power는 생각보다 강하게 치기

- Power를 약하게 하면 정확도가 높아지지만, 공을 넣을 수 있는 상황에 아깝게 못 넣는 경우가 생긴다.
- 또한, 공들의 위치가 이전보다 더 흩어져야만 Target을 넣을 수 있는 확률이 높아지므로 강하게 치는 전략을 선택했다.
