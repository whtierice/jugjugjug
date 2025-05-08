# 📘 Docker + VSCode DevContainer 기반 C 개발 환경 구축 가이드 (WebProxyLab)

이 문서는 **Windows**와 **macOS** 사용자가 Docker와 VSCode DevContainer 기능을 활용하여 C 개발 및 디버깅 환경을 빠르게 구축할 수 있도록 도와줍니다.

[**주의**] 지난 주차와 다른 점만 하시려면 4장부터 7장만 보세요.
[**주의**] webproxy-lab의 경우 tidy 웹 서버와 proxy 서버 두가지를 구현해야 해서 두가지 debugging 설정을 제공합니다. 이에 대한 설명은 7장에서 설명하니 꼭 읽어보시기 바랍니다.

---

## 1. Docker란 무엇인가요?

**Docker**는 애플리케이션을 어떤 컴퓨터에서든 **동일한 환경에서 실행**할 수 있게 도와주는 **가상화 플랫폼**입니다.  

Docker는 다음 구성요소로 이루어져 있습니다:

- **Docker Engine**: 컨테이너를 실행하는 핵심 서비스
- **Docker Image**: 컨테이너 생성에 사용되는 템플릿 (레시피 📃)
- **Docker Container**: 이미지를 기반으로 생성된 실제 실행 환경 (요리 🍜)

### ✅ AWS EC2와의 차이점

| 구분 | EC2 같은 VM | Docker 컨테이너 |
|------|-------------|-----------------|
| 실행 단위 | OS 포함 전체 | 애플리케이션 단위 |
| 실행 속도 | 느림 (수십 초 이상) | 매우 빠름 (거의 즉시) |
| 리소스 사용 | 무거움 | 가벼움 |

---

## 2. VSCode DevContainer란 무엇인가요?

**DevContainer**는 VSCode에서 Docker 컨테이너를 **개발 환경**처럼 사용할 수 있게 해주는 기능입니다.

- 코드를 실행하거나 디버깅할 때 **컨테이너 내부 환경에서 동작**
- 팀원 간 **환경 차이 없이 동일한 개발 환경 구성** 가능
- `.devcontainer` 폴더에 정의된 설정을 VSCode가 읽어 자동 구성

---

## 3. Docker Desktop 설치하기

1. Docker 공식 사이트에서 설치 파일 다운로드:  
   👉 [https://www.docker.com/products/docker-desktop](https://www.docker.com/products/docker-desktop)

2. 설치 후 Docker Desktop 실행  
   - Windows: Docker 아이콘이 트레이에 떠야 함  
   - macOS: 상단 메뉴바에 Docker 아이콘 확인

---

## 4. 프로젝트 파일 다운로드 (히스토리 없이)

터미널(CMD, PowerShell, zsh 등)에서 아래 명령어로 프로젝트 폴더만 내려받습니다:

```bash
git clone --depth=1 https://github.com/krafton-jungle/webproxy_lab_docker.git
```

- `--depth=1` 옵션은 git commit 히스토리를 생략하고 **최신 파일만 가져옵니다.**

### 📂 다운로드 후 폴더 구조 설명

```
webproxy_lab_docker/
├── .devcontainer/
│   ├── devcontainer.json      # VSCode에서 컨테이너 환경 설정
│   └── Dockerfile             # C 개발 환경 이미지 정의
│
├── .vscode/
│   ├── launch.json            # 디버깅 설정 (F5 실행용)
│   └── tasks.json             # 컴파일 자동화 설정
│
├── webproxy-lab
│   ├── tidy                    # tidy 웹 서버 구현 폴더
│   │  ├── cgi-bin              # tidy 웹 서버를 테스트하기 위한 동적 컨텐츠를 구현하기 위한 폴더
│   │  ├── home.html            # tidy 웹 서버를 테스트하기 위한 정적 HTML 파일
│   │  ├── tidy.c               # tidy 웹 서버 구현 파일
│   │  └── Makefile             # tidy 웹 서버를 컴파일하기 위한 파일
│   ├── Makefile                # proxy 웹 서버를 컴파일하기 위한 파일
│   └── proxy.c                 # proxy 웹 서버 구현 파일
│
└── README.md  # 설치 및 사용법 설명 문서
```
---

## 5. VSCode에서 해당 프로젝트 폴더 열기

1. VSCode를 실행
2. `파일 → 폴더 열기`로 방금 클론한 `webproxy_lab_docker` 폴더를 선택

---

## 6. 개발 컨테이너: 컨테이너에서 열기

1. VSCode에서 `Ctrl+Shift+P` (Windows/Linux) 또는 `Cmd+Shift+P` (macOS)를 누릅니다.
2. 명령어 팔레트에서 `Dev Containers: Reopen in Container`를 선택합니다.
3. 이후 컨테이너가 자동으로 실행되고 빌드됩니다. 처음 컨테이너를 열면 빌드하는 시간이 오래걸릴 수 있습니다. 빌드 후, 프로젝트가 **컨테이너 안에서 실행됨**.

---

## 7. C 파일에 브레이크포인트 설정 후 디버깅 (F5)

이제 본격적으로 문제를 풀 시간입니다. `webproxy-lab/README.md` 파일을 참조하셔서 webproxy 문제를 풀어보세요.
구현 순서는 tidy 웹서버(`webproxy-lab/tidy/tidy.c`)를 CSApp책에 있는 코드를 이용해서 구현하고, proxy서버(`webproxy-lab/proxy.c`)를 구현한 뒤에 최종 `webproxy-lab/mdriver`를 실행하여 70점 만점을 목표로 구현하세요.

C 언어로 문제를 풀다가 디버깅이 필요하시면 소스코드에 BreakPoint를 설정한 뒤에 키보드에서 `F5`를 눌러 디버깅을 시작할 수 있습니다. 디버깅은 tidy 서버와 proxy 서버용 2가지로 제공되며 각각 "Debug Tidy Server", "Debug Proxy Server" 이름을 가집니다. 두가지 중 원하는 디버깅 설정을 선택한 뒤에 `F5`를 누르면 해당 서버가 디버깅모드로 실행됩니다. 

* 기본적으로 "Debug Tidy Server"는 tidy 서버를 실행할때 포트를 `8000`을, "Debug Proxy Server"는 `4500`를  사용합니다. 해당 포트를 이미 다른 프로세스가 사용중이라면 새로운 포트로(`launch.json`파일에서 가능) 변경한 뒤에 디버깅을 진행합니다.


---

## 8. 새로운 Git 리포지토리에 Commit & Push 하기

금주 프로젝트를 개인 Git 리포와 같은 다른 리포지토리에 업로드하려면, 기존 Git 연결을 제거하고 새롭게 초기화해야 합니다.

### ✅ 완전히 새로운 Git 리포로 업로드하는 방법

아래 명령어를 순서대로 실행하세요:

```bash
rm -rf .git
git init
git remote add origin https://github.com/myusername/my-new-repo.git
git add .
git commit -m "Clean start"
git push -u origin main
```

### 📌 설명

- `rm -rf .git`: 기존 Git 기록과 연결을 완전히 삭제합니다.
- `git init`: 현재 폴더를 새로운 Git 리포지토리로 초기화합니다.
- `git remote add origin ...`: 새로운 리포지토리 주소를 origin으로 등록합니다.
- `git add .` 및 `git commit`: 모든 파일을 커밋합니다.
- `git push`: 새로운 리포에 최초 업로드(Push)합니다.

이 과정을 거치면 기존 리포와의 연결은 완전히 제거되고, **새로운 독립적인 프로젝트로 관리**할 수 있습니다.
