# 2인 협동 멀티플레이 플랫포머

다른 행동을 하는 두 캐릭터의 협동을 통해 퍼즐을 푸는 수직 등반 플랫포머.

| | |
|---|---|
| 개발 기간 | 2025.08 – 진행중 |
| 엔진 · 언어 | Unreal Engine 5.6 · C++ (블루프린트는 에디터 노출과 에셋 연결에만 사용) |
| 인원 | 2인 — 프로그래밍 · 애니메이션 전 영역 / 에셋 · 맵 디자인 1인 |
| 상태 | 게임 로직과 멀티플레이 구현 완료, 맵 · 퍼즐 기믹 구성 중 |

---

## 1. Steam 세션 관리와 리슨 서버

2인 규모에 맞춰 데디케이티드 서버 대신 리슨 서버 채택.

- 세션 생성 · ID 기반 검색 · 참가 · 파기를 게임 인스턴스 서브시스템으로 분리 → 레벨이 바뀌어도 세션 상태 유지
- 플레이어 역할 관리는 GameState에 배치 → 복제와 UI 갱신이 자동으로 따라옴
- 판정과 물리는 서버 권한, 애니메이션과 이펙트는 멀티캐스트
- 호스트가 세션 파기 시 로컬 컨트롤러 여부로 호스트/클라이언트를 구분해 원격 참가자만 내보냄

```
Source/FCJ/Subsystem/MultiSessionSubsystem.h / .cpp   세션 생성 · 검색 · 참가 · 파기
Source/FCJ/GameMode/LobbyGameState.h / .cpp           역할 정보 복제 · 교체
Source/FCJ/GameMode/MultiGameMode.h / .cpp            인게임 진행
Source/FCJ/PlayerController/MultiPlayerController.cpp
```

## 2. 델리게이트 기반 트리거 아키텍처

트리거와 반응 액터를 분리. 트리거가 상태 변화를 브로드캐스트하면 문 · 포탑 같은 반응 액터가 구독.

- Tick 폴링 없음 → 기믹이 늘어도 매 프레임 비용이 늘지 않음
- 상태 변화에 프레임 지연 없이 반응
- 상태 변경은 서버 권한, 복제 알림으로 클라이언트 시각 피드백
- 문이 영구히 열리는 경우처럼 한 번만 필요한 이벤트는 구독 해제

구현된 기믹 — 사라지는 발판(단발 · 연속 · 쌍 연동), 이동 · 회전 발판, 벽 점프 지점, 스프링 함정, 들어 옮기는 물체, 궤적 3종 투사체와 스폰 볼륨, 포탑, 문 4종. 모두 같은 규약을 따르므로 레벨 디자인 단계에서 조합만으로 새 퍼즐 구성 가능.

```
Source/FCJ/Actor/Triggers/          BaseTrigger + 밟기 · 물체 올리기 · 투사체 통과 · 방문 기록
Source/FCJ/Actor/TriggeredActors/   문 4종 · 포탑
Source/FCJ/Actor/Objects/           발판 기믹 · 스프링 함정 · 들어 옮기는 물체 · 투사체
Source/FCJ/Actor/Volumes/           투사체 스폰 · 디버프 · 리스폰 볼륨
```

## 3. 구역 기반 협동 규칙

한 명이 앞서 나가면 다른 한 명이 할 일이 없어지는 문제를 규칙으로 차단.

- 맵을 번호가 붙은 구역으로 분할
- 두 캐릭터의 거리가 벌어지면 앞선 쪽을 낮은 번호 구역으로 되돌림
- 각 구역이 내부의 클리어 조건을 모아 완료 여부를 스스로 판정. 상태는 복제되어 양쪽 화면에 동일 반영

```
Source/FCJ/Actor/Volumes/ZoneVolume.h / .cpp
```

## 5. 그외

```
Source/FCJ/PlayerCharacter/CatBase.h / .cpp        파쿠르 · 월 점프 · 카메라 · 특수 행동 기반 클래스
Source/FCJ/PlayerCharacter/AttackCat.cpp           패링 — 투사체 반사 · 물체 밀기
Source/FCJ/PlayerCharacter/BiteCat.cpp             차징 투척 — 서버 권한 차지 누적
Source/FCJ/Animation/ParryingNotifyState.cpp       애니메이션 구간 기반 판정
```
