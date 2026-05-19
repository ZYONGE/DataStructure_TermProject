# 자료구조 기말프로젝트 - TODO

## TODO
- [x] .exe 파일을 실행할때 출력 창 잘못나오고 있는것 수정:
  - setupTerminal()에서 SetConsoleScreenBufferSize / SetConsoleWindowInfo로 80×32 창 고정

- [x] 안쓰는 파일이나 디렉토리, 소스코드가 있으면 전부 삭제하고 코드 최적화
  - graph.h, tree.h, list.h, queue.h, family_diagram.csv, person.c 삭제
  - stack.h/deque.h에서 CSV import 함수 제거
  - dfs.c에서 불필요한 스택 헤더 제거

- [x] windows 버전과 MacOS 버전의 깃 브랜치 2개를 생성해서 따로 컴파일 및 실행 관리 다룰것
  - [x] windows 브랜치: .exe 파일로 제출을 염두해서 코드 작성
  - [x] MacOS 브랜치: 터미널 기반에서 실행을 염두해서 코드 작성
  - [x] Master 브랜치: 공통 소스코드 관리

- [x] csv 파일을 이용해서 미리 입력을 받아오는 기능 전면 삭제
- [x] .exe 파일은 사용자의 편의를 위해 디렉토리 가장 바깥에 배치(README.md와 같은 레벨에 위치)
