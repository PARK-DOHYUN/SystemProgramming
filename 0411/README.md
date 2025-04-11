# 🖥️ 제5장 쉘과 명령어 사용

---

## 📌 5.1 쉘 소개

### 🧠 쉘(Shell)이란?
- 사용자와 운영체제 사이의 창구 역할
- 명령어 처리기(Command Processor)
- 사용자 명령어를 해석하고 실행함

### 🧰 쉘의 종류
| 종류 | 실행 파일 |
|------|-------------|
| 본 쉘 (Bourne Shell) | `/bin/sh` |
| 콘 쉘 (Korn Shell) | `/bin/ksh` |
| C 쉘 (C Shell) | `/bin/csh` |
| Bash 쉘 | `/bin/bash` |
| tcsh 쉘 | `/bin/tcsh` |

**특징 요약**
- **본 쉘**: 유닉스 기본 쉘, Stephen Bourne 개발
- **콘 쉘**: 본 쉘 확장판
- **Bash**: GNU에서 개발한 가장 널리 사용되는 쉘
- **C 쉘**: C 언어 스타일 문법, BSD 계열에 적합

### 🔐 로그인 쉘
- 로그인 시 자동 실행
- `/etc/passwd` 파일에서 확인 가능
- 쉘 변경: `$ csh`, 로그인 쉘 변경: `$ chsh`

---

## ⚙️ 5.2 쉘의 기능

### ✅ 주요 기능
- 명령어 처리 및 실행
- 사용자 환경 설정 (시작 파일)
- 스크립트 기능 지원

### 🌐 환경 변수
```bash
$ TERM=xterm
$ echo $TERM
xterm
```
- 환경 변수 확인: `$ env`
- 사용자 정의 변수:
```bash
$ MESSAGE=hello
$ export MESSAGE
```

### 📂 시작 파일 예시
`~/.bash_profile`
```bash
PATH=$PATH:/usr/local/bin:/etc:.
TERM=xterm
export PATH TERM
stty erase ^?
echo $USER, Welcome to Linux!
```
- 적용 방법: `$ . .bash_profile`

---

## 🧵 5.3 전면 처리와 후면 처리

### ⏳ 전면 처리
```bash
$ 명령어
```
- 명령어 실행 후 결과가 나올 때까지 대기

### 🔄 후면 처리
```bash
$ 명령어 &
```
- 동시에 여러 작업 수행 가능

#### 예시
```bash
$ (sleep 100; echo done) &
$ find . -name test.c -print &
```

### 🔍 후면 작업 관리
```bash
$ jobs      # 후면 작업 목록 보기
$ fg %1     # 특정 작업 전면 전환
```

---

## 📤 5.4 입출력 재지정

### 📄 출력 재지정
```bash
$ ls -asl > ls.txt
```

### ✏️ 파일 직접 작성
```bash
$ cat > list1.txt
Hi !
This is the first list.
^D
```

### 📎 파일 합치기
```bash
$ cat list1.txt list2.txt > list3.txt
```

### ➕ 출력 추가
```bash
$ date >> list1.txt
```

### 📥 입력 재지정
```bash
$ wc < list1.txt
```

### 📝 Here Document
```bash
$ wc << END
hello !
word count
END
```

### 🚫 오류 재지정
```bash
$ ls -l /bin/usr 2> err.txt
```

### 🔗 파이프
```bash
$ ls | sort -r
$ who | wc -l
$ ls 디렉터리 | wc -w
```

---

## 🧩 5.5 여러 개 명령어 실행

### 🔗 명령어 열
```bash
$ 명령어1; 명령어2; 명령어3
```
- 순차적으로 실행

### 📦 명령어 그룹
```bash
$ (명령어1; 명령어2; 명령어3) > out.txt
```

### ✅ 조건 명령어
```bash
$ 명령어1 && 명령어2   # 명령어1 성공 시 명령어2 실행
$ 명령어1 || 명령어2   # 명령어1 실패 시 명령어2 실행
```

---

## 🧮 5.6 파일 이름 및 명령어 대치

### 📁 파일 이름 대치 (Globbing)
```bash
$ gcc *.c
$ ls [ac]*
```
| 기호 | 의미 |
|------|------|
| `*` | 0개 이상의 문자 |
| `?` | 1개의 문자 |
| `[abc]` | 괄호 안의 문자 중 하나 |

### 🔁 명령어 대치 (Command Substitution)
```bash
$ echo 현재 시간은 `date`
$ echo 파일 개수: `ls | wc -w`
```

### 🗨️ 따옴표 사용법
```bash
$ echo "3 * 4 = 12"
$ echo '3 * 4 = 12'
```
- `'작은따옴표'`: 모든 대치 기능 제한
- `"큰따옴표"`: 파일 이름 대치만 제한 (변수/명령어 허용)

---

## 📜 5.7 쉘 스크립팅 기초

### 📂 스크립트 파일 만들기
```bash
#!/bin/bash

echo "안녕하세요, 쉘 스크립트입니다."
```
- 파일 저장 후 실행 권한 부여:
```bash
$ chmod +x script.sh
$ ./script.sh
```

### 🔄 반복문 (for / while)
**for 반복문**
```bash
for i in 1 2 3
  do
    echo "숫자: $i"
  done
```

**while 반복문**
```bash
count=1
while [ $count -le 3 ]
  do
    echo "횟수: $count"
    count=$((count+1))
  done
```

### 🧭 조건문 (if)
```bash
if [ $USER = "root" ]; then
  echo "관리자입니다."
else
  echo "일반 사용자입니다."
fi
```

- 비교 연산자 예:
  - `-eq`, `-ne`, `-gt`, `-lt`, `-ge`, `-le`
  - 문자열 비교: `=`, `!=`, `-z`, `-n`

---

## 📚 핵심 요약

✅ 쉘은 사용자 명령어 처리기이며 환경 설정, 스크립트, 명령어 실행 등의 기능을 제공함.

✅ 입출력 재지정을 통해 파일을 활용한 작업이 가능함.

✅ 파이프를 통해 명령어 간 데이터를 직접 전달할 수 있음.

✅ 명령어를 조건부 또는 그룹으로 실행 가능하며, 다양한 대치 기능으로 효율적인 작업 수행이 가능함.

✅ 쉘 스크립트를 통해 반복 및 조건 로직을 자동화할 수 있음.

