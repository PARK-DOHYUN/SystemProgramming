
## 📌프로세스

### ✅ 프로세스란?
- 실행 중인 프로그램을 의미하며, 각 프로세스는 고유한 **PID(프로세스 ID)**를 가진다.
- 모든 프로세스는 **부모 프로세스**에 의해 생성된다.
- 📂 종류:
  - **시스템 프로세스**: 부팅 시 생성되며 백그라운드 서비스 수행
  - **사용자 프로세스**: 사용자 명령에 의해 실행됨

### 🔍 프로세스 상태 확인
- `ps`, `pgrep`, `top` 등 사용
```bash
$ ps -ef | more         # 전체 프로세스 상세 보기
$ pgrep -l sshd         # 'sshd' 관련 프로세스 확인
```

### ⚙️ 작업 제어 명령
- `&`, `fg`, `bg`, `jobs`로 작업 전환 제어
- `Ctrl+Z`: 실행 중인 작업 정지, `fg`: 전면 실행, `bg`: 백그라운드 전환

### 🔫 프로세스 제어
```bash
$ kill PID              # PID 프로세스 종료
$ wait PID              # 특정 PID 종료까지 대기
$ nice -n 10 CMD        # CMD 우선순위 조정
$ renice -n -5 -p PID   # 실행 중 프로세스 우선순위 변경
```

### ⭐ 사용자 ID와 그룹 ID
- `id`, `whoami`로 현재 사용자의 UID, GID 확인 가능

---

## 📌 인터넷과 서버

### 🌐 네트워크 기본
- **LAN**: 근거리 통신망
- **라우터**: 네트워크 간 데이터 전달
- **게이트웨이**: 외부망 연결

### 📡 IP와 도메인
- `ip addr`, `hostname`, `nslookup` 사용

### 💻 서버 설치
```bash
# apt install apache2 php mariadb-server     # APM 설치
# systemctl start apache2                    # 서비스 실행
# firewall-cmd --add-service=http --permanent
# firewall-cmd --reload
```

### 📤 FTP / SSH
```bash
# apt install vsftpd ssh
$ sftp user@host          # 안전한 파일 전송
$ ftp host                # 일반 FTP 접속
```

---

## 📌 파일 유틸리티

### 🔎 find 명령어
```bash
$ find . -name "*.c" -print
$ find / -user chang -exec rm {} \;
```

### 📑 grep 필터링
```bash
$ grep -n "with" file.txt      # 라인번호와 함께 검색
$ grep -v "except" file.txt    # 해당 문자열 제외 출력
```

### 🔠 정렬 sort
```bash
$ sort filename.txt
$ sort -r filename.txt        # 내림차순
$ sort -k 2 filename.txt      # 두 번째 필드 기준 정렬
```

---

## 📌 유틸리티

### 🕒 작업 스케줄링 cron
- `crontab` 명령 사용: 특정 시간마다 명령 실행
```bash
$ crontab -e                  # 편집
30 18 * * * rm ~/tmp/*        # 매일 18:30에 파일 삭제
```

### 📅 at: 단발성 예약 실행
```bash
$ at 1145 jan 31
at> echo "hi" > log.txt
at> <EOT>
```

### 💾 디스크 사용량
```bash
$ df -h      # 파일 시스템별 용량 확인
$ du -sh *   # 각 디렉터리 용량 요약
```

### 📦 파일 압축
```bash
$ tar -cvf files.tar *        # 파일 묶기
$ gzip files.tar              # 압축
$ gunzip files.tar.gz         # 압축 해제
$ tar -xvf files.tar          # 압축 풀기
```

### 📊 AWK
- 패턴과 필드를 이용한 텍스트 처리 언어
```bash
$ awk '{ print $1, $3 }' file.txt
```

---

## 📌 Bash 쉘 스크립트

### 📜 쉘 설정 파일
- `/etc/profile`, `~/.bashrc`, `~/.bash_profile` 등

### 📌 변수
```bash
$ name=홍길동
$ echo $name
```

### 🧠 환경 변수 vs 지역 변수
```bash
$ export name=홍길동    # 자식 프로세스에 전달
```

### 📃 리스트 변수
```bash
$ arr=(서울 부산 대구)
$ echo ${arr[1]}         # 부산
$ echo ${#arr[@]}        # 리스트 크기
```

### 📥 입력
```bash
$ read name
홍길동
$ echo $name
```

### 🧮 조건문/반복문
```bash
if [ $a -eq 1 ]; then
  echo "1입니다"
else
  echo "1이 아닙니다"
fi
```

### 🔁 반복문
```bash
for i in 1 2 3; do echo $i; done
```

---

## 📌 프로그래밍 환경

### ✍️ gedit
- GUI 편집기, 다양한 프로그래밍 언어 지원

### 🛠️ 컴파일
```bash
$ gcc -c main.c
$ gcc -o main main.o copy.o
```

### ⚙️ 메이크파일
```make
main: main.o copy.o
	gcc -o main main.o copy.o
main.o: main.c copy.h
	gcc -c main.c
copy.o: copy.c
	gcc -c copy.c
```

### 🐞 gdb 디버거
```bash
$ gcc -g main.c -o main
$ gdb main
(gdb) b main
(gdb) r
(gdb) n
(gdb) p 변수명
```

---

## 📌 파일 시스템과 파일 입출력

### 📂 파일 시스템 구조
- 부트 블록, 슈퍼 블록, i-노드, 데이터 블록 등으로 구성
- `df`, `du` 명령으로 디스크 정보 확인

### 🧾 i-노드
- 파일에 대한 상태정보 저장: 소유자, 크기, 블록 수, 시간 등
```bash
$ stat 파일명
```

### 🗂️ 디렉터리
- 디렉터리도 i-노드를 가지며 파일 이름과 i-노드 번호 저장
- `struct dirent` 구조체로 구성

### 🔗 링크
- **하드 링크**: 동일한 i-노드 번호 공유
- **심볼릭 링크**: 경로를 가리키는 특수 파일
```bash
$ ln 원본 새이름          # 하드링크
$ ln -s 원본 새이름       # 심볼릭링크
```

### 📄 파일 입출력
- `open()`, `read()`, `write()`, `close()` 시스템 호출 사용

