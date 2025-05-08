.PHONY: help build test 

## 이 단어들을 명령어 이름으로 간주하겠다는 선언, help라는 파일이 있어도 무시하고 help라는 행동을 실행함.




help:
# http://marmelab.com/blog/2016/02/29/auto-documented-makefile.html
	@grep -E '^[a-zA-Z0-9_%/-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

build:
build: ## Build executables
	$(MAKE) -C src

## ##로 시작하는 설명 코멘트를 찾아 출력해주는 기능임.

## -C 는 터미널에서 디렉토리를 바꾸는 cd 명령어와 같음. 
## $(MAKE) 이 곳에서 make 명령어를 실행하라는 말

## 터미널에 아래처럼 입력한 것과 같음,
## cd src
## make	

test:
test: ## Test rbtree implementation
	$(MAKE) -C test test
	
clean:
clean: ## Clear build environment
	$(MAKE) -C src clean
	$(MAKE) -C test clean
