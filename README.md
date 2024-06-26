# 언리얼 엔진 5 게임 클라이언트 연동 IOCP 서버
## 🎮 프로젝트 소개
언리얼 엔진 5로 개발한 게임 클라이언트와 연동하여 동작하는 IOCP 서버 입니다.
## 🗓 개발 기간
24.02.01 ~ 24.06.14
## 🖥 개발 환경 및 사용 언어
>Visual Studio 2022

>MSSQL

>C++
## 🕹 주요 클래스 소개
### IOCPServer - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/IOCPServer-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- DBConnector 객체 생성 및 초기화
- 서버 동작을 위한 소켓 생성 및 초기화
- 클라이언트 연결을 수락하는 어셉터 스레드 생성
- 클라이언트가 전송한 패킷을 수신하여 처리하는 워커 스레드 생성
### GameServer (IOCPServer 클래스를 상속) - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/GameServer-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 좀비 관련 동작을 처리하는 워커 스레드 생성
- 패킷을 타입에 따라 처리하는 함수 정의
- 플레이어, 좀비, 아이템과 같은 게임 내 모든 객체의 동작과 정보를 처리
- 처리한 데이터를 클라이언트들에 전송하여 동기화하거나 DB에 저장
### Character - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/Character-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 플레이어, 좀비와 같은 캐릭터들의 고유번호, 위치, 회전 값을 보유
- 해당 정보 값의 Getter, Setter 함수 제공 및 직렬화, 역직렬화 함수 제공
### Player (Character 클래스를 상속) - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/Player-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 클라이언트가 조종하는 캐릭터의 정보를 저장하고 인벤토리 연산을 수행
- 좀비와의 상호작용 상태를 갱신하고 관련 정보 제공
- 아이템 획득, 버리기, 장착, 장착 해제 동작의 결과를 저장하고 반환함
### Zombie (Character 클래스를 상속) - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/Zombie-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 게임 내 존재하는 좀비 캐릭터의 상태와 위치를 갱신하고 저장
- ZombieState라는 정적 클래스 객체로 주요 행동 분기를 결정하고 동작을 수행함
- 가장 가까운 플레이어를 찾고 각종 State 객체들이 호출하는 함수를 제공
### ZombieState - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/ZombieState-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 스테이트 패턴을 사용하여 구현한 추상 클래스 구조
- ChangeState(), Update(), GetStateEnum() 3개의 가상 함수를 제공
- ChangeState() : 좀비의 스테이트를 전환할 때 호출되는 함수로, 각 스테이트 객체는 다른 스테이트 객체로 전환할 자신만의 로직을 구현함
- Update() : 좀비 워커 스레드에서 매 루프 마다 호출되는 함수로, 각 스테이트가 갱신시 Zombie 클래스에서 호출해야 하는 내용을 구현함
- GetStateEnum() : 자신의 상태를 의미하는 Enum 값을 반환
### PathManager - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/PathManager-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- Pathfinder로 부터 좀비의 이동 경로를 받아와 관리하고 이동을 진행함
- 경로를 다시 계산해야 하는지 검증하고 경로가 짧으면 상태를 전환하도록 함
- 경로 배열에서 다음 위치 값을 만들고 좀비 객체에 전달하여 이동하도록 함
### Pathfinder - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/Pathfinder-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 좀비의 경로를 생성하는 정적 클래스
- 지형 데이터를 읽어와서 A* 알고리즘을 통해 경로를 생성하고 반환함
### ItemManager - [자세히 보기](https://github.com/hyk3633/IOCPServer/wiki/ItemManager-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 게임 내에 존재하는 아이템을 생성하고 관리
- itemID에 해당하는 아이템 구조체를 반환하는 함수 제공
- 플레이어가 획득하는 아이템들을 다른 플레이어가 획득할 수 없는 상태로 비활성화 처리
- 아이템 사용에 따른 처리 함수 제공
