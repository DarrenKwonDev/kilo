#include <ctype.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
  struct termios orig_termios;
};
struct editorConfig E;


void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

void disableRawMode() {

  // TCSAFLUSH: 남아있는 입력/출력을 모두 비우고 적용
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);   // 프로그램이 종료되는 시점에 트리거 되는 함수 등록(golang의 defer 비슷한 느낌)

  struct termios raw = E.orig_termios;

  // 입력 플래그 설정
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // BRKINT: Break condition 시 신호 중지
  // ICRNL: Carriage return을 newline으로 변환하지 않음
  // INPCK: Parity check 비활성화
  // ISTRIP: 8번째 비트 strip 비활성화
  // IXON: 소프트웨어 흐름 제어 비활성화

  // 출력 플래그 설정
  raw.c_oflag &= ~(OPOST);
  // OPOST: 포스트 프로세싱 출력 비활성화

  // 제어 플래그 설정
  raw.c_cflag |= (CS8);
  // CS8: 8비트 바이트로 캐릭터 크기 설정

  // 로컬 플래그 설정
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // ECHO: 에코 비활성화
  // ICANON: 캐논 모드 비활성화, 입력을 즉시 처리
  // IEXTEN: 확장 입력 처리 비활성화
  // ISIG: 시그널 발생 비활성화

  raw.c_cc[VMIN] = 0;     // VMIN (Minimum number of characters to read). 0이면 읽을 게 없어도 바로 리턴
  raw.c_cc[VTIME] = 1;    // VTIME (Time to wait for data)

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
  }
}

void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editorRefreshScreen() {
  // \x1b: 이스케이프 문자(ASCII 27, 16진수로 1B)
  // [: 이스케이프 시퀀스의 시작을 나타냄
  // 2: 인자 값
  // J: 명령어(Erase In Display)
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // H: cursor position
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H", 3);

}



int main()
{
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
