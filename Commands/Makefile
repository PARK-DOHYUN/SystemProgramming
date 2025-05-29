# 컴파일 설정
CC = gcc
CFLAGS = -Wall -g -D_GNU_SOURCE

# FNM_CASEFOLD 지원 여부 확인
CASEFOLD_SUPPORT := $(shell echo '\#include <fnmatch.h>' | $(CC) -E -dM - 2>/dev/null | grep -q 'FNM_CASEFOLD' && echo "yes" || echo "no")
ifeq ($(CASEFOLD_SUPPORT),yes)
    CFLAGS += -DHAVE_FNM_CASEFOLD=1
else
    CFLAGS += -DHAVE_FNM_CASEFOLD=0
endif

# 컴파일 정보 출력
$(info 컴파일러: $(CC))
$(info 플래그: $(CFLAGS))
$(info FNM_CASEFOLD 지원: $(CASEFOLD_SUPPORT))

# 특별한 타겟들
.PHONY: clean help

# help 타겟: 사용법 안내 (기본 타겟)
help:
	@echo "=== Linux Commands Makefile 사용법 ==="
	@echo ""
	@echo "컴파일:"
	@echo "  make build TARGET=실행파일명 CMD=명령어명"
	@echo "  예시: make build TARGET=ls_run CMD=ls"
	@echo "  예시: make build TARGET=cat_program CMD=cat"
	@echo "  예시: make build TARGET=my_touch CMD=touch"
	@echo ""
	@echo "또는 간단한 형식:"
	@echo "  make 실행파일명.exe 명령어명"
	@echo "  예시: make ls_run.exe ls"
	@echo ""
	@echo "정리:"
	@echo "  make clean        # 모든 .o 파일과 실행파일 삭제"
	@echo ""
	@echo "도움말:"
	@echo "  make help         # 이 도움말 표시"

# 메인 빌드 타겟
build:
	@if [ -z "$(TARGET)" ] || [ -z "$(CMD)" ]; then \
		echo "사용법: make build TARGET=실행파일명 CMD=명령어명"; \
		echo "예시: make build TARGET=ls_run CMD=ls"; \
		exit 1; \
	fi
	@if [ ! -d "$(CMD)" ]; then \
		echo "오류: 디렉토리 '$(CMD)'가 존재하지 않습니다."; \
		exit 1; \
	fi
	@echo "명령어 '$(CMD)'를 컴파일하여 실행파일 '$(TARGET)' 생성 중..."
	@C_FILES=$$(find $(CMD) -name "*.c" -type f); \
	if [ -z "$$C_FILES" ]; then \
		echo "오류: $(CMD) 디렉토리에 .c 파일이 없습니다."; \
		exit 1; \
	fi; \
	OBJ_FILES=""; \
	for c_file in $$C_FILES; do \
		obj_file=$$(echo $$c_file | sed 's/\.c$$/\.o/'); \
		OBJ_FILES="$$OBJ_FILES $$obj_file"; \
		echo "컴파일: $$c_file -> $$obj_file"; \
		$(CC) $(CFLAGS) -c $$c_file -o $$obj_file; \
		if [ $$? -ne 0 ]; then \
			echo "오류: $$c_file 컴파일 실패"; \
			exit 1; \
		fi; \
	done; \
	echo "링킹: $$OBJ_FILES -> $(TARGET)"; \
	$(CC) $(CFLAGS) $$OBJ_FILES -o $(TARGET); \
	if [ $$? -eq 0 ]; then \
		echo "성공: 실행파일 '$(TARGET)' 생성 완료"; \
	else \
		echo "오류: 링킹 실패"; \
		exit 1; \
	fi

# 패턴 규칙: *.exe 형식으로 편리하게 사용
%.exe:
	@if [ "$(words $(MAKECMDGOALS))" -ne 2 ]; then \
		echo "사용법: make 실행파일명.exe 명령어명"; \
		echo "예시: make ls_run.exe ls"; \
		exit 1; \
	fi
	@CMD_NAME="$(word 2,$(MAKECMDGOALS))"; \
	TARGET_NAME="$(basename $@)"; \
	if [ ! -d "$$CMD_NAME" ]; then \
		echo "오류: 디렉토리 '$$CMD_NAME'가 존재하지 않습니다."; \
		exit 1; \
	fi; \
	echo "명령어 '$$CMD_NAME'를 컴파일하여 실행파일 '$$TARGET_NAME' 생성 중..."; \
	C_FILES=$$(find $$CMD_NAME -name "*.c" -type f); \
	if [ -z "$$C_FILES" ]; then \
		echo "오류: $$CMD_NAME 디렉토리에 .c 파일이 없습니다."; \
		exit 1; \
	fi; \
	OBJ_FILES=""; \
	for c_file in $$C_FILES; do \
		obj_file=$$(echo $$c_file | sed 's/\.c$$/\.o/'); \
		OBJ_FILES="$$OBJ_FILES $$obj_file"; \
		echo "컴파일: $$c_file -> $$obj_file"; \
		$(CC) $(CFLAGS) -c $$c_file -o $$obj_file; \
		if [ $$? -ne 0 ]; then \
			echo "오류: $$c_file 컴파일 실패"; \
			exit 1; \
		fi; \
	done; \
	echo "링킹: $$OBJ_FILES -> $$TARGET_NAME"; \
	$(CC) $(CFLAGS) $$OBJ_FILES -o $$TARGET_NAME; \
	if [ $$? -eq 0 ]; then \
		echo "성공: 실행파일 '$$TARGET_NAME' 생성 완료"; \
	else \
		echo "오류: 링킹 실패"; \
		exit 1; \
	fi

# 두 번째 인자를 무시하는 더미 타겟
%:
	@if echo "$@" | grep -q "\.exe$$"; then \
		exit 0; \
	fi
	@if [ "$@" != "clean" ] && [ "$@" != "help" ] && [ "$@" != "build" ]; then \
		echo "알 수 없는 타겟: $@"; \
		echo "사용법을 보려면 'make help'를 실행하세요."; \
	fi

# clean 타겟: 모든 오브젝트 파일과 실행파일 삭제
clean:
	@echo "정리 중..."
	@find . -name "*.o" -type f -delete 2>/dev/null || true
	@find . -maxdepth 1 -type f -executable -not -name "Makefile" -not -name "*.sh" -not -name ".*" -delete 2>/dev/null || true
	@echo "정리 완료: .o 파일과 실행파일들을 삭제했습니다."

# 기본 타겟 설정
.DEFAULT_GOAL := help