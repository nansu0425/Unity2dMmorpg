# MmorpgServer

## 🧩 프로젝트 개요

MmorpgServer는 C++17 기반의 MMORPG 서버 구현 프로젝트입니다. Windows 환경에서 IOCP 기반 고성능 네트워크 서버를 설계하고, Job Queue 시스템과 멀티스레드 기반의 작업 분리를 통해 실제 온라인 게임 환경과 유사한 서버 구조를 구현하는 것을 목표로 합니다.

이 프로젝트는 다음과 같은 기술 역량을 포트폴리오로 보여주기 위한 목적으로 진행됩니다:

* Windows IOCP 기반 서버 네트워크 구조 설계
* 멀티스레드/비동기 작업 처리 설계 (IO-bound vs CPU-bound 분리)
* C++17을 활용한 구조적 설계 및 현대적 문법 적용
* 향후 데이터베이스, 인증, 게임 콘텐츠까지 포함한 실전 서버 기능 구현

---

## 📁 프로젝트 구조

```bash
MmorpgServer/
├── ServerEngine/     # 공용 네트워크/IO/로그/JobQueue 엔진 모듈
├── GameServer/       # 서버 실행 본체, 메인 루프, 서버 초기화
├── GameContent/      # 게임 컨텐츠 모듈 (Room, Chat 등)
├── DummyClient/      # 테스트용 더미 클라이언트 구현
├── Common/Protocol/  # Protobuf 정의 (패킷 구조 정의)
├── Tools/PacketGenerator/ # 패킷 코드 자동 생성 도구
└── vcpkg.json        # 의존성 패키지 관리
```

---

## 🧪 주요 설계 포인트

* **IOCP 기반 비동기 소켓 처리**: Windows I/O Completion Port를 활용한 고성능 네트워크 처리
* **Job Queue 구조 도입**: IO 작업과 CPU 작업을 명확히 분리하여 스레드 부하를 효율적으로 분산
* **세션 구조**: GameServer와 DummyClient 각각에 독립된 Session 구조 구현
* **패킷 자동화 생성**: `Common/Protocol/*.proto` 정의 후, `Tools/PacketGenerator/GenPacket.bat` 실행 시 핸들러 코드 자동 생성
* **유연한 구조 확장 가능성**: 향후 DB(MySQL/Redis), 인증, 외부 게임 클라이언트(Unity/Unreal) 연동 고려

---

## 🛠️ 사용 기술 스택

* **Language**: C++17
* **Platform**: Windows 10/11
* **Build System**: Visual Studio 2022, CMake, vcpkg
* **Networking**: Windows IOCP
* **Serialization**: Protobuf (`.proto` → 자동 핸들러 생성)
* **(예정)**: MySQL / Redis, spdlog, GoogleTest

---

## 🚀 빌드 및 실행 (예정)

```bash
# Visual Studio 2022에서 MmorpgServer.sln 열기 → 빌드 및 실행
# 또는 CMake 기반 CLI 빌드 지원 예정
```

---

## 🧭 개발 일정 및 상태

| 주차     | 주요 개발 내용                              | 상태     |
| ------ | ------------------------------------- | ------ |
| 1\~2   | 네트워크 엔진(IOCP) 구축, Session 구조화         | ✅ 완료   |
| 3\~4   | JobQueue 도입, 패킷 구조 설계                 | ✅ 완료   |
| 5\~6   | GameContent 모듈 설계 시작 (Room, Player 등) | ⏳ 진행 중 |
| 7\~8   | 더미 클라이언트 테스트 자동화, 부하 테스트              | ⏳ 예정   |
| 9\~12  | DB 연동, 인증, 로깅 도입                      | 🔜 예정  |
| 13\~16 | 게임 콘텐츠 구현, 리팩토링, 문서화                  | 🔜 예정  |

---

## 🔍 강조 포인트

* **IOCP & JobQueue의 분리 설계** → 실전 서버 환경을 고려한 구조
* **패킷 핸들러 자동화 생성** → 생산성 향상 및 일관성 보장
* **레이어 분리 설계** (엔진 / 서버 / 콘텐츠 / 테스트)
* **테스트 자동화 고려** (GoogleTest, DummyClient)
* **게임 콘텐츠 확장 고려** (Room, Chat, Player, NPC 등)
* **데이터베이스 확장성 고려** (MySQL 필수, Redis 옵션)

---

## 📌 TODO

* [ ] Room/Player 기반 게임 콘텐츠 구현 시작
* [ ] GoogleTest 기반 유닛 테스트 도입
* [ ] DummyClient 기반 자동 부하 테스트 스크립트 작성
* [ ] MySQL 연동 기반 로그인/인증 구현
* [ ] Unreal Engine 연동 테스트 (클라이언트 통신)
* [ ] README 최종화 및 배포 문서 작성

---

## 📜 라이선스

본 프로젝트는 학습 및 포트폴리오 목적으로 사용됩니다.

> ⚠️ 본 README는 프로젝트 진행 중 지속적으로 업데이트됩니다. 최신 상태는 커밋 기록과 함께 확인해주세요.
