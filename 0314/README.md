# ✨ 1개월: Unix/리눅스 소개 & GUI 환경

---

## 특징 및 구조

### ▶ Unix/리눅스 환경 기본

* **드래공 없고 간단한 구조**
* **이시성**: C어 버전 구현, 다양한 플랫폼에서 동작
* **개발 개방성**: 소스코드 공개, GNU/Linux 함께 배포
* **동시 다원 사용자 / 프로세스 가능**

### ▶ Unix 시스템 구조

* **여부 옵가**:

  * 크러남(Kernel): 현재 CPU, 메모리, 파일, 해외장치 관리
  * 시스템 호출(System call)
  * 쉘(Shell): 메인 명령을 적용/실행

---

## ✨ 2개월: X 윈도우 및 데스크톡 환경

---

### 2.1 X 윈도우 소개

* \*\*1984- **1984\uub144 MIT에서 개발**, X11…X11R7 (현재 개발 종결)
* **구조**: X 서버 + X 클라이언트
* **특징**:

  * 이시성 가장 높음
  * 다크/버그파이업·계층성 없음
  * 네트워크 투명성 (remote GUI possible)

### 2.2 데스크톡 환경(Desktop Environment)

* **GNOME**, KDE: X 위에 GUI와 아이콘, 창, 키 등 포함
* 원츠 (WM) 가능: TWM, MWM, FVWM, Window Maker

---

### 2.3 Ubuntu 데스크톡

#### GNOME 환경

* **바탕\uc화면** + **사이드바** + **상단바**
* 상단바

  * **Activities**: 하위 프로그램 게이트
  * **Notification Area**: 날짜, 시간, 알림
  * **System Status**: 네트워크, 음료, 전용
* 사이드바: 최근 활성화 프로그램 아이콘 및 그룹

#### 파일/폴더 관리

* 파일 오른쪽 사이드바에서 \[파일 관리자] 클릭
* 바탕\uc화면에서 \[홈 포더] 초과
* 보존 것으로: 파일 연락명, 사이콘 메뉴 관리

### 2.4 gedit 폼지트 텍스트 포트

* **GNOME 기본 폼지 텍스트 포트**
* **파일 작성, 소스 코드, HTML 등에 보유**
* 실행 방법

  * 프로그램에서: \[텍스트 폼지트] 클릭
  * 터미널에서 `$ gedit filename`
  * 파일에서 복사적 클릭

---

### 2.5 CentOS 데스크톡 환경

* 오른쪽 \[홈 포더], 확장은 Ubuntu와 일치
* 각 아이콘에서 복사, 이동, 삭제 가능
* 각 설정는 \[프로그램 표시] → \[설정] 등으로 입입

---

## 해외 기능

* **전체 파일 시스템**: \[다른 위치] 버튼 클릭 → \[커테]
* **쉘 명령**: `sudo`, `apt`, `su`
* **시스템 개인 계정 관리**: 사용자 추가, 삭제, root

---

## ⚡ 헤더

* X 윈도우 시스템 = 해당 화면과 입력을 관리하는 X Server + 프로그램의 검색 가능 구조
* GNOME = Ubuntu / CentOS 공통 GUI 환경
* gedit = GUI 버전 텍스트 폼지트 가사
