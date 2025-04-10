# 파일 사용 정보 (UNIX/Linux)

## 정보 개요
- 파일 가져기, 이동, 삭제, 링크 생성, 속성 확인과 변경, 가상 가능

---

## 4.1 파일 복사 (cp)

### 사용법
```bash
cp [과정형] 파일1 파일2
cp [과정형] 파일1 파일2 ... 디렉터리
```
- `-i`: 대화형(interactive) 옵션, 결정 전 확인 묻기
- `-r`: 디렉터리를 포함한 복사

### 예
```bash
cp cs1.txt cs2.txt
cp -i cs1.txt cs2.txt
cp cs1.txt /tmp
cp cs1.txt cs2.txt /tmp
cp -r dir1 dir2
```

---

## 4.2 파일 이동 (mv)

### 사용법
```bash
mv [과정형] 파일1 파일2
mv [과정형] 파일1 ... 디렉터리
```
- `-i`: 대화형 옵션

### 디렉터리 이름 변경
```bash
mv olddir newdir
```

### 예
```bash
mv cs2.txt cs3.txt
mv -i cs1.txt cs3.txt
mv cs1.txt cs3.txt /tmp
```

---

## 4.3 파일 삭제 (rm)

### 사용법
```bash
rm [과정형] 파일...
```
- `-i`: 대화형 옵션
- `-r`: 디렉터리를 범위로 (전체 삭제)

### 예
```bash
rm cs1.txt
rm -i cs1.txt
rm -ri test
```

---

## 4.4 링크 (Link)

### 파일에 대한 새로운 이름 제공
```bash
ln [과정형] 파일1 파일2
ln [과정형] 파일1 디렉터리
```
- `-s`: 심볼리크 링크 (symbolic link)

### 파일 종류
- 하드 링크: 기존 파일과 동단한 i-node 가리키기
- 심볼리크: 기존 파일을 가리키는 경로 정보의 특수 파일

### 예
```bash
ln hello.txt hi.txt     # hard link
ln -s hello.txt hi.txt  # symbolic link
```

---

## 4.5 파일 속성 (Attributes)

### `ls -l` 데이터 및 의미
| 가능 | 설명 |
|--------|------|
| 파일 종류 | - (일반), d (디렉터리), l (링크), b/c (디바이스), p (FIFO), s (소켓) |
| 접근 권한 | rwx 의 조합 / 소유자, 그룹, 기타 |
| 파일 크기 | 바이트 단위 |
| 수정 시간 | 마지막 수정 번실 |

### 파일 종류 확인
```bash
file 파일명
```

---

## 4.6 접근권한 (Permission)

### 권한 확인
```bash
ls -l
```

### 권한 구조
- rwxrwxrwx : 소유자 / 그룹 / 기타
- r: read, w: write, x: execute

### chmod 사용
```bash
chmod [옵션] 권한 파일
chmod -R 권한 디렉터리
chmod 755 file
chmod g+w,o+rw file
```
- 8진수 표기: 777, 755, 644 등
- 기호 표기: u/g/o/a 와 + / - / = 그리고 r/w/x

---

## 4.7 기타 파일 속성 변경

### 소유자 변경 (chown)
```bash
chown 사용자 파일
chown -R 사용자 디렉터리
```

### 그룹 변경 (chgrp)
```bash
chgrp 그룹 파일
chgrp -R 그룹 디렉터리
```

### 수정 시간 변경 (touch)
```bash
touch 파일
```

---

## 해석 기술
- 링크: 기존 파일에 대한 다른 이름
- 파일은 이름 이외에도 종류, 크기, 소유자, 접근권한, 수정시간 등 속성 가지고 있음
- 접근권한은 소유자, 그룹, 기타로 구분


