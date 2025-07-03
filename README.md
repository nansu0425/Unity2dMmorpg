# Unity2dMmorpg

## 🧩 프로젝트 개요

C++ IOCP 기반 서버와 Unity를 연동하여 Tod-down 스타일 2D Mmorpg를 구현하는 프로젝트입니다. 2025년 4월부터 개발을 시작했습니다. 아직 서버의 구조와 기반을 만드는 중이고, Unity는 추후에 연동 예정입니다.

## 🛠️ 서버 기술 스택

* **Language**: C++17
* **Platform**: Windows 11 64bit
* **IDE**: Visual Studio 2022
* **Package manager**: vcpkg manifest mode(`vcpkg install --triplet x64-windows-static` → 모든 패키지 설치)
* **Serialization**: [protobuf](https://github.com/protocolbuffers/protobuf) (`.proto` → 자동 핸들러 생성)
* **Logging**: [spdlog](https://github.com/gabime/spdlog)
* **Memory allocator**: [mimalloc](https://github.com/microsoft/mimalloc)
* **Lock-free queue**: [concurrentqueue](https://github.com/cameron314/concurrentqueue)

---

## 🚀 개발 관련 포스트

* [패킷 코드 자동 생성 스크립트 제작](https://nansu0425.oopy.io/database/unity-2d-mmorpg/%ED%8C%A8%ED%82%B7-%EC%BD%94%EB%93%9C-%EC%9E%90%EB%8F%99-%EC%83%9D%EC%84%B1-%EC%8A%A4%ED%81%AC%EB%A6%BD%ED%8A%B8-%EC%A0%9C%EC%9E%91)
* [1000개의 player가 있는 room에 일정 주기로 broadcast](https://nansu0425.oopy.io/database/unity-2d-mmorpg/room-broadcast)

---

## 📜 라이선스

본 프로젝트는 학습 및 포트폴리오 목적으로 사용됩니다.

> ⚠️ 본 README는 프로젝트 진행 중 지속적으로 업데이트됩니다. 최신 상태는 커밋 기록과 함께 확인해주세요.

---
